/*! 
 * \file  QEmacsTextEditKeyPressEventFilter.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#include"QtCore/QDebug"

#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs
{

  QEmacsTextEditKeyPressEventFilter::QEmacsTextEditKeyPressEventFilter(QEmacsTextEditBase& t)
    : QObject(&t)
  {} // end of QEmacsTextEditKeyPressEventFilter::QEmacsTextEditKeyPressEventFilter

  QEmacsTextEditKeyPressEventFilter::~QEmacsTextEditKeyPressEventFilter()
  {} // end of QEmacsTextEditKeyPressEventFilter::~QEmacsTextEditKeyPressEventFilter

} // end of namespace qemacs
