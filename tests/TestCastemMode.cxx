/*!
 * \file   TestCastemMode.cxx
 * \brief  tests for the session the `Cast3M` mode runs with the solver: the
 *         lines its sending commands hand to it, from the very first one.
 * \author Thomas Helfer
 * \date   9 nov. 2026
 */

#include <QtTest/QtTest>
#include <QtCore/QLibrary>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QTextCursor>
#include <QtWidgets/QApplication>
#include <functional>
#include <memory>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// The `Cast3M` mode is a plugin, and its class is not exported: the library is
// loaded the way the editor loads it, and the sending commands — protected
// slots of the mode — are reached by name through the meta-object, which is
// what `Ctrl-C Ctrl-L` of the buffers goes through.
//
// `/bin/cat` plays the solver. It writes back every line it is given, so what
// the interaction frame displays is exactly what reached the process, in the
// order it was given in: the cases ask which lines arrived, not what a solver
// makes of them, and the real program is none of the machines the tests run on
// holds.
namespace {

  //! the lines of the buffer the cases send
  auto firstLine() { return QStringLiteral("DEBPOIN"); }
  auto secondLine() { return QStringLiteral("TEMP/MODEL 293.;"); }
  auto thirdLine() { return QStringLiteral("FINPOIN;"); }

  /*!
   * \brief a buffer of the `Cast3M` mode, with the mode instance its sending
   * commands are called on
   */
  struct Session {
    //! the editor the buffer belongs to; the cases keep it alive
    std::unique_ptr<tfel::gui::EditorWidget> editor;
    //! the instance of the mode of the buffer, a null pointer on failure
    tfel::gui::MajorMode *mode = nullptr;
  };  // end of struct Session

  /*!
   * \brief the lines the interpreter wrote back to the interaction frame
   * \param[in] frame : the frame the output of the process is displayed in
   *
   * \note the text of a document spells the end of its blocks with the
   * paragraph separator, which is what `toPlainText` joins them with and which
   * is no visible character; the empty lines the appends of the output leave
   * around are dropped, so what is left is the lines the program received
   */
  QStringList receivedLines(tfel::gui::ProcessOutputFrame &frame) {
    auto text = frame.document()->toPlainText();
    text.replace(QChar(0x2029), QLatin1Char('\n'));
    return text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  }  // end of receivedLines

}  // end of anonymous namespace

struct TestCastemMode : QObject {
  Q_OBJECT

 public:
  TestCastemMode();

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void theInterpreterIsTheProgramTheConfigurationNames();
  void theLineWhichStartsTheSolverReachesIt();
  void theLinesOfASelectionReachTheSolverInOrder();
  void theWholeBufferReachesTheSolverFromItsFirstLine();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  /*!
   * \brief a buffer of the mode, its cursor on the line `n` (from 0), with the
   * interpreter of the cases named by the configuration
   */
  Session makeBuffer(const int n);
  //! the interaction frame of a buffer, null while nothing was launched
  static tfel::gui::ProcessOutputFrame *interactionFrame(tfel::gui::Buffer &);
  /*!
   * \brief call a sending command of the mode by name
   * \param[in] mode : the instance of the mode of the buffer
   * \param[in] slot : the name of the slot, one of the three sending commands
   * \return what the command answered, false when it was not reached
   */
  static bool send(QObject &mode, const char *const slot);
  //! spin the event loop until the predicate holds or the time is up
  static bool waitFor(const std::function<bool()> &predicate, const int ms);
  //! the directory the file of the cases is written in
  QTemporaryDir directory;
#ifdef TFEL_EDITOR_CASTEM_MODE
  //! the library of the packaged mode, named by the build
  QLibrary library;
#endif /* TFEL_EDITOR_CASTEM_MODE */
  //! whether a session can be started at all on this machine
  bool available = false;
};  // end of struct TestCastemMode

TestCastemMode::TestCastemMode() {
#ifdef TFEL_EDITOR_CASTEM_MODE
  // the build names the library of the packaged mode, which is what the cases
  // load in `initTestCase`
  this->library.setFileName(QStringLiteral(TFEL_EDITOR_CASTEM_MODE));
#endif /* TFEL_EDITOR_CASTEM_MODE */
}

void TestCastemMode::isolateSettings() {
  // an own configuration file: the cases name the program the mode launches and
  // must leave the configuration of the user alone
  QApplication::setOrganizationName("tfel-editor-tests");
  QApplication::setApplicationName("test-castem-mode");
  auto settings = QSettings();
  settings.remove("castem/executable");
  settings.sync();
}  // end of TestCastemMode::isolateSettings

void TestCastemMode::initTestCase() {
  isolateSettings();
  QVERIFY(directory.isValid());
#ifdef TFEL_EDITOR_CASTEM_MODE
  QVERIFY2(this->library.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      this->library.errorString()));
  this->available = QFileInfo::exists(QStringLiteral("/bin/cat"));
#else  /* TFEL_EDITOR_CASTEM_MODE */
  // this build names no library of the packaged modes: the cases below skip
  // themselves, as they can neither load a mode nor ask anything of it
  this->available = false;
#endif /* TFEL_EDITOR_CASTEM_MODE */
}  // end of TestCastemMode::initTestCase

void TestCastemMode::cleanupTestCase() {
  isolateSettings();
}  // end of TestCastemMode::cleanupTestCase

Session TestCastemMode::makeBuffer(const int n) {
  // the address is written for every buffer: the mode reads it at each launch,
  // and the program the tests launch is the solver of the cases, not the
  // release a person would have stored in the configuration of the editor
  auto session = Session();
  QSettings().setValue("castem/executable", QStringLiteral("/bin/cat"));
  const auto file = directory.filePath(QStringLiteral("session.procedur"));
  {
    auto handle = QFile(file);
    if (!handle.open(QIODevice::WriteOnly)) {
      return session;
    }
    handle.write((firstLine() + QLatin1Char('\n') + secondLine() +
                  QLatin1Char('\n') + thirdLine() + QLatin1Char('\n'))
                     .toLatin1());
    handle.close();
  }
  session.editor = std::make_unique<tfel::gui::EditorWidget>();
  session.editor->show();
  session.editor->openFile(file);
  auto &frame = session.editor->getCurrentBuffer().getMainFrame();
  // `Cast3M` is the mode a `.procedur` file is opened in already; asking for it
  // by name is what hands an instance of the mode in hand, and not the `const
  // MajorMode &` a frame answers with
  session.mode = frame.setMajorMode(QStringLiteral("Cast3M"));
  auto tc = frame.textCursor();
  tc.movePosition(QTextCursor::Start);
  for (auto i = 0; i != n; ++i) {
    tc.movePosition(QTextCursor::NextBlock);
  }
  frame.setTextCursor(tc);
  return session;
}  // end of TestCastemMode::makeBuffer

tfel::gui::ProcessOutputFrame *TestCastemMode::interactionFrame(
    tfel::gui::Buffer &buffer) {
  return qobject_cast<tfel::gui::ProcessOutputFrame *>(
      buffer.getCurrentSecondaryTask());
}  // end of TestCastemMode::interactionFrame

bool TestCastemMode::send(QObject &mode, const char *const slot) {
  auto sent = false;
  QMetaObject::invokeMethod(&mode, slot, Q_RETURN_ARG(bool, sent));
  return sent;
}  // end of TestCastemMode::send

bool TestCastemMode::waitFor(const std::function<bool()> &predicate,
                             const int ms) {
  auto waited = 0;
  while (!predicate()) {
    if (waited > ms) {
      return false;
    }
    QTest::qWait(50);
    waited += 50;
  }
  return true;
}  // end of TestCastemMode::waitFor

void TestCastemMode::theInterpreterIsTheProgramTheConfigurationNames() {
  if (!this->available) {
    QSKIP(
        "the packaged mode or an interpreter to launch is missing: no "
        "session can be started");
  }
  auto session = makeBuffer(0);
  QVERIFY(session.editor != nullptr);
  QVERIFY(session.mode != nullptr);
  auto &buffer = session.editor->getCurrentBuffer();
  QVERIFY(send(*session.mode, "sendLineToCastem"));
  auto *const co = interactionFrame(buffer);
  QVERIFY2(co != nullptr,
           "sending a line opened no interaction with the solver");
  // the session runs the program the configuration names, and nothing else
  QCOMPARE(co->getProcess().program(), QStringLiteral("/bin/cat"));
  // and that session is a secondary task of the buffer it belongs to, titled
  // after the solver, which is where a person reads what the solver answered
  QVERIFY2(buffer.getSecondaryTaskIndex(co) >= 0,
           "the interaction with the solver is not a task of the buffer");
  QCOMPARE(buffer.getSecondaryTaskTitle(co), QStringLiteral("* castem"));
  session.editor.reset();
}  // end of
   // TestCastemMode::theInterpreterIsTheProgramTheConfigurationNames

void TestCastemMode::theLineWhichStartsTheSolverReachesIt() {
  if (!this->available) {
    QSKIP(
        "the packaged mode or an interpreter to launch is missing: no "
        "session can be started");
  }
  auto session = makeBuffer(0);
  QVERIFY(session.editor != nullptr);
  QVERIFY(session.mode != nullptr);
  auto &buffer = session.editor->getCurrentBuffer();
  // `Ctrl-C Ctrl-L` on the first line of a buffer both starts the solver and
  // hands it that line. A process which was just asked to start is `Starting`
  // for a few milliseconds, and what is written to it then waits in its own
  // queue until the child is up: a command which refuses to write in that state
  // loses the line which opened the session — and says nothing about it but an
  // `unknown shortcut` for the keys, since the mode answers that it did nothing
  QVERIFY(send(*session.mode, "sendLineToCastem"));
  auto *const co = interactionFrame(buffer);
  QVERIFY(co != nullptr);
  QVERIFY2(waitFor([&co] { return receivedLines(*co).size() >= 1; }, 10000),
           qPrintable("the line which started the solver never reached it: it "
                      "received '" +
                      receivedLines(*co).join(QStringLiteral(" / ")) + "'"));
  QCOMPARE(receivedLines(*co), QStringList() << firstLine());
  // and the lines which follow, sent once the child is up for sure, go to the
  // same session, in the order they were sent, each without a trace of the line
  // which precedes it in the buffer
  auto tc = buffer.getMainFrame().textCursor();
  tc.movePosition(QTextCursor::NextBlock);
  buffer.getMainFrame().setTextCursor(tc);
  QVERIFY(send(*session.mode, "sendLineToCastem"));
  QVERIFY2(waitFor([&co] { return receivedLines(*co).size() >= 2; }, 10000),
           qPrintable("the second line of the buffer never reached the solver: "
                      "it received '" +
                      receivedLines(*co).join(QStringLiteral(" / ")) + "'"));
  QCOMPARE(receivedLines(*co), QStringList() << firstLine() << secondLine());
  session.editor.reset();
}  // end of TestCastemMode::theLineWhichStartsTheSolverReachesIt

void TestCastemMode::theLinesOfASelectionReachTheSolverInOrder() {
  if (!this->available) {
    QSKIP(
        "the packaged mode or an interpreter to launch is missing: no "
        "session can be started");
  }
  auto session = makeBuffer(0);
  QVERIFY(session.editor != nullptr);
  QVERIFY(session.mode != nullptr);
  auto &buffer = session.editor->getCurrentBuffer();
  auto &frame = buffer.getMainFrame();
  // the two first lines of the buffer, which is the shape of `Ctrl-C Ctrl-R`:
  // the first of them is sent while the solver is still starting
  auto tc = frame.textCursor();
  tc.movePosition(QTextCursor::Start);
  tc.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
  tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  frame.setTextCursor(tc);
  QVERIFY(tc.hasSelection());
  QVERIFY(send(*session.mode, "sendRegionToCastem"));
  auto *const co = interactionFrame(buffer);
  QVERIFY(co != nullptr);
  const auto expected = QStringList() << firstLine() << secondLine();
  QVERIFY2(
      waitFor([&co, &expected] { return receivedLines(*co) == expected; },
              10000),
      qPrintable("the lines of a selection did not all reach the solver, in "
                 "the order the buffer spells them: it received '" +
                 receivedLines(*co).join(QStringLiteral(" / ")) + "'"));
  session.editor.reset();
}  // end of TestCastemMode::theLinesOfASelectionReachTheSolverInOrder

void TestCastemMode::theWholeBufferReachesTheSolverFromItsFirstLine() {
  if (!this->available) {
    QSKIP(
        "the packaged mode or an interpreter to launch is missing: no "
        "session can be started");
  }
  auto session = makeBuffer(1);
  QVERIFY(session.editor != nullptr);
  QVERIFY(session.mode != nullptr);
  auto &buffer = session.editor->getCurrentBuffer();
  // `Ctrl-C Ctrl-B` from the second line of the buffer: the command walks the
  // lines from the first one, which is the line that starts the solver, so a
  // session which swallows that line refused the whole buffer — and said so by
  // answering that the keys mean nothing
  QVERIFY(send(*session.mode, "sendBufferToCastem"));
  auto *const co = interactionFrame(buffer);
  QVERIFY(co != nullptr);
  const auto expected = QStringList()
                        << firstLine() << secondLine() << thirdLine();
  QVERIFY2(
      waitFor([&co, &expected] { return receivedLines(*co) == expected; },
              10000),
      qPrintable("the lines of the buffer did not all reach the solver, in "
                 "the order the buffer spells them: it received '" +
                 receivedLines(*co).join(QStringLiteral(" / ")) + "'"));
  session.editor.reset();
}  // end of
   // TestCastemMode::theWholeBufferReachesTheSolverFromItsFirstLine

QTEST_MAIN(TestCastemMode)

#include "TestCastemMode.moc"
