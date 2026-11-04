/*!
 * \file   tests/TestMainWindow.cxx
 * \brief  tests of the real application shape: a main window with its menu bar
 *         and its central editor widget.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QTemporaryDir>
#include <QtWidgets/QMenuBar>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MainWindow.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// `MainWindow` used to be invisible to anything linking the shared library:
// the build hides every symbol which is not `TFEL_GUI_VISIBILITY_EXPORT`, and
// the probes of issue #121 had to build a `QMainWindow` with a menu bar and an
// editor as central widget to get the shape the height propagation of
// `MultiLineLineEdit` walks up. It is exported now, so the real thing is what
// is driven here.
struct TestMainWindow : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void opensOneBufferPerFile();
  void installingAUserInputLeavesTheMainWindowAlone();

 private:
  void pump(int n = 4);
};

void TestMainWindow::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-main-window");
}  // end of TestMainWindow::initTestCase

void TestMainWindow::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestMainWindow::pump

void TestMainWindow::opensOneBufferPerFile() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto path = dir.path() + "/greeting.txt";
  {
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("hello\n");
    f.close();
  }
  tfel::gui::MainWindow window(QStringList() << path);
  pump(8);
  auto* const ed = window.findChild<tfel::gui::EditorWidget*>();
  QVERIFY2(ed != nullptr, "the main window holds no editor widget");
  // `getCurrentBuffer` before any user input is constructed
  auto& b = ed->getCurrentBuffer();
  QVERIFY(b.getBufferRawName().endsWith("greeting.txt"));
  QCOMPARE(b.getMainFrame().document()->toPlainText(), QString("hello\n"));
}  // end of TestMainWindow::opensOneBufferPerFile

void TestMainWindow::installingAUserInputLeavesTheMainWindowAlone() {
  tfel::gui::MainWindow window;
  window.resize(1000, 700);
  window.show();
  pump(8);
  auto* const ed = window.findChild<tfel::gui::EditorWidget*>();
  QVERIFY(ed != nullptr);
  const auto window_height = window.height();
  const auto editor_height = ed->height();
  // the window gave the editor most of itself, below the menu bar: the height
  // of a one line input must not propagate up to the editor, which is where
  // the walk has to stop, since the window, not the editor, is what decides
  // how tall the editing area is
  QVERIFY2(editor_height > 400, "the editor was not given the window");
  QVERIFY2(window_height > 600, "the window did not keep the requested size");
  ed->openFile();
  pump(6);
  // an user input waiting for an answer replaced the message page of the
  // minibuffer: the assertion above is about *that* transition
  QVERIFY2(ed->findChild<tfel::gui::CommandLine*>() != nullptr,
           "no input was installed");
  QCOMPARE(window.height(), window_height);
  QCOMPARE(ed->height(), editor_height);
}  // end of TestMainWindow::installingAUserInputLeavesTheMainWindowAlone

QTEST_MAIN(TestMainWindow)

#include "TestMainWindow.moc"
