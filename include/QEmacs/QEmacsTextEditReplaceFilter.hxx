/*! 
 * \file  QEmacsTextEditReplaceFilter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 06 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_H_
#define _LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_H_ 

#include"QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs
{

  class QEmacsWidget;

  class QEmacsTextEdit;

  class QEmacsTextEditQueryReplace;


  /*!
   * A key press event filter which forward the user
   */
  struct QEmacsTextEditReplaceFilter
    : public QEmacsTextEditKeyPressEventFilter
  {
    QEmacsTextEditReplaceFilter(QEmacsWidget&,
				QEmacsTextEditBase&,
				QEmacsTextEditQueryReplace&,
				const QString&,
				const QString&);
    
    virtual bool
    isOk(void) const override;

    virtual bool
    filterKeyPressEvent(QKeyEvent * const) override;

    ~QEmacsTextEditReplaceFilter();

  protected:

    bool findNext(void);

    QEmacsWidget&   qemacs;
    
    QEmacsTextEditBase& textEdit;
    
    QEmacsTextEditQueryReplace& qr;

    QTextCursor bc;

    QTextCursor ec;

    const QString s1;
    
    const QString s2;
    
    int nb;
    
    bool hasSelection;

    bool bOK;

  }; // end of QEmacsTextEditReplaceFilter

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_H */

