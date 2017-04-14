/*! 
 * \file  QWebViewWrapper.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#ifdef QEMACS_QT4
#include<QtGui/QHBoxLayout>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QHBoxLayout>
#endif /* QEMACS_QT5 */

#include"QEmacs/Utilities.hxx"
#include"QEmacs/QWebViewWrapper.hxx"

namespace qemacs
{

  QWebViewWrapper::QWebViewWrapper(QWebView *s,
				   QWidget *p)
    : QWidget(p),
      wrappedObject(s)
  {
    QHBoxLayout *hl  = new QHBoxLayout;
    setQWebViewInLayout(hl,this->wrappedObject);
    this->setLayout(hl);
    }
  
  void
  QWebViewWrapper::focusInEvent(QFocusEvent *)
  {
    this->wrappedObject->setFocus();
  }

}
