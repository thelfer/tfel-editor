/*! 
 * \file  QEmacsStandardFunctionCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsStandardFunctionCommand.hxx"

namespace qemacs {

  QEmacsStandardFunctionCommand::QEmacsStandardFunctionCommand(
      QEmacsWidget &w, const Fct f)
      : QEmacsCommand(w),
        c(f) {
  }  // end of
     // QEmacsStandardFunctionCommand::QEmacsStandardFunctionCommand

  void QEmacsStandardFunctionCommand::execute(){
    auto *p = qobject_cast<QEmacsWidget *>(this->parent());
    if(p==nullptr){
      return;
    }
    this->c(*p);
  }  // end of QEmacsStandardFunctionCommand::execute

  QEmacsStandardFunctionCommand::~QEmacsStandardFunctionCommand() =
      default;

} // end of namespace qemacs
