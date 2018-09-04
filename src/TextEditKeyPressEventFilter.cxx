/*! 
 * \file  TextEditKeyPressEventFilter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include"QtCore/QDebug"

#include"TFEL/GUI/TextEditBase.hxx"
#include"TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel{

  namespace gui
{

  TextEditKeyPressEventFilter::TextEditKeyPressEventFilter(TextEditBase& t)
    : QObject(&t)
  {} // end of TextEditKeyPressEventFilter::TextEditKeyPressEventFilter

  TextEditKeyPressEventFilter::~TextEditKeyPressEventFilter() = default;

} // end of namespace gui
}// end of namespace tfel
