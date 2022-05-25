/*!
 * \file  Command.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Command.hxx"

namespace tfel::gui {

  Command::Command(EditorWidget& t) : QObject(&t) {}  // end of Command

  Command::~Command() = default;

}  // end of namespace tfel::gui
