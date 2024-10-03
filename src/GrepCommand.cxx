/*!
 * \file  GrepCommand.cxx
 * \brief
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#include "TFEL/GUI/Command.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/CommandFactory.hxx"
#include "TFEL/GUI/GrepCommand.hxx"

namespace tfel::gui {

  GrepCommand::GrepCommand(EditorWidget &p)
      : Command(p) {}  // end of GrepCommand

  void GrepCommand::execute() {
    auto *p = qobject_cast<EditorWidget *>(this->parent());
    if (p == nullptr) {
      return;
    }
    auto *l = new ShellProcessLineEdit("grep command :", "grep -nH -e ",
                                       "grep-output", *p);
    l->setInputHistorySettingAddress("command/grep/history");
    p->setUserInput(l);
  }  // end of execute

  GrepCommand::~GrepCommand() = default;

  static StandardCommandProxy<GrepCommand> proxy("grep");

}  // end of namespace tfel::gui
