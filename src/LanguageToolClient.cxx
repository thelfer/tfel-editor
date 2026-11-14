// TFEL Editor
// LanguageToolClient: the Java process which serves the checks of the
// proofreader and the requests the buffers of an editor make to it.
#include "TFEL/GUI/LanguageToolClient.hxx"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QTcpServer>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LanguageToolSupport.hxx"
#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {
    // one server per editor widget, found again by that widget
    QVector<LanguageToolClient *> clients;

    // how long the typing has to stop before a text is handed to the server
    constexpr int TYPING_SETTLES_IN = 500;

    /* how long a started process is waited for, in milliseconds: the server
     * of the version this editor was developed against binds its port in a
     * second and a half and answers its first request two seconds later, so
     * a minute bound the case where java is there and the jar is none, and
     * says so long before a person gives up on the editor */
    constexpr int SERVER_BOOT_TIMEOUT = 60000;

    // what the settings name of a server: the program it runs, the installation
    // whose rules it reads, the port it serves on and the options the Java virtual
    // machine runs with. The port of a setting which says 0 is the free one the
    // kernel gave, which is none of what a person can change, so the setting is
    // named and not the number: a client which reads its documents again under an
    // unchanged setting keeps the server it has
    QString serverSettings() {
      return QStringLiteral("%1\n%2\n%3\n%4")
          .arg(LanguageToolSupport::serverJar(),
               LanguageToolSupport::javaRuntime(),
               QString::number(LanguageToolSupport::port()),
               LanguageToolSupport::javaOptions().join(QLatin1Char(' ')));
    }  // end of serverSettings()
  }  // end of anonymous namespace

  LanguageToolClient::LanguageToolClient(EditorWidget *const w,
                                         QObject *const p)
    : QObject(p), editor(w) {
    setObjectName(QStringLiteral("language-tool"));
    check_timer = new QTimer(this);
    check_timer->setSingleShot(true);
    check_timer->setInterval(TYPING_SETTLES_IN);
    connect(check_timer,
            &QTimer::timeout,
            this,
            &LanguageToolClient::checkPendingDocuments);
    ready_timer = new QTimer(this);
    ready_timer->setInterval(200);
    connect(ready_timer,
            &QTimer::timeout,
            this,
            [this]() {
              // the timer is stopped while a probe is on its way: a server
              // which is asked again before it answered the first time is
              // asked twice about a state it has not reached
              ready_timer->stop();
              auto request = QNetworkRequest {
                  QUrl {endpoint() + QStringLiteral("/v2/languages")}};
              auto *const reply = network.get(request);
              connect(reply,
                      &QNetworkReply::finished,
                      this,
                      [this, reply]() {
                        reply->deleteLater();
                        if (shutting_down) {
                          return;
                        }
                        if ((reply->error() == QNetworkReply::NoError) &&
                            (reply->attribute(QNetworkRequest::
                                                  HttpStatusCodeAttribute)
                                 .toInt() == 200)) {
                          state = State::Ready;
                          emit ready();
                          checkPendingDocuments();
                          return;
                        }
                        if (ending_session) {
                          return;
                        }
                        if (QDateTime::currentMSecsSinceEpoch() - started_at >
                            SERVER_BOOT_TIMEOUT) {
                          const auto port = server_port;
                          endServer();
                          showInformativeMessage(
                              QStringLiteral(
                                  "the LanguageTool server did not answer "
                                  "on port %1")
                                  .arg(port));
                          return;
                        }
                        ready_timer->start();
                      });
            });
  }

  LanguageToolClient::~LanguageToolClient() {
    shutting_down = true;
    endServer();
    if (const auto pos = clients.indexOf(this); pos >= 0) {
      clients.remove(pos);
    }
  }

  LanguageToolClient &LanguageToolClient::get(EditorWidget &w) {
    for (auto *const c : clients) {
      if (c->editor == &w) {
        return *c;
      }
    }
    auto *const c = new LanguageToolClient(&w, &w);
    clients.append(c);
    return *c;
  }

  LanguageToolClient *LanguageToolClient::heldBy(EditorWidget &w) {
    for (auto *const c : clients) {
      if (c->editor == &w) {
        return c;
      }
    }
    return nullptr;
  }  // end of LanguageToolClient::heldBy

  bool LanguageToolClient::isAvailable() {
    return LanguageToolSupport::isAvailable();
  }

  bool LanguageToolClient::isIdle() const {
    return state == State::Idle;
  }

  bool LanguageToolClient::isReady() const {
    return state == State::Ready;
  }

  bool LanguageToolClient::startSession(QString &error) {
    ++users;
    if (state != State::Idle) {
      return true;
    }
    if (not startServer(error)) {
      --users;
      return false;
    }
    return true;
  }

  bool LanguageToolClient::stopSession() {
    if (users == 0) {
      return false;
    }
    --users;
    if (users == 0) {
      endServer();
    }
    return true;
  }

  void LanguageToolClient::openDocument(const QString &file,
                                        QTextDocument *const doc,
                                        TextEditBase *const frame) {
    if (doc == nullptr) {
      return;
    }
    if (auto it = documents.find(file); it != documents.end()) {
      it.value().refs++;
      it.value().frame = frame;
      it.value().pending = true;
      check_timer->start();
      return;
    }
    auto entry = Document();
    entry.doc = doc;
    entry.frame = frame;
    entry.refs = 1;
    documents.insert(file, entry);
    connect(doc,
            &QTextDocument::contentsChanged,
            this,
            [this, file, doc]() {
              const auto it = documents.find(file);
              if (it == documents.end()) {
                return;
              }
              it.value().doc = doc;
              it.value().pending = true;
              check_timer->start();
            });
    auto error = QString();
    if (not startSession(error)) {
      if (not failure_reported) {
        // a program which is not there is a mistake of the configuration
        // and every buffer of the kind would repeat the same line
        failure_reported = true;
        showInformativeMessage(error);
      }
      return;
    }
    check_timer->start();
  }  // end of LanguageToolClient::openDocument

  void LanguageToolClient::closeDocument(const QString &file,
                                         QTextDocument *const doc) {
    auto it = documents.find(file);
    if (it == documents.end()) {
      return;
    }
    if ((doc != nullptr) and (it.value().doc != nullptr) and
        (it.value().doc != doc)) {
      // the file is visited by another buffer: this one is not its last user
      return;
    }
    if (--(it.value().refs) > 0) {
      return;
    }
    documents.erase(it);
    stopSession();
    emit diagnosticsUpdated(file);
  }  // end of LanguageToolClient::closeDocument

  bool LanguageToolClient::isDocumentOpen(const QString &file) const {
    return documents.contains(file);
  }

  void LanguageToolClient::recheckDocuments() {
    if ((state != State::Idle) && (launch_settings != serverSettings())) {
      // the installation, the Java runtime, the port or the options of the virtual
      // machine are not what this server was started with: what it answers are the
      // rules of a program the person left behind, so it is ended here and the one
      // which is named now is started by \c checkPendingDocuments, which is what
      // the documents below ask for. The settings of a request — the language, the
      // categories of rules — are none of this: every request names them and a
      // server which runs answers both under the settings which are held now
      endServer();
    }
    for (auto it = documents.begin(); it != documents.end(); ++it) {
      auto &entry = it.value();
      // the text an entry holds is what keeps a document whose selection changed
      // from being sent again: what is asked about now is that same text read
      // under another language, another set of rules, or another server. The copy
      // is spent with the generation, so the replies of the requests which went
      // out under the settings which are gone draw nothing
      entry.text = QString();
      entry.generation++;
      entry.outstanding = 0;
      entry.incoming.clear();
      entry.error.clear();
      entry.pending = true;
    }
    check_timer->start();
  }  // end of LanguageToolClient::recheckDocuments

  QVector<LspDiagnostic>
  LanguageToolClient::diagnosticsFor(const QString &file) const {
    const auto it = documents.constFind(file);
    if (it == documents.constEnd()) {
      return {};
    }
    return it.value().diagnostics;
  }

  int LanguageToolClient::port() const {
    return server_port;
  }

  bool LanguageToolClient::startServer(QString &error) {
    if ((server != nullptr) and (server->state() != QProcess::NotRunning)) {
      return true;
    }
    const auto jar = LanguageToolSupport::serverJar();
    if (jar.isEmpty()) {
      error = QStringLiteral(
                  "LanguageTool is not configured: set the installation "
                  "directory in Options > Configuration > LanguageTool");
      return false;
    }
    const auto java = LanguageToolSupport::javaRuntime();
    if (java.isEmpty()) {
      error = QStringLiteral(
                  "no java runtime found: set it in Options > Configuration "
                  "> LanguageTool");
      return false;
    }
    server_port = LanguageToolSupport::port();
    if (server_port <= 0) {
      server_port = freePort();
      if (server_port <= 0) {
        error =
            QStringLiteral("no free port to start the LanguageTool server on");
        return false;
      }
    }
    if (server != nullptr) {
      // the corpse of a server which was ended before
      server->deleteLater();
    }
    server = new QProcess(this);
    server->setProgram(java);
    server->setArguments(LanguageToolSupport::serverArguments(jar,
                                                              server_port));
    connect(server,
            &QProcess::stateChanged,
            this,
            [this](QProcess::ProcessState const s) {
              if ((s != QProcess::NotRunning) or ending_session or
                  shutting_down) {
                return;
              }
              // the process is gone: what the buffers hold are the underlines
              // of a server which answers nothing anymore
              state = State::Idle;
              server_port = 0;
              ready_timer->stop();
              emit stopped();
            });
    started_at = QDateTime::currentMSecsSinceEpoch();
    launch_settings = serverSettings();
    server->start();
    if (not server->waitForStarted(3000)) {
      error = QStringLiteral("failed to start %1 to run %2").arg(java, jar);
      endServer();
      server_port = 0;
      return false;
    }
    state = State::Starting;
    ready_timer->start();
    return true;
  }  // end of LanguageToolClient::startServer

  void LanguageToolClient::endServer() {
    ready_timer->stop();
    check_timer->stop();
    state = State::Idle;
    if ((server == nullptr) or (server->state() == QProcess::NotRunning)) {
      return;
    }
    ending_session = true;
    server->terminate();
    if (not server->waitForFinished(3000)) {
      server->kill();
      server->waitForFinished(1000);
    }
    ending_session = false;
  }  // end of LanguageToolClient::endServer

  int LanguageToolClient::freePort() {
    auto probe = QTcpServer();
    if (not probe.listen(QHostAddress::LocalHost, 0)) {
      return -1;
    }
    const auto result = static_cast<int>(probe.serverPort());
    probe.close();
    return result;
  }

  QString LanguageToolClient::endpoint() const {
    return QStringLiteral("http://127.0.0.1:%1").arg(server_port);
  }

  void LanguageToolClient::checkPendingDocuments() {
    if (documents.isEmpty()) {
      return;
    }
    if (state == State::Idle) {
      auto error = QString();
      // `startServer`, and not `startSession`: the buffers which use this client
      // are already counted by the `openDocument` which asked for it, and counting
      // them again here would leave a server running when the last of them is gone
      if (not startServer(error)) {
        if (not failure_reported) {
          failure_reported = true;
          showInformativeMessage(error);
        }
        return;
      }
      // the requests go out when the server answers, which the `ready`
      // signal says: the documents stay pending until then
      return;
    }
    if (state != State::Ready) {
      return;
    }
    for (auto it = documents.begin(); it != documents.end(); ++it) {
      if (it.value().pending and (it.value().outstanding == 0)) {
        checkDocument(it.key());
      }
    }
  }  // end of LanguageToolClient::checkPendingDocuments

  void LanguageToolClient::checkDocument(const QString &file) {
    auto it = documents.find(file);
    if (it == documents.end()) {
      return;
    }
    auto &entry = it.value();
    const auto text = textOf(entry);
    entry.pending = false;
    if (text == entry.text) {
      // what changed in the document is a selection, nothing which is read
      return;
    }
    entry.text = text;
    entry.generation++;
    entry.incoming.clear();
    entry.error.clear();
    const auto language = LanguageToolSupport::language();
    const auto enabled = LanguageToolSupport::enabledCategories();
    const auto disabled = LanguageToolSupport::disabledCategories();
    const auto limit = LanguageToolSupport::maximumTextLength();
    const auto starts = LanguageToolSupport::sliceStarts(text);
    entry.outstanding = static_cast<int>(starts.size());
    for (const auto s : starts) {
      auto request = QNetworkRequest {
          QUrl {endpoint() + QStringLiteral("/v2/check")}};
      request.setHeader(QNetworkRequest::ContentTypeHeader,
                        QStringLiteral("application/x-www-form-urlencoded"));
      const auto body = LanguageToolSupport::requestBody(
          text.mid(s, qMin(limit, text.size() - s)), language, enabled,
          disabled);
      auto *const reply = network.post(request, body);
      const auto generation = entry.generation;
      connect(reply,
              &QNetworkReply::finished,
              this,
              [this, reply, file, generation, s]() {
                handleReply(file, generation, s, reply);
              });
    }
  }  // end of LanguageToolClient::checkDocument

  QString LanguageToolClient::textOf(const Document &entry) const {
    if (entry.doc == nullptr) {
      return {};
    }
    auto text = LspProtocol::textOf(*entry.doc);
    if ((entry.frame != nullptr) and (entry.frame->hasMajorMode())) {
      // the mode says what of its text a proofreader reads: the source as
      // it stands for Markdown, the markup blanked out for LaTeX
      text = entry.frame->getMajorMode().grammarCheckText(text);
    }
    return text;
  }  // end of LanguageToolClient::textOf

  void LanguageToolClient::handleReply(const QString &file,
                                       const quint64 generation,
                                       const int offset,
                                       QNetworkReply *const reply) {
    reply->deleteLater();
    const auto it = documents.find(file);
    if (it == documents.end()) {
      return;
    }
    auto &entry = it.value();
    if (entry.outstanding > 0) {
      --entry.outstanding;
    }
    if (entry.generation != generation) {
      // a newer set of requests already asks about that document
      return;
    }
    const auto body = reply->readAll();
    const auto parsed = QJsonDocument::fromJson(body);
    if (parsed.isNull()) {
      // a request the server refuses is answered as plain text, and that
      // sentence is what the person who changed the setting has to read
      entry.error = LanguageToolSupport::errorOf(body);
    } else {
      // the ranges are computed against the whole text of the document, the
      // place of the piece which was sent added to the offsets of the answer
      entry.incoming += LanguageToolSupport::diagnosticsOf(
          parsed.object().value(QStringLiteral("matches")).toArray(), entry.text,
          offset);
    }
    if (entry.outstanding != 0) {
      return;
    }
    entry.diagnostics = entry.incoming;
    entry.incoming.clear();
    emit diagnosticsUpdated(file);
  }  // end of LanguageToolClient::handleReply

  void LanguageToolClient::showInformativeMessage(const QString &message) {
    if (editor != nullptr) {
      editor->displayInformativeMessage(message);
    }
  }

}  // end of namespace tfel::gui
