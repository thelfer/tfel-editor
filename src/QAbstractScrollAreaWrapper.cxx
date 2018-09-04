/*! 
 * \file  QAbstractScrollAreaWrapper.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/12/2012
 */

#include<QtWidgets/QHBoxLayout>
#include"TFEL/GUI/Utilities.hxx"
#include"TFEL/GUI/QAbstractScrollAreaWrapper.hxx"

namespace tfel{

  namespace gui
{

  QAbstractScrollAreaWrapper::QAbstractScrollAreaWrapper(QAbstractScrollArea *s,
							 QWidget *p)
    : QWidget(p),
      wrappedObject(s)
    {
      auto *hl = new QHBoxLayout;
      setQAbstractScrollAreaInLayout(hl,this->wrappedObject);
      this->setLayout(hl);
    }

  QAbstractScrollArea * 
  QAbstractScrollAreaWrapper::getWrappedObject()
  {
    return this->wrappedObject;
  }
  
  void
  QAbstractScrollAreaWrapper::focusInEvent(QFocusEvent *)
  {
    this->wrappedObject->setFocus();
  }

} // end of namespace gui
}// end of namespace tfel

