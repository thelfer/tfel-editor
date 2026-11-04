/*!
 * \file   tests/TestLspClient.cxx
 * \brief  tests of the Language Server Protocol client against a real
 *         `clangd`, skipped when the machine has none.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QElapsedTimer>
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
#include "TFEL/GUI/CxxMajorMode.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
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
  void testDefinition();
  void testHover();
  void testCompletion();
  void testModeIntegration();
  void testModeContextMenu();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  //! write `content` in a temporary file and return its name
  QString writeSource(const QString &name, const QString &content);

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
  QVERIFY(client->waitUntilReady(20000));
  QVERIFY(client->isReady());
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
