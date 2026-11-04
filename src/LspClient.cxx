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
    QObject::connect(connection, &LspConnection::notification, this,
                     &LspClient::handleNotification);
    QObject::connect(connection, &LspConnection::stopped, this, [this] {
      const auto was_ready = (state == State::Ready);
      state = State::Dead;
      if (was_ready) {
        showInformativeMessage(QStringLiteral("clangd exited"));
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
    if (connection->isRunning()) {
      // the ritual: the server is asked to shut down, told that the client is
      // leaving, and only then does the process lose the argument; a server
      // which lingers past the grace period is killed by the transport
      connection->request(QStringLiteral("shutdown"), QJsonObject(),
                          [](const QJsonValue &, const QString &) {});
      connection->notify(QStringLiteral("exit"));
      transport->terminate();
    }
  }  // end of LspClient::~LspClient

  LspClient &LspClient::get(EditorWidget &w) {
    if (auto *const c = w.findChild<LspClient *>()) {
      return *c;
    }
    return *(new LspClient(&w, &w));
  }  // end of LspClient::get

  bool LspClient::isReady() const {
    return state == State::Ready;
  }  // end of LspClient::isReady

  bool LspClient::ensureStarted(QString &error) {
    if (state != State::Idle) {
      if (state == State::Dead) {
        error = QStringLiteral("the language server exited");
        return false;
      }
      return true;
    }
    const auto program = LspCxxSupport::serverExecutable();
    if (program.isEmpty()) {
      error = QStringLiteral(
          "clangd was not found: install it or set the 'clangd/executable' "
          "option");
      return false;
    }
    state = State::Starting;
    if (!transport->start(program, LspCxxSupport::serverArguments())) {
      state = State::Idle;
      error = QStringLiteral("'%1' could not be started").arg(program);
      return false;
    }
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
        {"definition", QJsonObject{{"linkSupport", true}}}};
    const auto params = QJsonObject{
        {"capabilities",
         QJsonObject{{"textDocument", text_document},
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
  }  // end of LspClient::ensureStarted

  void LspClient::handleInitializeAnswer(const QJsonValue &,
                                         const QString &error_message) {
    if (!error_message.isEmpty()) {
      state = State::Dead;
      showInformativeMessage(QStringLiteral("clangd refused to initialize: %1")
                                 .arg(error_message));
      return;
    }
    state = State::Ready;
    connection->notify(QStringLiteral("initialized"), QJsonObject());
    flushDocuments();
    emit ready();
  }  // end of LspClient::handleInitializeAnswer

  bool LspClient::waitUntilReady(const int timeout_ms) {
    auto error = QString();
    if (!ensureStarted(error)) {
      showInformativeMessage(error);
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
                                         {"languageId", QStringLiteral("cpp")},
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
    const auto change = LspProtocol::diffAgainst(it->text, text);
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
                                   {"languageId", QStringLiteral("cpp")},
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
    syncDocument(file);
    if (!isReady() || (!documents.contains(file))) {
      reply(QVector<LspLocation>());
      return;
    }
    connection->request(
        QStringLiteral("textDocument/definition"),
        QJsonObject{
            {"textDocument", QJsonObject{{"uri", LspProtocol::uriOf(file)}}},
            {"position", LspProtocol::positionOf(doc, position)}},
        [reply](const QJsonValue &result, const QString &) {
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
  }  // end of LspClient::requestDefinition

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
