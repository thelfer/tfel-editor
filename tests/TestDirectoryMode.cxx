/*!
 * \file   tests/TestDirectoryMode.cxx
 * \brief  tests of the directory mode: the listing a buffer of a directory
 *         shows, and the `Enter` key which opens the file or enters the
 *         directory that the line it is pressed on names.
 * \author Thomas Helfer
 * \date   19/11/2026
 */

#include <memory>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QColor>
#include <QtWidgets/QTextEdit>
#include <QtTest/QtTest>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/GitStatusClient.hxx"
#include "TFEL/GUI/GitSupport.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::GitStatusClient;
  using tfel::gui::PlainTextEdit;

  // the names of the fixture: three files of unlike lengths, so the columns of
  // the listing are not what a single name width gives them, and a
  // sub-directory holding a file of its own
  // the last name is not `gamma`: the math library of this libc declares
  // `double gamma(double)`, and an unqualified `gamma` in a range-for over an
  // initializer list is ambiguous
  const char *const alpha = "alpha.txt";
  const char *const beta = "beta.txt";
  const char *const long_name = "gamma_long.cc";

  // the names of the repository fixture: one file the commit holds and nothing
  // else says of, one the working tree holds differently from that commit, one no
  // commit and no index has ever seen, one new file which is *also* an executable
  // (the state is what wins, as it does in the file explorer panel, which knows no
  // executable face), and one executable the commit holds, which is what keeps the
  // face of an executable on the screen and gives the ink of a name of no state
  // something to be
  const char *const committed_file = "tracked.txt";
  const char *const changed_file = "modified.txt";
  const char *const new_file = "brand_new.txt";
  const char *const new_program = "runnable.sh";
  const char *const committed_program = "committed_runnable.sh";
  const char *const sub_directory = "sub";

  /*!\brief run a program, wait for it, and answer what it wrote on its standard
   * output
   *
   * The states of the files are the answer of a program, so the fixture of a case
   * about those states is a working tree that program built: a `.git` directory
   * which holds no repository is answered *nothing on stdout and exit 128*, and the
   * client refuses what such a run says, so every entry of such a fixture reads
   * Unknown — which is a case about a program which refuses to answer, not about
   * the states of files.
   */
  QByteArray run(const QString &program,
                 const QStringList &arguments,
                 const QString &workingDirectory) {
    auto process = QProcess();
    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(workingDirectory);
    process.start();
    if (!process.waitForStarted(5000)) {
      return QByteArrayLiteral("<not started>");
    }
    process.waitForFinished(20000);
    return process.readAllStandardOutput();
  }  // end of run()

  //! a working tree of a case's own, built by the program which reads it
  struct Repository {
    //! root of the working tree
    const QString root;

    explicit Repository(const QString &r) : root(r) {
      QDir().mkpath(root);
      git({"init", "-q"});
      // the identity of a commit is what a machine of the tests has no opinion
      // about, and a global configuration of a person must not decide it either
      git({"config", "user.email", "tests@tfel.invalid"});
      git({"config", "user.name", "tests"});
      git({"config", "commit.gpgsign", "false"});
    }  // end of Repository()

    //! \brief run git about this repository
    QByteArray git(const QStringList &arguments) const {
      return run("git", arguments, root);
    }  // end of git()

    //! \brief commit the given files, which become what the last commit holds
    void commit(const QStringList &files) const {
      auto arguments = QStringList{"add"};
      for (const auto &f : files) {
        arguments << f;
      }
      git(arguments);
      git({"commit", "-q", "-m", "what the cases start from"});
    }  // end of commit()

    //! \brief write a file of this working tree
    void write(const QString &name, const QString &text) const {
      auto handle = QFile(root + '/' + name);
      if (!handle.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
      }
      handle.write(text.toUtf8());
      handle.close();
    }  // end of write()

    //! \brief make a file of this working tree an executable
    void makeExecutable(const QString &name) const {
      // the three executable bits are what the mode asks, so they are the only
      // thing added: the read and write bits of the file are left as they are
      const auto info = QFileInfo(root + '/' + name);
      QFile::setPermissions(info.absoluteFilePath(),
                            info.permissions() | QFileDevice::ExeOwner |
                                QFileDevice::ExeGroup | QFileDevice::ExeOther);
    }  // end of makeExecutable()
  };  // end of struct Repository

  //! \return whether this machine has the program the states of a file are asked of
  bool hasGit() {
    return !run("git", {"--version"}, QDir::currentPath()).isEmpty();
  }  // end of hasGit

  // the two inks of the states, spelled out here and not read from
  // `GitStatusClient::colorOf`: a case which asks the code under test what its own
  // answer looks like asserts nothing about the answer wearing it
  QColor untracked_ink() {
    return QColor(0, 0, 255);
  }  // end of untracked_ink()

  QColor modified_ink() {
    return QColor(139, 0, 0);
  }  // end of modified_ink()

  //! the faces of the kind of entry, which the states are drawn over
  QColor directory_ink() {
    return QColor(Qt::darkBlue);
  }  // end of directory_ink()

  QColor executable_ink() {
    return QColor(Qt::darkGreen);
  }  // end of executable_ink()

  //! \return the ink of a colour the way a failure of a case names it
  QString inkName(const QColor &c) {
    return QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
  }  // end of inkName()

  /*!\return the ink drawn over the given name of the listing, empty when none is
   *
   * What a test can ask of a text edit is the keyed contributions —
   * `setExtraSelections` — and the faces of a `QSyntaxHighlighter` are not among
   * them: they are stored in the document's private extra selections, which
   * `QTextCursor::charFormat()` does not read either. So the ink of a *state* is
   * what these helpers answer, and the ink of a *kind of entry* is only to be seen
   * on the pixels, which is what `countExactInk` is for.
   *
   * The band of the current line sets a background and no foreground, so it is
   * what the test for a solid foreground leaves out.
   */
  QString inkOf(const PlainTextEdit &e, const QString &name) {
    for (const auto &s : e.extraSelections()) {
      if (s.format.foreground().style() != Qt::SolidPattern) {
        continue;
      }
      if (s.cursor.selectedText() != name) {
        continue;
      }
      return inkName(s.format.foreground().color());
    }
    return QString();
  }  // end of inkOf()

  //! \return every name of the listing an ink of a state is drawn over
  QStringList inkedNames(const PlainTextEdit &e) {
    auto names = QStringList();
    for (const auto &s : e.extraSelections()) {
      if (s.format.foreground().style() != Qt::SolidPattern) {
        continue;
      }
      names << s.cursor.selectedText();
    }
    return names;
  }  // end of inkedNames()

  //! \return whether the band of the current line stands among the contributions
  bool hasCurrentLineBand(const PlainTextEdit &e) {
    for (const auto &s : e.extraSelections()) {
      if (s.format.background().style() == Qt::SolidPattern) {
        return true;
      }
    }
    return false;
  }  // end of hasCurrentLineBand()

  /*!\return the number of pixels of the widget drawn in exactly that colour
   *
   * The only way to see what the highlighter of a mode decided. Exact matches are
   * few because antialiasing blends the glyphs; what makes the count evidence is
   * the control of a listing which holds nothing of that kind, as the pixel case of
   * `TestPythonMode` carries the rule.
   */
  int countExactInk(QWidget &w, const QColor &c) {
    auto count = 0;
    const auto image = w.grab().toImage();
    for (int y = 0; y != image.height(); ++y) {
      for (int x = 0; x != image.width(); ++x) {
        if (image.pixelColor(x, y) == c) {
          ++count;
        }
      }
    }
    return count;
  }  // end of countExactInk()

}  // end of anonymous namespace

//! tests of the listing of a directory and of the `Enter` key of the mode
/*!
 * One window serves the run: the listing of a directory is written by the mode
 * as the buffer is made, so nothing has to be waited for — the listing of a
 * `QFileSystemModel` arrives through a socket notifier, the text of this mode
 * is built in its constructor — and what a case asserts is a transition
 * (`Enter` on a name uncovers a buffer of that name) counted before and after
 * the key, never an absolute number of buffers.
 *
 * Each case has a directory of its own and shares nothing: `Enter` on the name
 * of a sub-directory *renames* the buffer which shows the listing, so a case
 * reusing the directory of another would find that buffer somewhere else in the
 * tree, and a reused directory is a buffer whose listing an earlier `Enter`
 * already changed.
 */
struct TestDirectoryMode : QObject {
  Q_OBJECT

 private slots:

  void initTestCase();

  //! \brief a buffer of a directory shows the entries of that directory
  void aDirectoryIsShownAsAListingOfItsEntries();

  //! \brief `Enter` on the name of a file opens that file
  void enterOpensTheFileTheLineNames();

  /*!
   * \brief `Enter` opens a file whose line is not the first of the listing,
   *        which is the shape of every file and every sub-directory
   */
  void enterOpensAFileBelowTheFirstLineOfTheListing();

  //! \brief `Enter` names the line, wherever on it the cursor stands
  void enterAnswersForTheLineAndNotForTheWordUnderTheCursor();

  //! \brief every entry of the listing is reachable by `Enter`
  void everyEntryOfTheListingIsReachable();

  //! \brief `Enter` on the name of a directory shows that directory
  void enterEntersTheDirectoryTheLineNames();

  //! \brief `Enter` on the `..` entry goes up one directory
  void enterOnTheParentEntryGoesUpOneDirectory();

  //! \brief `Enter` where the listing names nothing changes nothing
  void enterOnAnEmptyLineOpensNothing();

  /*!
   * \brief a listing stands on its first entry, so the first `Enter` of a hand
   *        which has not moved the cursor names the parent directory
   */
  void aFreshListingStandsOnItsFirstEntry();

  /*!
   * \brief a listing rewritten because the directory changed keeps the line the
   *        cursor stood on
   */
  void aRefreshKeepsTheLineTheCursorStandsOn();

  /*!
   * \brief the names of a listing of a repository are drawn with the two inks of
   *        the states of a file, and the entries the answer says nothing of are
   *        left to the face of their kind
   */
  void theNamesOfAListingWearTheStateTheirRepositoryHolds();

  /*!
   * \brief entering a sub-directory colours the entries of that directory, which
   *        are not those of the directory the listing showed before
   */
  void enteringASubDirectoryWearsTheStatesOfThatDirectory();

  //! \brief a directory of no repository draws no state at all
  void aListingOfNoRepositoryWearsNoState();

  /*!
   * \brief the ink of a directory is dark blue, which is what leaves the pure blue
   *        of a new file to the state of a file
   *
   * Pixels, not selections: the face of a kind of entry is drawn by the highlighter
   * of the mode, which a test can not ask anything about.
   */
  void aDirectoryIsInkedDarkBlueAndANewFileBlue();

  /*!
   * \brief a mode which is replaced takes the states it drew with it: the listing
   *        is gone and the text edit is not
   */
  void aReplacedModeTakesTheStatesOfTheNamesWithIt();

 private:
  //! \brief run the event loop, and the deferred deletions it holds
  static void pump(int n = 4);

  /*!
   * \brief build `<temporary directory>/<name>/project` and answer its path
   *
   * A helper which answers something can not use `QVERIFY`: the macro expands
   * to a bare `return`. An empty answer is a fixture which was not written, and
   * the caller asserts on it.
   */
  QString makeTree(const QString &name);

  /*!\brief build `<temporary directory>/<name>/project` as a working tree and
   * answer its path
   *
   * The tree every case about the states of a file shares: one file the last commit
   * holds, one the working tree holds differently from it, one no commit has seen,
   * one new file which is an executable besides, one executable the commit holds,
   * and a sub-directory with a file of its own which the commit holds.
   */
  QString makeRepositoryTree(const QString &name);

  //! \brief the git client of the window the buffers are open in
  GitStatusClient &client();

  /*!\return whether the ink of the given colour stands over the given name within
   * a bound of time
   *
   * The answer of a repository reaches the listing after the text was written: the
   * client starts no process of its own about a buffer on its own account, and the
   * first answer of a tree asked for by entering it comes when the process it
   * started says what the tree holds. A helper which answers something can not use
   * `QVERIFY`, which expands to a bare `return`.
   */
  bool waitForInk(const QString &name,
                  const QColor &ink,
                  int bound_in_ms = 15000);

  //! \brief open the given directory and answer the text edit which shows it
  tfel::gui::PlainTextEdit &openDirectory(const QString &path);

  /*!
   * \return the line of the listing whose first word is `name`, -1 when none
   *
   * The same reason as `makeTree` to answer a number instead of asserting.
   */
  static int lineOf(tfel::gui::PlainTextEdit &, const QString &name);

  /*!
   * \return the line of the listing the cursor of the given text edit stands on
   *
   * `QTextCursor::blockNumber()` is not used: it asks the layout, which is only
   * brought up to date when the text is painted, and a listing rewritten
   * programmatically has not been painted since.
   */
  static int cursorLine(const tfel::gui::PlainTextEdit &);

  //! \brief stand the cursor on the given line, `column` characters into it
  void goToLine(tfel::gui::PlainTextEdit &, int line, int column = 0);

  /*!
   * \brief press `Return` in the given text edit
   *
   * The key goes to `widget()`, the scroll area which carries the event filter
   * of `TextEditBase`: that filter asks the major mode before the text edit
   * sees the key, which is how the mode gets `Enter` at all — and it is asked
   * before the read-only state of the buffer is consulted, which a listing
   * always is. The key is a `Qt::Key` and not an `int`: `QTest::keyClick(widget,
   * int)` selects the `char` overload, which maps its argument through
   * `QTest::asciiToKey`.
   */
  static void pressEnter(tfel::gui::PlainTextEdit &);

  //! \return the lines of the listing, the empty one the last newline opens
  static QStringList listing(const tfel::gui::PlainTextEdit &);

  //! \return the first blank-separated word of a line of the listing
  static QString nameOf(const QString &line);

  //! \return the file the buffer on the screen visits
  QString visited() const;

  //! \return the number of buffers the window holds
  int bufferCount() const;

  QTemporaryDir root;
  std::unique_ptr<tfel::gui::EditorWidget> editor;
};  // end of TestDirectoryMode

void TestDirectoryMode::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-directory-mode");
  // the file constructor of a text edit restores the place a previous session
  // left in a file: a stale answer would move the cursor of a listing the cases
  // have just placed
  auto settings = QSettings();
  settings.remove("positions in files");
  settings.sync();
  QVERIFY(root.isValid());
}  // end of TestDirectoryMode::initTestCase

void TestDirectoryMode::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestDirectoryMode::pump

QString TestDirectoryMode::makeTree(const QString &name) {
  const auto project = root.filePath(name + "/project");
  if (!QDir().mkpath(project + "/sub")) {
    return QString();
  }
  for (const auto *const n : {alpha, beta, long_name}) {
    auto f = QFile(project + "/" + n);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      return QString();
    }
    f.write("hello\n");
    f.close();
  }
  auto inner = QFile(project + "/sub/inner.txt");
  if (!inner.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return QString();
  }
  inner.write("inner\n");
  inner.close();
  // the path is spelled as the mode spells it — an absolute path with no
  // `..` and no trailing separator — because that is what the name of a buffer
  // is compared to: `setFileName` stores `QFileInfo::absoluteFilePath`, which
  // cleans the path and resolves no symbolic link
  return QFileInfo(project).absoluteFilePath();
}  // end of TestDirectoryMode::makeTree

QString TestDirectoryMode::makeRepositoryTree(const QString &name) {
  const auto project = root.filePath(name + "/project");
  auto repository = Repository(project);
  repository.write(committed_file, "what the commit holds\n");
  repository.write(changed_file, "what the commit holds\n");
  repository.write(committed_program, "#!/bin/sh\necho committed\n");
  repository.makeExecutable(committed_program);
  QDir().mkpath(project + "/" + sub_directory);
  repository.write(QString(sub_directory) + "/inner.txt", "inner\n");
  repository.commit({committed_file, changed_file, committed_program,
                     QString(sub_directory) + "/inner.txt"});
  // what the tree holds *now*, which is what the answer of the client is about:
  // one file edited, one file no commit has ever seen, and one new file which is an
  // executable besides — the state is what the reader is told, as the file explorer
  // panel tells it, which has no executable face to lose
  repository.write(changed_file, "what a hand changed\n");
  repository.write(new_file, "new\n");
  repository.write(new_program, "#!/bin/sh\necho new\n");
  repository.makeExecutable(new_program);
  return QFileInfo(project).absoluteFilePath();
}  // end of TestDirectoryMode::makeRepositoryTree

GitStatusClient &TestDirectoryMode::client() {
  return GitStatusClient::get(*editor);
}  // end of client

bool TestDirectoryMode::waitForInk(const QString &name, const QColor &ink,
                                   int bound_in_ms) {
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  for (auto waited = 0; waited != bound_in_ms; waited += 50) {
    if (inkOf(frame, name) == inkName(ink)) {
      return true;
    }
    QTest::qWait(50);
    pump();
  }
  return inkOf(frame, name) == inkName(ink);
}  // end of waitForInk

tfel::gui::PlainTextEdit &
TestDirectoryMode::openDirectory(const QString &path) {
  if (editor == nullptr) {
    editor = std::make_unique<tfel::gui::EditorWidget>();
    editor->show();
    pump();
  }
  // a directory is opened as a file is: `openFile` on a path which is a
  // directory gives its buffer the directory mode, as `Ctrl-X Ctrl-F` and a
  // double click of the explorer panel do
  editor->openFile(path);
  pump();
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the cursor is left where the mode put it: a listing the mode has just
  // written stands on its first entry, which is what the case about the place a
  // fresh listing opens on is about
  return frame;
}  // end of TestDirectoryMode::openDirectory

int TestDirectoryMode::lineOf(tfel::gui::PlainTextEdit &e, const QString &n) {
  const auto lines = listing(e);
  for (int i = 0; i != lines.size(); ++i) {
    if (nameOf(lines.at(i)) == n) {
      return i;
    }
  }
  return -1;
}  // end of TestDirectoryMode::lineOf

void TestDirectoryMode::goToLine(tfel::gui::PlainTextEdit &e, int line,
                                 int column) {
  auto tc = e.textCursor();
  tc.movePosition(QTextCursor::Start);
  for (int k = 0; k != line; ++k) {
    tc.movePosition(QTextCursor::NextBlock);
  }
  for (int k = 0; k != column; ++k) {
    tc.movePosition(QTextCursor::NextCharacter);
  }
  e.setTextCursor(tc);
  pump();
}  // end of TestDirectoryMode::goToLine

void TestDirectoryMode::pressEnter(tfel::gui::PlainTextEdit &e) {
  QTest::keyClick(e.widget(), Qt::Key_Return);
  pump();
}  // end of TestDirectoryMode::pressEnter

int TestDirectoryMode::cursorLine(const tfel::gui::PlainTextEdit &e) {
  return e.document()->findBlock(e.textCursor().position()).blockNumber();
}  // end of cursorLine

QStringList TestDirectoryMode::listing(const tfel::gui::PlainTextEdit &e) {
  // the end of a block of these documents is spelled with a bare newline by
  // some readings and with U+2029 by others, so both are cut on, as elsewhere
  // in the suite
  return e.document()
      ->toPlainText()
      .replace(QChar(0x2029), QChar('\n'))
      .split('\n');
}  // end of TestDirectoryMode::listing

QString TestDirectoryMode::nameOf(const QString &line) {
  const auto words =
      line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  return words.isEmpty() ? QString() : words.first();
}  // end of TestDirectoryMode::nameOf

QString TestDirectoryMode::visited() const {
  return editor->getCurrentBuffer().getMainFrame().getCompleteFileName();
}  // end of TestDirectoryMode::visited

int TestDirectoryMode::bufferCount() const {
  return static_cast<int>(editor->getBuffers().size());
}  // end of TestDirectoryMode::bufferCount

void TestDirectoryMode::aDirectoryIsShownAsAListingOfItsEntries() {
  const auto project = makeTree("listing");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  QVERIFY(e.hasMajorMode());
  QCOMPARE(e.getMajorMode().getName(), QString("directory-mode"));
  // the listing is read-only: what stands there is what the directory holds
  QVERIFY(e.isReadOnly());

  const auto lines = listing(e);
  QVERIFY2(lines.size() > 1, "the listing holds nothing");
  // the parent directory is the first entry, which is what puts every real name
  // below the first line
  QCOMPARE(nameOf(lines.at(0)), QString(".."));
  for (const auto *const n : {alpha, beta, long_name, "sub"}) {
    const auto line = lineOf(e, n);
    QVERIFY2(line > 0,
             qPrintable(
                 QString("'%1' is not an entry below the first line").arg(n)));
  }
  // a name of another directory is not an entry of this one: what the cases
  // about `Enter` ask of the listing is a file of the directory shown, not of
  // its parent or of a child of it
  QVERIFY2(lineOf(e, "inner.txt") == -1,
           "the listing shows a file of a sub-directory");
  QVERIFY2(lineOf(e, "project") == -1, "the listing shows an entry of the "
                                       "parent directory");
}  // end of TestDirectoryMode::aDirectoryIsShownAsAListingOfItsEntries

void TestDirectoryMode::enterOpensTheFileTheLineNames() {
  const auto project = makeTree("open");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  const auto line = lineOf(e, alpha);
  QVERIFY2(line >= 0, "the fixture is not an entry of the listing");
  goToLine(e, line);

  const auto before = bufferCount();
  pressEnter(e);

  QCOMPARE(bufferCount(), before + 1);
  QVERIFY2(visited() == project + "/" + alpha,
           qPrintable(QString("Enter opened '%1' instead of the file the line "
                              "it was pressed on names")
                          .arg(visited())));
}  // end of TestDirectoryMode::enterOpensTheFileTheLineNames

void TestDirectoryMode::enterOpensAFileBelowTheFirstLineOfTheListing() {
  // the shape of the defect: the selection of the line under the cursor of a
  // block which is not the first of the document carries the U+2029 PARAGRAPH
  // SEPARATOR which precedes that block too, and that character is no
  // whitespace for the `\s+` of `QRegularExpression` — the ASCII class of PCRE,
  // where the JavaScript `\s` of the `QRegExp` this split with until the Qt 6
  // port knew it. The name asked of the file system was therefore
  // `<U+2029>name`, which no directory holds, and since a listing begins with
  // its parent entry, no file and no sub-directory could be reached with this
  // key at all.
  const auto project = makeTree("below");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  // the assertion on the line is not a shape the fixture happens to have: the
  // first line of a listing names the parent directory, so every file answers a
  // line strictly below it, which is why nothing used to open
  const auto line = lineOf(e, long_name);
  QVERIFY2(line > 0, "the last entry of the listing stands on the first line");
  goToLine(e, line);

  const auto before = bufferCount();
  pressEnter(e);

  QCOMPARE(bufferCount(), before + 1);
  QVERIFY2(visited() == project + "/" + long_name,
           qPrintable(QString("Enter on line %1 opened '%2'").arg(line)
                          .arg(visited())));
}  // end of TestDirectoryMode::enterOpensAFileBelowTheFirstLineOfTheListing

void TestDirectoryMode::enterAnswersForTheLineAndNotForTheWordUnderTheCursor() {
  // a line of the listing is `name` and padding, then the owner, then the date:
  // a hand at the middle of the line, or near its end, means the name of that
  // line, as much as a hand at its start does
  const auto project = makeTree("column");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  const auto line = lineOf(e, beta);
  QVERIFY2(line >= 0, "the fixture is not an entry of the listing");
  const auto length = listing(e).at(line).size();
  QVERIFY2(length > 10, "the line of the entry is too short to hold a date");
  goToLine(e, line, length - 3);

  const auto before = bufferCount();
  pressEnter(e);

  QCOMPARE(bufferCount(), before + 1);
  QVERIFY2(visited() == project + "/" + beta,
           qPrintable(QString("Enter near the end of the line opened '%1'")
                          .arg(visited())));
}  // end of TestDirectoryMode::enterAnswersForTheLineAndNotForTheWordUnderTh
    // eCursor

void TestDirectoryMode::everyEntryOfTheListingIsReachable() {
  // the defect was a function of the line the cursor stands on, so the case
  // which names it names one line and this one is what says no line is an
  // exception: each name is reached from a listing opened for it
  const auto project = makeTree("all");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  for (const auto *const n : {alpha, beta, long_name, "sub"}) {
    auto &e = openDirectory(project);
    const auto line = lineOf(e, n);
    QVERIFY2(line > 0,
             qPrintable(
                 QString("'%1' is not an entry below the first line").arg(n)));
    goToLine(e, line);
    pressEnter(e);
    QVERIFY2(
        visited() == project + "/" + n,
        qPrintable(QString("Enter on '%1' reached '%2'").arg(n).arg(visited())));
  }
}  // end of TestDirectoryMode::everyEntryOfTheListingIsReachable

void TestDirectoryMode::enterEntersTheDirectoryTheLineNames() {
  const auto project = makeTree("enter");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  const auto line = lineOf(e, "sub");
  QVERIFY2(line > 0, "the sub-directory is not an entry of the listing");
  goToLine(e, line);

  const auto before = bufferCount();
  pressEnter(e);

  // a directory makes no new buffer: the buffer which showed the listing shows
  // the entered directory, and the name it visits follows
  QCOMPARE(bufferCount(), before);
  QVERIFY2(visited() == project + "/sub",
           qPrintable(QString("Enter on a directory reached '%1'")
                          .arg(visited())));
  QVERIFY(e.hasMajorMode());
  QCOMPARE(e.getMajorMode().getName(), QString("directory-mode"));
  // the listing is the one of the entered directory: the file it holds is an
  // entry now, and the parent entry stands where it always stands
  QVERIFY2(lineOf(e, "inner.txt") > 0,
           "the listing did not change to the entered directory");
  QCOMPARE(nameOf(listing(e).at(0)), QString(".."));
}  // end of TestDirectoryMode::enterEntersTheDirectoryTheLineNames

void TestDirectoryMode::enterOnTheParentEntryGoesUpOneDirectory() {
  // the first line of every listing names the parent directory: the one entry
  // the defect left working, which is what makes this case the control saying
  // the fix of the others did not move it
  const auto project = makeTree("up");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  const auto lines = listing(e);
  QCOMPARE(nameOf(lines.at(0)), QString(".."));
  goToLine(e, 0);

  const auto before = bufferCount();
  pressEnter(e);

  QCOMPARE(bufferCount(), before);
  // `QFileInfo::absoluteFilePath`, which is what the name of a buffer is, takes
  // the `..` of the path out lexically, so the parent is named in the same
  // spelling as the directory which was left
  QVERIFY2(visited() == QFileInfo(project).absolutePath(),
           qPrintable(QString("Enter on the parent entry reached '%1'")
                          .arg(visited())));
  // the directory which was left is an entry of the listing now shown
  QVERIFY2(lineOf(e, "project") > 0,
           "the listing did not change to the parent directory");
}  // end of TestDirectoryMode::enterOnTheParentEntryGoesUpOneDirectory

void TestDirectoryMode::enterOnAnEmptyLineOpensNothing() {
  // the newline which ends the listing opens an empty last block: `Enter` there
  // names nothing, and must neither open a buffer nor write in the listing
  const auto project = makeTree("empty");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  const auto lines = listing(e);
  const auto last = lines.size() - 1;
  QVERIFY2(nameOf(lines.at(last)).isEmpty(),
           "the last line of the listing is not empty");
  goToLine(e, last);

  const auto before = bufferCount();
  pressEnter(e);

  QCOMPARE(bufferCount(), before);
  QVERIFY2(visited() == project, "Enter on an empty line moved somewhere");
  QVERIFY2(listing(e) == lines, "Enter on an empty line wrote in the listing");
}  // end of TestDirectoryMode::enterOnAnEmptyLineOpensNothing

void TestDirectoryMode::aFreshListingStandsOnItsFirstEntry() {
  // the second shape of "pressing Enter does nothing": the listing is written by
  // inserting at the place the cursor of the text edit holds, which left that
  // cursor on the empty line the last newline opens, so a window opened on a
  // directory and pressed `Enter` without moving named nothing at all
  const auto project = makeTree("fresh");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  QVERIFY2(cursorLine(e) == 0,
           qPrintable(QString("a fresh listing opens on line %1").arg(
               cursorLine(e))));
  QCOMPARE(nameOf(listing(e).at(cursorLine(e))), QString(".."));

  // and `Enter` says what that line names: the parent directory
  const auto before = bufferCount();
  pressEnter(e);
  QCOMPARE(bufferCount(), before);
  QVERIFY2(visited() == QFileInfo(project).absolutePath(),
           qPrintable(QString("Enter on a fresh listing reached '%1'")
                          .arg(visited())));
}  // end of aFreshListingStandsOnItsFirstEntry

void TestDirectoryMode::aRefreshKeepsTheLineTheCursorStandsOn() {
  // the mode watches the directory it shows and rewrites the listing when it
  // changes; a hand reading the fifth line of a long directory has to still be
  // reading it after the file system noticed something new
  const auto project = makeTree("refresh");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  auto &e = openDirectory(project);

  const auto line = lineOf(e, beta);
  QVERIFY2(line > 0, "the fixture is not an entry of the listing");
  goToLine(e, line);

  // a new entry which sorts after every name of the fixture, so the line the
  // cursor stands on is the same thing to ask for before and after the refresh
  auto added = QFile(project + "/zz-added.txt");
  QVERIFY(added.open(QIODevice::WriteOnly | QIODevice::Truncate));
  added.write("new\n");
  added.close();

  // the answer of the watcher arrives through the event loop, and a listing of a
  // directory is no signal a plain `processEvents` waits for
  auto refreshed = false;
  for (int k = 0; (k != 50) && (!refreshed); ++k) {
    QTest::qWait(100);
    pump();
    refreshed = lineOf(e, "zz-added.txt") > 0;
  }
  QVERIFY2(refreshed, "the listing did not follow the directory");

  const auto lines = listing(e);
  QVERIFY2(nameOf(lines.at(cursorLine(e))) == beta,
           qPrintable(QString("the refresh moved the cursor to the line '%1'")
                          .arg(nameOf(lines.at(cursorLine(e))))));
}  // end of aRefreshKeepsTheLineTheCursorStandsOn

void TestDirectoryMode::theNamesOfAListingWearTheStateTheirRepositoryHolds() {
  if (!hasGit()) {
    QSKIP("the states of a file are asked of git, which is not installed");
  }
  const auto project = makeRepositoryTree("states");
  auto &e = openDirectory(project);
  QVERIFY(e.hasMajorMode());
  QCOMPARE(e.getMajorMode().getName(), QString("directory-mode"));

  // the listing is written before anything is known about the files: the answer
  // arrives with the client, and that is what draws the inks
  QVERIFY2(waitForInk(new_file, untracked_ink()),
           qPrintable(QString("'%1' wears '%2' instead of the blue of a file no "
                              "commit holds")
                          .arg(new_file)
                          .arg(inkOf(e, new_file))));
  QVERIFY2(waitForInk(changed_file, modified_ink()),
           qPrintable(QString("'%1' wears '%2' instead of the dark red of a file "
                              "the working tree holds differently")
                          .arg(changed_file)
                          .arg(inkOf(e, changed_file))));
  // a new file which is an executable besides is drawn by its state: what a
  // repository says of a file is what a reader looks for in a listing of a project,
  // and the file explorer panel, which colours the same file, has no executable
  // face for it to lose
  QVERIFY2(inkOf(e, new_program) == inkName(untracked_ink()),
           qPrintable(QString("'%1' wears '%2' instead of the state of a file")
                          .arg(new_program)
                          .arg(inkOf(e, new_program))));

  // A name the answer holds nothing about keeps the face of its kind, which the
  // contributions do not touch. The wait above says the answer reached the screen —
  // one run answers the whole of a tree and one signal draws all of its entries —
  // so the absence of an ink below is the answer saying nothing, not a client which
  // has not answered yet.
  QTest::qWait(300);
  const auto inked = inkedNames(e);
  QVERIFY2(!inked.contains(committed_file),
           "the name of a file the commit holds is drawn as a detail of it");
  QVERIFY2(!inked.contains(committed_program),
           "the name of an executable the commit holds is drawn as a detail of it");
  QVERIFY2(!inked.contains(sub_directory),
           "the name of a sub-directory of nothing changed is drawn as a detail");
  QVERIFY2(!inked.contains(".."),
           "the parent entry, which the listing does not show and often stands out "
           "of the repository, is drawn as a detail of a directory");
  QVERIFY2(!inked.contains(alpha),
           "a name of no repository is drawn as a detail of a file");

  // the inks are a contribution of their own, not the whole list of the text edit:
  // the band of the current line is another source, and a mode which hands over the
  // whole list takes it away on every cursor movement. The band is drawn on a cursor
  // movement, so one is asked for before the band is looked for
  goToLine(e, lineOf(e, new_file));
  QVERIFY2(hasCurrentLineBand(e),
           "drawing the states of the names took the band of the current line "
           "away");
  QVERIFY2(inkOf(e, new_file) == inkName(untracked_ink()),
           "moving the cursor took the state of a name away");
}  // end of theNamesOfAListingWearTheStateTheirRepositoryHolds

void TestDirectoryMode::enteringASubDirectoryWearsTheStatesOfThatDirectory() {
  if (!hasGit()) {
    QSKIP("the states of a file are asked of git, which is not installed");
  }
  const auto project = makeRepositoryTree("subdir");
  auto &e = openDirectory(project);

  const auto line = lineOf(e, sub_directory);
  QVERIFY2(line > 0, "the sub-directory is not an entry of the listing");
  goToLine(e, line);
  pressEnter(e);

  // the buffer now shows the sub-directory, and its entries are not the ones the
  // listing showed before: the states drawn are those of that directory, in the
  // same repository
  QVERIFY2(visited() == project + "/" + sub_directory,
           qPrintable(QString("Enter reached '%1'").arg(visited())));
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(lineOf(frame, "inner.txt") > 0,
           "the listing does not show the entries of the directory entered");

  // a new file of that directory, noticed only after the directory was entered
  {
    auto added = QFile(project + "/" + sub_directory + "/fresh.txt");
    QVERIFY(added.open(QIODevice::WriteOnly | QIODevice::Truncate));
    added.write("fresh\n");
    added.close();
  }
  QVERIFY2(waitForInk("fresh.txt", untracked_ink()),
           qPrintable(QString("a new file of a directory entered after the "
                              "listing was written wears '%1' instead of the blue "
                              "of a new file")
                          .arg(inkOf(frame, "fresh.txt"))));
  // and the entry which names the directory of the whole listing is not among the
  // names of this one: the colours follow the text they were drawn on
  QVERIFY2(!inkedNames(frame).contains(new_file),
           "the names of the directory left behind are still drawn");
}  // end of enteringASubDirectoryWearsTheStatesOfThatDirectory

void TestDirectoryMode::aListingOfNoRepositoryWearsNoState() {
  const auto project = makeTree("no-repository");
  QVERIFY2(!project.isEmpty(), "the fixture could not be written");
  // the temporary directory of the run must stand in no repository of somebody
  // else, or the case asserts on a listing which is of a project after all
  QVERIFY2(tfel::gui::GitSupport::repositoryRootOf(project).isEmpty(),
           "the directory of the fixtures stands in a repository");
  auto &e = openDirectory(project);
  QVERIFY2(lineOf(e, alpha) > 0, "the fixture is not shown as a listing");
  // a listing of a directory of no repository is a listing the client is never
  // asked about, so the wait of the other cases has nothing to wait for here: what
  // the case asks is what the contributions of such a text edit hold, after the
  // moment an answer of a repository would have reached it
  QTest::qWait(500);
  QVERIFY2(inkedNames(e).isEmpty(),
           qPrintable(QString("a name of no repository is drawn with a state: %1")
                          .arg(inkedNames(e).join(", "))));
}  // end of aListingOfNoRepositoryWearsNoState

void TestDirectoryMode::aDirectoryIsInkedDarkBlueAndANewFileBlue() {
  if (!hasGit()) {
    QSKIP("the states of a file are asked of git, which is not installed");
  }
  const auto project = makeRepositoryTree("inks");
  auto &e = openDirectory(project);
  QVERIFY2(waitForInk(new_file, untracked_ink()),
           "the states of the names did not reach the listing");

  // the four inks of a listing of a repository, counted on the pixels: the face of
  // a directory, the face of an executable the commit holds, and the two inks of the
  // states. Exact matches are few because antialiasing blends the glyphs; what makes
  // the counts evidence is the control below, of a listing which holds none of the
  // two inks of the states at all.
  auto &widget = *e.widget();
  const auto directories = countExactInk(widget, directory_ink());
  const auto programs = countExactInk(widget, executable_ink());
  const auto untracked = countExactInk(widget, untracked_ink());
  const auto modified = countExactInk(widget, modified_ink());
  QVERIFY2(directories > 0,
           "no name is drawn in dark blue: the directory face and the blue of a new "
           "file are not told apart");
  QVERIFY2(programs > 0, "no name is drawn in dark green: the face of an "
                        "executable is missing");
  QVERIFY2(untracked > 0, "no name is drawn in the blue of a new file");
  QVERIFY2(modified > 0, "no name is drawn in the dark red of a modified file");

  // the control: a listing of a directory of no repository, with one executable of
  // its own, draws the two faces and neither of the two inks
  const auto plain = makeTree("inks-control");
  QVERIFY2(!plain.isEmpty(), "the control could not be written");
  auto program = QFile(plain + "/" + new_program);
  QVERIFY(program.open(QIODevice::WriteOnly | QIODevice::Truncate));
  program.write("#!/bin/sh\n");
  program.close();
  program.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                         QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                         QFileDevice::ExeGroup | QFileDevice::ReadOther |
                         QFileDevice::ExeOther);
  auto &control = openDirectory(plain);
  QTest::qWait(300);
  auto &control_widget = *control.widget();
  QVERIFY2(countExactInk(control_widget, untracked_ink()) == 0,
           "the blue of a new file stands over a listing of no repository");
  QVERIFY2(countExactInk(control_widget, modified_ink()) == 0,
           "the dark red of a modified file stands over a listing of no repository");
  QVERIFY2(countExactInk(control_widget, directory_ink()) > 0,
           "the control holds no directory to ink, and says nothing about the rest");
}  // end of aDirectoryIsInkedDarkBlueAndANewFileBlue

void TestDirectoryMode::aReplacedModeTakesTheStatesOfTheNamesWithIt() {
  if (!hasGit()) {
    QSKIP("the states of a file are asked of git, which is not installed");
  }
  const auto project = makeRepositoryTree("replaced");
  auto &e = openDirectory(project);
  QVERIFY2(waitForInk(new_file, untracked_ink()),
           "the states of the names did not reach the listing");
  QVERIFY2(!inkedNames(e).isEmpty(), "nothing was drawn, so nothing can be left");

  // the mode of a buffer is what draws the states, and a buffer can be given another
  // one: the listing is what the text edit holds, and its names are of no repository
  // any more once the mode which asked about them is gone
  e.setMajorMode("compilation-output");
  pump();

  QVERIFY2(inkedNames(e).isEmpty(),
           qPrintable(QString("a mode which was replaced leaves its states on the "
                              "text edit: %1")
                          .arg(inkedNames(e).join(", "))));
}  // end of aReplacedModeTakesTheStatesOfTheNamesWithIt

QTEST_MAIN(TestDirectoryMode)

#include "TestDirectoryMode.moc"
