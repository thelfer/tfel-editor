/*!
 * \file   tests/TestLicosMode.cxx
 * \brief  tests of the links of the `Licos` mode: the file a name the study holds
 *         between quotes opens, through the key which follows a link and the click
 *         which does the same.
 * \author Thomas Helfer
 * \date   20/11/2026
 */

#include <memory>
#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLibrary>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtWidgets/QPlainTextEdit>
#include <QtTest/QtTest>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::PlainTextEdit;
  using tfel::gui::TextEditBase;

  //! the file the cases visit: the suffix `ple` is what chooses the mode
  const char *const study_name = "study.ple";

  // The lines of the study, one link per line. A case names the shape it is about
  // by the line the cursor stands on, so the study written by `makeFixture` and
  // these numbers are one thing read two ways and have to be kept together.
  const int plain_line = 1;           // 'plain.txt', a file beside the study
  const int directory_line = 2;       // 'inputs/inpath.txt', a name with a `/`
  const int double_quote_line = 3;    // "dquote.txt", the other quote style
  const int user_input_line = 4;      // 'user_input.txt', of LICOS_USER_FILES
  const int user_script_line = 5;     // 'table.dat', in its `scripts` directory
  const int user_data_line = 6;       // 'user_data.txt', in its `data` directory
  const int shared_line = 7;          // 'shared.txt', of the LICOS installation
  const int report_line = 8;          // 'report.ps', a PostScript report
  const int mfront_line = 9;          // 'behaviour.mfront', a source of the study
  const int nested_mfront_line = 10;  // a source of its `mfront` sub-directories
  const int absent_line = 11;         // 'absent.txt', a name of no file
  const int mismatched_line = 12;     // 'mixed.txt", one quote and the other
  const int plus_line = 13;           // 'a+b.txt', a name with a `+` in it
  const int accented_line = 14;       // a name which is no ASCII

  // the name the accented line names, spelled with the bytes of the file itself:
  // what the study holds on disk does not depend on the execution character set
  const char *const accented_target = "r\xC3\xA9sultat.txt";

  /*!\brief the window the cases are run in, which keeps what the minibuffer was
   * told
   *
   * Where a link went and what the mode said about it are the two halves of the
   * answer: a link which opens a file and then reports that it found none, and one
   * which opens nothing and says nothing at all, are both what this suite is about,
   * and neither is visible without the messages.
   */
  struct RecordingEditor : tfel::gui::EditorWidget {
    QStringList messages;

    void displayInformativeMessage(const QString &m) override {
      messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct RecordingEditor

}  // end of namespace

/*!
 * \brief the links of the `Licos` mode
 *
 * The mode comes from the packaged plugin, loaded as the editor loads it; its class
 * is not exported, so everything the cases ask goes through the keys, the click and
 * the public API of the editor, which is the face the hand has.
 *
 * The fixture is a study and the two search paths of the language: the files beside
 * the study, one of them in a sub-directory of it; a tree of `LICOS_USER_FILES`
 * whose files stand in the three sub-directories a study lays its own files in; and
 * a tree standing for an installation of the language, whose files stand under
 * `share/licos`. Both search paths are directories of this test's own temporary
 * directory, named by the environment for the whole run, so a name which is no file
 * of the study is answered by one of them or by nothing at all, and an assertion
 * about where a name was found names the candidate which answered.
 *
 * The window is built by the first case which needs one and goes away at the end of
 * every case, with the places the editor saves for each file it visits. A mode is a
 * child of a text edit and a buffer restored from a previous run stands where the
 * cursor of that run was left, which are two ways for a case to ask about a state
 * another case made.
 */
struct TestLicosMode : QObject {
  Q_OBJECT

 public:
  TestLicosMode();

 private slots:
  void initTestCase();
  void cleanupTestCase();

  //! \brief a window of the cases which inherits nothing from the case before it
  void init();
  //! \brief take the window of the case away
  void cleanup();

  // the two ways of following a link, and the shape which is no link at all
  void theKeyWhichFollowsALinkOpensTheFileBesideTheStudy();
  void theClickWhichFollowsALinkOpensTheSameFile();
  void aNameBetweenTwoDifferentQuotesIsNoLink();

  // what a name is, which is one answer shared by the underline and the jump
  void aNameNamingASubDirectoryIsALink();
  void aNameBetweenDoubleQuotesIsALink();
  void aNameHoldingAPlusSignIsALink();
  void aNameWhichIsNoAsciiIsALink();

  // where a name is looked for
  void theFilesOfTheUsersTreeAreFoundInTheirOwnSubDirectory();
  void theFileOfTheInstallationIsFound();
  void aFileOfTheStudyIsNotAnsweredWithAComplaintAboutTheEnvironment();
  void aNameOfNoFileIsReportedAndOpensNothing();

  // the two extensions which are not looked for among the files of a study
  void aPostScriptReportOfTheStudyOpensInABuffer();
  void aSourceOfTheStudyOpensOnceAndSaysNothing();
  void aSourceOfTheTreeOfMaterialsOfTheStudyOpens();

 private:
  //! \brief let the event loop run, twice, and reap what it dropped
  void pump(int = 2);

  /*!
   * \brief write the fixture of the study and of the two search paths
   * \return the path of the study file, or an empty name when nothing was built
   */
  QString makeFixture();

  /*!
   * \brief build the window of the cases and put the study on the screen
   *
   * The window is built on the first call; `openFile` of a file the window already
   * visits reuses its buffer and makes it current, which is how a case follows a
   * second link of one study after the first one took the screen away.
   *
   * What this asserts is the precondition of the whole suite: the mode the buffer
   * wears is the one the plugin registered for the suffix `ple`, and the buffer has
   * a view — a key pressed in a buffer of no mode, or a click aimed at no viewport,
   * reaches no link at all. It stands as a function of its own because `QVERIFY`
   * expands to a bare `return`, which a function answering a reference can not do.
   */
  void ensureStudy();

  //! \brief answer the text edit of the study, in the window of the cases
  TextEditBase &studyFrame();

  /*!\brief stand the cursor inside the quoted name of the given line of the study
   *
   * The place is the second character of the name: the place in front of it is the
   * place between the quote and the name, and a name which is a link is followed
   * from anywhere between its two quotes.
   */
  void standOnTheName(int line);

  //! \return the text of the given line of the study, for the words of a failure
  QString lineText(int line);

  /*!\brief follow the link of the given line and answer which file the buffer on
   * the screen visits afterwards
   * \param[in] line : the line of the study the cursor stands on
   * \param[in] click : follow the link with a left click rather than with `Enter`
   *
   * Following a link opens the file it names and makes its buffer current, whether
   * that buffer was built then or held by the window before, which is what a hand
   * sees and what a case asks. A press which found no link is not a link at all and
   * reaches the text edit, where it writes the newline of a `Return`: the text of
   * the study is put back before the answer is given, or the lines the presses
   * after it stand on are the ones that press broke.
   */
  QString follow(int line, bool click = false);

  /*!\brief follow the link of the given line and assert the answer it gave
   * \param[in] line : the line of the study the cursor stands on
   * \param[in] expected : the file the link must open, empty when it must open none
   * \param[in] message : a text the minibuffer must name, empty when the link must
   * say nothing about itself
   * \param[in] click : follow the link with a left click rather than with `Enter`
   *
   * A link which opens nothing is a defect, and so is one which opens a file and
   * reports that it found none.
   */
  void followTheLink(int line,
                     const QString &expected,
                     const QString &message = QString(),
                     bool click = false);

  //! \return whether any message the window holds says `what`
  bool told(const QString &what) const;

  QTemporaryDir root;
#ifdef TFEL_EDITOR_LICOS_MODE
  //! the library of the packaged mode, named by the build
  QLibrary library;
#endif /* TFEL_EDITOR_LICOS_MODE */
  //! the path of the study the cases stand in
  QString study;
  //! the text of the study as the file holds it, to put the buffer back with
  QString study_text;
  //! the window, built by the first case which needs one
  std::unique_ptr<RecordingEditor> editor;
};  // end of struct TestLicosMode

TestLicosMode::TestLicosMode() {
#ifdef TFEL_EDITOR_LICOS_MODE
  // the build names the library of the packaged mode, which is what the cases load
  // in `initTestCase`
  this->library.setFileName(QStringLiteral(TFEL_EDITOR_LICOS_MODE));
#endif /* TFEL_EDITOR_LICOS_MODE */
}

void TestLicosMode::initTestCase() {
  // an own configuration file: the cases leave the configuration of the user alone,
  // and a stale answer of a previous run has nothing to say about a study this test
  // wrote a moment ago
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-licos-mode");
#ifdef TFEL_EDITOR_LICOS_MODE
  QVERIFY2(this->library.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      this->library.errorString()));
#else  /* TFEL_EDITOR_LICOS_MODE */
  qWarning("this build names no library of the `Licos` mode: the cases can neither "
           "load a mode nor ask anything of it");
#endif /* TFEL_EDITOR_LICOS_MODE */
  QVERIFY(root.isValid());
  this->study = makeFixture();
  QVERIFY2(!this->study.isEmpty(), "the fixture could not be written");
  // the two search paths of the language, named for the whole run: both are trees of
  // this test's own, so a name of no file of the study is answered by one of them or
  // by nothing at all
  qputenv("LICOS_USER_FILES", root.filePath("user").toUtf8());
  qputenv("LICOSHOME", root.filePath("install").toUtf8());
}  // end of TestLicosMode::initTestCase

void TestLicosMode::cleanupTestCase() {
  qputenv("LICOS_USER_FILES", QByteArray());
  qputenv("LICOSHOME", QByteArray());
}  // end of TestLicosMode::cleanupTestCase

void TestLicosMode::init() {
  // where the cursor stood in a file the window of the case before built is restored
  // when a buffer of that file is built again, which would stand every case on a
  // line of the study of its own choosing
  auto settings = QSettings();
  settings.remove("positions in files");
  settings.sync();
}  // end of TestLicosMode::init

void TestLicosMode::cleanup() {
  // the window goes with the case: it holds the buffers, their modes, and what the
  // minibuffer was told about the links they followed
  this->editor.reset();
  pump();
  auto settings = QSettings();
  settings.remove("positions in files");
  settings.sync();
}  // end of TestLicosMode::cleanup

void TestLicosMode::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestLicosMode::pump

QString TestLicosMode::makeFixture() {
  auto write = [](const QString &path, const QString &content) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    auto file = QFile(path);
    if (!file.open(QIODevice::WriteOnly)) {
      return false;
    }
    file.write(content.toUtf8());
    return true;
  };
  const auto study = root.filePath("study");
  // the study itself: one link per line, in the order of the numbers of the lines
  // named at the top of this file. The line of two different quotes is deliberately
  // quoted twice, which is the shape the case about it is about
  this->study_text =
      QStringLiteral("Study 'links'\n"
                     "Curve 'plain.txt'\n"
                     "Curve 'inputs/inpath.txt'\n"
                     "Curve \"dquote.txt\"\n"
                     "Curve 'user_input.txt'\n"
                     "Curve 'table.dat'\n"
                     "Curve 'user_data.txt'\n"
                     "Curve 'shared.txt'\n"
                     "Curve 'report.ps'\n"
                     "Type 'behaviour.mfront'\n"
                     "Type 'nested.mfront'\n"
                     "Curve 'absent.txt'\n"
                     "Curve 'mixed.txt\"\n"
                     "Curve 'a+b.txt'\n"
                     "Curve '%1'\n"
                     "EndOf\n")
          .arg(QString::fromUtf8(accented_target));
  if (!write(study + "/" + study_name, this->study_text)) {
    return QString();
  }
  // the files of the study itself, one of them in a sub-directory of it, as a study
  // of the language lays its own files about the study
  for (const auto *const n : {"plain.txt", "dquote.txt", "report.ps",
                              "behaviour.mfront", "a+b.txt"}) {
    if (!write(study + "/" + n, "a file of the study\n")) {
      return QString();
    }
  }
  if (!write(study + "/inputs/inpath.txt", "a file of a sub-directory\n") ||
      !write(study + "/mfront/behaviours/nested.mfront", "@file nested\n") ||
      !write(study + "/" + QString::fromUtf8(accented_target),
             "un fichier de l'etude\n")) {
    return QString();
  }
  // the tree of `LICOS_USER_FILES`: the same three sub-directories a study lays its
  // files in, which is what the list of candidates of the language is made of
  const auto user = root.filePath("user");
  if (!write(user + "/inputs/user_input.txt", "an input of the user\n") ||
      !write(user + "/scripts/table.dat", "a table of the user\n") ||
      !write(user + "/data/user_data.txt", "a data file of the user\n")) {
    return QString();
  }
  // the tree standing for an installation of the language, whose files stand under
  // `share/licos`
  return write(root.filePath("install/share/licos/inputs/shared.txt"),
               "a file of the installation\n")
             ? study + "/" + study_name
             : QString();
}  // end of TestLicosMode::makeFixture

void TestLicosMode::ensureStudy() {
  if (editor == nullptr) {
    // the window is built once per case: a mode is a child of a text edit, and what
    // the cases ask is what the same window answers under a hand
    editor = std::make_unique<RecordingEditor>();
    editor->show();
    pump();
  }
  editor->openFile(this->study);
  pump();
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(frame.getMajorMode().getName() == QLatin1String("Licos"),
           qPrintable(QString("the study is not read by the `Licos` mode: it is "
                              "read by ") +
                      frame.getMajorMode().getName()));
  QVERIFY2(frame.widget() != nullptr, "the buffer of the study has no view");
}  // end of TestLicosMode::ensureStudy

TextEditBase &TestLicosMode::studyFrame() {
  ensureStudy();
  return editor->getCurrentBuffer().getMainFrame();
}  // end of TestLicosMode::studyFrame

void TestLicosMode::standOnTheName(const int line) {
  auto &frame = studyFrame();
  auto tc = frame.textCursor();
  tc.movePosition(QTextCursor::Start);
  tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
  tc.movePosition(QTextCursor::StartOfBlock);
  // the second character of the quoted name, wherever the quote of the line stands:
  // a name is quoted where the line puts it
  const auto text = tc.block().text();
  const auto single = text.indexOf('\'');
  const auto double_quote = text.indexOf('"');
  auto quote = single;
  if ((quote < 0) || ((double_quote >= 0) && (double_quote < quote))) {
    quote = double_quote;
  }
  QVERIFY2(quote >= 0,
           qPrintable(QString("the line names no quoted name: ") + text));
  tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, quote + 2);
  frame.setTextCursor(tc);
  pump();
}  // end of TestLicosMode::standOnTheName

QString TestLicosMode::lineText(const int line) {
  // read from the buffer of the window, which is the text a press is asked about —
  // the file on disk holds the same text until a press which found no link writes
  // one of its own into the buffer, which `follow` puts back
  auto &frame = studyFrame();
  return frame.document()->findBlockByNumber(line).text();
}  // end of TestLicosMode::lineText

QString TestLicosMode::follow(const int line, const bool click) {
  auto &frame = studyFrame();
  standOnTheName(line);
  auto &view = *qobject_cast<QPlainTextEdit *>(frame.widget());
  editor->messages.clear();
  if (click) {
    // the press a real click is made of, at the place the cursor stands: a window of
    // the offscreen platform is inactive and answers no mouse event at all unless the
    // event carries the press itself
    const auto place = view.cursorRect(view.textCursor()).center();
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, place);
  } else {
    // the key is a `Qt::Key`: `QTest::keyClick(widget, int)` selects the `char`
    // overload, which maps its argument through `QTest::asciiToKey`
    QTest::keyClick(frame.widget(), Qt::Key_Return);
  }
  pump();
  const auto on_screen =
      editor->getCurrentBuffer().getMainFrame().getCompleteFileName();
  // the study is put back as the file holds it: a press which found a link is
  // consumed by the mode and wrote nothing, and a press which found none is a
  // `Return` like any other, which the text edit answers with a newline of its own
  // in front of the cursor — a study left with one more line than it was written
  // with would move every line the presses after it stand on
  frame.setPlainText(this->study_text);
  pump();
  return on_screen;
}  // end of TestLicosMode::follow

void TestLicosMode::followTheLink(const int line,
                                  const QString &expected,
                                  const QString &message,
                                  const bool click) {
  const auto shape = lineText(line);
  const auto on_screen = follow(line, click);
  const auto wanted = expected.isEmpty()
                          ? QFileInfo(this->study).absoluteFilePath()
                          : QFileInfo(expected).absoluteFilePath();
  QVERIFY2(QFileInfo(on_screen).absoluteFilePath() == wanted,
           qPrintable(QString("the link of the shape `") + shape +
                      "` answered with the buffer of `" + on_screen +
                      "', while the file wanted is `" + wanted + "'"));
  if (expected.isEmpty()) {
    return;
  }
  QVERIFY2(message.isEmpty() || told(message),
           qPrintable(QString("the link of the shape `") + shape +
                      "` opened its file without saying `" + message +
                      "`; it said: " + editor->messages.join(" | ")));
  // a link which opened the file it names has answered the whole of what the hand
  // asked: nothing is left to report about a file which was found
  QVERIFY2(!told("not found"),
           qPrintable(QString("the link of the shape `") + shape + "` opened " +
                      wanted + " and reported a file it had not found anyway; it "
                              "said: " +
                      editor->messages.join(" | ")));
}  // end of TestLicosMode::followTheLink

bool TestLicosMode::told(const QString &what) const {
  for (const auto &m : editor->messages) {
    if (m.contains(what)) {
      return true;
    }
  }
  return false;
}  // end of TestLicosMode::told

void TestLicosMode::theKeyWhichFollowsALinkOpensTheFileBesideTheStudy() {
  // the shape which was never broken: a name of a file beside the study, followed
  // with `Enter`, which is what the buffer of a study is asked about
  followTheLink(plain_line, root.filePath("study/plain.txt"));
}  // end of theKeyWhichFollowsALinkOpensTheFileBesideTheStudy

void TestLicosMode::theClickWhichFollowsALinkOpensTheSameFile() {
  // the tooltip of the underline says `open in new buffer`, which is what the hand
  // which reads it does with the left button
  followTheLink(plain_line, root.filePath("study/plain.txt"), QString(), true);
}  // end of theClickWhichFollowsALinkOpensTheSameFile

void TestLicosMode::aNameBetweenTwoDifferentQuotesIsNoLink() {
  // a name opened by one quote and closed by the other is no name of this language:
  // the hand which reads such a line asked for nothing, and a pattern which read it
  // would read the text between the quotes of two different names as one
  followTheLink(mismatched_line, QString());
}  // end of aNameBetweenTwoDifferentQuotesIsNoLink

void TestLicosMode::aNameNamingASubDirectoryIsALink() {
  // a study names the files of its sub-directories with the directory in front of
  // the name, which is the file as it stands on the disk — and which was no name at
  // all for the pattern the jump was read with
  followTheLink(directory_line, root.filePath("study/inputs/inpath.txt"));
}  // end of aNameNamingASubDirectoryIsALink

void TestLicosMode::aNameBetweenDoubleQuotesIsALink() {
  // the underline is drawn under both quote styles, so both have to be followed: the
  // wizards of the mode write the single quotes and a hand writes the other
  followTheLink(double_quote_line, root.filePath("study/dquote.txt"));
}  // end of aNameBetweenDoubleQuotesIsALink

void TestLicosMode::aNameHoldingAPlusSignIsALink() {
  // the characters of a name are one answer for the two faces of a link, and the
  // list of them holds the `+` of a name such as `a+b.txt`
  followTheLink(plus_line, root.filePath("study/a+b.txt"));
}  // end of aNameHoldingAPlusSignIsALink

void TestLicosMode::aNameWhichIsNoAsciiIsALink() {
  // the characters of a name are read with the unicode properties, which is what
  // lets a name of an accented word be a link at all, underlined and followed
  followTheLink(accented_line,
                root.filePath(QStringLiteral("study/") +
                              QString::fromUtf8(accented_target)));
}  // end of aNameWhichIsNoAsciiIsALink

void TestLicosMode::theFilesOfTheUsersTreeAreFoundInTheirOwnSubDirectory() {
  // the directories `LICOS_USER_FILES` names are looked at as the directory of a
  // study is: as itself and through each of the three sub-directories a study lays
  // its files in
  followTheLink(user_input_line, root.filePath("user/inputs/user_input.txt"));
  followTheLink(user_script_line, root.filePath("user/scripts/table.dat"));
  followTheLink(user_data_line, root.filePath("user/data/user_data.txt"));
}  // end of theFilesOfTheUsersTreeAreFoundInTheirOwnSubDirectory

void TestLicosMode::theFileOfTheInstallationIsFound() {
  // `LICOSHOME` names an installation and the files it provides stand under
  // `share/licos`: they are a candidate, and the last of them
  followTheLink(shared_line,
                root.filePath("install/share/licos/inputs/shared.txt"));
}  // end of theFileOfTheInstallationIsFound

void TestLicosMode::
    aFileOfTheStudyIsNotAnsweredWithAComplaintAboutTheEnvironment() {
  // the name of a file of the study needs no installation to be found, so the
  // variable which names one is not asked about at all — and a report about a
  // variable which is not defined is not what a hand which follows such a name
  // should be left reading
  const auto licosHome = qgetenv("LICOSHOME");
  qunsetenv("LICOSHOME");
  followTheLink(plain_line, root.filePath("study/plain.txt"));
  QVERIFY2(!told("LICOSHOME"),
           qPrintable(QStringLiteral(
                          "a name of the study was answered with a complaint "
                          "about a variable: ") +
                      editor->messages.join(" | ")));
  qputenv("LICOSHOME", licosHome);
}  // end of aFileOfTheStudyIsNotAnsweredWithAComplaintAboutTheEnvironment

void TestLicosMode::aNameOfNoFileIsReportedAndOpensNothing() {
  // a name which is nowhere is the one answer the mode gives: the file is not found,
  // and the report names the name the hand stood on
  const auto on_screen = follow(absent_line);
  QVERIFY2(QFileInfo(on_screen).absoluteFilePath() ==
               QFileInfo(this->study).absoluteFilePath(),
           qPrintable(QStringLiteral("a name of no file left the study and put `") +
                      on_screen + QStringLiteral("` on the screen")));
  QVERIFY2(told("absent.txt"),
           qPrintable(QStringLiteral("a name of no file said nothing about the "
                                     "name it looked for: ") +
                      editor->messages.join(" | ")));
}  // end of aNameOfNoFileIsReportedAndOpensNothing

void TestLicosMode::aPostScriptReportOfTheStudyOpensInABuffer() {
  // a report of a study is a file of the study: the underline which names it
  // promises a new buffer, as it does for every other extension it draws
  followTheLink(report_line, root.filePath("study/report.ps"));
}  // end of aPostScriptReportOfTheStudyOpensInABuffer

void TestLicosMode::aSourceOfTheStudyOpensOnceAndSaysNothing() {
  // a source is looked for where the study stands, whatever directory the editor was
  // started in, and opening it is the end of the question: a report behind the
  // opened file says the search did not stop where the file was found
  followTheLink(mfront_line, root.filePath("study/behaviour.mfront"));
}  // end of aSourceOfTheStudyOpensOnceAndSaysNothing

void TestLicosMode::aSourceOfTheTreeOfMaterialsOfTheStudyOpens() {
  // the sources of the materials of a study stand in the `mfront` sub-directories of
  // the directory of the study, and a name of one of them is followed as the file
  // beside the study is
  followTheLink(nested_mfront_line,
                root.filePath("study/mfront/behaviours/nested.mfront"));
}  // end of aSourceOfTheTreeOfMaterialsOfTheStudyOpens

QTEST_MAIN(TestLicosMode)

#include "TestLicosMode.moc"
