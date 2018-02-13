/*!
 * \file   DirectoryMajorMode.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#ifndef LIB_QEMACS_DIRECTORYMAJORMODE_HXX
#define LIB_QEMACS_DIRECTORYMAJORMODE_HXX

#include<QtCore/QFileSystemWatcher>
#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs{

  //! A major mode dedicated to exploring a directory
  struct QEMACS_VISIBILITY_EXPORT DirectoryMajorMode final
      : QEmacsMajorModeBase {
    /*!
     * \param[in] d: initial directory
     * \param[in] w: qemacs widget
     * \param[in] b: qemacs buffer
     * \param[in] t: qemacs text
     */
    DirectoryMajorMode(const QString&,
                       QEmacsWidget&,
                       QEmacsBuffer&,
                       QEmacsTextEditBase&);
    //! \return the name of this major mode
    QString getName() const override;
    //! \return a description of this major mode
    QString getDescription() const override;
    //! \brief set the syntaxHighlighter of the document
    void setSyntaxHighlighter(QTextDocument*) override;
    //! \brief method called to format the document
    void format() override;
    //! \return the icon associated to the current mode
    QIcon getIcon() const override;
    /*!
     * This method is called before any treatment by QEmacsTextEditBase
     * and allows the mode to override default shortcuts
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
     * \param[in] d: directory path
     */
    void updateDirectoryDescription(const QString&);

   private:
    //! object in charged of watching the current directory changed
    QFileSystemWatcher watcher;
    //! directory explored
    QString directory;
    
    Q_OBJECT
  }; // end of DirectorMajorMode 

} // end of namespace qemacs

#endif /* LIB_QEMACS_DIRECTORYMAJORMODE_HXX */
