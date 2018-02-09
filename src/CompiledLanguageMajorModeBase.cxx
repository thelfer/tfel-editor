/*! 
 * \file  CompiledLanguageMajorModeBase.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 août 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QSettings>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsTextEdit.hxx"
#include"QEmacs/CompiledLanguageMajorModeBase.hxx"

namespace qemacs
{

  CompiledLanguageMajorModeBase::CompiledLanguageMajorModeBase(QEmacsWidget& w,
							       QEmacsBuffer& b,
							       QEmacsTextEditBase& t)
    : QEmacsMajorModeBase(w,b,t,&t)
  {} // end of CompiledLanguageMajorModeBase::CompiledLanguageMajorModeBase

  QString
  CompiledLanguageMajorModeBase::getDefaultCompilationCommand() const {
    return "make";
  } // end of CompiledLanguageMajorModeBase::getDefaultCompilationCommand

  void CompiledLanguageMajorModeBase::runCompilation() {
    // choosing the default command
    QString d;
    QSettings s;
    auto ch = s.value(this->getLanguageName() + "/compilation/history")
                  .toStringList();
    if(ch.isEmpty()){
      d = this->getDefaultCompilationCommand();
    } else {
      d = ch.back();
    }
    auto* l = new QEmacsShellProcessLineEdit(
        "compilation command :", d, "compilation output", this->qemacs);
    l->setInputHistorySettingAddress(this->getLanguageName()+"/compilation/history");
    this->qemacs.setUserInput(l);
  } // end of CompiledLanguageMajorModeBase::runCompilation
  
  CompiledLanguageMajorModeBase::~CompiledLanguageMajorModeBase() = default;
  
} // end of namespace qemacs
