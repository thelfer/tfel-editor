/*!
 * \file  GitStatusClient.cxx
 * \brief the client which asks `git status` what it thinks of the files of the
 *        working trees the editor has open.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FileExplorer.hxx"
#include "TFEL/GUI/GitStatusClient.hxx"
#include "TFEL/GUI/GitSupport.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/SettingsKey.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"

namespace tfel::gui {

  namespace {

    //! the object name the client of an editor is found by
    const char *const client_name = "git-status";

    /*!
     * \brief how long a watched repository waits between two questions
     *
     * Five seconds: the state of a file is something a person changes by
     * working, and working takes more than five seconds between a save and a
     * look at the mode line. A shorter poll buys a state nobody has had time to
     * change and costs a process.
     */
    constexpr int poll_interval_in_ms = 5000;

    /*!
     * \return the program to run, empty when there is none
     *
     * The address is read on every question so that an answer given in the
     * configuration dialog reaches the next poll of a repository without a
     * restart. A name which is no program is answered as nothing: the editor
     * says once that git was not found and asks nothing afterwards.
     */
    QString git_program() {
      const auto name =
          QSettings().value(GitStatusClient::executableSettingAddress())
              .toString();
      const auto program =
          name.isEmpty() ? QStringLiteral("git") : name;
      return QStandardPaths::findExecutable(program);
    }  // end of git_program()

    /*!
     * \return whether the editor runs the program on its own account
     *
     * The address is read on every question, so that an answer given in the
     * configuration dialog reaches the next poll of a repository without a
     * restart: the states of the files of a tree are the kind of thing a person
     * turns off the first time they see a subprocess of their own tool being run
     * behind their back.
     */
    bool launchedByDefault() {
      // The default is held here, as `git_program` holds the name it falls back
      // to: what the configuration entry declares as its default is the same
      // value, and the two are kept together by the entry being declared next to
      // this function
      return QSettings()
        .value(GitStatusClient::launchByDefaultSettingAddress(), true)
        .toBool();
    }  // end of launchedByDefault

    /*!
     * \return the two characters git wrote about a path, of a record of the
     * answer of `git status --porcelain -z`
     *
     * The record is `XY <path>` and the fields of the answer are separated by a
     * NUL, so a record is what stands between two of them and begins at the
     * first of its characters: the third is the space which separates what git
     * says from the name it says it of.
     */
    bool record_of(const QByteArray &field,
                   char &index_status,
                   char &work_tree_status,
                   QByteArray &path) {
      if (field.size() < 4) {
        return false;
      }
      if (field.at(0) == '#') {
        // `## main...origin/main`, the one line `git` writes about the branch
        // itself rather than about a path, and only when `--branch` is asked
        // for, which this client never does
        return false;
      }
      if (field.at(2) != ' ') {
        // a field which is no record: the answer of `--porcelain -z` holds one
        // more field per renamed or copied path, and that field names the path
        // the entry came from with no status in front of it
        return false;
      }
      index_status = field.at(0);
      work_tree_status = field.at(1);
      path = field.mid(3);
      return true;
    }  // end of record_of()

  }  // end of anonymous namespace

  GitStatusClient::GitStatusClient(EditorWidget &e)
      : QObject(&e), editor(e), timer(new QTimer(this)) {
    this->setObjectName(QString::fromLatin1(client_name));
    // The first question is asked as soon as the client is built, so that the
    // state of the file a buffer was opened on reaches the mode line with the
    // buffer rather than five seconds after it
    QObject::connect(this->timer, &QTimer::timeout, this,
                     &GitStatusClient::pollWatchedRepositories);
    this->timer->start(poll_interval_in_ms);
    QMetaObject::invokeMethod(this, "pollWatchedRepositories",
                              Qt::QueuedConnection);
  }  // end of GitStatusClient::GitStatusClient

  GitStatusClient::~GitStatusClient() {
    // A run which is on its way is ended rather than waited for: the client is
    // a child of the editor window it asks about, and a window which goes away
    // has no reason to wait for a `git status` of a repository it will never
    // show again
    for (auto *r : std::as_const(this->runs)) {
      if (auto *const p = qobject_cast<QProcess *>(r)) {
        if (p->state() != QProcess::NotRunning) {
          p->kill();
          p->waitForFinished(100);
        }
      }
    }
  }  // end of GitStatusClient::~GitStatusClient

  GitStatusClient &GitStatusClient::get(EditorWidget &e) {
    if (auto *const c = e.findChild<GitStatusClient *>(
            QString::fromLatin1(client_name), Qt::FindDirectChildrenOnly)) {
      return *c;
    }
    return *(new GitStatusClient(e));
  }  // end of GitStatusClient::get

  QString GitStatusClient::executableSettingAddress() {
    return QStringLiteral("git/executable");
  }  // end of GitStatusClient::executableSettingAddress

  QString GitStatusClient::launchByDefaultSettingAddress() {
    return QStringLiteral("git/launch-by-default");
  }  // end of GitStatusClient::launchByDefaultSettingAddress

  void GitStatusClient::reportMissingProgram() {
    if (this->reported_missing_program) {
      return;
    }
    this->reported_missing_program = true;
    this->editor.displayInformativeMessage(
        QObject::tr("git could not be started: check the `git` entry of the "
                    "configuration"));
  }  // end of GitStatusClient::reportMissingProgram

  QStringList GitStatusClient::answeredRepositories() const {
    auto l = QStringList{};
    for (auto i = this->answers.constBegin(); i != this->answers.constEnd();
         ++i) {
      if (i.value().answered) {
        l.append(i.key());
      }
    }
    return l;
  }  // end of GitStatusClient::answeredRepositories

  GitStatusClient::Answer GitStatusClient::answerOf(const QByteArray &output) {
    auto answer = Answer{};
    answer.answered = true;
    // The fields of the answer are separated by a NUL, and the answer ends with
    // one: splitting on it leaves an empty field at the end, which is no record
    // and is skipped by the shape test of record_of
    const auto fields = output.split('\0');
    for (auto f = fields.cbegin(); f != fields.cend(); ++f) {
      auto x = '\0';
      auto y = '\0';
      auto path = QByteArray();
      if (!record_of(*f, x, y, path)) {
        continue;
      }
      auto entry = Entry{};
      // `-z` is what keeps the bytes of the answer the UTF-8 encoding of the name
      // as it stands on disk: without it a name with a byte above 127, or a space,
      // is answered as a quoted and octal-escaped C string, which under
      // `core.quotepath=true` — the default — turns `nom_é.txt` into
      // `"nom_\303\251.txt"` and makes the answer name no file of the tree
      entry.path = QString::fromUtf8(path);
      // `??` is the one answer which says that neither the last commit nor the
      // index holds the path. Everything else `git` reports — ` M`, `M `, `MM`,
      // `A `, `D `, `R `, `T `, and the `UU` and `AA` of an unresolved conflict
      // — says the path differs from the commit the branch names, which is what
      // this library calls modified
      entry.untracked = ((x == '?') && (y == '?'));
      answer.entries.append(entry);
      if ((x == 'R') || (x == 'C') || (y == 'R') || (y == 'C')) {
        // A renamed or copied path is two fields: the record names where the
        // entry stands now, and the field behind it names where it came from.
        // That second field carries no status of its own, and reading it as a
        // record would make a path out of its first three characters
        ++f;
      }
    }
    return answer;
  }  // end of GitStatusClient::answerOf

  GitStatusClient::State GitStatusClient::stateOf(const Answer &answer,
                                                  const QString &root,
                                                  const QString &file) {
    if (!answer.answered) {
      return Unknown;
    }
    // The name the question is asked with is a name of the editor: it may be
    // relative — `tfel-editor foo.cxx` keeps the name it was given — and it is
    // spelled with the separators of the platform. The answer holds the names of
    // `git`: relative to the root of the working tree, with `/`, and quoting
    // nothing. Both are brought to the second spelling here, once, and the
    // comparison after that is between names of one kind.
    const auto absolute = QFileInfo(file).absoluteFilePath();
    auto relative = QDir(root).relativeFilePath(absolute);
    if (relative.isEmpty() || (relative == ".") ||
        relative.startsWith("../")) {
      // the file stands outside of the working tree whose answer this is, or is
      // the root itself, which is no working file of it
      return Unknown;
    }
    for (const auto &e : answer.entries) {
      if (e.path == relative) {
        return e.untracked ? Untracked : Modified;
      }
    }
    // A tree of new files is answered as its top directory, with a trailing
    // `/`: `--untracked-files=normal` names no file of an untracked directory,
    // so a file of one is untracked because the directory it stands in is. The
    // same answer is what makes the directory itself an untracked entry of the
    // panel: the reader sees one entry of the tree, and what the program says of
    // it is that it is new.
    const auto as_a_directory = relative + '/';
    for (const auto &e : answer.entries) {
      if (e.untracked && (e.path == as_a_directory)) {
        return Untracked;
      }
    }
    auto directory = QFileInfo(relative).path();
    while (!directory.isEmpty() && (directory != ".")) {
      const auto entry = directory + '/';
      for (const auto &e : answer.entries) {
        if (e.untracked && (e.path == entry)) {
          return Untracked;
        }
      }
      directory = QFileInfo(directory).path();
    }
    return Unmodified;
  }  // end of GitStatusClient::stateOf

  GitStatusClient::State GitStatusClient::statusOf(const QString &file) const {
    const auto root = GitSupport::repositoryRootOf(file);
    if (root.isEmpty()) {
      return Unknown;
    }
    return this->statusIn(root, file);
  }  // end of GitStatusClient::statusOf

  GitStatusClient::State GitStatusClient::statusIn(const QString &root,
                                                   const QString &file) const {
    const auto i = this->answers.constFind(root);
    if (i == this->answers.constEnd()) {
      return Unknown;
    }
    return stateOf(i.value(), root, file);
  }  // end of GitStatusClient::statusIn

  QStringList GitStatusClient::watchedRepositories() const {
    auto roots = QSet<QString>();
    for (auto *const b : this->editor.getBuffers()) {
      const auto f = b->getMainFrame().getFileName();
      if (f.isEmpty()) {
        // a `*scratch*` buffer, the output of a process: no path a repository
        // could hold
        continue;
      }
      const auto r = GitSupport::repositoryRootOf(f);
      if (!r.isEmpty()) {
        roots.insert(r);
      }
    }
    // The panel shows a directory of its own choosing, and the entries it names
    // are of the repository that directory belongs to, which may be none of the
    // buffers' business: what the person asked to see is watched
    if (auto *const ex = this->editor.findChild<FileExplorer *>();
        (ex != nullptr) && ex->isVisible()) {
      const auto r = GitSupport::repositoryRootOf(ex->rootDirectory());
      if (!r.isEmpty()) {
        roots.insert(r);
      }
    }
    auto l = QStringList(roots.constBegin(), roots.constEnd());
    l.sort();
    return l;
  }  // end of GitStatusClient::watchedRepositories

  void GitStatusClient::poll(const QString &root) {
    if (this->runs.contains(root)) {
      // a question about this tree is on its way: a repository which is slow to
      // answer, or a program which hangs, costs one process, not a new one every
      // five seconds.
      //
      // The question is kept and not dropped, because the run which is going
      // does not answer it: that program read the tree before this question was
      // asked, so the answer it brings is silent about whatever happened since —
      // which is exactly the case of the editor writing a file while its
      // repository happens to be asked. The run which ends the question spent is
      // what asks again, at the end of this method's own handler.
      this->pending_questions.insert(root);
      return;
    }
    if (!launchedByDefault()) {
      // The switch is read here, at the one place a process is started, and on
      // every call: an answer given in the configuration dialog reaches the next
      // question of a repository, and the whole of what turning it off costs is
      // the states — the `(git)` of the mode line and the desktop colours of the
      // panel, which is what the editor drew before it asked the program
      if (this->answers.remove(root) > 0) {
        // An answer taken without the switch is an answer on the screen of a
        // repository the window is no longer allowed to ask about, and the next
        // question is what takes it away: a state which stayed after the switch
        // which forbids it would be a state with no one to answer for it, shown
        // for as long as nobody happened to redraw the line
        emit repositoryStatusesChanged(root);
      }
      return;
    }
    const auto program = git_program();
    if (program.isEmpty()) {
      this->reportMissingProgram();
      return;
    }
    // The run is a child of the client, so that the window going away takes the
    // process with it; it is not waited for anywhere, and the answer reaches the
    // screen through the signal of its finish
    auto *const process = new QProcess(this);
    this->runs.insert(root, process);
    process->setProgram(program);
    // `--porcelain` is the format which does not change between versions of git,
    // `-z` is what keeps the names out of quotes and off the risk of a name
    // holding a space, and `--untracked-files=normal` is asked explicitly so
    // that the `status.showUntrackedFiles=no` of somebody's configuration does
    // not silence the untracked files this is all about
    process->setArguments({"status", "--porcelain", "-z",
                           "--untracked-files=normal"});
    process->setWorkingDirectory(root);
    // `git status` refreshes the index of the tree it is asked about, which is a
    // write to the bookkeeping of a repository somebody may be working in. This
    // variable is what says that the question needs no lock and no refresh, and
    // the answer is the same without it
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("GIT_OPTIONAL_LOCKS", "0");
    process->setProcessEnvironment(environment);
    QObject::connect(
        process,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        [this, root, process](int code, QProcess::ExitStatus status) {
          process->deleteLater();
          if (this->runs.value(root) != process) {
            // the answer of a repository this client stopped caring about
            return;
          }
          this->runs.remove(root);
          const auto before = this->answers.value(root);
          // A run which the program refused is no answer. `git status` answers
          // nothing and exits 128 about a directory whose `.git` entry holds no
          // repository, and the empty output of such a run read as an answer
          // would draw every file of that tree as one the repository holds
          // unchanged: a lie about a program which said nothing at all. The
          // state of the files of such a tree is what it was before the first
          // answer of a repository, which is drawn as the plain `(git)`.
          if ((status != QProcess::NormalExit) || (code != 0)) {
            if (before.answered) {
              // the answer this tree gave before is worth taking away: an
              // index locked, or a repository made something else of since,
              // leaves the states on the screen no one to answer for them
              this->answers.remove(root);
              emit repositoryStatusesChanged(root);
            }
          } else {
            const auto answer = GitStatusClient::answerOf(
                process->readAllStandardOutput());
            if ((!before.answered) || (before.entries != answer.entries)) {
              this->answers.insert(root, answer);
              emit repositoryStatusesChanged(root);
            }
            // nothing changed: a repository whose files stand where the
            // previous poll said they stood gets no redraw of the buffers and
            // of the panel which look at it
          }
          // A question which was asked while this run was going is spent here,
          // and asks the tree again at once rather than at the next tick: what
          // was asked of the tree is something this run could not answer, since
          // its program read the tree before the question was made.
          if (this->pending_questions.remove(root) > 0) {
            this->poll(root);
          }
        });
    QObject::connect(
        process, &QProcess::errorOccurred, this,
        [this, root, process](QProcess::ProcessError error) {
          if (error != QProcess::FailedToStart) {
            return;
          }
          process->deleteLater();
          this->runs.remove(root);
          // The program was named by `git_program`, so it was found on the paths:
          // a name which resolves to something which refuses to be run is the same
          // mistake of the configuration seen from the other side, and is reported
          // the same way
          this->reportMissingProgram();
        });
    process->start();
  }  // end of GitStatusClient::poll

  void GitStatusClient::pollWatchedRepositories() {
    const auto roots = this->watchedRepositories();
    auto forgotten = QStringList{};
    // A repository nobody looks at any more is forgotten: the answers of a long
    // session of browsing directories do not pile up, and a tree which comes back
    // on the screen is asked again.
    //
    // What is to be dropped is gathered first and removed after: `QHash::remove`
    // invalidates the iterators of the container it is called on, so dropping
    // inside the walk over the same hash walks a table which was rebuilt under it,
    // which skips an entry or stops at no end at all. Nothing else in the client
    // does this — every other removal happens in a slot, with no walk going on
    for (auto i = this->answers.constBegin(); i != this->answers.constEnd();
         ++i) {
      if (!roots.contains(i.key())) {
        forgotten.append(i.key());
      }
    }
    for (const auto &r : std::as_const(forgotten)) {
      this->answers.remove(r);
    }
    for (const auto &r : roots) {
      this->poll(r);
    }
  }  // end of GitStatusClient::pollWatchedRepositories

  void GitStatusClient::refresh(const QString &root) {
    if (!root.isEmpty()) {
      // a repository somebody has a reason to ask about right now: the poll of
      // one repository is asked at most once per run on its way, so this is a way
      // to skip the wait of the timer, never a way to have two runs at once
      this->poll(root);
      return;
    }
    this->pollWatchedRepositories();
  }  // end of GitStatusClient::refresh

  // The program which says what the files of a working tree are is a choice,
  // and the theme it is chosen under is named after it, as the theme of `clangd`
  // and that of `castem` are. Those three, and the TeX engines, are what the
  // editor runs beside itself: the heading of the block they stand under says
  // that they are no part of the editor and no language it reads
  static const StandardSettingsGroupProxy git(
      "git",
      QObject::tr("The program which says what the files of a working tree "
                  "are, and whether the editor asks it"),
      QVector<SettingsKey>()
          << SettingsKey::text(
                 GitStatusClient::executableSettingAddress(),
                 QObject::tr("git program"),
                 QObject::tr("The program asked what the files of a working "
                             "tree are, for the indicator of the mode line and "
                             "for the colors of the file explorer panel. A name "
                             "which is no program takes both of them away"),
                 QStringLiteral("git"))
          << SettingsKey::boolean(
                 GitStatusClient::launchByDefaultSettingAddress(),
                 QObject::tr("Ask git what the files of a working tree are"),
                 QObject::tr("When this is off, the editor runs no program "
                             "about the files it has open: the indicator of "
                             "the mode line says that the file is a working "
                             "file of a repository and says nothing else, and "
                             "the file explorer panel wears the colors of the "
                             "desktop. The answer is read at every question, "
                             "so it reaches the next poll of a repository with "
                             "no restart"),
                 true),
      QString(),
      QObject::tr("External tools"));

}  // end of namespace tfel::gui
