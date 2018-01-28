/*! 
 * \file  QEmacsTextEditReplaceFilter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 06 août 2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX
#define LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX 

#include"QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs
{

  class QEmacsWidget;

  class QEmacsTextEdit;

  struct QEmacsTextEditQueryReplace;


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
    
    bool isOk() const override;

    bool filterKeyPressEvent(QKeyEvent * const) override;

    ~QEmacsTextEditReplaceFilter() override;

  protected:

    bool findNext();

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

#endif /* LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX */

