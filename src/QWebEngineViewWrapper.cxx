/*! 
 * \file  QWebEngineViewWrapper.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/12/2012
 */

#include<QtWidgets/QHBoxLayout>
#include"TFEL/GUI/Utilities.hxx"
#include"TFEL/GUI/QWebEngineViewWrapper.hxx"

namespace tfel{

  namespace gui{

  QWebEngineViewWrapper::QWebEngineViewWrapper(QWebEngineView *s,
				   QWidget *p)
    : QWidget(p),
      wrappedObject(s)
  {
    auto *hl  = new QHBoxLayout;
    setQWebEngineViewInLayout(hl,this->wrappedObject);
    this->setLayout(hl);
    }
  
  void
  QWebEngineViewWrapper::focusInEvent(QFocusEvent *)
  {
    this->wrappedObject->setFocus();
  }

} // end of namespace gui
}// end of namespace tfel
