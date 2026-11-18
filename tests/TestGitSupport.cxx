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
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QFileSystemModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTreeView>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FileExplorer.hxx"
#include "TFEL/GUI/GitStatusClient.hxx"
#include "TFEL/GUI/GitSupport.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::FileExplorer;
  using tfel::gui::GitStatusClient;
  using tfel::gui::GitSupport;

  // Run a program, wait for it, and answer what it wrote on its standard
  // output. The cases use it to build the repositories they ask the client
  // about, which is the only way to ask a question about what `git status` says
  // rather than about a fixture which stands for it.
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

  // A repository built by the program itself. The client answers what `git
  // status` says about a working tree, so the fixture of a case about the answer
  // is a working tree: `git init`, a commit of the files the case starts from,
  // and then whatever the case does to the tree and to the index after that.
  struct Repository {
    //! root of the working tree
    const QString root;

    Repository(const QString &directory, const QString &name)
        : root(directory + '/' + name) {
      QDir().mkpath(root);
      git({"init", "-q"});
      // the identity of a commit is what a machine of the tests has no opinion
      // about, and a global configuration of a person must not decide it either
      git({"config", "user.email", "tests@tfel.invalid"});
      git({"config", "user.name", "tests"});
      git({"config", "commit.gpgsign", "false"});
    }

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

    //! \brief put the given files in the index, without committing them
    void stage(const QStringList &files) const {
      auto arguments = QStringList{"add"};
      for (const auto &f : files) {
        arguments << f;
      }
      git(arguments);
    }  // end of stage()

    //! \brief write a file of this working tree and return its name
    QString write(const QString &name, const QString &text) const {
      auto handle = QFile(root + '/' + name);
      if (!handle.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {};
      }
      handle.write(text.toUtf8());
      handle.close();
      return root + '/' + name;
    }  // end of write()
  };  // end of struct Repository

  // the program which says what a file is to its repository, when this machine
  // has one
  bool hasGit() {
    return !run("git", {"--version"}, QDir::currentPath()).isEmpty();
  }  // end of hasGit

  // what the settings of the window are when no case has touched them. The two
  // addresses of the client of the working tree are read at every question, so
  // what a case leaves there is the answer of every window opened after it, and a
  // case which asserts what a switch does can not tell its own answer from the
  // one a previous case left behind
  void restoreGitSettings() {
    auto settings = QSettings();
    settings.remove(GitStatusClient::executableSettingAddress());
    settings.remove(GitStatusClient::launchByDefaultSettingAddress());
  }  // end of restoreGitSettings

  // an editor which keeps what it was told: the report of a program which was
  // not to be found is the whole of what a wrong answer of the configuration has
  // to say for itself, and the minibuffer of an offscreen window is nothing a
  // test reads back
  struct RecordingEditor : tfel::gui::EditorWidget {
    //! what the editor was asked to show, oldest first
    QStringList messages;
    void displayInformativeMessage(const QString &m) override {
      messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
    //! what the editor was told, oldest first, which a case may empty first
    QStringList &getMessages() { return this->messages; }
  };  // end of struct RecordingEditor

  // the shape of the application: a main window with a menu bar and the editor
  // as its central widget, which is the shape a label of the information line
  // gets a real geometry in (see the note on `MainWindow`)
  struct Shell : QMainWindow {
    RecordingEditor *const ed;
    explicit Shell(const QSize &s) : ed(new RecordingEditor()) {
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

  //! put back what the cases of the program of a working tree may have written
  void cleanup();

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

  // the state of the file, which is what `git status` says

  void aCleanWorkingFileIsAnsweredUnmodified();

  void aFileWhoseTextChangedIsAnsweredModified();

  void aFileNoCommitHoldsIsAnsweredUntracked();

  void aFileOfAnUntrackedDirectoryIsAnsweredUntracked();

  void aFileTheIndexHoldsDifferentlyIsAnsweredModified();

  void aFileMovedByTheProgramItselfIsAnsweredModified();

  void aFileWhoseNameIsNoAsciiIsAnswered();

  void aFileOfNoRepositoryIsAnsweredUnknown();

  void aRepositoryWhichRefusesToAnswerIsAnsweredAsNothingKnown();

  void theIndicatorOfABufferNamesWhatItsFileIs();

  void theExplorerColorsWhatTheStatusClientAnswers();

  void aProgramWhichIsNotFoundIsSaidOnceAndAnswersNothing();

  void turningTheQuestionsOffTakesTheStatesAway();

  void aRepositoryNobodyLooksAtIsNoLongerAnswered();

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

  /*!
   * \brief wait until the client holds an answer about a repository
   * \param[in] client : the client of the window the buffers are open in
   * \param[in] root : root of the repository an answer is waited for
   *
   * The answer comes with the finish of a process, which is an event of the loop
   * and not something `processEvents` alone expires: `QTest::qWait` runs the
   * notifiers a pipe is read through, which is what the frame of a process reads
   * through too (see the note in `TestSharedCompilationOutput`).
   */
  bool answered(GitStatusClient &client, const QString &root) {
    for (auto waited = 0; waited != 20000; waited += 50) {
      if (client.answeredRepositories().contains(root)) {
        return true;
      }
      QTest::qWait(50);
    }
    return false;
  }  // end of answered()

  //! \return the buffer of the window visiting the given file, null when none
  tfel::gui::Buffer *bufferVisiting(const QString &file) {
    for (auto *const b : shell->ed->getBuffers()) {
      if (b->getMainFrame().getFileName() == file) {
        return b;
      }
    }
    return nullptr;
  }  // end of bufferVisiting()

  //! \return whether the model of the panel has listed the entries of a
  //! directory, which is what a listing through a socket notifier is waited for
  static bool listed(QFileSystemModel &model, const QString &directory) {
    const auto root = model.index(directory);
    for (auto waited = 0; waited != 8000; waited += 50) {
      if (root.isValid() && (model.rowCount(root) > 0)) {
        return true;
      }
      QTest::qWait(50);
    }
    return false;
  }  // end of listed()

  /*!
   * \return the colour an entry of the panel is drawn in
   * \param[in] model : the model of the panel
   * \param[in] file : name of the entry asked about
   *
   * An entry the editor colours answers the colour of its state; an entry it
   * leaves to the desktop answers the foreground the desktop gives it, which is
   * what the model of the file system answers of its own accord. What a case
   * asserts is hence the two colours it asks for and the absence of both, never
   * a colour of a theme.
   */
  static QColor colorOf(QFileSystemModel &model, const QString &file) {
    const auto value = model.data(model.index(file), Qt::ForegroundRole);
    if (!value.canConvert<QBrush>()) {
      return QColor(Qt::black);
    }
    const auto color = qvariant_cast<QBrush>(value).color();
    return color.isValid() ? color : QColor(Qt::black);
  }  // end of colorOf()
};  // end of struct TestGitSupport

void TestGitSupport::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-git-support");
  // The cases which name a program which is not to be found, and which turn the
  // questions off, write the two addresses of the program of the working tree:
  // what they leave behind would decide what every later window asks, which is
  // what the case of the switch is about
  restoreGitSettings();
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

void TestGitSupport::cleanup() {
  restoreGitSettings();
  // A case which did not wait for the program it started leaves a run behind:
  // the answer of that run belongs to a repository the next case knows nothing
  // of, and the client of the window is the one object both cases ask.
  shell->pump(4);
}  // end of TestGitSupport::cleanup

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

// The state of the file, which is what `git status` says
//
// The cases of this half build their fixture with the program itself: what the
// client answers is what a working tree and its index say about a path, and a
// fixture which only stands for a `git status` answer would test the reading of
// the fixture and nothing of the client.

void TestGitSupport::aCleanWorkingFileIsAnsweredUnmodified() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-clean");
  const auto file = repository.write("kept.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("kept.txt")});
  open(file);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  QCOMPARE(client.statusOf(file), GitStatusClient::Unmodified);
}  // end of TestGitSupport::aCleanWorkingFileIsAnsweredUnmodified

void TestGitSupport::aFileWhoseTextChangedIsAnsweredModified() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-modified");
  repository.write("kept.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("kept.txt")});
  // the text of the file is not what the last commit has of it, and the index
  // says nothing of it: the ` M` of the answer of the program
  repository.write("kept.txt", "what the commit holds\nand what the hand added\n");
  const auto file = repository.root + "/kept.txt";
  open(file);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  QCOMPARE(client.statusOf(file), GitStatusClient::Modified);
}  // end of TestGitSupport::aFileWhoseTextChangedIsAnsweredModified

void TestGitSupport::aFileNoCommitHoldsIsAnsweredUntracked() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-untracked");
  repository.write("kept.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("kept.txt")});
  const auto file = repository.write("brand-new.txt", "nobody has this yet\n");
  open(file);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  QCOMPARE(client.statusOf(file), GitStatusClient::Untracked);
  // and the file the commit holds is not touched by its neighbour standing
  // beside it: the two states are answered of the same run and about different
  // paths, and a reader of the answer which took every path of the tree for one
  // of them would fail here
  QCOMPARE(client.statusOf(repository.root + "/kept.txt"),
           GitStatusClient::Unmodified);
}  // end of TestGitSupport::aFileNoCommitHoldsIsAnsweredUntracked

void TestGitSupport::aFileOfAnUntrackedDirectoryIsAnsweredUntracked() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-untracked-tree");
  repository.write("kept.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("kept.txt")});
  // `--untracked-files=normal`, which the client asks for, names the top of a
  // tree of new files and no file of it: a file of such a directory is new
  // because the directory it stands in is
  QVERIFY(QDir().mkpath(repository.root + "/new/inside"));
  const auto file = repository.write("new/inside/deep.txt", "two levels down\n");
  open(file);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  QCOMPARE(client.statusOf(file), GitStatusClient::Untracked);
  // And the directories themselves, which are the entries the panel draws: what
  // the program names is the top of the tree, `new/`, and a reader of the panel
  // who saw that entry plain and its contents blue would not be able to tell
  // what is new about them. An answer which names a directory of new files as
  // its top is what both of the two questions below are answered by.
  QCOMPARE(client.statusIn(repository.root, repository.root + "/new"),
           GitStatusClient::Untracked);
  QCOMPARE(client.statusIn(repository.root, repository.root + "/new/inside"),
           GitStatusClient::Untracked);
}  // end of
   // TestGitSupport::aFileOfAnUntrackedDirectoryIsAnsweredUntracked

void TestGitSupport::aFileTheIndexHoldsDifferentlyIsAnsweredModified() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-index");
  repository.write("kept.txt", "what the commit holds\n");
  repository.write("gone.txt", "to be taken away\n");
  repository.commit(
      {QStringLiteral("kept.txt"), QStringLiteral("gone.txt")});
  // the `A ` of the answer, which is a file the index holds and no commit holds
  repository.write("staged.txt", "in the index, not in a commit\n");
  repository.stage({QStringLiteral("staged.txt")});
  QVERIFY(QFile::remove(repository.root + "/gone.txt"));
  open(repository.root + "/kept.txt");
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  // an addition the index holds and a deletion the working tree holds are both
  // a difference with the commit the branch names, which is what this library
  // calls modified: what the indicator of a buffer drawn over a deletion — an
  // empty buffer — and over an addition has to say is that the file is not what
  // the repository holds of it
  QCOMPARE(client.statusOf(repository.root + "/staged.txt"),
           GitStatusClient::Modified);
  QCOMPARE(client.statusOf(repository.root + "/gone.txt"),
           GitStatusClient::Modified);
}  // end of
   // TestGitSupport::aFileTheIndexHoldsDifferentlyIsAnsweredModified

void TestGitSupport::aFileMovedByTheProgramItselfIsAnsweredModified() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-rename");
  repository.write("before.txt", "what the commit holds\n");
  repository.write("ab cd.txt", "what the commit holds\n");
  repository.write("cd.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("before.txt"), QStringLiteral("ab cd.txt"),
                     QStringLiteral("cd.txt")});
  // `R ` is what the program answers of a file it moved itself, and such a record
  // is two fields: the answer names where the entry stands now and, behind it,
  // where it came from. That second field carries no status in front of it, so a
  // reader which took it for a record would answer the state of a file the tree
  // does not hold — and miss the file which is there, the one the buffer visits.
  repository.git({"mv", "before.txt", "after.txt"});
  // And a name whose fourth character is a space, which is what the explicit skip
  // of the extra field is for: walking over that field as a record would read a
  // status out of the two first characters of the old path and a name out of the
  // rest, and answer `cd.txt` — a file the commit holds unchanged — as one which
  // moved. `ab cd.txt` is the old path which does it, and `cd.txt` is the file of
  // the tree the mangled name is the name of.
  repository.git({"mv", "ab cd.txt", "moved.txt"});
  const auto file = repository.root + "/after.txt";
  open(file);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  QCOMPARE(client.statusOf(file), GitStatusClient::Modified);
  QCOMPARE(client.statusOf(repository.root + "/moved.txt"),
           GitStatusClient::Modified);
  QCOMPARE(client.statusOf(repository.root + "/cd.txt"),
           GitStatusClient::Unmodified);
}  // end of
   // TestGitSupport::aFileMovedByTheProgramItselfIsAnsweredModified

void TestGitSupport::aFileWhoseNameIsNoAsciiIsAnswered() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "state-utf8");
  // A name which is no ASCII is what `-z` is for: without it the answer is a
  // quoted and octal-escaped C string under `core.quotepath=true`, the default of
  // the program, which names no file of the tree. The state of such a file is the
  // state of the file, and nothing about the name of it should be special — which
  // is what a reading of the answer that mangles the bytes of a path shows at once
  const auto file = repository.write(QString::fromUtf8("nom_\xc3\xa9.txt"),
                                     "ce que le commit tient\n");
  repository.commit({QString::fromUtf8("nom_\xc3\xa9.txt")});
  repository.write(QString::fromUtf8("nom_\xc3\xa9.txt"),
                   "ce que le commit tient\net ce que la main a ajout\xc3\xa9\n");
  open(file);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  QCOMPARE(client.statusOf(file), GitStatusClient::Modified);
}  // end of TestGitSupport::aFileWhoseNameIsNoAsciiIsAnswered

void TestGitSupport::aFileOfNoRepositoryIsAnsweredUnknown() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto &client = GitStatusClient::get(*shell->ed);
  // a file of no repository is answered without a question ever being asked: the
  // name holds no `.git` entry above it, and there is no working tree whose
  // answer could hold anything about it
  QCOMPARE(client.statusOf(fileOfNoRepository), GitStatusClient::Unknown);
  // and the empty name, which is what a buffer visiting no file answers, is
  // answered the same way
  QCOMPARE(client.statusOf(QString()), GitStatusClient::Unknown);
}  // end of TestGitSupport::aFileOfNoRepositoryIsAnsweredUnknown

void TestGitSupport::aRepositoryWhichRefusesToAnswerIsAnsweredAsNothingKnown() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  // A `.git` entry which holds no repository is what a half-finished `git init`,
  // or a hand which made a directory of that name, leaves behind. `git status`
  // answers nothing and exits 128 about such a tree, and the empty answer read
  // as an answer would draw every file of it as one the repository holds
  // unchanged: a claim about a program which said nothing at all.
  auto root = directory.filePath("refused");
  QVERIFY(QDir().mkpath(root + "/.git"));
  const auto file = write(root + "/kept.txt", "a file of a tree which refuses\n");
  QVERIFY(!file.isEmpty());
  open(file);
  QTest::qWait(1500);
  shell->pump(6);
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(!client.answeredRepositories().contains(root),
           qPrintable("the tree which refuses to answer was answered about " +
                      root));
  // the line of the buffer is drawn, and it says what it said before any answer:
  // an indicator which went away while a repository was asked would make the
  // line flicker every five seconds
  auto &buffer = shell->ed->getCurrentBuffer();
  QVERIFY(indicatorOf(buffer)->isVisible());
  QCOMPARE(indicatorOf(buffer)->text().trimmed(), QString("(git)"));
}  // end of
   // TestGitSupport::aRepositoryWhichRefusesToAnswerIsAnsweredAsNothingKnown

void TestGitSupport::theIndicatorOfABufferNamesWhatItsFileIs() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "indicator-states");
  repository.write("clean.txt", "what the commit holds\n");
  repository.write("modified.txt", "what the commit holds\n");
  repository.commit(
      {QStringLiteral("clean.txt"), QStringLiteral("modified.txt")});
  repository.write("modified.txt", "what the commit holds\nand more\n");
  repository.write("new.txt", "nobody has this\n");
  // The three texts of the same repository: what the reader of the line of a
  // buffer reads is the state of the file it visits, and the three buffers of
  // the same tree are answered by the same run of the program.
  struct Expected {
    QString file;
    QString text;
  };
  auto expected = QVector<Expected>();
  open(repository.root + "/clean.txt");
  expected << Expected{repository.root + "/clean.txt", QStringLiteral("(git)")};
  open(repository.root + "/new.txt");
  expected << Expected{repository.root + "/new.txt",
                       QStringLiteral("(git:untracked)")};
  open(repository.root + "/modified.txt");
  expected << Expected{repository.root + "/modified.txt",
                       QStringLiteral("(git:modified)")};
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  shell->pump(4);
  for (const auto &e : expected) {
    auto *const b = bufferVisiting(e.file);
    QVERIFY2(b != nullptr, qPrintable("no buffer visits " + e.file));
    QVERIFY2(!b->getBufferName().contains("git"),
             qPrintable(QString("the name of the buffer holds git: %1")
                          .arg(b->getBufferName())));
    const auto named = indicatorOf(*b)->text().trimmed();
    QVERIFY2(named == e.text,
             qPrintable(e.file + " is named " + named + ", expected " +
                        e.text));
  }
}  // end of TestGitSupport::theIndicatorOfABufferNamesWhatItsFileIs

void TestGitSupport::theExplorerColorsWhatTheStatusClientAnswers() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto repository = Repository(directory.path(), "colors");
  // three entries of one directory, in the three states the panel is about: what
  // the commit holds unchanged, what it holds differently, and what neither a
  // commit nor the index holds
  repository.write("same.txt", "what the commit holds\n");
  repository.write("changed.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("same.txt"),
                     QStringLiteral("changed.txt")});
  repository.write("changed.txt", "what the commit holds\nand more\n");
  repository.write("fresh.txt", "nobody has this\n");
  // a directory holding nothing but new files is answered as its own top, and the
  // entry the panel draws is that directory: an entry plain above contents blue
  // says nothing a reader can act on
  QVERIFY(QDir().mkpath(repository.root + "/drafts"));
  repository.write("drafts/notes.txt", "of a directory nobody has added\n");
  // a buffer of the repository is what makes it watched by the window; the panel
  // needs no buffer of its own to show what the answer says
  open(repository.root + "/same.txt");
  auto *const explorer = shell->ed->findChild<FileExplorer *>();
  QVERIFY2(explorer != nullptr, "the editor holds no file explorer panel");
  explorer->setRootDirectory(repository.root);
  shell->pump(4);
  auto *const tree = explorer->findChild<QTreeView *>();
  QVERIFY2(tree != nullptr, "the panel holds no tree of entries");
  // the model of the panel is the model of the file system, and the colours of
  // the entries are answered by the subclass of it the panel builds: the cast is
  // what says the rows of the tree are the entries of the directory and not the
  // rows of a proxy or of a model of somebody else
  auto *const model = qobject_cast<QFileSystemModel *>(tree->model());
  QVERIFY2(model != nullptr,
           "the tree of the panel shows no model of the file system");
  QVERIFY2(listed(*model, repository.root),
           "the panel never listed the directory it was rooted at");
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  // the colors themselves, which is what the reader of the panel reads: the blue
  // of a new file and the dark red of a file its repository holds differently
  QCOMPARE(colorOf(*model, repository.root + "/fresh.txt"), QColor(0, 0, 255));
  QCOMPARE(colorOf(*model, repository.root + "/changed.txt"),
           QColor(139, 0, 0));
  // and the entry the repository holds unchanged wears what the desktop gives it,
  // which is neither of the two: an answer which drew every file of a repository
  // it was asked about, or which took the state of the neighbour for the state of
  // the entry, is what this assertion is for
  const auto unchanged = colorOf(*model, repository.root + "/same.txt");
  QVERIFY2((unchanged != QColor(0, 0, 255)) && (unchanged != QColor(139, 0, 0)),
           qPrintable(QString("the entry the repository holds unchanged is drawn "
                              "in %1").arg(unchanged.name())));
  // And the directory of new files wears the same blue as the files it holds:
  // what the program names is the directory itself, and a reader who saw that
  // entry plain and its contents blue would not know what is new about them
  QCOMPARE(colorOf(*model, repository.root + "/drafts"), QColor(0, 0, 255));
}  // end of TestGitSupport::theExplorerColorsWhatTheStatusClientAnswers

void TestGitSupport::aProgramWhichIsNotFoundIsSaidOnceAndAnswersNothing() {
  auto repository = Repository(directory.path(), "missing");
  repository.write("kept.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("kept.txt")});
  QSettings().setValue(GitStatusClient::executableSettingAddress(),
                       QStringLiteral("/no/such/git-on-this-machine"));
  shell->ed->getMessages().clear();
  open(repository.root + "/kept.txt");
  GitStatusClient::get(*shell->ed).refresh(repository.root);
  shell->pump(6);
  auto told = QStringList();
  for (const auto &m : shell->ed->getMessages()) {
    if (m.contains("git")) {
      told << m;
    }
  }
  QCOMPARE(told.size(), 1);
  QVERIFY2(told.at(0).contains("could not be started"), qPrintable(told.at(0)));
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY(!client.answeredRepositories().contains(repository.root));
  // a second question about a repository says the same line no second time: the
  // mistake is in the settings file and every poll of every repository would
  // repeat it for as long as the window lives
  GitStatusClient::get(*shell->ed).refresh(repository.root);
  shell->pump(4);
  auto repeats = 0;
  for (const auto &m : shell->ed->getMessages()) {
    if (m.contains("could not be started")) {
      ++repeats;
    }
  }
  QCOMPARE(repeats, 1);
}  // end of
   // TestGitSupport::aProgramWhichIsNotFoundIsSaidOnceAndAnswersNothing

void TestGitSupport::turningTheQuestionsOffTakesTheStatesAway() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  QSettings().setValue(GitStatusClient::launchByDefaultSettingAddress(), false);
  auto repository = Repository(directory.path(), "switched-off");
  repository.write("kept.txt", "what the commit holds\n");
  repository.commit({QStringLiteral("kept.txt")});
  repository.write("kept.txt", "what the commit holds\nand more\n");
  open(repository.root + "/kept.txt");
  auto &client = GitStatusClient::get(*shell->ed);
  client.refresh(repository.root);
  QTest::qWait(800);
  shell->pump(4);
  QVERIFY2(!client.answeredRepositories().contains(repository.root),
           "a repository was answered while the switch said to ask nothing");
  auto &buffer = shell->ed->getCurrentBuffer();
  QVERIFY(indicatorOf(buffer)->isVisible());
  QCOMPARE(indicatorOf(buffer)->text().trimmed(), QString("(git)"));
  // and turning it back on reaches the next question without a restart of
  // anything: the answer of the entry is read where the program is started
  QSettings().setValue(GitStatusClient::launchByDefaultSettingAddress(), true);
  client.refresh(repository.root);
  QVERIFY2(answered(client, repository.root),
           qPrintable("nothing was answered about " + repository.root));
  shell->pump(4);
  QCOMPARE(indicatorOf(buffer)->text().trimmed(), QString("(git:modified)"));
  // And the answer which is already on the screen goes with the switch. The
  // states were drawn from a question the switch now forbids: a state which
  // stayed on the line after it would be a state with no one to answer for it,
  // and it would stay for as long as nothing else happened to redraw the line.
  QSettings().setValue(GitStatusClient::launchByDefaultSettingAddress(), false);
  client.refresh(repository.root);
  QTest::qWait(200);
  shell->pump(4);
  QVERIFY2(!client.answeredRepositories().contains(repository.root),
           "the answer of a repository stayed after the switch said that no "
           "question is to be asked about it");
  QVERIFY(indicatorOf(buffer)->isVisible());
  QCOMPARE(indicatorOf(buffer)->text().trimmed(), QString("(git)"));
}  // end of TestGitSupport::turningTheQuestionsOffTakesTheStatesAway

void TestGitSupport::aRepositoryNobodyLooksAtIsNoLongerAnswered() {
  if (!hasGit()) {
    QSKIP("git is not to be found on this machine");
  }
  auto kept = Repository(directory.path(), "kept-watched");
  kept.write("a.txt", "of the repository which stays\n");
  kept.commit({QStringLiteral("a.txt")});
  auto dropped = Repository(directory.path(), "dropped-watched");
  dropped.write("b.txt", "of the repository which goes\n");
  dropped.commit({QStringLiteral("b.txt")});
  open(dropped.root + "/b.txt");
  open(kept.root + "/a.txt");
  auto &client = GitStatusClient::get(*shell->ed);
  QVERIFY2(answered(client, kept.root), "the repository of the second buffer "
                                        "was never answered");
  QVERIFY2(answered(client, dropped.root),
           "the repository of the first buffer was never answered");
  // The answers are what a poll recomputes from what is on the screen: a
  // question asked twice costs nothing, and a question about a tree nobody has
  // open any more is what the answers of a long session of browsing would pile
  // up into. Walking the answers and dropping what is no longer watched is the
  // one place this happens, and it is a walk over the container the drop
  // rebuilds: with a case which has two answers and one to drop, doing it in the
  // walk skips an entry or stops at no end at all.
  auto *const victim = bufferVisiting(dropped.root + "/b.txt");
  QVERIFY(victim != nullptr);
  shell->ed->closeBuffer(*victim, false);
  shell->pump(6);
  client.refresh();
  QVERIFY2(!client.answeredRepositories().contains(dropped.root),
           "the answer of a repository nothing looks at stayed");
  QVERIFY2(client.answeredRepositories().contains(kept.root),
           "the answer of a repository which is watched went with it");
}  // end of TestGitSupport::aRepositoryNobodyLooksAtIsNoLongerAnswered

QTEST_MAIN(TestGitSupport)
#include "TestGitSupport.moc"
