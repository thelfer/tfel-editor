/*! 
 * \file  MTestMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#ifndef _LIB_QEMACS_MTESTMAJORMODE_H_
#define _LIB_QEMACS_MTESTMAJORMODE_H_ 

#include"QEmacs/CxxMajorMode.hxx"

namespace qemacs
{

  class QEmacsTextEditBase;
  
  /*!
   * major mode
   */
  class MTestMajorMode
    : public CxxMajorMode
  {

    Q_OBJECT

  public:

    MTestMajorMode(QEmacsWidget&,
		    QEmacsBuffer&,
		    QEmacsTextEditBase&);

    virtual QString
    getName(void) const override;

    virtual QString
    getDescription(void) const override;
    
    virtual void
    completeContextMenu(QMenu *const,
			const QTextCursor&) override;

    virtual void
    setSyntaxHighlighter(QTextDocument *const);

    virtual QCompleter*
    getCompleter(void) override;

    QString
    getCompletionPrefix(void) override;

    virtual QMenu*
    getSpecificMenu(void) override;

    virtual bool
    keyPressEvent(QKeyEvent * const) override;
    
    virtual ~MTestMajorMode();

  protected slots:
    //! method called when the Help menu is called
    virtual void run(void);
    //! method called when the Help menu is called
    virtual void start(void);
    //! method called when the Help menu is called
    virtual void
    actionTriggered(QAction *);
    /*!
     * \param[in] a: action
     */
    virtual void
    insertKeyword(QAction *);
  protected:
    //! return the list of MTest keywords
    virtual QStringList
    getKeyWordsList() const;
    //! return the scheme name
    virtual QString
    getScheme(void) const;
    //! completer
    QCompleter* c = nullptr;
    //! help action
    QAction *ha = nullptr;
    //! run mtest
    QAction *ra = nullptr;
  }; // end of struct MTestMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_MTESTMAJORMODE_H */

