/*! 
 * \file  QEmacsTextEditKeyPressEventFilter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include"QtCore/QDebug"

#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs
{

  QEmacsTextEditKeyPressEventFilter::QEmacsTextEditKeyPressEventFilter(QEmacsTextEditBase& t)
    : QObject(&t)
  {} // end of QEmacsTextEditKeyPressEventFilter::QEmacsTextEditKeyPressEventFilter

  QEmacsTextEditKeyPressEventFilter::~QEmacsTextEditKeyPressEventFilter() = default;

} // end of namespace qemacs
