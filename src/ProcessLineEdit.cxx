/*!
 * \file   ProcessLineEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#include <QtCore/QDir>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"
#include "TFEL/GUI/ProcessLineEdit.hxx"

namespace tfel::gui {

  ProcessLineEdit::ProcessLineEdit(const QString& l,
                                   const QString& c,
                                   const QString& m,
                                   EditorWidget& p)
      : CommandLine(l, p), mode(m) {
    if (!c.isEmpty()) {
      this->input->setText(c);
    }
  }  // end ofProcessLineEdit

  void ProcessLineEdit::treatUserInput() {
    const auto c = this->input->text();
    auto args = c.split(" ", Qt::SkipEmptyParts);
    if (args.empty()) {
      this->editor.displayInformativeMessage(QObject::tr("empty command"));
      return;
    }
    const auto c0 = args.front();
    args.pop_front();
    this->run(c0, c0, args);
    return;
  }

  void ProcessLineEdit::run(const QString& on,
                            const QString& c,
                            const QStringList& args) {
    if (c.isEmpty()) {
      this->editor.displayInformativeMessage(QObject::tr("empty command"));
      return;
    }
    auto& b = this->editor.getCurrentBuffer();
    auto* po = new ProcessOutputFrame(this->editor, b);
    const auto dn = [&b] {
      QDir d(b.getMainFrame().getDirectory());
      if (d.exists()) {
        return d.absolutePath();
      }
      return QDir::current().absolutePath();
    }();
    // issue #72: the substitution below used to be dead code (`nargs` was
    // never read), and its old per-token form could not work as written:
    // `std::string(' ', n)` compiled as the fill constructor with swapped
    // arguments, the tokenizer splits "$(cmd)" and "%cmd%" across several
    // tokens, and a "." token cannot be told from the "." of "a.py". It
    // now substitutes whole arguments only, which keeps every shell
    // command line the subclass passes untouched.
    auto nargs = QStringList{};
    nargs.reserve(args.size());
    for (const auto& a : args) {
      if ((a == ".") || (a == "$(cmd)") || (a == "%cmd%")) {
        nargs << dn;
      } else {
        nargs << a;
      }
    }
    auto& p = po->getProcess();
    p.setWorkingDirectory(dn);
    if (!this->mode.isEmpty()) {
      auto* m = qobject_cast<ProcessOutputMajorModeBase*>(
          po->setMajorMode(this->mode));
      if (m != nullptr) {
        m->setDirectory(p.workingDirectory());
        m->setCommand(c);
        // issue #72: the directory substitution built `nargs` above and
        // then ignored it, so the documented replacement of ".", "$(cmd)"
        // and "%cmd%" silently did nothing
        m->setArguments(nargs);
        m->setMajorMode(this->mode);
      }
    }
    p.start(c, nargs);
    b.attachSecondaryTask("*" + on + "* output", po);
    b.setSecondaryTaskIcon(po, QIcon::fromTheme("system-run"));
  }  // end ofrun

  ProcessLineEdit::~ProcessLineEdit() = default;

}  // end of namespace tfel::gui
