/*! 
 * \file  MFrontMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef _LIB_QEMACS_MFRONTMAJORMODE_H_
#define _LIB_QEMACS_MFRONTMAJORMODE_H_ 

#include<QtCore/QTimer>
#include"QEmacs/CxxMajorMode.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
  
  /*!
   * major mode
   */
  class MFrontMajorMode
    : public CxxMajorMode
  {

    Q_OBJECT

  public:

    MFrontMajorMode(QEmacsWidget&,
		    QEmacsBuffer&,
		    QEmacsTextEditBase&);

    virtual QString
    getName(void) const override;

    virtual QString
    getDescription(void) const override;
    
    virtual void
    setSyntaxHighlighter(QTextDocument *const) override;

    virtual void
    completeContextMenu(QMenu *const,
			const QTextCursor&) override;
    
    virtual QCompleter*
    getCompleter(void) override;

    QString
    getCompletionPrefix(void) override;

    virtual QIcon
    getIcon(void) const override;
    //! destructor    
    virtual ~MFrontMajorMode();

  protected slots:
    //! method called periodically to refresh the syntax highlighter
    //! and the completer
    virtual void
    updateSyntaxHighlighterAndCompleter(void);
    //! method called when the Help menu is called
    virtual void
    actionTriggered(QAction *);
  protected:
    //! \return the name of the DSL
    virtual QString getDSLName(void) const;
    //! return the list of MTest keywords
    virtual QStringList
    getKeyWordsList() const;
    /*!
     * \brief a timer to refresh syntax highlighting
     * This is mandatory since the @DSL can change with the user inputs
     */
    QTimer *rt = nullptr;
    //! completer
    QCompleter* c = nullptr;
    //! help action
    QAction *ha = nullptr;
  }; // end of struct MFrontMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_MFRONTMAJORMODE_H */

