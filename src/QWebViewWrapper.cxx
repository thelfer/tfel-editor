/*! 
 * \file  QWebViewWrapper.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#include<QtWidgets/QHBoxLayout>
#include"QEmacs/Utilities.hxx"
#include"QEmacs/QWebViewWrapper.hxx"

namespace qemacs
{

  QWebViewWrapper::QWebViewWrapper(QWebView *s,
				   QWidget *p)
    : QWidget(p),
      wrappedObject(s)
  {
    auto *hl  = new QHBoxLayout;
    setQWebViewInLayout(hl,this->wrappedObject);
    this->setLayout(hl);
    }
  
  void
  QWebViewWrapper::focusInEvent(QFocusEvent *)
  {
    this->wrappedObject->setFocus();
  }

} // end of namespace qemacs
