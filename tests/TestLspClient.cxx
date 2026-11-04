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
#include <QtWidgets/QMenu>
#include <QtWidgets/QPlainTextEdit>
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
  void testDefinition();
  void testHover();
  void testCompletion();
  void testModeMenu();
  void testModeIntegration();
  void testModeLaunchesWithACompilationDatabase();
  void testModePlainLaunchFindsTheDatabase();
  void testModePlainLaunchUsesTheDefaultDatabase();
  void testModeContextMenu();

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
  QCOMPARE(static_cast<int>(actions.size()), 2);
  QVERIFY(actions.at(0)->text().contains(QStringLiteral("Launch clangd")));
  QVERIFY(
      actions.at(1)->text().contains(QStringLiteral("compile_commands.json")));
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
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &lsp = tfel::gui::LspClient::get(*editor);
  QVERIFY(lsp.isIdle());
  // the first session is a plain one: the project has no database yet, and
  // clangd finds the file harmless without one
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY2(lsp.waitUntilReady(30000),
           "the plain session of a project with no database did not answer");
  // the database appears where the search looks for an out-of-source build,
  // and a relaunch of the same entry is supposed to find it: the editor
  // searches so that it can tell what the session was started with, and a
  // session which reads the database says what the database says
  QVERIFY(makeDatabase(database, file, "-DTFEL_TEST_DATABASE"));
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY2(lsp.waitUntilReady(30000),
           "the plain session of a project with a database did not answer");
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
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &lsp = tfel::gui::LspClient::get(*editor);
  cxxMenu(*editor)->actions().at(0)->trigger();
  QVERIFY2(lsp.waitUntilReady(30000),
           "the session started with the default database did not answer");
  QVERIFY2(waitFor([&] { return !lsp.diagnosticsFor(file).isEmpty(); }, 30000),
           "the default database did not reach the server");
  editor.reset();
  // a default which points nowhere is reported and the search takes over: the
  // mistake of a stale command line does not void the launch
  const auto project = QDir(directory.path()).filePath("fallback-project");
  const auto fallback_file = QDir(project).filePath("src/fallback.cxx");
  QVERIFY(makeFile(project + "/CMakeLists.txt", "project(f)\n"));
  QVERIFY(makeFile(fallback_file,
                   "#ifdef TFEL_TEST_FALLBACK\n"
                   "int x = \"not an int\";\n"
                   "#endif\n"
                   "int main() { return 0; }\n"));
  QVERIFY(makeDatabase(project + "/build/compile_commands.json", fallback_file,
                       "-DTFEL_TEST_FALLBACK"));
  tfel::gui::LspCxxSupport::setDefaultCompileCommandsFile(
      QStringLiteral("/does/not/exist/compile_commands.json"));
  auto fallback = std::make_unique<tfel::gui::EditorWidget>();
  fallback->show();
  fallback->openFile(fallback_file);
  auto &fallback_lsp = tfel::gui::LspClient::get(*fallback);
  cxxMenu(*fallback)->actions().at(0)->trigger();
  QVERIFY2(fallback_lsp.waitUntilReady(30000),
           "a default which does not exist voided the launch");
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
  // definition" entry only where the server has a definition to offer, and the
  // answer the mode asked for is what the entry jumps to, without a second
  // request
  const auto source = QStringLiteral(
      "int counter = 0;\n"
      "\n"
      "int main() {\n"
      "  return counter;\n"
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
  auto ask = [&frame, &mode](const int position) {
    auto answer = Answer();
    auto menu = QMenu();
    auto cursor = QTextCursor(frame.document());
    cursor.setPosition(position);
    mode.completeContextMenu(&menu, cursor);
    const auto actions = menu.actions();
    for (auto *const a : actions) {
      if (!a->text().startsWith(QStringLiteral("Go to definition"))) {
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
    answer = ask(use);
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
  QCOMPARE(ask(blank).entries, 0);
  editor.reset();
}  // end of TestLspClient::testModeContextMenu

QTEST_MAIN(TestLspClient)

#include "TestLspClient.moc"
