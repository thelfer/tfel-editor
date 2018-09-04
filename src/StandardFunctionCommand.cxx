/*! 
 * \file  StandardFunctionCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include"TFEL/GUI/EditorWidget.hxx"
#include"TFEL/GUI/StandardFunctionCommand.hxx"

namespace tfel{

  namespace gui{

  StandardFunctionCommand::StandardFunctionCommand(
      EditorWidget &w, const Fct f)
      : Command(w),
        c(f) {
  }  // end of
     // StandardFunctionCommand::StandardFunctionCommand

  void StandardFunctionCommand::execute(){
    auto *p = qobject_cast<EditorWidget *>(this->parent());
    if(p==nullptr){
      return;
    }
    this->c(*p);
  }  // end of StandardFunctionCommand::execute

  StandardFunctionCommand::~StandardFunctionCommand() =
      default;

} // end of namespace gui
}// end of namespace tfel
