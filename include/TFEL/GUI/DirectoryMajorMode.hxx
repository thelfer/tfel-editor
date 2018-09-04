/*!
 * \file   DirectoryMajorMode.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#ifndef LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX
#define LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX

#include<QtCore/QFileSystemWatcher>
#include"TFEL/GUI/Config.hxx"
#include"TFEL/GUI/MajorModeBase.hxx"

namespace tfel{

  namespace gui{

  //! A major mode dedicated to exploring a directory
  struct TFEL_GUI_VISIBILITY_EXPORT DirectoryMajorMode final
      : MajorModeBase {
    /*!
     * \param[in] d: initial directory
     * \param[in] w: editor widget
     * \param[in] b: editor buffer
     * \param[in] t: editor text
     */
    DirectoryMajorMode(const QString&,
                       EditorWidget&,
                       Buffer&,
                       TextEditBase&);
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
     * This method is called before any treatment by TextEditBase
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

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_DIRECTORYMAJORMODE_HXX */
