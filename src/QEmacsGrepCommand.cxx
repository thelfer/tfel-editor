/*! 
 * \file  QEmacsGrepCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#include"QEmacs/QEmacsCommand.hxx"
#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsCommandFactory.hxx"

#include"QEmacs/QEmacsGrepCommand.hxx"

namespace qemacs {

  QEmacsGrepCommand::QEmacsGrepCommand(QEmacsWidget& p)
    : QEmacsCommand(p)
  {} // end of QEmacsGrepCommand::QEmacsGrepCommand
  
  void QEmacsGrepCommand::execute()
  {
    auto *p = qobject_cast<QEmacsWidget *>(this->parent());
    if(p==nullptr){
      return;
    }
    auto *l = new QEmacsShellProcessLineEdit(
        "grep command :", "grep -nH -e ", "grep-output", *p);
    l->setInputHistorySettingAddress("command/grep/history");
    p->setUserInput(l);
  } // end of QEmacsGrepCommand::execute
    
  QEmacsGrepCommand::~QEmacsGrepCommand() = default;

  static StandardQEmacsCommandProxy<QEmacsGrepCommand> proxy("grep");

} // end of namespace qemacs
