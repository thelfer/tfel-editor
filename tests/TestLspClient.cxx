/*!
 * \file   tests/TestLspClient.cxx
 * \brief  tests of the Language Server Protocol client against a real
 *         `clangd`, skipped when the machine has none.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QToolTip>
#include <functional>
#include <memory>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/CxxMajorMode.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspCxxSupport.hxx"
#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// the client talks to a process, not to a window: the test needs no display,
// but it does need `clangd`. The documents are held by `QPlainTextEdit`s, as
// in the editor, and that is not a decoration: a bare `QTextDocument` does not
// emit `contentsChange` when a cursor edits it, so the incremental
// synchronization could only be observed on a document attached to a widget
struct TestLspClient : QObject {
  Q_OBJECT

 public:
  //! spin the event loop until the predicate holds or the time is up
  static bool waitFor(const std::function<bool()> &predicate, const int ms) {
    auto timer = QElapsedTimer();
    timer.start();
    while (!predicate()) {
      if (timer.elapsed() > ms) {
        return false;
      }
      QTest::qWait(50);
    }
    return true;
  }

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void testHandshake();
  void testDocumentOpenAndIncrementalSync();
  void testDiagnostics();
  void testRelaunchReplacesTheSession();
  void testStopEndsTheSession();
  void testDefinition();
  void testHover();
  void testCompletion();
  void testModeMenu();
  void testModeIntegration();
  void testWordNavigationUnderCursorAndHover();
  void testModeLaunchesWithACompilationDatabase();
  void testModePlainLaunchFindsTheDatabase();
  void testModePlainLaunchUsesTheDefaultDatabase();
  void testModeContextMenu();
  void testDiagnosticEchoAtCursor();
  void testFindReferences();
  void testRenameSymbol();
  void testSignatureHelp();
  void testQuickFixInTheContextMenu();
  void testDocumentSymbols();
  void testSemanticTokens();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  //! write `content` in a temporary file and return its name
  QString writeSource(const QString &name, const QString &content);
  /*!
   * \brief launch a session on the client and wait for its handshake, and
   * fail if either does not work out
   *
   * \note the client launches nothing on its own, so every test which talks to
   * a server has to ask for one, exactly as the user does with the entries of
   * the "C/C++" menu
   */
  void launchClient(const QStringList &arguments = QStringList());
  //! the "C/C++" menu a buffer of the given editor offers, null if none
  static QMenu *cxxMenu(tfel::gui::EditorWidget &);
  //! the file path question the editor waits for an answer to, null if none
  static tfel::gui::FilePathUserInput *userInput(tfel::gui::EditorWidget &);
  /*!
   * \brief the line edit a user input shows
   *
   * \note `findChild<LineEdit *>()` alone finds the line edit the constructor
   * of the input replaced, so the one which is still laid out is looked for
   */
  static tfel::gui::LineEdit *lineEditOf(tfel::gui::CommandLine &);
  /*!
   * \brief true when a widget is still laid out, that is displayed
   *
   * \note dropping a widget of the minibuffer, an input which was answered or
   * a line edit which was replaced, only takes it out of the layout and asks
   * for its deletion: the widget stays a child, and a live one, until the event
   * loop runs that deletion, which `processEvents` does not do promptly. What
   * the layout still holds is what the editor shows
   */
  static bool isLaidOut(QWidget &);
  /*!
   * \brief replace the text of the question the editor waits for with
   * \c text and validate it, which is what typing an answer and pressing
   * `Return` does
   *
   * \note the question is answered through the public slot a line edit's
   * `Return` triggers, not through the focus: an offscreen window is not an
   * active one, and \c setFocus on a widget of an inactive window puts the
   * focus nowhere
   */
  static void answerUserInput(tfel::gui::EditorWidget &, const QString &text);

  QTemporaryDir directory;
  tfel::gui::LspClient *client = nullptr;
};

void TestLspClient::isolateSettings() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-lsp-client");
  auto settings = QSettings();
  settings.remove("clangd/executable");
  // the colors of the identifiers are on by default; a run which ended before
  // it restored the setting would leave them off, and a test which asserts on
  // the colors would then ask about a state nobody chose
  settings.remove("clangd/semantic-tokens");
  // the background index would build a cache of the whole standard library
  // for each run of the test; the handshake and the requests do not need it
  settings.setValue("clangd/arguments",
                    QStringList{QStringLiteral("--background-index=false"),
                                QStringLiteral("--header-insertion=never")});
  settings.sync();
}  // end of TestLspClient::isolateSettings()

void TestLspClient::launchClient(const QStringList &arguments) {
  auto error = QString();
  QVERIFY2(client->launch(arguments, error), error.toUtf8().constData());
  QVERIFY2(client->waitUntilReady(30000), "the handshake did not complete");
}  // end of TestLspClient::launchClient

QMenu *TestLspClient::cxxMenu(tfel::gui::EditorWidget &editor) {
  for (auto *const menu : editor.getCurrentBufferSpecificMenus()) {
    if (menu->title() == QStringLiteral("C/C++")) {
      return menu;
    }
  }
  return nullptr;
}  // end of TestLspClient::cxxMenu

bool TestLspClient::isLaidOut(QWidget &w) {
  auto *const parent = w.parentWidget();
  auto *const layout = (parent != nullptr) ? parent->layout() : nullptr;
  return (layout != nullptr) && (layout->indexOf(&w) >= 0);
}  // end of TestLspClient::isLaidOut

tfel::gui::FilePathUserInput *TestLspClient::userInput(
    tfel::gui::EditorWidget &editor) {
  // an input which was answered lingers as a child of the editor until the
  // event loop deletes it, so the question which is displayed is the one the
  // minibuffer still lays out
  for (auto *const input :
       editor.findChildren<tfel::gui::FilePathUserInput *>()) {
    if (isLaidOut(*input)) {
      return input;
    }
  }
  return nullptr;
}  // end of TestLspClient::userInput

tfel::gui::LineEdit *TestLspClient::lineEditOf(tfel::gui::CommandLine &input) {
  tfel::gui::LineEdit *found = nullptr;
  for (auto *const line : input.findChildren<tfel::gui::LineEdit *>()) {
    // of the line edits the input has, the one it lays out is the one its
    // constructor left in place
    if (isLaidOut(*line)) {
      return line;
    }
    // the newest of the ones which are not laid out is the next best guess
    found = line;
  }
  return found;
}  // end of TestLspClient::lineEditOf

void TestLspClient::answerUserInput(tfel::gui::EditorWidget &editor,
                                    const QString &text) {
  auto *const input = userInput(editor);
  QVERIFY2(input != nullptr, "the editor asks for no file path");
  auto *const line = lineEditOf(*input);
  QVERIFY2(line != nullptr, "the question holds no line edit to type in");
  // the answer is the whole text of the input: the guess it is pre-filled with
  // is replaced, not completed
  line->setText(text);
  QVERIFY(line->text() == text);
  input->userEditingFinished();
}  // end of TestLspClient::answerUserInput

QString TestLspClient::writeSource(const QString &name,
                                   const QString &content) {
  const auto file = directory.filePath(name);
  auto handle = QFile(file);
  if (!handle.open(QIODevice::WriteOnly)) {
    return QString();
  }
  handle.write(content.toUtf8());
  handle.close();
  return file;
}  // end of TestLspClient::writeSource

void TestLspClient::initTestCase() {
  isolateSettings();
  if (tfel::gui::LspCxxSupport::serverExecutable().isEmpty()) {
    QSKIP(
        "clangd was not found: the Language Server Protocol client can not "
        "be tested against a server");
  }
  QVERIFY(directory.isValid());
  // no editor widget: the client reports nowhere, which is what a headless
  // test wants
  client = new tfel::gui::LspClient(nullptr, this);
}  // end of TestLspClient::initTestCase

void TestLspClient::cleanupTestCase() {
  // deleting the client shuts the server down: `shutdown`, `exit`, and the
  // transport does not have to fall back on `terminate`
  delete client;
  client = nullptr;
}  // end of TestLspClient::cleanupTestCase

void TestLspClient::testHandshake() {
  // the client never launches a server of its own accord: what a session runs
  // with, the compilation database above all, is not the editor's to guess
  QVERIFY(client->isIdle());
  // waiting for a handshake which was never asked for returns at once instead
  // of holding whoever waits for nothing
  QVERIFY(!client->waitUntilReady(100));
  QVERIFY(client->isIdle());
  launchClient();
  QVERIFY(client->isReady());
  QVERIFY(!client->isIdle());
  // a second call does not start a second server nor a second handshake
  QVERIFY(client->waitUntilReady(1000));
}  // end of TestLspClient::testHandshake

void TestLspClient::testDocumentOpenAndIncrementalSync() {
  const auto source = QStringLiteral("int main() { return 0; }\n");
  const auto file = writeSource(QStringLiteral("sync.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  client->openDocument(file, doc);
  QVERIFY(client->isDocumentOpen(file));
  QCOMPARE(client->snapshotOf(file), tfel::gui::LspProtocol::textOf(*doc));
  // every change of the document reaches the snapshot, as the range of that
  // snapshot the change replaces, not as a whole new text
  auto cursor = QTextCursor(doc);
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QStringLiteral("// a note\n"));
  QCOMPARE(client->snapshotOf(file), tfel::gui::LspProtocol::textOf(*doc));
  cursor.movePosition(QTextCursor::Start);
  cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 3);
  cursor.insertText(QStringLiteral("long"));
  QCOMPARE(client->snapshotOf(file), tfel::gui::LspProtocol::textOf(*doc));
  // the document is closed with the document of the buffer, not with this
  // test's local one; close it here, or the next tests would see it twice
  client->closeDocument(file, doc);
  QVERIFY(!client->isDocumentOpen(file));
  // the server is still answering, so the changes did not throw it off
  QVERIFY(client->isReady());
}  // end of TestLspClient::testDocumentOpenAndIncrementalSync

void TestLspClient::testDiagnostics() {
  // a file with one clear error, which is what diagnostics are published for
  const auto source = QStringLiteral(
      "int main() {\n  int x = \"not an int\";\n  return x;\n}\n");
  const auto file = writeSource(QStringLiteral("err.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  auto updates = QSignalSpy(client, &tfel::gui::LspClient::diagnosticsUpdated);
  client->openDocument(file, doc);
  // clangd answers with an empty set first, then with the real one; the
  // preamble of even a tiny file is not built in an instant
  QVERIFY(
      waitFor([&] { return !client->diagnosticsFor(file).isEmpty(); }, 30000));
  const auto diagnostics = client->diagnosticsFor(file);
  auto found = false;
  for (const auto &d : diagnostics) {
    // the error of initializing an `int` with a string literal
    if ((d.severity == 1) && (d.startLine == 1)) {
      found = true;
    }
  }
  QVERIFY(found);
  // the signal fired at least once for this file
  auto signaled = false;
  for (const auto &arguments : updates) {
    if (arguments.at(0).toString() == file) {
      signaled = true;
    }
  }
  QVERIFY(signaled);
  client->closeDocument(file, doc);
  // a file never published for has no diagnostics, never stale ones
  QVERIFY(
      client->diagnosticsFor(QStringLiteral("/does/not/exist.cxx")).isEmpty());
}  // end of TestLspClient::testDiagnostics

void TestLspClient::testRelaunchReplacesTheSession() {
  // the options of a session are fixed when the server starts, so launching
  // again with another compilation database replaces the running session
  // instead of asking it to change its mind
  QVERIFY(client->isReady());
  const auto source = QStringLiteral(
      "int main() {\n  int x = \"not an int\";\n  return x;\n}\n");
  const auto file = writeSource(QStringLiteral("restart.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  client->openDocument(file, doc);
  QVERIFY(
      waitFor([&] { return !client->diagnosticsFor(file).isEmpty(); }, 30000));
  // the underlines of a server which is about to stop knowing the buffers must
  // not outlive it: watch for the file being published empty while the new
  // session is launched, which is a thing `launch` does before it returns
  auto cleared = false;
  const auto on_update = QObject::connect(
      client, &tfel::gui::LspClient::diagnosticsUpdated, [&](const QString &f) {
        if ((f == file) && (client->diagnosticsFor(f).isEmpty())) {
          cleared = true;
        }
      });
  launchClient(QStringList() << (QStringLiteral("--compile-commands-dir=") +
                                 directory.path()));
  QVERIFY(cleared);
  QObject::disconnect(on_update);
  // the session is over and a new one answered: the documents the editor holds
  // have been opened on it again, which the diagnostics coming back proves, as
  // this second set can only come from a server which saw the text itself
  QVERIFY(client->isReady());
  QVERIFY(
      waitFor([&] { return !client->diagnosticsFor(file).isEmpty(); }, 30000));
  client->closeDocument(file, doc);
}  // end of TestLspClient::testRelaunchReplacesTheSession

void TestLspClient::testStopEndsTheSession() {
  // `stop` is what the "Stop clangd" entry and the `stop-clangd` command do:
  // the session is dismissed, the underlines of the server which is gone go
  // away with it, and the client is as if no session had ever been launched
  QVERIFY(client->isReady());
  const auto source = QStringLiteral(
      "int main() {\n  int x = \"not an int\";\n  return x;\n}\n");
  const auto file = writeSource(QStringLiteral("stop.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  client->openDocument(file, doc);
  QVERIFY(
      waitFor([&] { return !client->diagnosticsFor(file).isEmpty(); }, 30000));
  QVERIFY(client->stop());
  // the answer says a server was running, and the client is back to the state
  // of a session which was never launched
  QVERIFY(client->isIdle());
  // the underlines of a server which no longer knows the buffers must not
  // outlive it
  QVERIFY(client->diagnosticsFor(file).isEmpty());
  // a server which is gone has nothing left to dismiss: the second stop
  // cleans up nothing and says it was not running
  QVERIFY(!client->stop());
  // the documents were marked as never sent, so the next session gets their
  // whole text anew, which the diagnostics coming back prove: this second set
  // can only come from a server which saw the text itself
  launchClient();
  QVERIFY(client->isReady());
  QVERIFY(
      waitFor([&] { return !client->diagnosticsFor(file).isEmpty(); }, 30000));
  client->closeDocument(file, doc);
}  // end of TestLspClient::testStopEndsTheSession

void TestLspClient::testDefinition() {
  const auto source = QStringLiteral(
      "#include <iostream>\n"
      "\n"
      "int counter = 0;\n"
      "\n"
      "int main() {\n"
      "  std::cout << counter;\n"
      "  return counter;\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("def.cxx"), source);
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  client->openDocument(file, doc);
  // the last `counter`, the one of the `return`
  const auto position = source.lastIndexOf(QStringLiteral("counter"));
  QVERIFY(position > 0);
  auto locations = std::make_shared<QVector<tfel::gui::LspLocation>>();
  QVERIFY(tfel::gui::LspProtocol::boundedWait(
      [&, this](const std::function<void()> &stop) {
        client->requestDefinition(
            file, *doc, position,
            [locations, stop](const QVector<tfel::gui::LspLocation> &l) {
              *locations = l;
              stop();
            });
      },
      30000));
  QVERIFY(!locations->isEmpty());
  QCOMPARE(locations->at(0).file, file);
  // the definition of `counter` is on its line of declaration, on the `c`
  QCOMPARE(locations->at(0).line, 2);
  QCOMPARE(locations->at(0).character, 4);
  client->closeDocument(file, doc);
}  // end of TestLspClient::testDefinition

void TestLspClient::testHover() {
  const auto source = QStringLiteral(
      "int counter = 0;\n"
      "\n"
      "int main() {\n"
      "  return counter;\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("hover.cxx"), source);
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  client->openDocument(file, doc);
  const auto position = source.lastIndexOf(QStringLiteral("counter"));
  QVERIFY(position > 0);
  struct Answer {
    QString text;
    bool markdown = false;
  };
  auto answer = std::make_shared<Answer>();
  QVERIFY(tfel::gui::LspProtocol::boundedWait(
      [&, this](const std::function<void()> &stop) {
        client->requestHover(
            file, *doc, position,
            [answer, stop](const QString &text, const bool markdown) {
              answer->text = text;
              answer->markdown = markdown;
              stop();
            });
      },
      30000));
  // whatever the server's format, the hover of `counter` says `counter` and
  // says it is an `int`
  QVERIFY(answer->text.contains(QStringLiteral("counter")));
  QVERIFY(answer->text.contains(QStringLiteral("int")));
  Q_UNUSED(answer->markdown);
  client->closeDocument(file, doc);
}  // end of TestLspClient::testHover

void TestLspClient::testCompletion() {
  // an incomplete `std::cou` is a completion waiting to happen; the error it
  // leaves behind is of no consequence for this request
  const auto source = QStringLiteral(
      "#include <iostream>\n"
      "int main() {\n"
      "  std::cou\n"
      "  return 0;\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("completion.cxx"), source);
  auto edit = QPlainTextEdit();
  edit.setPlainText(source);
  auto *const doc = edit.document();
  client->openDocument(file, doc);
  const auto position =
      source.lastIndexOf(QStringLiteral("cou")) + QStringLiteral("cou").size();
  QVERIFY(position > 0);
  // the first completion has to wait for the preamble of `<iostream>`, which
  // can take a moment on a cold cache; the later ones are instant, so the
  // request is asked again until it answers
  auto labels = std::make_shared<QStringList>();
  auto asked = QElapsedTimer();
  asked.start();
  while (labels->isEmpty() && (asked.elapsed() < 60000)) {
    QVERIFY(tfel::gui::LspProtocol::boundedWait(
        [&, this](const std::function<void()> &stop) {
          client->requestCompletion(
              file, *doc, position,
              [labels, stop](const QStringList &l, const QStringList &) {
                *labels = l;
                stop();
              });
        },
        20000));
    if (labels->isEmpty()) {
      QTest::qWait(500);
    }
  }
  QVERIFY(!labels->isEmpty());
  // `std::cout` is a completion of `std::cou`, under some spelling of its
  // name: the text to insert names it
  auto found = false;
  for (const auto &label : *labels) {
    if (label.contains(QStringLiteral("cout"))) {
      found = true;
    }
  }
  QVERIFY(found);
  client->closeDocument(file, doc);
}  // end of TestLspClient::testCompletion

void TestLspClient::testModeMenu() {
  // the entries the mode puts in the menu bar, which are the only way a session
  // starts: what they launch, and when they launch it
  const auto source = QStringLiteral("int main() { return 0; }\n");
  const auto file = writeSource(QStringLiteral("menu-entries.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &lsp = tfel::gui::LspClient::get(*editor);
  QVERIFY(lsp.isIdle());
  auto *const menu = cxxMenu(*editor);
  QVERIFY(menu != nullptr);
  const auto actions = menu->actions();
  // the two launch entries, a separator, the overview of the file, the entry
  // which keeps the colors of the identifiers, a separator, and the entry which
  // ends the session
  QCOMPARE(static_cast<int>(actions.size()), 7);
  QVERIFY(actions.at(0)->text().contains(QStringLiteral("Launch clangd")));
  QVERIFY(
      actions.at(1)->text().contains(QStringLiteral("compile_commands.json")));
  QVERIFY(actions.at(2)->isSeparator());
  QVERIFY(actions.at(3)->text().contains(QStringLiteral("Go to symbol")));
  // the colors are the one thing the server says about the text which is
  // nobody's answer to a question, so the entry which says whether to hear it
  // is checkable
  QVERIFY(actions.at(4)->text().contains(QStringLiteral("Color identifiers")));
  QVERIFY(actions.at(4)->isCheckable());
  QVERIFY2(actions.at(4)->isChecked(), "the colors are off by default");
  QVERIFY(actions.at(5)->isSeparator());
  QVERIFY(actions.at(6)->text().contains(QStringLiteral("Stop clangd")));
  // the plain entry launches at once, with no question asked first
  actions.at(0)->trigger();
  QVERIFY(!editor->hasUserInput());
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  // the other entry asks first: the session which is running has to be
  // replaced by the one the answer names, not by a guess, and a session
  // started before the answer would have to be replaced before it was of any
  // use
  actions.at(1)->trigger();
  QVERIFY(editor->hasUserInput());
  // the question is a file path, with completion, and it starts where a
  // database is likely to be found: the directory of the file which is open
  auto *const question = userInput(*editor);
  QVERIFY(question != nullptr);
  auto *const line = lineEditOf(*question);
  QVERIFY(line != nullptr);
  QCOMPARE(line->text(), directory.path() + '/');
  // an answer which names nothing is an answer: the plain session again
  answerUserInput(*editor, QStringLiteral("/does/not/exist.json"));
  QVERIFY(!editor->hasUserInput());
  // a refusal leaves the running session alone
  QVERIFY(lsp.isReady());
  actions.at(1)->trigger();
  answerUserInput(*editor, QStringLiteral(""));
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  QVERIFY(!editor->hasUserInput());
  // the same through the commands, which are what a `Meta-X` offers and what
  // a key binding is written against
  editor->launchCommand(QStringLiteral("launch-clangd"));
  QVERIFY(!editor->hasUserInput());
  QVERIFY2(lsp.waitUntilReady(30000),
           "the 'launch-clangd' command launched no session");
  editor->launchCommand(QStringLiteral("launch-clangd-with-compile-commands"));
  QVERIFY(editor->hasUserInput());
  QVERIFY(userInput(*editor) != nullptr);
  answerUserInput(*editor, QStringLiteral(""));
  QVERIFY(!editor->hasUserInput());
  QVERIFY2(lsp.waitUntilReady(30000),
           "the answer of 'launch-clangd-with-compile-commands' launched no "
           "session");
  // the last entry ends the session: the client goes back to the state of a
  // session which was never launched
  actions.at(6)->trigger();
  QVERIFY2(lsp.isIdle(), "'Stop clangd' left a running session");
  // stopping again asks nothing of a server which is gone, and says so
  actions.at(6)->trigger();
  QVERIFY(lsp.isIdle());
  // the `stop-clangd` command ends a session which is running: the entry and
  // the command are two doors to one function, and a command which was never
  // registered is an unknown command, not a stop, so the wait below is what
  // tells the two apart
  editor->launchCommand(QStringLiteral("launch-clangd"));
  QVERIFY2(lsp.waitUntilReady(30000),
           "the launch following a 'stop' did not complete");
  QVERIFY(lsp.isDocumentOpen(file));
  editor->launchCommand(QStringLiteral("stop-clangd"));
  QVERIFY2(lsp.isIdle(), "the 'stop-clangd' command left a running session");
  // and the buffers the mode holds are opened again on the next session, which
  // is what the version reset of `stop` is for
  editor->launchCommand(QStringLiteral("launch-clangd"));
  QVERIFY2(lsp.waitUntilReady(30000),
           "the launch following a 'stop-clangd' did not complete");
  QVERIFY(lsp.isDocumentOpen(file));
  editor.reset();
}  // end of TestLspClient::testModeMenu

void TestLspClient::testModeIntegration() {
  // the whole chain at once: a buffer opened in the editor takes the C++
  // mode, the mode opens the document on the client of its editor widget,
  // clangd publishes a diagnostic, and the diagnostic ends up as an underline
  // of the text edit, next to the current line highlight and not replacing it
  const auto source = QStringLiteral(
      "int main() {\n  int x = \"not an int\";\n  return x;\n}\n");
  const auto file = writeSource(QStringLiteral("mode.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY(dynamic_cast<const tfel::gui::CxxMajorMode *>(
              &frame.getMajorMode()) != nullptr);
  // opening the file registered the document on the client of the editor
  // widget and launched no server; the session is what the user asks for, and
  // the whole chain below only runs once that happened
  auto &lsp = tfel::gui::LspClient::get(*editor);
  QVERIFY(lsp.isIdle());
  // opening the file registered the document on the client all the same, so
  // the session which is launched next has the buffer to send it
  QVERIFY(lsp.isDocumentOpen(file));
  cxxMenu(*editor)->actions().at(0)->trigger();
  // an underlined diagnostic is the visible half of the chain: the current
  // line highlight shares the document with it, see
  // `TextEditBase::setExtraSelections(const QString &, ...)`; the style is
  // `SingleUnderline` because that is what this Qt draws in a plain text
  // edit, see `CxxMajorMode::applyDiagnostics`
  auto underlined = [&frame]() {
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.format.underlineStyle() != QTextCharFormat::NoUnderline) &&
          !selection.format.hasProperty(QTextFormat::FullWidthSelection)) {
        return true;
      }
    }
    return false;
  };
  QVERIFY(waitFor(underlined, 30000));
  // the two contributions coexist: the current line is highlighted too
  auto highlighted = false;
  for (const auto &selection : frame.extraSelections()) {
    if (selection.format.hasProperty(QTextFormat::FullWidthSelection)) {
      highlighted = true;
    }
  }
  QVERIFY(highlighted);
  editor.reset();
}  // end of TestLspClient::testModeIntegration

void TestLspClient::testWordNavigationUnderCursorAndHover() {
  // the word the cursor sits on gains an underline, and the word the mouse
  // rests on gains the icon of the jump as its mouse cursor, when clangd
  // knows where the symbol is defined or declared
  const auto source = QStringLiteral(
      "int counter = 0;\n"
      "\n"
      "int twice() {\n"
      "  return 2 * counter;\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("navigation.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  auto *const doc = frame.document();
  QVERIFY(doc != nullptr);
  // `counter` in the body of `twice`: its definition is the line above, so
  // both `go-to-definition` and `go-to-declaration` answer it
  const auto text = doc->toPlainText();
  const auto twice = text.indexOf("twice");
  QVERIFY(twice > 0);
  const auto word = text.indexOf("counter", twice);
  QVERIFY(word > 0);
  auto underlined = [&frame, word](const int start, const int length) {
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.cursor.selectionStart() == start) &&
          (selection.cursor.selectionEnd() == start + length) &&
          (selection.format.underlineStyle() != QTextCharFormat::NoUnderline)) {
        return true;
      }
    }
    return false;
  };
  auto c = QTextCursor(doc);
  c.setPosition(word + 2);
  frame.setTextCursor(c);
  QVERIFY(waitFor([&] { return underlined(word, 7); }, 30000));
  // the word loses the underline as the cursor moves off it, and a word
  // which is no symbol does not gain one: `return` is a keyword, and clangd
  // points nowhere
  const auto keyword = text.indexOf("return");
  QVERIFY(keyword > 0);
  auto k = QTextCursor(doc);
  k.setPosition(keyword + 2);
  frame.setTextCursor(k);
  QVERIFY(waitFor([&] { return !underlined(word, 7); }, 30000));
  QVERIFY(!waitFor([&] { return underlined(keyword, 6); }, 3000));
  // the mouse resting on a jumpable word is the other half: the viewport
  // cursor becomes the icon of the jump, the pointing hand where the theme
  // has no such icon, and neither once the mouse is off the word
  auto *const view = frame.widget()->viewport();
  auto wc = QTextCursor(doc);
  wc.setPosition(word + 2);
  const auto rect = frame.cursorRect(wc);
  const QPoint over(rect.x() + 3, rect.y() + rect.height() / 2);
  // the point has to name the word it is supposed to hover, or the test
  // would prove nothing about the mouse and everything about the mapping
  const auto at = frame.cursorForPosition(over).position();
  QVERIFY((at >= word) && (at < word + 7));
  auto jumped = [view]() {
    const auto shape = view->cursor().shape();
    return (shape == Qt::PointingHandCursor) || (shape == Qt::BitmapCursor);
  };
  QVERIFY(!jumped());
  QTest::mouseMove(view, over);
  QVERIFY(waitFor(jumped, 30000));
  // away from the text, in the empty corner below it: the nearest character
  // is no word, and the icon goes back to the editor's own cursor
  QTest::mouseMove(view, QPoint(2, view->height() - 2));
  QVERIFY(waitFor([&] { return !jumped(); }, 30000));
  editor.reset();
}  // end of TestLspClient::testWordNavigationUnderCursorAndHover

void TestLspClient::testModeLaunchesWithACompilationDatabase() {
  // the second entry of the "C/C++" menu: the database is asked for in the
  // minibuffer, and the session which is launched reads the buffers as that
  // database says they are compiled
  const auto source = QStringLiteral(
      "int main() {\n  int x = \"not an int\";\n  return x;\n}\n");
  // the file and its database live in a subtree of the temporary directory: a
  // database at its root would be found by the plain launches of the tests
  // which run after this one, which launch the sessions they do not ask for
  const auto project = QDir(directory.path()).filePath("db-project");
  QVERIFY(QDir().mkpath(project));
  const auto file = QDir(project).filePath("database.cxx");
  {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::WriteOnly));
    handle.write(source.toUtf8());
    handle.close();
  }
  QVERIFY(!file.isEmpty());
  // a database of one entry: the file and how it is compiled. Whether the
  // server reads the answer as a file or as the directory of that file is what
  // `LspCxxSupport::compileCommandsArguments` is tested for, see
  // `test-lsp-protocol`; what is tested here is that the answer reaches the
  // server it is meant for, which the diagnostics it publishes say
  const auto build = QDir(project).filePath("build");
  QVERIFY(QDir(project).mkpath(QStringLiteral("build")));
  const auto database = QDir(build).filePath("compile_commands.json");
  const auto entry = QJsonObject{{"directory", project},
                                 {"file", file},
                                 {"command", QStringLiteral("c++ -std=c++11 "
                                                            "-fsyntax-only "
                                                            "%1")
                                                 .arg(file)}};
  {
    auto handle = QFile(database);
    QVERIFY(handle.open(QIODevice::WriteOnly));
    handle.write(QJsonDocument(QJsonArray{entry}).toJson());
    handle.close();
  }
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &lsp = tfel::gui::LspClient::get(*editor);
  QVERIFY(lsp.isIdle());
  const auto actions = cxxMenu(*editor)->actions();
  actions.at(1)->trigger();
  QVERIFY(editor->hasUserInput());
  // the file of the database, not its directory: the server is told which
  // directory to look in, so the name of the file is dropped on the way
  answerUserInput(*editor, database);
  QVERIFY(!editor->hasUserInput());
  QVERIFY2(waitFor([&] { return lsp.isReady(); }, 30000),
           "the session asked for through the menu did not answer");
  // and the buffer of the editor reaches that session: the error the file is
  // written for is published for it
  QVERIFY2(waitFor([&] { return !lsp.diagnosticsFor(file).isEmpty(); }, 30000),
           "the session launched with a database published nothing");
  editor.reset();
}  // end of TestLspClient::testModeLaunchesWithACompilationDatabase

namespace {

  // create a file and the directories which hold it; the mode tests build
  // little project trees in the temporary directory of the class so that the
  // search of a plain launch has something to find, or something to miss
  bool makeFile(const QString &path, const QByteArray &content) {
    const auto info = QFileInfo(path);
    if (!QDir().mkpath(info.absolutePath())) {
      return false;
    }
    auto handle = QFile(path);
    if (!handle.open(QIODevice::WriteOnly)) {
      return false;
    }
    const auto written = handle.write(content) == content.size();
    handle.close();
    return written;
  }  // end of makeFile()

  // a compilation database of one entry: the given file compiled with the
  // given flags, which is all a test needs to make the server say something
  // it would not say without the database
  bool makeDatabase(const QString &path,
                    const QString &file,
                    const QString &flags) {
    const auto entry = QJsonObject{
        {QStringLiteral("directory"), QFileInfo(file).absolutePath()},
        {QStringLiteral("file"), file},
        {QStringLiteral("command"),
         QStringLiteral("c++ -std=c++11 -fsyntax-only %1 %2")
             .arg(flags, file)}};
    return makeFile(path, QJsonDocument(QJsonArray{entry}).toJson());
  }  // end of makeDatabase()

  // an editor widget which remembers what the minibuffer was told: which
  // database a launch ended up using is part of the behaviour under test, and
  // overriding this public virtual is the only door to the messages
  struct RecordingEditor : tfel::gui::EditorWidget {
    QStringList messages;
    void displayInformativeMessage(const QString &m) override {
      messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct RecordingEditor

}  // end of namespace

void TestLspClient::testModePlainLaunchFindsTheDatabase() {
  // the plain entry of the "C/C++" menu, and the `launch-clangd` command,
  // take the database of the project when there is one: the file below is
  // clean as long as no database defines `TFEL_TEST_DATABASE` for it, and
  // holds an error once a session reads the database which does
  const auto project = QDir(directory.path()).filePath("search-project");
  const auto file = QDir(project).filePath("src/searched.cxx");
  QVERIFY(makeFile(project + "/CMakeLists.txt", "project(s)\n"));
  QVERIFY(makeFile(file,
                   "#ifdef TFEL_TEST_DATABASE\n"
                   "int x = \"not an int\";\n"
                   "#endif\n"
                   "int main() { return 0; }\n"));
  const auto database = project + "/build/compile_commands.json";
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &lsp = tfel::gui::LspClient::get(*editor);
  QVERIFY(lsp.isIdle());
  // the first session is a plain one: the project has no database yet, and
  // clangd finds the file harmless without one; the message says so, as the
  // end result, not as a search which came up empty
  auto answered = [&lsp, &editor](const QString &expected) {
    const auto before = editor->messages.size();
    cxxMenu(*editor)->actions().at(0)->trigger();
    QVERIFY2(lsp.waitUntilReady(30000),
             "the session asked for through the plain entry did not answer");
    QVERIFY(before < editor->messages.size());
    QCOMPARE(editor->messages.back(), expected);
  };
  answered(QStringLiteral(
      "no compile_commands.json found, clangd is restricted to current "
      "C++ file"));
  // the database appears where the search looks for an out-of-source build,
  // and a relaunch of the same entry is supposed to find it: the editor
  // searches so that it can tell what the session was started with, and a
  // session which reads the database says what the database says
  QVERIFY(makeDatabase(database, file, "-DTFEL_TEST_DATABASE"));
  answered(QStringLiteral("using %1 to start clangd").arg(database));
  QVERIFY2(waitFor([&] { return !lsp.diagnosticsFor(file).isEmpty(); }, 30000),
           "the database found by the search did not reach the server");
  editor.reset();
}  // end of TestLspClient::testModePlainLaunchFindsTheDatabase

void TestLspClient::testModePlainLaunchUsesTheDefaultDatabase() {
  // the `--compile-commands-file` the command line can hand over: a plain
  // launch prefers it over any search, and falls back to the search when it
  // names a file which does not exist
  const auto db_directory = QDir(directory.path()).filePath("default-db");
  const auto database = QDir(db_directory).filePath("compile_commands.json");
  const auto file = QDir(directory.path()).filePath("default-used.cxx");
  QVERIFY(makeFile(file,
                   "#ifdef TFEL_TEST_DEFAULT\n"
                   "int x = \"not an int\";\n"
                   "#endif\n"
                   "int main() { return 0; }\n"));
  // the database is deliberately not named `build`: only the option can bring
  // a session to it, the search of the project would never look there
  QVERIFY(makeDatabase(database, file, "-DTFEL_TEST_DEFAULT"));
  tfel::gui::LspCxxSupport::setDefaultCompileCommandsFile(database);
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &lsp = tfel::gui::LspClient::get(*editor);
  const auto before = editor->messages.size();
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY2(lsp.waitUntilReady(30000),
           "the session started with the default database did not answer");
  QVERIFY(before < editor->messages.size());
  QCOMPARE(editor->messages.back(),
           QStringLiteral("using %1 to start clangd").arg(database));
  QVERIFY2(waitFor([&] { return !lsp.diagnosticsFor(file).isEmpty(); }, 30000),
           "the default database did not reach the server");
  editor.reset();
  // a default which points nowhere does not fail the launch and is not
  // reported as a miss of its own: the search takes over, and the message
  // says what was used, with the default that played no part in brackets
  const auto missing_default =
      QStringLiteral("/does/not/exist/compile_commands.json");
  const auto project = QDir(directory.path()).filePath("fallback-project");
  const auto fallback_file = QDir(project).filePath("src/fallback.cxx");
  QVERIFY(makeFile(project + "/CMakeLists.txt", "project(f)\n"));
  QVERIFY(makeFile(fallback_file,
                   "#ifdef TFEL_TEST_FALLBACK\n"
                   "int x = \"not an int\";\n"
                   "#endif\n"
                   "int main() { return 0; }\n"));
  const auto fallback_database = project + "/build/compile_commands.json";
  tfel::gui::LspCxxSupport::setDefaultCompileCommandsFile(missing_default);
  auto fallback = std::make_unique<RecordingEditor>();
  fallback->show();
  fallback->openFile(fallback_file);
  auto &fallback_lsp = tfel::gui::LspClient::get(*fallback);
  // the project has no database yet either: the message holds neither a
  // `using` nor a path, and names the default it could not use either
  auto fallback_before = fallback->messages.size();
  cxxMenu(*fallback)->actions().at(0)->trigger();
  QVERIFY2(fallback_lsp.waitUntilReady(30000),
           "a default which does not exist voided the launch");
  QVERIFY(fallback_before < fallback->messages.size());
  QCOMPARE(fallback->messages.back(),
           QStringLiteral("no compile_commands.json found and default file "
                          "%1 is not found, clangd is restricted to current "
                          "C++ file")
               .arg(missing_default));
  // the database appears where the search looks, and the next launch says
  // that it used it, with the default it did not use on the side
  QVERIFY(
      makeDatabase(fallback_database, fallback_file, "-DTFEL_TEST_FALLBACK"));
  fallback_before = fallback->messages.size();
  cxxMenu(*fallback)->actions().at(0)->trigger();
  QVERIFY2(fallback_lsp.waitUntilReady(30000),
           "the session of a project with a database and no usable default "
           "did not answer");
  QVERIFY(fallback_before < fallback->messages.size());
  QCOMPARE(
      fallback->messages.back(),
      QStringLiteral("using %1 to start clangd (default file %2 not found)")
          .arg(fallback_database, missing_default));
  QVERIFY2(
      waitFor(
          [&] { return !fallback_lsp.diagnosticsFor(fallback_file).isEmpty(); },
          30000),
      "the search did not take over after a default which points nowhere");
  fallback.reset();
  // the static of the editor is none of the tests' business afterwards, and
  // it outlives this one
  tfel::gui::LspCxxSupport::setDefaultCompileCommandsFile(QString());
}  // end of TestLspClient::testModePlainLaunchUsesTheDefaultDatabase

void TestLspClient::testModeContextMenu() {
  // the right click entry of the mode: the menu is completed with a "Go to
  // definition" entry only where the server has somewhere to jump to, and the
  // answer the mode asked for is what the entry jumps to, without a second
  // request — including for a symbol without a body, whose definition question
  // clangd answers with its declaration
  const auto source = QStringLiteral(
      "int counter = 0;\n"
      "\n"
      "int main() {\n"
      "  return counter;\n"
      "}\n"
      "\n"
      "int maybe();\n"
      "\n"
      "int call_maybe() {\n"
      "  return maybe();\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("menu.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the mode answers a right click only from a running session, so one is
  // launched the way the user launches it
  auto &lsp = tfel::gui::LspClient::get(*editor);
  QVERIFY(lsp.isIdle());
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY2(lsp.waitUntilReady(30000), "the session did not answer");
  // `getMajorMode` only hands the mode out as a constant, and completing a
  // context menu is a non-const operation; the menu the mode is about to fill
  // is a local of this scope, nothing else survives the call
  auto &mode = const_cast<tfel::gui::MajorMode &>(frame.getMajorMode());
  struct Answer {
    //! the entries the mode added
    int entries = 0;
    //! true when one of them comes before the entries of the text edit
    bool on_top = false;
    //! where the first entry puts the cursor once triggered, -1 if none
    int position = -1;
  };
  // the menu is a local, as it is in `TextEditBase::event`, and it owns the
  // entries the mode created: they can not leave this scope alive, which is why
  // the answer is carried as numbers and the jump is done here
  auto ask = [&frame, &mode](const int position, const QString &wanted) {
    auto answer = Answer();
    auto menu = QMenu();
    auto cursor = QTextCursor(frame.document());
    cursor.setPosition(position);
    mode.completeContextMenu(&menu, cursor);
    const auto actions = menu.actions();
    for (auto *const a : actions) {
      if (!a->text().startsWith(wanted)) {
        continue;
      }
      ++answer.entries;
      answer.on_top = answer.on_top || (a == actions.first());
      if (answer.position < 0) {
        a->trigger();
        answer.position = frame.textCursor().position();
      }
    }
    return answer;
  };
  // the preamble of even a tiny file is not built in an instant, and the mode
  // waits for the server a moment only: the request is asked again until the
  // menu holds the entry
  const auto use =
      static_cast<int>(source.lastIndexOf(QStringLiteral("counter")));
  auto answer = Answer();
  auto asked = QElapsedTimer();
  asked.start();
  while ((answer.entries == 0) && (asked.elapsed() < 60000)) {
    answer = ask(use, QStringLiteral("Go to definition"));
    if (answer.entries == 0) {
      QTest::qWait(500);
    }
  }
  QCOMPARE(answer.entries, 1);
  // the entry is at the top of the menu, before the undo and copy entries the
  // text edit puts in it
  QVERIFY(answer.on_top);
  // the definition of `counter` is its declaration on the first line, on the
  // name itself
  QCOMPARE(answer.position,
           static_cast<int>(source.indexOf(QStringLiteral("counter"))));
  // no symbol where the menu was opened, no entry: the mode asked the server
  // and got nothing, so the plain menu is shown undelayed
  const auto blank =
      static_cast<int>(source.indexOf(QStringLiteral("\n\n"))) + 1;
  QCOMPARE(ask(blank, QStringLiteral("Go to")).entries, 0);
  // a symbol which is only declared, `maybe` has no body anywhere, is no
  // special case either: `clangd` answers the definition question with the
  // declaration of a symbol it cannot find a body for, so the entry is
  // offered and jumps to the prototype — where the hover icon of the mode
  // promised the jump was
  const auto call =
      static_cast<int>(source.lastIndexOf(QStringLiteral("maybe")));
  auto from_decl = QElapsedTimer();
  from_decl.start();
  answer = Answer();
  while ((answer.entries == 0) && (from_decl.elapsed() < 60000)) {
    answer = ask(call, QStringLiteral("Go to definition"));
    if (answer.entries == 0) {
      QTest::qWait(500);
    }
  }
  QCOMPARE(answer.entries, 1);
  // the prototype on its own line, on the name itself
  QCOMPARE(answer.position,
           static_cast<int>(source.indexOf(QStringLiteral("maybe"))));
  editor.reset();
}  // end of TestLspClient::testModeContextMenu

void TestLspClient::testDiagnosticEchoAtCursor() {
  // the text of a diagnostic, which the underlines only locate, is echoed in
  // the minibuffer once the cursor rests on the underlined range; a clean
  // spot repeats nothing, and the mouse is told the same as a tooltip
  const auto source = QStringLiteral(
      "int twice() {\n"
      "  return 2 * missing_thing;\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("diag.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  const auto word = source.indexOf("missing_thing");
  QVERIFY(word > 0);
  // the underlines, and the echo with them, wait for the server to have
  // parsed the file and published what it found
  QVERIFY(waitFor([&] { return !lsp.diagnosticsFor(file).isEmpty(); }, 30000));
  auto echoed = [&editor](const QString &what) {
    for (const auto &m : editor->messages) {
      if (m.contains(what)) {
        return true;
      }
    }
    return false;
  };
  auto c = QTextCursor(frame.document());
  c.setPosition(word + 4);
  frame.setTextCursor(c);
  // the echo comes once the settling timer of the mode goes off, and it is
  // what the underline hid: severity, checker and the server's own words
  QVERIFY(waitFor([&] { return echoed("undeclared"); }, 30000));
  const auto echoed_so_far = editor->messages.size();
  // a clean spot says nothing, and says nothing again: the memory of what
  // was last echoed is what keeps a settling timer restarted by an edit on
  // the same line from repeating the message
  auto away = QTextCursor(frame.document());
  away.setPosition(0);
  frame.setTextCursor(away);
  QTest::qWait(500);
  QVERIFY(!echoed("twice"));
  // and the same diagnostic under the cursor once more is told once more,
  // which is what proves the memory is what silences the repetition rather
  // than the echo being a one-shot
  frame.setTextCursor(c);
  QVERIFY(
      waitFor([&] { return editor->messages.size() > echoed_so_far; }, 30000));
  QVERIFY(editor->messages.back().contains("undeclared"));
  // the mouse half: the virtual pointer's position is what the tooltip path
  // reads, so the move comes first and the resting of the mouse, which is
  // what a `ToolTip` event says, comes as the event
  auto *const view = frame.widget()->viewport();
  auto wc = QTextCursor(frame.document());
  wc.setPosition(word + 4);
  const auto rect = frame.cursorRect(wc);
  const QPoint over(rect.x() + 3, rect.y() + rect.height() / 2);
  const auto at = frame.cursorForPosition(over).position();
  QVERIFY((at >= word) && (at < word + 13));
  QToolTip::hideText();
  QTest::mouseMove(view, over);
  auto resting = QEvent(QEvent::ToolTip);
  QApplication::sendEvent(view, &resting);
  // the tip is a label of its own, wherever the platform put it; its text is
  // the same sentence the minibuffer echoed
  auto tip_text = QString();
  QVERIFY(waitFor(
      [&] {
        for (auto *const w : QApplication::topLevelWidgets()) {
          auto *const l = qobject_cast<QLabel *>(w);
          if ((l != nullptr) && l->isVisible() && (!l->text().isEmpty())) {
            tip_text = l->text();
            return true;
          }
        }
        return false;
      },
      5000));
  QVERIFY(tip_text.contains("undeclared"));
  QToolTip::hideText();
  editor.reset();
}  // end of TestLspClient::testDiagnosticEchoAtCursor

void TestLspClient::testFindReferences() {
  // Alt-, asks the server for every use of the symbol under the cursor and
  // pops a menu of `file:line` entries which jump: the definition, counted in
  // as a reference, and each use
  const auto source = QStringLiteral(
      "int counter = 0;\n"
      "\n"
      "int a() { return counter; }\n"
      "\n"
      "int b() { return counter; }\n");
  const auto file = writeSource(QStringLiteral("refs.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  const auto in_a =
      static_cast<int>(source.indexOf("counter", source.indexOf("int a()")));
  auto c = QTextCursor(frame.document());
  c.setPosition(in_a + 2);
  frame.setTextCursor(c);
  // the key reaches the mode through the filter of the text edit, and the
  // answer is waited for at most three seconds; the first request can meet a
  // server still parsing the file, so the key is pressed again as a user
  // would, until the menu or an answer says the question is settled
  QMenu *menu = nullptr;
  auto asked = QElapsedTimer();
  asked.start();
  while ((menu == nullptr) && (asked.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_Comma, Qt::AltModifier);
    menu = frame.findChild<QMenu *>("clangd-references");
    if ((menu != nullptr) && (!menu->isVisible())) {
      menu = nullptr;
    } else if (menu == nullptr) {
      QTest::qWait(500);
    }
  }
  QVERIFY2(menu != nullptr, "the references menu never came up");
  // the definition and the two uses: `includeDeclaration` is what makes the
  // first of the three there
  QCOMPARE(menu->actions().size(), 3);
  // choosing an entry jumps: the use inside `b` is the last one, and the
  // cursor lands on the name it is a use of
  menu->actions().at(2)->trigger();
  QCOMPARE(frame.textCursor().position(),
           static_cast<int>(source.lastIndexOf("counter")));
  menu->close();
  // a keyword names no symbol: the server answers there is nowhere this
  // points, and the minibuffer says so instead of popping an empty menu
  auto k = QTextCursor(frame.document());
  k.setPosition(static_cast<int>(source.indexOf("return")) + 2);
  frame.setTextCursor(k);
  const auto before = editor->messages.size();
  asked.start();
  while ((editor->messages.size() == before) && (asked.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_Comma, Qt::AltModifier);
    if (editor->messages.size() == before) {
      QTest::qWait(500);
    }
  }
  QVERIFY(editor->messages.back().contains("no references found"));
  editor.reset();
}  // end of TestLspClient::testFindReferences

void TestLspClient::testRenameSymbol() {
  // Alt-R asks a new name in the minibuffer, the `rename` request answers a
  // workspace edit, and the editor applies it wherever the symbol lives:
  // here one file with its definition and one use
  const auto source = QStringLiteral(
      "int counter = 0;\n"
      "\n"
      "int use_it() { return counter; }\n");
  const auto file = writeSource(QStringLiteral("rename.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  const auto in_use =
      static_cast<int>(source.indexOf("counter", source.indexOf("int use_it")));
  auto c = QTextCursor(frame.document());
  c.setPosition(in_use + 2);
  frame.setTextCursor(c);
  // the question of the new name: found the way any user of `findChild`
  // finds it, by the name the input gives itself, and only while the
  // minibuffer still lays it out — an answered question lingers as a child
  // of the editor until its deferred deletion runs
  auto asked = [this, ed = editor.get()]() -> tfel::gui::CommandLine * {
    for (auto *const candidate : ed->findChildren<tfel::gui::CommandLine *>()) {
      if ((candidate->objectName() == QLatin1String("rename-symbol")) &&
          isLaidOut(*candidate)) {
        return candidate;
      }
    }
    return nullptr;
  };
  // a first request can meet a server still parsing the file it just saw,
  // so the whole ritual — key, question, answer — is retried until the text
  // says the rename went through or the minute is spent
  tfel::gui::CommandLine *question = nullptr;
  auto spent = QElapsedTimer();
  spent.start();
  while ((question == nullptr) && (spent.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_R, Qt::AltModifier);
    question = asked();
    if (question == nullptr) {
      QTest::qWait(200);
    }
  }
  QVERIFY2(question != nullptr, "the rename question never came up");
  // answering it renames the use and the definition alike; an empty answer
  // would have cancelled instead, and the old name is not pre-filled so
  // that a bare Return can not rewrite a symbol with its own name
  auto renamed = [&frame] {
    const auto text = frame.document()->toPlainText();
    return text.contains(QStringLiteral("countess")) &&
           (!text.contains(QStringLiteral("counter")));
  };
  auto told = [ed = editor.get()](const QString &what) {
    for (const auto &m : ed->messages) {
      if (m.contains(what)) {
        return true;
      }
    }
    return false;
  };
  while ((!renamed()) && (!told(QStringLiteral("refused"))) &&
         (spent.elapsed() < 60000)) {
    if (!isLaidOut(*question)) {
      // the question of a refused rename: ask a new one
      question = nullptr;
      while ((question == nullptr) && (spent.elapsed() < 60000)) {
        QTest::keyClick(frame.widget(), Qt::Key_R, Qt::AltModifier);
        question = asked();
        if (question == nullptr) {
          QTest::qWait(200);
        }
      }
      if (question == nullptr) {
        break;
      }
    }
    auto *const line = lineEditOf(*question);
    QVERIFY(line != nullptr);
    line->setText(QStringLiteral("countess"));
    question->userEditingFinished();
    // `renameToAt` waits for the server in a bounded loop before applying,
    // so the text is rewritten by the time the answer is taken; a refused
    // or timed-out attempt leaves the text alone and says so
    QTest::qWait(500);
  }
  QVERIFY2(renamed(), qPrintable(frame.document()->toPlainText()));
  QVERIFY(told(QStringLiteral("occurrences renamed")));
  // the report names what changed and where: the two places of one file
  QVERIFY2(told(QStringLiteral("2 occurrences renamed in 1")),
           qPrintable(editor->messages.back()));
  // and the buffer of the file is the one on screen: the rename walked the
  // buffers and came back to the one it started from
  QCOMPARE(editor->getCurrentBuffer().getMainFrame().getCompleteFileName(),
           file);
  // a name the server refuses — a number is no identifier — changes
  // nothing and is reported with the server's own reason
  const auto before = editor->messages.size();
  spent.start();
  while ((editor->messages.size() == before) && (spent.elapsed() < 60000)) {
    question = nullptr;
    while ((question == nullptr) && (spent.elapsed() < 60000)) {
      QTest::keyClick(frame.widget(), Qt::Key_R, Qt::AltModifier);
      question = asked();
      if (question == nullptr) {
        QTest::qWait(200);
      }
    }
    if (question == nullptr) {
      break;
    }
    lineEditOf(*question)->setText(QStringLiteral("123"));
    question->userEditingFinished();
    QTest::qWait(500);
  }
  QVERIFY2((editor->messages.size() > before) &&
               (editor->messages.back().contains(QStringLiteral("refused")) ||
                editor->messages.back().contains(
                    QStringLiteral("nothing to rename"))),
           qPrintable(editor->messages.back()));
  QVERIFY(renamed());
  editor.reset();
}  // end of TestLspClient::testRenameSymbol

void TestLspClient::testSignatureHelp() {
  // the parameter list of the call the cursor stands in, echoed in the
  // minibuffer with the argument being typed picked out of it: asked for with
  // Alt-S, and asked by the editor itself on the characters clangd says a
  // call is typed around
  const auto source = QStringLiteral(
      "void store(int a);\n"
      "void store(int a, double b);\n"
      "void store(int a, double b, const char *c);\n"
      "\n"
      "void f() {\n"
      "  store(1, 2.0, \"x\");\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("sighelp.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  // one message saying all of the given things at once: a message which
  // merely mentions the function name could be the echo of a diagnostic, not
  // the parameter list this test is about
  auto said = [ed = editor.get()](const QStringList &whats) {
    for (const auto &m : ed->messages) {
      auto all = true;
      for (const auto &what : whats) {
        all = all && m.contains(what);
      }
      if (all) {
        return true;
      }
    }
    return false;
  };
  auto at = [&frame](const int line, const int character) {
    auto c = QTextCursor(frame.document());
    c.setPosition(
        tfel::gui::LspProtocol::positionOf(*frame.document(), line, character));
    frame.setTextCursor(c);
  };
  // line 5 holds the call, and its third argument fits the three-parameter
  // overload: the answer names the call and marks the argument being typed
  at(5, 16);
  QTest::keyClick(frame.widget(), Qt::Key_S, Qt::AltModifier);
  QVERIFY(waitFor([&] { return said({"store(", "<const char *c>"}); }, 30000));
  // right after the open parenthesis no argument has been typed yet: every
  // overload is still a candidate, and the first parameter is the one marked
  at(5, 8);
  QTest::keyClick(frame.widget(), Qt::Key_S, Qt::AltModifier);
  QVERIFY(waitFor(
      [&] {
        return said({QStringLiteral("store("), "<int a>", "3 candidates"});
      },
      30000));
  // a name which is a declaration, not a call, has no parameter list to
  // show: the key asked for something, so the answer is reported rather than
  // held back as it would be while typing
  at(0, 6);
  const auto before = editor->messages.size();
  QTest::keyClick(frame.widget(), Qt::Key_S, Qt::AltModifier);
  auto spent = QElapsedTimer();
  spent.start();
  while ((editor->messages.size() == before) && (spent.elapsed() < 30000)) {
    QTest::qWait(100);
  }
  QVERIFY(editor->messages.back().contains("no call around the cursor"));
  // the automatic half: typing the comma of an argument list asks the
  // question on its own, with no key involved. The stray comma is no
  // obstacle, clangd computes the list from the call as far as it parses it
  at(5, 9);
  QTest::keyClick(frame.widget(), Qt::Key_Comma);
  QVERIFY2(frame.document()->toPlainText().contains("store(1,,"),
           "the typed comma never reached the document");
  QVERIFY(waitFor([&] { return said({"store(", "<double b>"}); }, 30000));
  editor.reset();
}  // end of TestLspClient::testSignatureHelp
void TestLspClient::testQuickFixInTheContextMenu() {
  // the context menu offers the fixes clangd computes for the diagnostics
  // it published, and triggering one writes the text the server sent,
  // through the same path a rename goes by
  const auto source = QStringLiteral(
      "void f() {\n"
      "  int x = 1\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("quickfix.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  // a fix is an answer to a diagnostic, and the question only carries the
  // diagnostics the client holds: waiting for the published one is what
  // makes the menu ask a question which can be answered
  QVERIFY(waitFor([&] { return !lsp.diagnosticsFor(file).isEmpty(); }, 30000));
  auto &mode = const_cast<tfel::gui::MajorMode &>(frame.getMajorMode());
  // the menu is a local, as it is in the mouse handler of the text edit,
  // and it owns the entries the mode created
  // the entries of the text edit its own menu would have put in — undo, copy,
  // paste — are represented by one entry of this test, since what matters is
  // where the entries of the mode land next to them
  auto stock_is_last = false;
  auto fixes = [&frame, &mode, &stock_is_last]() {
    auto found = QVector<QPair<QString, QString>>();
    auto menu = QMenu();
    auto *const stock = menu.addAction(QStringLiteral("Paste"));
    auto cursor = QTextCursor(frame.document());
    cursor.setPosition(cursor.document()->find("int x = 1").selectionStart() +
                       9);
    mode.completeContextMenu(&menu, cursor);
    const auto actions = menu.actions();
    stock_is_last = (!actions.isEmpty()) && (actions.last() == stock);
    for (auto *const a : actions) {
      if (a->text().startsWith(QStringLiteral("Fix:"))) {
        found.append({a->text(), a->statusTip()});
      }
    }
    return found;
  };
  // the first menu of a file the server has just read can arrive before
  // the answer does: the mode waits a moment only, and asks again
  auto offered = QVector<QPair<QString, QString>>();
  QVERIFY(waitFor(
      [&] {
        offered = fixes();
        return !offered.isEmpty();
      },
      60000));
  // the fix of a missing semicolon says what it inserts, and the
  // diagnostic it answers to is the status tip of the entry: that is the
  // whole reason the entry is in the menu
  auto named = false;
  auto told = false;
  for (const auto &f : std::as_const(offered)) {
    named = named || f.first.contains(";");
    told = told || f.second.contains("Expected");
  }
  QVERIFY2(named, qPrintable(offered.first().first));
  QVERIFY2(told, qPrintable(offered.first().second));
  // the fixes the mode added stay above the entry of the text edit: a right
  // click came for what the server knows, not for the stock entries
  QVERIFY2(stock_is_last, "an entry of the mode landed below the stock one");
  // triggering the entry applies the edit of the server: the semicolon is
  // in the document, and the minibuffer says what was applied
  auto menu = QMenu();
  auto cursor = QTextCursor(frame.document());
  cursor.setPosition(cursor.document()->find("int x = 1").selectionStart() + 9);
  auto triggered = false;
  mode.completeContextMenu(&menu, cursor);
  for (auto *const a : menu.actions()) {
    if (a->text().contains(QStringLiteral("Fix:"))) {
      a->trigger();
      triggered = true;
      break;
    }
  }
  QVERIFY(triggered);
  QVERIFY2(frame.document()->toPlainText().contains("int x = 1;"),
           qPrintable(frame.document()->toPlainText()));
  QVERIFY(waitFor(
      [ed = editor.get()] {
        return (!ed->messages.isEmpty()) &&
               ed->messages.back().contains("edit(s)");
      },
      5000));
  // a clean spot offers no fix: the server was asked and had nothing to
  // offer, so the menu stays the plain one, undelayed
  auto clean = writeSource(QStringLiteral("clean.cxx"),
                           QStringLiteral("int ok() { return 0; }\n"));
  editor->openFile(clean);
  auto &second = editor->getCurrentBuffer().getMainFrame();
  // the client has to know the document for the menu to ask anything of it
  QVERIFY(waitFor([&] { return !lsp.snapshotOf(clean).isEmpty(); }, 30000));
  auto &second_mode = const_cast<tfel::gui::MajorMode &>(second.getMajorMode());
  auto plain = QMenu();
  auto at_zero = QTextCursor(second.document());
  second_mode.completeContextMenu(&plain, at_zero);
  for (auto *const a : plain.actions()) {
    QVERIFY2(!a->text().startsWith(QStringLiteral("Fix:")),
             qPrintable(a->text()));
  }
  editor.reset();
}  // end of TestLspClient::testQuickFixInTheContextMenu

void TestLspClient::testDocumentSymbols() {
  // Alt-O asks clangd for the overview of the file and asks, in the minibuffer,
  // which of its symbols to go to: the names are the qualified ones of the
  // tree the server answers with, the answer is a place in the buffer, and a
  // name the file does not hold is refused without moving the cursor anywhere
  const auto source = QStringLiteral(
      "struct Widget {\n"
      "  void paint(int width);\n"
      "};\n"
      "\n"
      "void Widget::paint(int width) {\n"
      "  (void)width;\n"
      "}\n"
      "\n"
      "int counter = 0;\n"
      "\n"
      "int main() {\n"
      "  return counter;\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("overview.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY(waitFor([&] { return lsp.isReady(); }, 30000));
  // the question, by the name it gives itself and while the minibuffer still
  // lays it out; the overview is not there before the answer is in hand, so a
  // server which has not read the file yet brings no question at all and the
  // key is pressed again as a user would
  auto asked = [this, ed = editor.get()]() -> tfel::gui::CommandLine * {
    for (auto *const candidate : ed->findChildren<tfel::gui::CommandLine *>()) {
      if ((candidate->objectName() == QLatin1String("go-to-symbol")) &&
          isLaidOut(*candidate)) {
        return candidate;
      }
    }
    return nullptr;
  };
  tfel::gui::CommandLine *question = nullptr;
  auto spent = QElapsedTimer();
  spent.start();
  while ((question == nullptr) && (spent.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_O, Qt::AltModifier);
    question = asked();
    if (question == nullptr) {
      QTest::qWait(200);
    }
  }
  QVERIFY2(question != nullptr, "the symbol question never came up");
  // the overview is shown as the list of completions of the question, under
  // the title every one of them shares: what is listed there is what the
  // answer is picked from
  auto &buffer = editor->getCurrentBuffer();
  auto *const overview = buffer.getCurrentSecondaryTask();
  QVERIFY2(overview != nullptr, "the overview of the file is shown nowhere");
  // the title is the one every list of completions shares, cut to the eight
  // characters a tab of the secondary tasks shows
  QVERIFY2(buffer.getSecondaryTaskTitle(overview).startsWith(
               QStringLiteral("*complet")),
           qPrintable(buffer.getSecondaryTaskTitle(overview)));
  auto *const listed = qobject_cast<tfel::gui::TextEdit *>(overview);
  auto *const view = (listed != nullptr)
                         ? listed
                         : overview->findChild<tfel::gui::TextEdit *>();
  QVERIFY2(view != nullptr, "the overview holds no text to read");
  // a question which was not answered as a tree would hold neither the
  // qualified name of the definition out of the class nor the signature the
  // `detail` of its entry carries: what is listed here is the proof of what
  // the handshake said about the shape of the answer
  const auto entries = view->document()->toPlainText();
  QVERIFY2(entries.contains(QStringLiteral("Widget::paint")),
           qPrintable(entries));
  QVERIFY2(entries.contains(QStringLiteral("void (int)")), qPrintable(entries));
  // the listing follows the name being typed: the entries are picked out of
  // the whole overview, not out of what the completion of a prefix leaves
  auto *const line = lineEditOf(*question);
  QVERIFY(line != nullptr);
  line->setText(QStringLiteral("counter"));
  QVERIFY(waitFor(
      [v = view] {
        const auto text = v->document()->toPlainText();
        return text.contains(QStringLiteral("counter")) &&
               (!text.contains(QStringLiteral("Widget::paint")));
      },
      5000));
  // answering a name goes to it: the cursor stands on the definition of the
  // global, not on the use inside `main`, and the minibuffer says where
  question->userEditingFinished();
  const auto definition = static_cast<int>(source.indexOf("counter"));
  QVERIFY(waitFor(
      [&frame, definition] {
        return frame.textCursor().position() == definition;
      },
      5000));
  auto told = [ed = editor.get()](const QString &what) {
    for (const auto &m : ed->messages) {
      if (m.contains(what)) {
        return true;
      }
    }
    return false;
  };
  QVERIFY2(told(QStringLiteral("counter (line 9)")),
           qPrintable(editor->messages.back()));
  // a name the file does not hold moves nothing and is refused; the cursor
  // stays on the symbol the last answer named
  const auto before = editor->messages.size();
  spent.start();
  while ((editor->messages.size() == before) && (spent.elapsed() < 60000)) {
    question = nullptr;
    while ((question == nullptr) && (spent.elapsed() < 60000)) {
      QTest::keyClick(frame.widget(), Qt::Key_O, Qt::AltModifier);
      question = asked();
      if (question == nullptr) {
        QTest::qWait(200);
      }
    }
    if (question == nullptr) {
      break;
    }
    lineEditOf(*question)->setText(QStringLiteral("no_such_symbol"));
    question->userEditingFinished();
    QTest::qWait(200);
  }
  QVERIFY2(
      (editor->messages.size() > before) &&
          (editor->messages.back().contains(QStringLiteral("no symbol named"))),
      qPrintable(editor->messages.back()));
  QCOMPARE(frame.textCursor().position(), definition);
  editor.reset();
}  // end of TestLspClient::testDocumentSymbols

void TestLspClient::testSemanticTokens() {
  // the colors of the identifiers of a file come from what the server says they
  // are: the words it names are drawn with the face of their kind, as a
  // contribution to the extra selections of the text edit, next to the
  // underlines of the diagnostics and the highlight of the current line, which
  // are the other contributions
  const auto source = QStringLiteral(
      "#define TWICE(x) ((x) + (x))\n"
      "\n"
      "[[deprecated(\"use something else\")]] int legacy() { return 1; }\n"
      "\n"
      "struct Widget {\n"
      "  int paint(int n) { return n * 2; }\n"
      "};\n"
      "\n"
      "int main() {\n"
      "  Widget w;\n"
      "  return w.paint(TWICE(legacy()));\n"
      "}\n");
  const auto file = writeSource(QStringLiteral("semantic-tokens.cxx"), source);
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto &lsp = tfel::gui::LspClient::get(*editor);
  // the legend of the session, which is what the numbers of an answer are read
  // against, is announced with the capabilities of the handshake: a session
  // which was never launched has nothing to ask about
  QVERIFY(!lsp.hasSemanticTokens());
  const auto actions = cxxMenu(*editor)->actions();
  // the entry which keeps the colors, and the state it shows: on, since the
  // colors are what a person gets before they have been asked about
  auto *const colors = actions.at(4);
  QVERIFY(colors->text().contains(QStringLiteral("Color identifiers")));
  QVERIFY(colors->isCheckable());
  QVERIFY2(colors->isChecked(), "the colors are off by default");
  actions.at(0)->trigger();
  QVERIFY2(lsp.waitUntilReady(30000), "no session was launched");
  QVERIFY(lsp.hasSemanticTokens());
  // the faces, seen the way the screen sees them: the words of the buffer which
  // are drawn in a color the highlighter would never have put there. What the
  // highlighter draws — the keywords, the strings, the comments — is held by
  // the document itself and is none of these selections' business
  auto *const text = frame.document();
  // the faces the text edit holds as selections, with the word each of them
  // inked: what the server said the names are, seen the way the screen sees
  // them. The underlines of the diagnostics and the highlight of the current
  // line are not faces and are not in this list; what the highlighter draws —
  // the keywords, the strings, the comments — is held by the document itself
  // and is none of these selections' business
  auto faces = [&frame, text] {
    auto painted = QStringList();
    for (const auto &selection : frame.extraSelections()) {
      const auto color = selection.format.foreground().color();
      const auto struck = selection.format.fontStrikeOut();
      if ((!struck) && ((!color.isValid()) || (color == QColor(Qt::black)))) {
        continue;
      }
      const auto start = selection.cursor.selectionStart();
      painted << QStringLiteral("%1:%2%3").arg(
          text->toPlainText()
              .mid(start, selection.cursor.selectionEnd() - start)
              .trimmed(),
          color.name(), struck ? QStringLiteral("|strike") : QString());
    }
    painted.sort();
    painted.removeDuplicates();
    return painted.join(QLatin1Char(' '));
  };
  // whether one of the faces names a word and is the color of the kind the
  // server says that word is
  auto inks = [&](const QString &word, const QColor &color) {
    const auto inked = QStringLiteral("%1:%2").arg(word, color.name());
    for (const auto &entry : faces().split(QLatin1Char(' '))) {
      if ((entry == inked) || (entry == inked + QLatin1String("|strike"))) {
        return true;
      }
    }
    return false;
  };
  // a type, a method, and a macro: what a regular expression can not tell from
  // a name, since to it an identifier is an identifier
  QVERIFY2(waitFor([&] { return inks("Widget", QColor(Qt::darkBlue)); }, 30000),
           qPrintable(QStringLiteral("no face of a type: ") + faces()));
  QVERIFY2(inks("paint", QColor(Qt::darkCyan)),
           qPrintable(QStringLiteral("no face of a method: ") + faces()));
  QVERIFY2(inks("TWICE", QColor(Qt::darkMagenta)),
           qPrintable(QStringLiteral("no face of a macro: ") + faces()));
  // the name the server says is not worth using anymore is struck out: the one
  // thing said about a name which no face of a kind could say, since it is not
  // a color but a mark over whatever color the word already had
  auto strikes_out = [&](const QString &word) {
    const auto inked = word + QLatin1Char(':');
    for (const auto &entry : faces().split(QLatin1Char(' '))) {
      if (entry.startsWith(inked) && entry.endsWith(QLatin1String("|strike"))) {
        return true;
      }
    }
    return false;
  };
  QVERIFY2(waitFor([&] { return strikes_out("legacy"); }, 30000),
           qPrintable(QStringLiteral("nothing was struck out: ") + faces()));
  // the entry turns the colors off, and the buffer of the file on screen
  // follows at once: the other buffers follow on their next settle, since the
  // setting is read on every question
  colors->trigger();
  QVERIFY2(waitFor([&] { return faces().isEmpty(); }, 5000),
           qPrintable(QStringLiteral("left painted: ") + faces()));
  QVERIFY(!QSettings().value("clangd/semantic-tokens", true).toBool());
  // and the entry turns them back on, which is what the buffer of the file is
  // asked about again, and what the rest of the suite finds as it was left: a
  // setting which stayed false would silent every later test's colors
  colors->trigger();
  QVERIFY2(waitFor([&] { return !faces().isEmpty(); }, 30000),
           "the colors did not come back");
  QVERIFY(QSettings().value("clangd/semantic-tokens", true).toBool());
  // a session which ends takes the colors of its answers with it: what the
  // server said about the names of the file is nothing a server which is gone
  // stands behind anymore, and a struck-out name nobody can confirm is a lie
  editor->launchCommand(QStringLiteral("stop-clangd"));
  QVERIFY2(waitFor([&] { return faces().isEmpty(); }, 5000),
           qPrintable(QStringLiteral("the colors outlived their session: ") +
                      faces()));
  // the setting is left on: it is the default state of the editor, and a test
  // which left it off would silent the colors every later test asks about
  editor.reset();
}  // end of TestLspClient::testSemanticTokens
QTEST_MAIN(TestLspClient)

#include "TestLspClient.moc"
