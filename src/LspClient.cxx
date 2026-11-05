#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QJsonArray>
#include <QtCore/QTimer>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspConnection.hxx"
#include "TFEL/GUI/LspCxxSupport.hxx"
#include "TFEL/GUI/LspTransport.hxx"

namespace tfel::gui {

  LspClient::LspClient(EditorWidget *const w, QObject *const p)
      : QObject(p),
        editor(w),
        transport(new LspTransport(this)),
        connection(new LspConnection(*transport, this)),
        state(State::Idle) {
    // the name of the server this client speaks with is its object name: an
    // editor widget can hold a client of `clangd` and clients of the grammar
    // checkers at once, and each kind is looked up by its own name
    this->setObjectName(server_name);
    QObject::connect(connection, &LspConnection::notification, this,
                     &LspClient::handleNotification);
    QObject::connect(connection, &LspConnection::stopped, this, [this] {
      const auto was_ready = (state == State::Ready);
      // a session the client ended of its own accord, to launch another one
      // or to die with the editor, did not exit: it was dismissed, and there
      // is nothing to report about it
      if (!ending_session) {
        state = State::Dead;
        if (was_ready) {
          showInformativeMessage(QStringLiteral("%1 exited").arg(server_name));
        }
      }
      emit stopped();
    });
  }  // end of LspClient::LspClient

  LspClient::~LspClient() {
    // the transport's `terminate` runs the event loop until the process is
    // gone, and the process reports its exit through `stopped`: whoever is
    // told must not talk to the widget this client dies with, which is being
    // destroyed around us
    this->shutting_down = true;
    this->endSession();
  }  // end of ~LspClient

  void LspClient::endSession() {
    if (!connection->isRunning()) {
      return;
    }
    // the ritual: the server is asked to shut down, told that the client is
    // leaving, and only then does the process lose the argument; a server
    // which lingers past the grace period is killed by the transport
    this->ending_session = true;
    connection->request(QStringLiteral("shutdown"), QJsonObject(),
                        [](const QJsonValue &, const QString &) {});
    connection->notify(QStringLiteral("exit"));
    transport->terminate();
    this->ending_session = false;
  }  // end of endSession

  LspClient &LspClient::get(EditorWidget &w) {
    // by name, not just by type: the clients of the grammar checkers are
    // children of the same widget tree, and one of them is no answer to a call
    // which asks for `clangd`
    if (auto *const c = w.findChild<LspClient *>(QLatin1String("clangd"))) {
      return *c;
    }
    return *(new LspClient(&w, &w));
  }  // end of LspClient::get

  void LspClient::setServerName(const QString &name) {
    this->server_name = name;
    this->setObjectName(name);
  }  // end of setServerName

  void LspClient::setLanguageId(const QString &id) {
    this->language_id = id;
  }  // end of setLanguageId

  void LspClient::setConfiguration(const QJsonValue &settings) {
    this->configuration = settings;
    this->connection->setConfigurationAnswer(settings);
  }  // end of setConfiguration

  void LspClient::setFullTextSync(const bool on) {
    this->full_text_sync = on;
  }  // end of setFullTextSync

  void LspClient::recheckDocuments() {
    if (!this->isReady()) {
      // nothing to tell yet: the settings are read at the handshake of a
      // session which asks for them, so a launch after `setConfiguration`
      // needs no recheck at all
      return;
    }
    // the settings live on the server side of a session too: a server which
    // reads them at the handshake alone, and not on every analysis, is to be
    // told that they changed
    this->connection->notify(QStringLiteral("workspace/didChangeConfiguration"),
                             QJsonObject{{"settings", this->configuration}});
    // and the documents have to be read again under the new settings: nothing
    // of theirs changed by itself, so no change of them would ever come
    for (auto it = documents.begin(); it != documents.end(); ++it) {
      if ((it->version == 0) || (it->doc == nullptr)) {
        continue;
      }
      ++(it->version);
      this->connection->notify(
          QStringLiteral("textDocument/didChange"),
          QJsonObject{{"textDocument",
                       QJsonObject{{"uri", LspProtocol::uriOf(it.key())},
                                   {"version", it->version}}},
                      {"contentChanges",
                       QJsonArray{QJsonObject{
                           {"text", LspProtocol::textOf(*it->doc)}}}}});
      it->text = LspProtocol::textOf(*it->doc);
    }
  }  // end of recheckDocuments

  bool LspClient::isReady() const {
    return state == State::Ready;
  }  // end of LspClient::isReady

  bool LspClient::isIdle() const {
    return state == State::Idle;
  }  // end of isIdle

  bool LspClient::launch(const QStringList &extra_arguments, QString &error) {
    const auto program = LspCxxSupport::serverExecutable();
    if (program.isEmpty()) {
      error = QStringLiteral(
          "clangd was not found: install it or set the 'clangd/executable' "
          "option");
      return false;
    }
    auto arguments = LspCxxSupport::serverArguments();
    arguments += extra_arguments;
    return this->launch(program, arguments, error);
  }  // end of launch

  bool LspClient::launch(const QString &program,
                         const QStringList &arguments,
                         QString &error) {
    // the options of a session, the compilation database above all, are fixed
    // when the server starts, so a session which is running is dismissed and
    // replaced rather than asked to change its mind
    this->endSession();
    // whatever the old server published describes buffers it read under other
    // options, and the new one has seen none of them at all: both are to be
    // redone from scratch, and the underlines of the old session must not
    // outlive it
    for (auto &document : documents) {
      document.version = 0;
    }
    this->clearDiagnostics();
    state = State::Idle;
    if (!transport->start(program, arguments)) {
      error = QStringLiteral("'%1' could not be started").arg(program);
      return false;
    }
    state = State::Starting;
    // the capabilities this editor actually implements, see the issue
    auto completion_item = QJsonObject{
        {"snippetSupport", false},
        {"documentationFormat",
         QJsonArray{QStringLiteral("markdown"), QStringLiteral("plaintext")}},
        {"insertReplaceSupport", false},
        {"resolveSupport",
         QJsonObject{
             {"properties", QJsonArray{QStringLiteral("detail"),
                                       QStringLiteral("documentation")}}}}};
    auto text_document = QJsonObject{
        {"synchronization",
         QJsonObject{{"dynamicRegistration", false}, {"didSave", true}}},
        {"hover", QJsonObject{{"contentFormat",
                               QJsonArray{QStringLiteral("markdown"),
                                          QStringLiteral("plaintext")}}}},
        {"completion", QJsonObject{{"completionItem", completion_item}}},
        {"publishDiagnostics", QJsonObject{{"relatedInformation", false}}},
        {"definition", QJsonObject{{"linkSupport", true}}},
        // the mode asks declarations as well: the prototype of a function
        // whose body lives in another file is a declaration, and a symbol
        // which is only declared is worth an underline and a jump icon too
        {"declaration", QJsonObject{}},
        {"references", QJsonObject{}},
        // the mode can rename: the answer is a workspace edit the editor
        // applies to its own buffers, no server-side file writing involved
        {"rename", QJsonObject{{"prepareSupport", false}}},
        // the overview of a file is asked as a tree: a server which is not
        // told the client can read one answers a flat list, where a symbol
        // which belongs to a class loses the signature the tree holds in its
        // `detail` and keeps only the name of its container
        {"documentSymbol",
         QJsonObject{
             {"hierarchicalDocumentSymbolSupport", true},
             {"symbolKind",
              QJsonObject{
                  {"valueSet", QJsonArray{1,  2,  3,  4,  5,  6,  7,  8,  9,
                                          10, 11, 12, 13, 14, 15, 16, 17, 18,
                                          19, 20, 21, 22, 23, 24, 25, 26}}}}}},
        // the context menu offers the quick fixes of the diagnostics the
        // server published, which wants the actions as data the editor can
        // apply, not as commands the server would have to be told to run
        {"codeAction",
         QJsonObject{
             {"codeActionLiteralSupport",
              QJsonObject{
                  {"codeActionKind",
                   QJsonObject{
                       {"valueSet", QJsonArray{QStringLiteral("quickfix"),
                                               QStringLiteral("refactor")}}}}}},
             {"isPreferredSupport", true}}},
        // the mode shows the parameter list of the call the cursor is inside
        // of, which wants the offsets of each parameter in the label of its
        // signature: without them a parameter cannot be picked out of the
        // label, and the list says no more than the text already on screen
        {"signatureHelp",
         QJsonObject{
             {"signatureInformation",
              QJsonObject{{"documentationFormat",
                           QJsonArray{QStringLiteral("markdown"),
                                      QStringLiteral("plaintext")}},
                          {"parameterInformation",
                           QJsonObject{{"labelOffsetSupport", true}}}}}}},
        // the mode colors the identifiers the server knows what they are:
        // the question is asked for the whole file, since `clangd` answers no
        // range at all — its legend says `range: false` —, and the legend the
        // numbers of an answer are read against comes back with the
        // capabilities of this very handshake, which is where
        // `requestSemanticTokens` takes them from
        {"semanticTokens",
         QJsonObject{
             {"requests", QJsonObject{{"range", false}, {"full", true}}},
             // the kinds and modifiers of the protocol, spelled as the
             // protocol spells them: what a server answers is its own legend,
             // which may name fewer of these and names none by these numbers
             {"tokenTypes",
              QJsonArray{
                  QStringLiteral("namespace"),     QStringLiteral("type"),
                  QStringLiteral("class"),         QStringLiteral("enum"),
                  QStringLiteral("interface"),     QStringLiteral("struct"),
                  QStringLiteral("typeParameter"), QStringLiteral("parameter"),
                  QStringLiteral("variable"),      QStringLiteral("property"),
                  QStringLiteral("enumMember"),    QStringLiteral("event"),
                  QStringLiteral("function"),      QStringLiteral("method"),
                  QStringLiteral("macro"),         QStringLiteral("keyword"),
                  QStringLiteral("modifier"),      QStringLiteral("comment"),
                  QStringLiteral("string"),        QStringLiteral("number"),
                  QStringLiteral("regexp"),        QStringLiteral("operator"),
                  QStringLiteral("decorator")}},
             {"tokenModifiers",
              QJsonArray{
                  QStringLiteral("declaration"), QStringLiteral("definition"),
                  QStringLiteral("readonly"), QStringLiteral("static"),
                  QStringLiteral("deprecated"), QStringLiteral("abstract"),
                  QStringLiteral("virtual"), QStringLiteral("async"),
                  QStringLiteral("modification"),
                  QStringLiteral("documentation"),
                  QStringLiteral("defaultLibrary")}},
             {"formats", QJsonArray{QStringLiteral("relative")}}}}};
    const auto params = QJsonObject{
        {"capabilities",
         QJsonObject{{"textDocument", text_document},
                     // a server which keeps its settings on the client side —
                     // `harper-ls`, whose dialect is such a setting, asks them
                     // of the client at the handshake and sometimes on every
                     // analysis; the answer is what `setConfiguration` set,
                     // and null for a server which is asked none
                     {"workspace", QJsonObject{{"configuration", true}}},
                     {"window", QJsonObject{{"workDoneProgress", false}}}}},
        {"clientInfo", QJsonObject{{"name", QStringLiteral("tfel-editor")},
                                   {"version", QStringLiteral("1.1.0-dev")}}},
        {"processId", QCoreApplication::applicationPid()},
        // the server is left to take the working directory of the editor as its
        // root, and to look for a compilation database next to the files it is
        // given, which is what a person editing a checkout expects
        {"rootUri", QJsonValue()}};
    connection->request(
        QStringLiteral("initialize"), params,
        [this](const QJsonValue &result, const QString &error_message) {
          handleInitializeAnswer(result, error_message);
        });
    return true;
  }  // end of LspClient::launch

  bool LspClient::stop() {
    // a server which exited of its own accord has nothing left to dismiss, but
    // the client still holds the text and the underlines of a session which is
    // gone: stopping cleans that up either way, and the answer says whether a
    // server was actually running
    const auto running = connection->isRunning();
    if (running) {
      this->endSession();
    }
    // the same dismissal of the session's legacy as when `launch` replaces a
    // session: the documents are marked as never sent, so the next session
    // gets their whole text anew, and the underlines go away with the session
    for (auto &document : documents) {
      document.version = 0;
    }
    this->clearDiagnostics();
    // the legend the ended session announced names numbers a next one may
    // answer differently, and an answer of a server which is gone is not
    // coming: what the next session announces, if anything, replaces it
    this->semantic_legend = LspSemanticLegend();
    state = State::Idle;
    return running;
  }  // end of LspClient::stop

  void LspClient::handleInitializeAnswer(const QJsonValue &result,
                                         const QString &error_message) {
    if (!error_message.isEmpty()) {
      state = State::Dead;
      showInformativeMessage(QStringLiteral("%1 refused to initialize: %2")
                                 .arg(server_name)
                                 .arg(error_message));
      return;
    }
    const auto capabilities =
        result.toObject().value("capabilities").toObject();
    // how the server wants its documents: a server which reads the text it is
    // given as the text of the document (`change: 1`) can be sent nothing
    // else, since the range of a change means nothing to it. A server which
    // says nothing, as `harper-ls` does, is left to whoever set the client up
    const auto sync_value = capabilities.value("textDocumentSync");
    const auto sync = sync_value.toObject();
    const auto change = (sync_value.isDouble() ? sync_value.toInt()
                                               : sync.value("change").toInt(1));
    if (change == 1) {
      this->full_text_sync = true;
    }
    // what the server says a call is typed around: the mode asks its
    // signature question on those characters, and only on those, since a
    // server which lists `{` means it and would otherwise be asked on every
    // brace of every body
    auto triggers = QVector<QChar>();
    const auto provider =
        capabilities.value("signatureHelpProvider").toObject();
    for (const auto &v : provider.value("triggerCharacters").toArray()) {
      const auto character = v.toString();
      if (!character.isEmpty()) {
        triggers.append(character.at(0));
      }
    }
    if (!triggers.isEmpty()) {
      this->signature_triggers = triggers;
    }
    // the names the numbers of a semantic tokens answer are read against: the
    // answer itself carries numbers only, so a session which announced no
    // legend can not be asked the question at all
    this->semantic_legend = semanticLegendOf(QJsonValue(capabilities));
    state = State::Ready;
    connection->notify(QStringLiteral("initialized"), QJsonObject());
    flushDocuments();
    emit ready();
  }  // end of LspClient::handleInitializeAnswer

  bool LspClient::waitUntilReady(const int timeout_ms) {
    if ((state == State::Idle) || (state == State::Dead)) {
      // no handshake was ever asked for, or the server which answered is
      // gone: waiting would only stall whoever waits for nothing
      return false;
    }
    if (isReady()) {
      return true;
    }
    auto done = false;
    auto loop = QEventLoop();
    auto quit = [&done, &loop] {
      done = true;
      loop.quit();
    };
    const auto on_ready =
        QObject::connect(this, &LspClient::ready, &loop, [&quit] { quit(); });
    const auto on_stopped =
        QObject::connect(this, &LspClient::stopped, &loop, [&quit] { quit(); });
    QTimer::singleShot(timeout_ms, &loop, &QEventLoop::quit);
    loop.exec();
    QObject::disconnect(on_ready);
    QObject::disconnect(on_stopped);
    return done && isReady();
  }  // end of LspClient::waitUntilReady

  void LspClient::openDocument(const QString &file, QTextDocument *const doc) {
    if (file.isEmpty() || doc == nullptr) {
      return;
    }
    auto &st = documents[file];
    if (st.refs == 0) {
      st.doc = doc;
      st.text = LspProtocol::textOf(*doc);
      st.version = 0;
      // every change of the document goes to the server as it happens; the
      // document is the sender, so the connection dies with it
      QObject::connect(doc, &QTextDocument::contentsChange, this,
                       [this, file](const int, const int, const int) {
                         syncDocument(file);
                       });
      // a document which dies without a `closeDocument`, as the buffer's does
      // when the editor goes away, releases its own reference: `QPointer`s
      // are nulled before `destroyed` is emitted, so the tracked document is
      // already gone from the state and this release is the one which closes
      // the file on the server
      QObject::connect(doc, &QObject::destroyed, this,
                       [this, file] { closeDocument(file, nullptr); });
      if (isReady()) {
        auto opened = QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)},
                                         {"languageId", language_id},
                                         {"version", 1},
                                         {"text", st.text}}}};
        connection->notify(QStringLiteral("textDocument/didOpen"), opened);
        st.version = 1;
      }
    }
    ++st.refs;
  }  // end of LspClient::openDocument

  void LspClient::closeDocument(const QString &file, QTextDocument *const doc) {
    const auto it = documents.find(file);
    if (it == documents.end()) {
      return;
    }
    if (doc != nullptr) {
      // the caller hands its document back: the connections made for it at
      // `openDocument` fall away, whichever of them this document is — the
      // tracked one whose text the server sees, or another user of the same
      // file whose text it does not
      QObject::disconnect(doc, nullptr, this, nullptr);
    }
    if (--(it->refs) > 0) {
      return;
    }
    if (it->version > 0) {
      connection->notify(
          QStringLiteral("textDocument/didClose"),
          QJsonObject{{"textDocument",
                       QJsonObject{{"uri", LspProtocol::uriOf(file)}}}});
    }
    documents.erase(it);
  }  // end of LspClient::closeDocument

  bool LspClient::isDocumentOpen(const QString &file) const {
    return documents.contains(file);
  }  // end of LspClient::isDocumentOpen

  QString LspClient::snapshotOf(const QString &file) const {
    const auto it = documents.find(file);
    return (it == documents.end()) ? QString() : it->text;
  }  // end of LspClient::snapshotOf

  void LspClient::syncDocument(const QString &file) {
    const auto it = documents.find(file);
    if ((it == documents.end()) || (it->doc == nullptr)) {
      return;
    }
    const auto text = LspProtocol::textOf(*it->doc);
    if (text == it->text) {
      return;
    }
    if ((it->version == 0) || (!isReady())) {
      // the server has not seen the document, or will not see the change: the
      // snapshot is what the next `didOpen` will send, no more, no less
      it->text = text;
      return;
    }
    const auto change = full_text_sync
                            ? std::optional<LspProtocol::Change>()
                            : LspProtocol::diffAgainst(it->text, text);
    ++(it->version);
    if (change) {
      connection->notify(
          QStringLiteral("textDocument/didChange"),
          QJsonObject{
              {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)},
                                           {"version", it->version}}},
              {"contentChanges",
               QJsonArray{QJsonObject{{"range", change->range},
                                      {"text", change->text}}}}});
    } else {
      connection->notify(
          QStringLiteral("textDocument/didChange"),
          QJsonObject{
              {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)},
                                           {"version", it->version}}},
              {"contentChanges", QJsonArray{QJsonObject{{"text", text}}}}});
    }
    it->text = text;
  }  // end of LspClient::syncDocument

  void LspClient::flushDocuments() {
    for (auto it = documents.begin(); it != documents.end(); ++it) {
      if ((it->version != 0) || (it->doc == nullptr)) {
        continue;
      }
      it->text = LspProtocol::textOf(*it->doc);
      connection->notify(
          QStringLiteral("textDocument/didOpen"),
          QJsonObject{{"textDocument",
                       QJsonObject{{"uri", LspProtocol::uriOf(it.key())},
                                   {"languageId", language_id},
                                   {"version", 1},
                                   {"text", it->text}}}});
      it->version = 1;
    }
  }  // end of LspClient::flushDocuments

  void LspClient::requestCompletion(const QString &file,
                                    const QTextDocument &doc,
                                    const int position,
                                    const CompletionReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QStringList(), QStringList());
      return;
    }
    connection->request(
        QStringLiteral("textDocument/completion"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
            {"position", LspProtocol::positionOf(doc, position)}},
        [reply](const QJsonValue &result, const QString &) {
          auto labels = QStringList();
          auto details = QStringList();
          auto items = QJsonArray();
          if (result.isArray()) {
            items = result.toArray();
          } else if (result.isObject()) {
            items = result.toObject().value("items").toArray();
          }
          for (const auto &v : items) {
            const auto item = v.toObject();
            const auto label = item.value("label").toString();
            if (label.isEmpty()) {
              continue;
            }
            // what to insert is not what is shown: a function completion is
            // labeled with its signature and inserts its name
            auto insert = label;
            if (item.value("textEdit").isObject()) {
              insert =
                  item.value("textEdit").toObject().value("newText").toString();
            } else if (item.contains("insertText")) {
              insert = item.value("insertText").toString();
            }
            labels.append(insert.isEmpty() ? label : insert);
            details.append(item.value("detail").toString(label));
          }
          reply(labels, details);
        });
  }  // end of LspClient::requestCompletion

  void LspClient::requestDefinition(const QString &file,
                                    const QTextDocument &doc,
                                    const int position,
                                    const DefinitionReply &reply) {
    requestLocations(QStringLiteral("textDocument/definition"), file, doc,
                     position, reply);
  }  // end of LspClient::requestDefinition

  void LspClient::requestDeclaration(const QString &file,
                                     const QTextDocument &doc,
                                     const int position,
                                     const DefinitionReply &reply) {
    requestLocations(QStringLiteral("textDocument/declaration"), file, doc,
                     position, reply);
  }  // end of LspClient::requestDeclaration

  void LspClient::requestReferences(const QString &file,
                                    const QTextDocument &doc,
                                    const int position,
                                    const DefinitionReply &reply) {
    // the protocol wants a context telling whether the declaration of the
    // symbol counts as a reference; the mode shows the whole life of the
    // symbol, so it counts
    requestLocations(
        QStringLiteral("textDocument/references"), file, doc, position, reply,
        QJsonObject{{"context", QJsonObject{{"includeDeclaration", true}}}});
  }  // end of LspClient::requestReferences

  void LspClient::requestRename(const QString &file,
                                const QTextDocument &doc,
                                const int position,
                                const QString &newName,
                                const RenameReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QMap<QString, QVector<LspTextEdit>>(),
            QStringLiteral("%1 is not answering").arg(server_name));
      return;
    }
    connection->request(
        QStringLiteral("textDocument/rename"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
            {"position", LspProtocol::positionOf(doc, position)},
            {"newName", newName}},
        [reply](const QJsonValue &result, const QString &error) {
          // the answer is a `WorkspaceEdit`, in one of its two shapes, or null
          // when the server refused the name, which `error` then tells
          reply(LspProtocol::editsOf(result), error);
        });
  }  // end of LspClient::requestRename

  void LspClient::requestCodeActions(const QString &file,
                                     const QTextDocument &doc,
                                     const int position,
                                     const int end,
                                     const CodeActionsReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QVector<LspCodeAction>());
      return;
    }
    // the range asked about is a zero-width one at the position, which is
    // where the cursor is: `clangd` returns the fixes of the diagnostics it is
    // handed whatever the range they live in, so it is the diagnostics, not
    // the range, which decide what comes back
    auto context =
        QJsonObject{{"triggerKind", 1}, {"diagnostics", QJsonArray()}};
    auto published = QJsonArray();
    const auto it = diagnostics.constFind(file);
    if (it != diagnostics.constEnd()) {
      for (const auto &d : *it) {
        published.append(LspProtocol::jsonOf(d));
      }
    }
    context.insert(QStringLiteral("diagnostics"), published);
    const auto start = LspProtocol::positionOf(doc, position);
    const auto stop = LspProtocol::positionOf(doc, end);
    connection->request(
        QStringLiteral("textDocument/codeAction"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
            {"range", LspProtocol::rangeOf(start.value("line").toInt(),
                                           start.value("character").toInt(),
                                           stop.value("line").toInt(),
                                           stop.value("character").toInt())},
            {"context", context}},
        [reply](const QJsonValue &result, const QString &) {
          auto actions = QVector<LspCodeAction>();
          for (const auto &v : result.toArray()) {
            const auto o = v.toObject();
            auto action = LspCodeAction();
            action.title = o.value("title").toString();
            action.kind = o.value("kind").toString();
            action.is_preferred = o.value("isPreferred").toBool();
            action.edits = LspProtocol::editsOf(o.value("edit"));
            // the identity of a server-side action is the command it comes
            // with; its title is what the user would read, and both are kept
            // under one name since the mode only uses it to say "not mine"
            const auto command = o.value("command").toObject();
            action.command = command.value("title").toString();
            if (action.command.isEmpty()) {
              action.command = command.value("command").toString();
            }
            const auto fixed = o.value("diagnostics").toArray();
            if (!fixed.isEmpty()) {
              action.diagnostic =
                  fixed.at(0).toObject().value("message").toString();
            }
            if (!action.title.isEmpty()) {
              actions.append(action);
            }
          }
          reply(actions);
        });
  }  // end of LspClient::requestCodeActions

  void LspClient::requestDocumentSymbols(const QString &file,
                                         const DocumentSymbolsReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QVector<LspDocumentSymbol>());
      return;
    }
    // the question names no position: the answer is the whole file, and the
    // shape of it is decided by what the client advertised at the handshake
    connection->request(
        QStringLiteral("textDocument/documentSymbol"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}}},
        [reply](const QJsonValue &result, const QString &) {
          reply(documentSymbolsOf(result));
        });
  }  // end of LspClient::requestDocumentSymbols

  void LspClient::requestSemanticTokens(const QString &file,
                                        const SemanticTokensReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file)) ||
        (!this->semantic_legend.isValid())) {
      // the question has no shape without the legend the server announced with
      // its capabilities: the answer would be numbers naming nothing
      reply(QVector<LspSemanticToken>());
      return;
    }
    const auto legend = this->semantic_legend;
    connection->request(
        QStringLiteral("textDocument/semanticTokens/full"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}}},
        [reply, legend](const QJsonValue &result, const QString &) {
          reply(semanticTokensOf(result, legend));
        });
  }  // end of LspClient::requestSemanticTokens

  bool LspClient::hasSemanticTokens() const {
    return this->semantic_legend.isValid();
  }  // end of LspClient::hasSemanticTokens

  QStringList LspClient::semanticTokenKinds() const {
    return this->semantic_legend.types;
  }  // end of LspClient::semanticTokenKinds

  QVector<QChar> LspClient::signatureHelpTriggers() const {
    return this->signature_triggers;
  }  // end of LspClient::signatureHelpTriggers

  void LspClient::requestSignatureHelp(const QString &file,
                                       const QTextDocument &doc,
                                       const int position,
                                       const SignatureHelpReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QVector<LspSignature>(), 0, 0);
      return;
    }
    connection->request(
        QStringLiteral("textDocument/signatureHelp"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
            {"position", LspProtocol::positionOf(doc, position)},
            // the protocol wants a context; this client is asked by a
            // key or by a typed character either way, and clangd
            // computes the same answer from either kind
            {"context",
             QJsonObject{{"triggerKind", 1}, {"isRetrigger", false}}}},
        [reply](const QJsonValue &result, const QString &) {
          auto signatures = QVector<LspSignature>();
          const auto help = result.toObject();
          for (const auto &v : help.value("signatures").toArray()) {
            const auto s = v.toObject();
            auto signature = LspSignature();
            signature.label = s.value("label").toString();
            for (const auto &p : s.value("parameters").toArray()) {
              const auto parameter = p.toObject();
              auto info = LspParameterInfo();
              const auto label = parameter.value("label");
              if (label.isArray()) {
                // the offsets into the label of the signature, which is what
                // this client advertised and what makes a parameter findable
                // in a label it shares with the others
                const auto offsets = label.toArray();
                info.start = offsets.at(0).toInt();
                info.end = offsets.at(1).toInt();
                const auto text =
                    signature.label.mid(info.start, info.end - info.start);
                info.text = text;
              } else {
                // the label of the parameter on its own: its place in the
                // signature is whatever the server left to find there
                info.text = label.toString();
              }
              signature.parameters.append(info);
            }
            if (!signature.label.isEmpty()) {
              signatures.append(signature);
            }
          }
          reply(signatures, help.value("activeSignature").toInt(),
                help.value("activeParameter").toInt());
        });
  }  // end of LspClient::requestSignatureHelp

  void LspClient::requestLocations(const QString &method,
                                   const QString &file,
                                   const QTextDocument &doc,
                                   const int position,
                                   const DefinitionReply &reply,
                                   const QJsonObject &extra) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QVector<LspLocation>());
      return;
    }
    auto params = QJsonObject{
        {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
        {"position", LspProtocol::positionOf(doc, position)}};
    for (auto it = extra.begin(); it != extra.end(); ++it) {
      params.insert(it.key(), it.value());
    }
    connection->request(
        method, params, [reply](const QJsonValue &result, const QString &) {
          auto locations = QVector<LspLocation>();
          auto one = [&locations](const QJsonObject &o) {
            auto location = LspLocation();
            auto uri = o.value("uri").toString();
            auto range = o.value("range").toObject();
            if (uri.isEmpty()) {
              // a `LocationLink` names its target differently
              uri = o.value("targetUri").toString();
              range = o.value("targetSelectionRange").toObject();
              if (range.isEmpty()) {
                range = o.value("targetRange").toObject();
              }
            }
            if (uri.isEmpty()) {
              return;
            }
            location.file = LspProtocol::fileOfUri(uri);
            const auto start = range.value("start").toObject();
            location.line = start.value("line").toInt();
            location.character = start.value("character").toInt();
            locations.append(location);
          };
          if (result.isArray()) {
            for (const auto &v : result.toArray()) {
              one(v.toObject());
            }
          } else if (result.isObject()) {
            one(result.toObject());
          }
          reply(locations);
        });
  }  // end of LspClient::requestLocations

  void LspClient::requestHover(const QString &file,
                               const QTextDocument &doc,
                               const int position,
                               const HoverReply &reply) {
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QString(), false);
      return;
    }
    connection->request(
        QStringLiteral("textDocument/hover"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
            {"position", LspProtocol::positionOf(doc, position)}},
        [reply](const QJsonValue &result, const QString &) {
          auto parts = QStringList();
          auto markdown = false;
          const auto contents = result.toObject().value("contents");
          auto one = [&parts, &markdown](const QJsonValue &v) {
            if (v.isString()) {
              parts.append(v.toString());
              return;
            }
            const auto o = v.toObject();
            if (o.contains("value")) {
              // a MarkupContent or a MarkedString; the latter names the
              // language its value is written in, which only markdown
              // renders well
              if (o.value("kind").toString() == QLatin1String("markdown") ||
                  o.contains("language")) {
                markdown = true;
              }
              parts.append(o.value("value").toString());
            }
          };
          if (contents.isArray()) {
            for (const auto &v : contents.toArray()) {
              one(v);
            }
          } else {
            one(contents);
          }
          reply(parts.join(QStringLiteral("\n\n")), markdown);
        });
  }  // end of LspClient::requestHover

  QVector<LspDiagnostic> LspClient::diagnosticsFor(const QString &file) const {
    const auto it = diagnostics.find(file);
    return (it == diagnostics.end()) ? QVector<LspDiagnostic>() : *it;
  }  // end of LspClient::diagnosticsFor

  void LspClient::clearDiagnostics() {
    if (diagnostics.isEmpty()) {
      return;
    }
    const auto files = diagnostics.keys();
    diagnostics.clear();
    // each file which had an underline has to learn it has none, see
    // `CxxMajorMode::updateDiagnostics`
    for (const auto &file : files) {
      emit diagnosticsUpdated(file);
    }
  }  // end of clearDiagnostics

  void LspClient::handleNotification(const QString &method,
                                     const QJsonValue &params) {
    if (method == QLatin1String("textDocument/publishDiagnostics")) {
      publishDiagnostics(params);
    } else if (method == QLatin1String("window/showMessage")) {
      showInformativeMessage(params.toObject().value("message").toString());
    }
    // `window/logMessage` and `$/progress` are noise this editor does not
    // display; the log is on the transport for whoever wants it
  }  // end of LspClient::handleNotification

  void LspClient::publishDiagnostics(const QJsonValue &params) {
    const auto p = params.toObject();
    const auto file = LspProtocol::fileOfUri(p.value("uri").toString());
    if (file.isEmpty()) {
      return;
    }
    auto published = QVector<LspDiagnostic>();
    for (const auto &v : p.value("diagnostics").toArray()) {
      const auto d = v.toObject();
      auto diagnostic = LspDiagnostic();
      diagnostic.severity = d.value("severity").toInt(1);
      diagnostic.message = d.value("message").toString();
      diagnostic.source = d.value("source").toString();
      // the rule a checker names its finding by, which is what a person looks
      // up when they disagree with it; the protocol allows a number, and a
      // number is spelled as the string of it
      const auto code = d.value("code");
      diagnostic.code =
          code.isDouble() ? QString::number(code.toInt()) : code.toString();
      const auto range = d.value("range").toObject();
      const auto start = range.value("start").toObject();
      const auto end = range.value("end").toObject();
      diagnostic.startLine = start.value("line").toInt();
      diagnostic.startCharacter = start.value("character").toInt();
      diagnostic.endLine = end.value("line").toInt();
      diagnostic.endCharacter = end.value("character").toInt();
      published.append(diagnostic);
    }
    diagnostics.insert(file, published);
    emit diagnosticsUpdated(file);
  }  // end of LspClient::publishDiagnostics

  void LspClient::showInformativeMessage(const QString &m) {
    if ((editor != nullptr) && (!shutting_down)) {
      editor->displayInformativeMessage(m);
    }
  }  // end of LspClient::showInformativeMessage

}  // end of namespace tfel::gui
