/*!
 * \file  GitStatusClient.hxx
 * \brief the state of the files of a git working tree, and the client which
 *        asks the program about it.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#ifndef LIB_TFEL_GUI_GITSTATUSCLIENT_HXX
#define LIB_TFEL_GUI_GITSTATUSCLIENT_HXX

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QVector>
#include <QtGui/QColor>

#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  //! forward declaration
  struct EditorWidget;

  /*!
   * \brief the client which asks `git status` what it thinks of the files of
   *        the working trees the editor has open
   *
   * One client per editor widget, held as its child and found by the object
   * name `git-status`. The information line of a buffer and the file explorer
   * panel both ask it what a file is, and it is the only place which runs the
   * program.
   *
   * \note The answer is polled, not watched: every five seconds, each
   * repository which is watched gets a `git status --porcelain -z` of its whole
   * working tree, and the same question is asked of the repository of a file
   * the moment the editor writes that file or reads it back — which is what
   * makes the state of a saved file reach the line and the panel at once,
   * instead of at the next tick of the timer, a wait of up to five seconds
   * which a person who has just saved and looked has no patience for. A watch
   * of the contents would say nothing of the state — the same untouched file is
   * modified or clean according to what the index and the last commit hold of
   * it — and would answer a question about one file when the question is about
   * the whole tree, which is what one run of the program already costs.
   *
   * \note What is watched are the repositories of the files the buffers visit
   * and the repository of the directory the explorer panel shows. Nothing else
   * is asked: a repository nobody looks at is polled by nobody, and a buffer
   * which is killed stops its repository being polled with no bookkeeping of
   * the killed buffer, since the set is recomputed at every poll.
   *
   * \note A repository which is already being asked is never asked twice at
   * once: the poll of a tree is skipped while its run is on its way, so a slow
   * repository or a `git` which hangs costs one process, not a new one every
   * five seconds. A question skipped that way is not lost, however: the client
   * remembers that the tree was asked about and asks it again when the run
   * which was on its way ends. That is what keeps the state of a file the
   * editor has just written from being the state the file had before the write,
   * which is what the run which was already going answers.
   *
   * \note The run is made with `GIT_OPTIONAL_LOCKS=0`, which is what keeps the
   * poll from refreshing the index of the repository: a background question has
   * no business writing to the bookkeeping of a tree somebody may be working
   * in, and the answer is the same without it.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT GitStatusClient : QObject {
    Q_OBJECT

  public:
    //! what git holds about one path of a working tree
    enum State {
      /*!
       * \brief nothing is known: the file is of no repository, the answer of
       * the repository it belongs to was never asked, or `git` is not to be
       * found. It is drawn as an unmodified file, which is the honest reading
       * of an answer which was not had
       */
      Unknown,
      //! a tracked file whose text and index entry are what the last commit has
      Unmodified,
      //! a file of the working tree which no commit and no index entry holds
      Untracked,
      /*!
       * \brief a tracked file which differs from the last commit, in its text,
       * in the index, or both, which covers a staged addition, a deletion and
       * an unresolved conflict alike
       */
      Modified
    };

    /*!
     * \brief constructor
     * \param[in] e : the editor widget this client asks about, which becomes
     * the Qt parent of this client
     *
     * \note \c get is what builds the client of an editor, which is what a
     * buffer and the explorer both do; building one by hand is only worth it in
     * a test of the client alone.
     */
    GitStatusClient(EditorWidget &e);
    //! destructor: ends the runs which are on their way
    ~GitStatusClient() override;

    /*!
     * \brief the client of an editor widget, built on the first question
     * \param[in] e : the editor widget asked about
     */
    static GitStatusClient &get(EditorWidget &e);

    /*!
     * \return what git holds about the given file
     * \param[in] file : name of a file, absolute or not
     *
     * The answer is what the last poll of the repository the file belongs to
     * said, which is nothing until that repository was watched and polled once:
     * the first answer reaches the screen at most five seconds after a buffer
     * of a repository is opened or an explorer panel is rooted in one.
     *
     * \note The file needs no existence: a buffer visiting a file which was not
     * written yet is answered from what the tree holds about the name it is
     * about to be written under.
     */
    State statusOf(const QString &file) const;

    /*!
     * \return the state of a file, asked of the answer of the whole tree it
     * stands in rather than of the file alone
     * \param[in] root : root of the repository
     * \param[in] file : name of a file of that working tree
     *
     * This is the question \c statusOf asks once it found the root; it is
     * public because the file explorer, which walks a whole directory of one
     * repository at a time, already knows the root and has no reason to look it
     * up per entry.
     */
    State statusIn(const QString &root, const QString &file) const;

    /*!
     * \brief ask a repository, or every repository which is watched, without
     * waiting for the timer
     * \param[in] root : root of the repository to ask, empty for all of them
     *
     * This is what makes the state of a file reach the screen while a person is
     * working rather than five seconds later: a buffer whose repository has
     * never been asked asks for its first answer when its line is drawn, and
     * the repository of a file the editor has just written or read back is
     * asked by `TextEditBase::save` and `TextEditBase::reloadFile` at the
     * moment of the write.
     *
     * \note Asking is not waiting: the answer comes with the signal. A
     * repository whose run is on its way swallows the request only for
     * the moment, and is asked again when that run ends, so a call which lands in
     * the middle of a poll costs nothing and loses nothing either.
     *
     * \note An empty \a root asks every watched repository, which is what a
     * caller who has a file in hand must not do: whoever asks about one file
     * asks about \c GitSupport::repositoryRootOf(file) and asks nothing when
     * that answers the empty string.
     */
    void refresh(const QString &root = QString());

    /*!
     * \return the address of the entry of the configuration which names the
     * program this client runs, `git/executable`
     *
     * The entry is declared with `GitStatusClient`, next to the only code which
     * reads it.
     */
    static QString executableSettingAddress();

    /*!
     * \return the address of the configuration entry which says whether the
     * program is run at all, `git/launch-by-default`
     */
    static QString launchByDefaultSettingAddress();

    /*!
     * \return the colour the two states of a file are drawn with
     * \param[in] s : what \c statusOf or \c statusIn answered
     *
     * The colours of the states are answered here and not by the surfaces which
     * draw them because they are one decision with the states: the information
     * line of a buffer, the entries of the file explorer panel and the lines of a
     * directory listing all ask the same question of this client, and a *modified*
     * file which wore one ink in the panel and another in a listing would be read
     * as two different facts about the same file.
     *
     * \note The blue of a new file is the colour the file managers and the version
     * control plugins of this vintage give it. The dark red of a modified file is
     * not the red of an error: the two stand beside each other in a buffer of a
     * C++ file which is both modified and full of what clangd complains of, and an
     * entry which reads as an error where the text reads as a warning of the
     * compiler is a surface which has stopped informing anybody.
     *
     * \note \c Unknown and \c Unmodified are answered with an \e invalid colour,
     * which is how whoever draws learns that the state says nothing and the ink of
     * the desktop must be kept. An ink of "unchanged" would be an invention: the
     * absence of a report from the program about a file is not a report.
     */
    static QColor colorOf(State s);

    /*!
     * \return the names of the repositories this client holds an answer about
     *
     * \note Only of interest to whoever wants to know whether a poll reached a
     * repository, which is what a test does before it asserts on an answer.
     */
    QStringList answeredRepositories() const;

  signals:
    /*!
     * \brief emitted when the answer about a repository changed, or when the
     * first answer about it arrived
     * \param[in] root : root of the repository whose answer is new
     *
     * \note Emitted only when the answer is \e new: a repository whose files
     * stand where the previous poll said they stood gets no signal, so the
     * redraw of every buffer and of the panel of a window is not asked for
     * twenty times a minute about a tree in which nothing happens.
     *
     * \note The state of \a root is already recorded when the signal is emitted,
     * so whoever is reached by it has only to draw.
     */
    void repositoryStatusesChanged(const QString &root);

  private slots:
    //! ask every watched repository which has no run on its way
    void pollWatchedRepositories();

  private:
    /*!
     * \brief the answer about one path of a working tree
     *
     * The path is the name `git` answered, relative to the root of the
     * repository and with `/` as its separator. An untracked directory is named
     * with a trailing `/` and stands for the whole of it, which is what the
     * answer of `--untracked-files=normal` holds a tree of new files to.
     */
    struct Entry {
      //! name of the path, relative to the root of the repository
      QString path;
      //! whether the path is what no commit and no index holds
      bool untracked = false;

      /*!
       * \return whether the two entries say the same thing about the same path
       *
       * What makes an answer worth a redraw is that it says something else about
       * a path than the answer before it did, which is the question the vector
       * of entries of the two answers is compared on.
       */
      friend bool operator==(const Entry &a, const Entry &b) {
        return (a.path == b.path) && (a.untracked == b.untracked);
      }
    };

    /*!
     * \brief what one poll of one repository answered
     *
     * The paths are held in the order the program answered them, which is the
     * order of the tree. The question the two surfaces ask is about one name at
     * a time, and a few hundred names compared in order is nothing beside the
     * process it cost to get them.
     */
    struct Answer {
      //! whether a first answer was had at all
      bool answered = false;
      //! the paths git holds something about
      QVector<Entry> entries;
    };

    //! \brief start the poll of one repository, if none of its own is running
    void poll(const QString &root);

    /*!
     * \brief say once that the program was not to be found
     *
     * A program which is not to be found is a mistake of the configuration, and
     * every poll of every repository would repeat the line for as long as the
     * window lives: it is said once, and the indicator and the colours of the
     * panel, which are what the mistake takes away, are what the person has to go
     * on.
     */
    void reportMissingProgram();

    //! \brief the repositories the editor has reason to ask about
    QStringList watchedRepositories() const;

    //! \brief turn the raw answer of one repository into an \c Answer
    static Answer answerOf(const QByteArray &output);

    /*!
     * \brief what the recorded answer says about a path of the tree
     * \param[in] answer : what one poll answered
     * \param[in] root : root of the repository the answer is about
     * \param[in] file : name of a file, as the editor names it
     *
     * The name the question is asked with is the name of the editor — absolute
     * or not, with the separators of the platform — and the answer holds the
     * names of `git`: relative to the root, with `/`, and quoting nothing. The
     * two are brought together here, once, and the comparison after that is on
     * names of one kind.
     */
    static State stateOf(const Answer &,
                         const QString &root,
                         const QString &file);

    //! the editor widget this client asks about
    EditorWidget &editor;
    //! the polled answer of every repository which was watched, by root
    QHash<QString, Answer> answers;
    //! the run of every repository which is on its way, by root
    QHash<QString, QObject *> runs;
    /*!
     * \brief the repositories somebody asked about while their run was going
     *
     * A question about a tree whose run is on its way is swallowed — no second
     * process for one tree — but it is not \e answered by that run, whose
     * program read the tree before the question was asked. What asked is
     * recorded here and spent when the run ends, which is what keeps the answer
     * of a repository from being the state of a file as it stood before the
     * editor wrote it, the case `TextEditBase::save` reaches: a save which
     * lands while the poll of the tree is on its way is a save whose state
     * would otherwise wait for the next tick, five seconds off.
     */
    QSet<QString> pending_questions;
    /*!
     * \brief whether the loss of the program has been reported
     *
     * A program which is not to be found is a mistake in the configuration, and
     * every poll of every repository repeats it: the line is said once, and the
     * person who owns the settings file has the answer of the indicator and of
     * the panel to go on.
     */
    bool reported_missing_program = false;
    //! the timer which asks every five seconds
    QTimer *timer;
  };  // end of struct GitStatusClient

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GITSTATUSCLIENT_HXX */
