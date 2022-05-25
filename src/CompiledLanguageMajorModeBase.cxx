/*!
 * \file  CompiledLanguageMajorModeBase.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/TextEdit.hxx"
#include "TFEL/GUI/CompiledLanguageMajorModeBase.hxx"

namespace tfel::gui {

  CompiledLanguageMajorModeBase::CompiledLanguageMajorModeBase(EditorWidget& w,
                                                               Buffer& b,
                                                               TextEditBase& t)
      : MajorModeBase(w, b, t, &t) {}  // end of CompiledLanguageMajorModeBase

  QString CompiledLanguageMajorModeBase::getDefaultCompilationCommand() const {
    return "make";
  }  // end of getDefaultCompilationCommand

  void CompiledLanguageMajorModeBase::runCompilation() {
    // choosing the default command
    QString d;
    QSettings s;
    auto ch = s.value(this->getLanguageName() + "/compilation/history")
                  .toStringList();
    if (ch.isEmpty()) {
      d = this->getDefaultCompilationCommand();
    } else {
      d = ch.back();
    }
    auto* l = new ShellProcessLineEdit("compilation command :", d,
                                       "compilation-output", this->editor);
    l->setInputHistorySettingAddress(this->getLanguageName() +
                                     "/compilation/history");
    this->editor.setUserInput(l);
  }  // end of runCompilation

  CompiledLanguageMajorModeBase::~CompiledLanguageMajorModeBase() = default;

}  // end of namespace tfel::gui
