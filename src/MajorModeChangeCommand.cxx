/*!
 * \file  MajorModeChangeCommand.cxx
 * \brief
 * \author Thomas Helfer
 * \date   30/09/2012
 */

#include "TFEL/GUI/Command.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/CommandFactory.hxx"

#include "TFEL/GUI/MajorModeChangeCommand.hxx"

namespace tfel {

  namespace gui {

    MajorModeChangeCommand::MajorModeChangeCommand(EditorWidget& p,
                                                   const QString& m)
        : Command(p), mode(m) {}  // end of
    // MajorModeChangeCommand::MajorModeChangeCommand

    void MajorModeChangeCommand::execute() {
      auto* p = qobject_cast<EditorWidget*>(this->parent());
      if (p != nullptr) {
        auto& b = p->getCurrentBuffer();
        auto& t = b.getMainFrame();
        t.setMajorMode(this->mode);
      }
    }  // end of MajorModeChangeCommand::execute

    MajorModeChangeCommand::~MajorModeChangeCommand() = default;

    MajorModeChangeCommandProxy::MajorModeChangeCommandProxy(const QString& c,
                                                             const QString& m)
        : name(c), mode(m) {}  // end of
    // MajorModeChangeCommandProxy::MajorModeChangeCommandProxy

    QString MajorModeChangeCommandProxy::getName() const { return this->name; }

    Command* MajorModeChangeCommandProxy::getCommand(EditorWidget& t) const {
      return new MajorModeChangeCommand(t, this->mode);
    }

    MajorModeChangeCommandProxy::~MajorModeChangeCommandProxy() = default;

  }  // end of namespace gui
}  // end of namespace tfel
