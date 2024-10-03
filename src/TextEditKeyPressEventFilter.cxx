/*!
 * \file  TextEditKeyPressEventFilter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   02/08/2012
 */

#include "QtCore/QDebug"

#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel::gui {

  TextEditKeyPressEventFilter::TextEditKeyPressEventFilter(TextEditBase& t)
      : QObject(&t) {
  }  // end of TextEditKeyPressEventFilter::TextEditKeyPressEventFilter

  TextEditKeyPressEventFilter::~TextEditKeyPressEventFilter() = default;

}  // end of namespace tfel::gui
