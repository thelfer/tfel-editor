/*!
 * \file   tests/TestGitSupport.cxx
 * \brief  tests of the git support: the repository a file belongs to, and the
 *         `(git)` indicator the information line of a buffer shows beside the
 *         name of the file it visits.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <memory>
#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/GitSupport.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::GitSupport;

  // the shape of the application: a main window with a menu bar and the editor
  // as its central widget, which is the shape a label of the information line
  // gets a real geometry in (see the note on `MainWindow`)
  struct Shell : QMainWindow {
    tfel::gui::EditorWidget *const ed;
    explicit Shell(const QSize &s) : ed(new tfel::gui::EditorWidget()) {
      menuBar()->addMenu("File")->addAction("Open");
      setCentralWidget(ed);
      resize(s);
    }
    ~Shell() override {
      // no buffer is modified: no buffer is asked what to do with it when the
      // window, hence the editor and its buffers, goes away
      pump(2);
    }
    void pump(int n = 4) {
      for (int k = 0; k != n; ++k) {
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
      }
    }
  };  // end of struct Shell

  // write a file holding the given text; \return its name, empty when it failed
  QString write(const QString &path, const QString &text) {
    auto handle = QFile(path);
    if (!handle.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      return {};
    }
    handle.write(text.toUtf8());
    handle.close();
    return path;
  }  // end of write()

  // the git indicator of the information line of a buffer, by its object name
  QLabel *indicatorOf(tfel::gui::Buffer &b) {
    return b.getInformationLine().findChild<QLabel *>("git-indicator");
  }  // end of indicatorOf()

  // The labels of the information line, in the order the line was built: the
  // name of the buffer first, its git indicator second, the positions after
  // them. `findChildren` answers the children of an object in the order they
  // were made children of it, which is the order of the constructions.
  QList<QLabel *> labelsOf(tfel::gui::Buffer &b) {
    return b.getInformationLine().findChildren<QLabel *>();
  }  // end of labelsOf()

}  // end of anonymous namespace

//! tests of what the editor knows of git
struct TestGitSupport : QObject {
  Q_OBJECT

 private slots:

  void initTestCase();

  void theRepositoryOfAFileIsTheDirectoryHoldingItsGitEntry();

  void aFileOfNoRepositoryBelongsToNoRepository();

  void theRootOfARepositoryIsAWorkingDirectoryOfIt();

  void aWorkingTreeWhoseGitEntryIsAFileIsFound();

  void theGitEntryOfARepositoryIsNoWorkingFile();

  void aFileWhichWasNotWrittenYetIsAskedOfItsDirectory();

  void aBufferVisitingAFileOfARepositoryShowsTheIndicator();

  void aBufferVisitingAFileOfNoRepositoryShowsNothing();

  void aBufferVisitingNoFileShowsNothing();

  void theIndicatorStandsBesideTheNameAndNotInsideIt();

  void theIndicatorFollowsTheFileTheBufferComesToVisit();

 private:
  //! where the repositories of the cases are built
  QTemporaryDir directory;
  //! `<directory>/repo`, a repository whose `.git` entry is a directory
  QString repository;
  //! `<directory>/repo/sub/a.txt`, a file of that repository
  QString fileOfRepository;
  //! `<directory>/plain/b.txt`, a file of no repository
  QString fileOfNoRepository;
  //! the window the cases open their buffers in
  std::unique_ptr<Shell> shell;

  //! open the given file and let the editor lay its information line out
  void open(const QString &file) {
    shell->ed->openFile(file);
    shell->pump(4);
  }  // end of open()
};  // end of struct TestGitSupport

void TestGitSupport::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-git-support");
  QVERIFY(directory.isValid());
  // The precondition of every negative case: the temporary directory the
  // repositories of the cases are built in must not stand in a repository of
  // somebody else, or the answer `no repository` would be an accident of where
  // the test ran and not what the code answers.
  QVERIFY2(GitSupport::repositoryRootOf(directory.path()).isEmpty(),
           qPrintable(QString("the temporary directory %1 stands in the "
                              "repository %2: what the cases ask of a file of "
                              "no repository can not be decided here")
                        .arg(directory.path())
                        .arg(GitSupport::repositoryRootOf(directory.path()))));
  repository = directory.filePath("repo");
  QVERIFY(QDir().mkpath(repository + "/.git"));
  QVERIFY(QDir().mkpath(repository + "/sub"));
  fileOfRepository =
      write(repository + "/sub/a.txt", "a file of a repository\n");
  QVERIFY(!fileOfRepository.isEmpty());
  QVERIFY(QDir().mkpath(directory.filePath("plain")));
  fileOfNoRepository = write(directory.filePath("plain/b.txt"), "a free file\n");
  QVERIFY(!fileOfNoRepository.isEmpty());
  shell = std::make_unique<Shell>(QSize(1000, 700));
  shell->show();
  shell->pump(8);
}  // end of TestGitSupport::initTestCase

void TestGitSupport::theRepositoryOfAFileIsTheDirectoryHoldingItsGitEntry() {
  QCOMPARE(GitSupport::repositoryRootOf(fileOfRepository), repository);
  // the same answer from the file standing right under the entry which makes
  // the repository one, and from a name which says nothing of the directories
  // above it: the search is about the hierarchy, not about the depth of the file
  QCOMPARE(GitSupport::repositoryRootOf(repository + "/top.txt"), repository);
  const auto relative = QDir(repository).relativeFilePath(fileOfRepository);
  QVERIFY2(!relative.startsWith('/'), "the relative name is an absolute one");
  QVERIFY(QDir::setCurrent(repository));
  QCOMPARE(GitSupport::repositoryRootOf(relative), repository);
  QDir::setCurrent(directory.path());
}  // end of
   // TestGitSupport::theRepositoryOfAFileIsTheDirectoryHoldingItsGitEntry

void TestGitSupport::aFileOfNoRepositoryBelongsToNoRepository() {
  QVERIFY(GitSupport::repositoryRootOf(fileOfNoRepository).isEmpty());
  QVERIFY(GitSupport::repositoryRootOf(directory.filePath("plain")).isEmpty());
  // the empty name, which is what a buffer visiting no file answers, and a name
  // whose directories lead to no repository, answer nothing too
  QVERIFY(GitSupport::repositoryRootOf(QString()).isEmpty());
  QVERIFY(
      GitSupport::repositoryRootOf(directory.filePath("nowhere/and/nowhere.txt"))
          .isEmpty());
}  // end of TestGitSupport::aFileOfNoRepositoryBelongsToNoRepository

void TestGitSupport::theRootOfARepositoryIsAWorkingDirectoryOfIt() {
  // a buffer visiting a directory names what that directory is a working
  // directory of, which is the same question a file of it answers
  QCOMPARE(GitSupport::repositoryRootOf(repository), repository);
  QCOMPARE(GitSupport::repositoryRootOf(repository + "/"), repository);
  QCOMPARE(GitSupport::repositoryRootOf(repository + "/sub"), repository);
}  // end of TestGitSupport::theRootOfARepositoryIsAWorkingDirectoryOfIt

void TestGitSupport::aWorkingTreeWhoseGitEntryIsAFileIsFound() {
  // A linked working tree, and a submodule, hold a `.git` *file* naming the
  // repository elsewhere: the entry is a pointer and no directory, and what
  // makes the tree a working one is that the entry stands in it. `exists` asks
  // the two shapes at once.
  const auto tree = directory.filePath("tree");
  QVERIFY(QDir().mkpath(tree));
  QVERIFY(!write(tree + "/.git", "gitdir: /somewhere/.git/worktrees/x\n")
             .isEmpty());
  QVERIFY(
      !write(tree + "/c.txt", "a file of a linked working tree\n").isEmpty());
  QCOMPARE(GitSupport::repositoryRootOf(tree + "/c.txt"), tree);
  QCOMPARE(GitSupport::repositoryRootOf(tree), tree);
}  // end of TestGitSupport::aWorkingTreeWhoseGitEntryIsAFileIsFound

void TestGitSupport::theGitEntryOfARepositoryIsNoWorkingFile() {
  // The entry and what it holds are the bookkeeping of the program, not files
  // one commits: `HEAD`, `config`, and a name under `refs` are no working files
  // of the repository whose entry they are, and the buffer reading them names
  // no working tree
  QVERIFY(!write(repository + "/.git/config", "[core]\n").isEmpty());
  QVERIFY(GitSupport::repositoryRootOf(repository + "/.git/config").isEmpty());
  QVERIFY(GitSupport::repositoryRootOf(repository + "/.git").isEmpty());
  QVERIFY(QDir().mkpath(repository + "/.git/refs/heads"));
  QVERIFY(GitSupport::repositoryRootOf(repository + "/.git/refs/heads/master")
              .isEmpty());
  // the same answer for the two shapes the entry comes in, the directory of a
  // checkout and the file of a linked working tree
  const auto tree = directory.filePath("tree2");
  QVERIFY(QDir().mkpath(tree));
  QVERIFY(!write(tree + "/.git", "gitdir: /somewhere/.git/worktrees/y\n")
             .isEmpty());
  QVERIFY(GitSupport::repositoryRootOf(tree + "/.git").isEmpty());
}  // end of TestGitSupport::theGitEntryOfARepositoryIsNoWorkingFile

void TestGitSupport::aFileWhichWasNotWrittenYetIsAskedOfItsDirectory() {
  // A new file of a project is a working file of it before it is written: the
  // ascent asks the directories the name names, which are there, and not the
  // file, which is not. `QFileInfo::path` is a lexical operation, which is what
  // lets it go over a directory holding nothing yet.
  QCOMPARE(GitSupport::repositoryRootOf(repository + "/new/yet.txt"),
           repository);
  QVERIFY(
      GitSupport::repositoryRootOf(directory.filePath("plain/new/z.txt"))
          .isEmpty());
}  // end of
   // TestGitSupport::aFileWhichWasNotWrittenYetIsAskedOfItsDirectory

void TestGitSupport::aBufferVisitingAFileOfARepositoryShowsTheIndicator() {
  open(fileOfRepository);
  auto &buffer = shell->ed->getCurrentBuffer();
  auto *const indicator = indicatorOf(buffer);
  QVERIFY2(indicator != nullptr,
           "the information line of the buffer holds no git indicator");
  QVERIFY2(indicator->isVisible(), "the git indicator is not shown");
  QCOMPARE(indicator->text().trimmed(), QString("(git)"));
  // what the reader sees on the line names the file and says what it is
  auto shown = QString();
  for (const auto *const l : labelsOf(buffer)) {
    shown += l->text();
  }
  QVERIFY2(shown.contains("a.txt") && shown.contains("(git)"),
           qPrintable(shown));
}  // end of TestGitSupport::aBufferVisitingAFileOfARepositoryShowsTheIndicator

void TestGitSupport::aBufferVisitingAFileOfNoRepositoryShowsNothing() {
  open(fileOfNoRepository);
  auto &buffer = shell->ed->getCurrentBuffer();
  auto *const indicator = indicatorOf(buffer);
  QVERIFY(indicator != nullptr);
  // the line is drawn — its name label is on the screen — so what follows is
  // about the indicator standing hidden and not about a line nobody ever showed
  QVERIFY(labelsOf(buffer).at(0)->isVisible());
  QVERIFY2(!indicator->isVisible(),
           qPrintable(QString("the indicator says %1 of a file git knows "
                              "nothing about")
                        .arg(indicator->text())));
  // and no label of the line holds the text either: an indicator which does not
  // stand there shows nothing, wherever it stands
  for (const auto *const l : labelsOf(buffer)) {
    QVERIFY2(!l->text().contains("(git)"),
             qPrintable(QString("%1 names git in a buffer of no repository")
                          .arg(l->text())));
  }
}  // end of TestGitSupport::aBufferVisitingAFileOfNoRepositoryShowsNothing

void TestGitSupport::aBufferVisitingNoFileShowsNothing() {
  // The buffer of a new file names no path a repository could hold a working
  // file of — and it is the common buffer of an editor started with no argument,
  // so an indicator standing there would name git in every window ever opened
  shell->ed->createEmptyBuffer();
  shell->pump(4);
  auto &buffer = shell->ed->getCurrentBuffer();
  auto *const indicator = indicatorOf(buffer);
  QVERIFY(indicator != nullptr);
  QVERIFY2(buffer.getMainFrame().getFileName().isEmpty(),
           "the buffer of a new file visits a file");
  QVERIFY(labelsOf(buffer).at(0)->isVisible());
  QVERIFY2(!indicator->isVisible(),
           qPrintable(QString("the indicator says %1 of a buffer visiting no "
                              "file")
                        .arg(indicator->text())));
}  // end of TestGitSupport::aBufferVisitingNoFileShowsNothing

void TestGitSupport::theIndicatorStandsBesideTheNameAndNotInsideIt() {
  open(fileOfRepository);
  auto &buffer = shell->ed->getCurrentBuffer();
  auto *const indicator = indicatorOf(buffer);
  QVERIFY(indicator != nullptr);
  QVERIFY(indicator->isVisible());
  // The name of the buffer is what identifies it: in the tabs, in the list of
  // `Ctrl-X b`, in the `bufferNameChanged` signal. What the visited file is to
  // git is an information about that file, so it is drawn by a label of its own
  // and is nowhere in the name.
  QVERIFY2(!buffer.getBufferName().contains("git"),
           qPrintable(QString("the name of the buffer holds git: %1")
                        .arg(buffer.getBufferName())));
  const auto labels = labelsOf(buffer);
  QVERIFY(labels.size() > 2);
  auto *const name = labels.at(0);
  QVERIFY2(name != indicator, "the indicator is the label of the name");
  QVERIFY2(name->text().contains("a.txt") && (!name->text().contains("(git)")),
           qPrintable(QString("the label of the name says %1")
                        .arg(name->text())));
  // and the two labels stand on the one line, the indicator right behind the
  // name it qualifies, which is what makes the pair read as a single item
  QVERIFY2(name->isVisible(), "the label of the name is not drawn");
  QCOMPARE(indicator->y(), name->y());
  QVERIFY2((indicator->x() >= name->x() + name->width() - 1) &&
               (indicator->x() <= name->x() + name->width() + 8),
           qPrintable(QString("the indicator stands at %1, the name ends at %2")
                        .arg(indicator->x())
                        .arg(name->x() + name->width())));
}  // end of TestGitSupport::theIndicatorStandsBesideTheNameAndNotInsideIt

void TestGitSupport::theIndicatorFollowsTheFileTheBufferComesToVisit() {
  // A save under a new name is the moment the answer changes, and the only one:
  // the question is about the name of the visited file and about nothing else,
  // so neither typing in the buffer nor saving it under the name it already has
  // has anything to say about the indicator.
  open(fileOfNoRepository);
  auto &buffer = shell->ed->getCurrentBuffer();
  QVERIFY(!indicatorOf(buffer)->isVisible());
  buffer.getMainFrame().setFileName(fileOfRepository);
  shell->pump(4);
  QVERIFY2(indicatorOf(buffer)->isVisible(),
           "the indicator did not come to a buffer which came to visit a file "
           "of a repository");
  // and the way back: the file left behind is no repository's business anymore
  buffer.getMainFrame().setFileName(fileOfNoRepository);
  shell->pump(4);
  QVERIFY2(!indicatorOf(buffer)->isVisible(),
           "the indicator stayed on a buffer which left the repository behind");
}  // end of TestGitSupport::theIndicatorFollowsTheFileTheBufferComesToVisit

QTEST_MAIN(TestGitSupport)
#include "TestGitSupport.moc"
