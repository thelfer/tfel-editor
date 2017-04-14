/*! 
 * \file  QEmacsGrepCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#include"QEmacs/QEmacsCommand.hxx"
#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsCommandFactory.hxx"

#include"QEmacs/QEmacsGrepCommand.hxx"

namespace qemacs
{
  
  QEmacsGrepCommand::QEmacsGrepCommand(QEmacsWidget& p)
    : QEmacsCommand(p)
  {} // end of QEmacsGrepCommand::QEmacsGrepCommand
  
  void
  QEmacsGrepCommand::execute(void)
  {
    QEmacsWidget *p = qobject_cast<QEmacsWidget *>(this->parent());
    if(p==nullptr){
      return;
    }
    QEmacsLineEdit * l = new QEmacsShellProcessLineEdit("grep command :",
							"grep -nH -e ",
							"grep output",*p);
    l->setInputHistorySettingAddress("command/grep/history");
    p->setUserInput(l);
  } // end of QEmacsGrepCommand::execute
    
  QEmacsGrepCommand::~QEmacsGrepCommand()
  {} // end of ~QEmacsGrepCommand

  static StandardQEmacsCommandProxy<QEmacsGrepCommand> proxy("grep");

} // end of namespace qemacs
