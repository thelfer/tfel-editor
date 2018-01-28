/*! 
 * \file  QAbstractScrollAreaWrapper.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#include<QtWidgets/QHBoxLayout>
#include"QEmacs/Utilities.hxx"
#include"QEmacs/QAbstractScrollAreaWrapper.hxx"

namespace qemacs
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

} // end of namespace qemacs

