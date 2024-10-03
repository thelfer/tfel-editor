/*!
 * \file  StandardFunctionCommand.cxx
 * \brief
 * \author Thomas Helfer
 * \date   02/08/2012
 */

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"

namespace tfel::gui {

  StandardFunctionCommand::StandardFunctionCommand(EditorWidget &w, const Fct f)
      : Command(w), c(f) {}  // end of StandardFunctionCommand

  void StandardFunctionCommand::execute() {
    auto *p = qobject_cast<EditorWidget *>(this->parent());
    if (p == nullptr) {
      return;
    }
    this->c(*p);
  }  // end of execute

  StandardFunctionCommand::~StandardFunctionCommand() = default;

}  // end of namespace tfel::gui
