/*!
 * \file   DirectoryMajorMode.hxx
 * \brief major mode dedicated to exploring a directory.
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#ifndef LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX
#define LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX

#include <QtCore/QFileSystemWatcher>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"

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
    //! \note the installed highlighter colours sub-directories in blue and
    //! executable files in dark green
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
     * \brief explore another directory and replace the watched one
     * \param[in] d: directory path
     *
     * \note the buffer content, the watched path and the syntax
     * highlighter are updated accordingly; nothing happens when the given
     * path is not an existing directory
     */
    void updateDirectoryDescription(const QString&);

   private:
    //! object in charged of watching the current directory changed
    QFileSystemWatcher watcher;
    //! directory explored
    QString directory;

    Q_OBJECT
  };  // end of DirectorMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX */
