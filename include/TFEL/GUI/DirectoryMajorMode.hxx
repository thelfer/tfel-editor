/*!
 * \file   DirectoryMajorMode.hxx
 * \brief major mode dedicated to exploring a directory.
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#ifndef LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX
#define LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX

#include <QtCore/QFileSystemWatcher>
#include <QtCore/QPointer>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/GitStatusClient.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"

class QAbstractScrollArea;

namespace tfel::gui {

  //! \brief a major mode dedicated to exploring a directory
  struct TFEL_GUI_VISIBILITY_EXPORT DirectoryMajorMode final : MajorModeBase {
    /*!
     * \param[in] d: initial directory
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text
     */
    DirectoryMajorMode(const QString&, EditorWidget&, Buffer&, TextEditBase&);
    //! \return the name of this major mode
    QString getName() const override;
    //! \return a description of this major mode
    QString getDescription() const override;
    //! \brief set the syntaxHighlighter of the document
    //! \note the installed highlighter colours sub-directories in dark blue and
    //! executable files in dark green; the pure blue of a new file and the dark
    //! red of a modified one are the two inks git has for the states of a file,
    //! and are drawn over those faces by `updateGitColors` — which is why the
    //! directory face is dark blue and not blue: the ink of a name has to say
    //! what the file *is* to its repository and what kind of entry it is, and two
    //! inks can not share one hue
    void setSyntaxHighlighter(QTextDocument*) override;
    //! \brief method called to format the document
    //! \note this mode does nothing: the buffer content is generated from
    //! the directory listing and is read-only
    void format() override;
    //! \return the icon associated to the current mode
    QIcon getIcon() const override;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     * \param[in] e : key event
     *
     * \return true when \c Return or \c Enter is pressed without any
     * modifier: the first word of the line is then opened as a file, or,
     * when it names a sub-directory, explored in place of the current
     * directory; otherwise the event is delegated to
     * \c MajorModeBase::keyPressEvent
     */
    bool keyPressEvent(QKeyEvent* const) override;
    //! destructor
    ~DirectoryMajorMode() final;

   private slots:
    /*!
     * \brief update the content of editor when a change on the watched
     * directory occurs
     */
    void updateDirectoryDescription();
    /*!
     * \brief colour the names of the listing with the state their file is in
     *
     * The names are the only thing the listing draws about an entry, so the state
     * a repository holds about a file is drawn on them — in the two inks the file
     * explorer panel uses, which `GitStatusClient::colorOf` answers so that the
     * panel and a listing can not disagree about what a *modified* file looks like.
     *
     * \note The inks are handed to the text edit as the contribution keyed
     * `directory-git`, not to the syntax highlighter: what the state of a file is
     * is an answer of another process, arrived after the text was written, which is
     * the shape of a contribution of the editor (`current-line`, `clangd`,
     * `grammar`) rather than of a face of the mode. It is also what makes a *known*
     * state win over the executable face, since a contribution is drawn over the
     * words the highlighter inked.
     *
     * \note Nothing is drawn for an entry of no repository, for one the answer holds
     * nothing about, and for `..`: the ink of the highlighter, which says what the
     * entry *is*, is all the reader has and all the editor knows.
     */
    void updateGitColors();
    /*!
     * \brief explore another directory and replace the watched one
     * \param[in] d: directory path
     *
     * \note the buffer content, the watched path and the syntax
     * highlighter are updated accordingly; nothing happens when the given
     * path is not an existing directory
     */
    void updateDirectoryDescription(const QString&);

   private:
    /*!
     * \return the state of the entry the given name names in the directory shown
     *
     * `..` is answered `Unknown` without asking: it names a directory the listing
     * does not show, and often enough a directory out of the repository — and the
     * parent of the directory shown is never what a reader looks the state of.
     */
    GitStatusClient::State stateOfEntry(const QString& name) const;

    //! object in charged of watching the current directory changed
    QFileSystemWatcher watcher;
    /*!
     * \brief the Qt widget the listing is drawn in, as long as it lives
     *
     * Only of use to the destructor, and the reason it is the *inner* Qt widget of
     * the text edit and not the `TextEditBase` which is the mode's own parent: that
     * widget is a child created before the mode, so `QObjectPrivate::deleteChildren`
     * reaches it first and the pointer is null by the time the mode is deleted, which
     * is what keeps the destructor from asking a text edit whose own members are
     * already gone — among them the map of the keyed contributions — to redraw. The
     * pointer is therefore the guard `GrammarCheckSession` needs for the same reason.
     */
    QPointer<QAbstractScrollArea> area;
    //! directory explored
    QString directory;
    /*!
     * \brief the repository the entries of the listing are asked of, empty when
     *        they are of none
     *
     * Answered once for the directory shown and kept: the listing has one entry per
     * file of a single directory, so the ascent which finds the `.git` entry is made
     * once per listing rather than once per line, and `repositoryStatusesChanged`
     * needs the answer to know which of its signals this buffer is about.
     */
    QString repository_root;

    Q_OBJECT
  };  // end of DirectorMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX */
