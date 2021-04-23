/*!
 * \file   ShellProcessLineEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"

namespace tfel {

  namespace gui {

    ShellProcessLineEdit::ShellProcessLineEdit(const QString& l,
                                               const QString& c,
                                               const QString& m,
                                               EditorWidget& p)
        : ProcessLineEdit(l, c, m, p) {
    }  // end of ShellProcessLineEdit::ShellProcessLineEdit

    void ShellProcessLineEdit::treatUserInput() {
      const auto c = this->input->text();
      if (c.isEmpty()) {
        this->editor.displayInformativeMessage(QObject::tr("empty command"));
        return;
      }
#ifdef Q_OS_WIN
      this->run(c, "cmd.exe", QStringList() << "/c" << c);
#else  /*  Q_OS_WIN */
      const auto* s = ::getenv("SHELL");
      if (s == nullptr) {
        this->editor.displayInformativeMessage(QObject::tr("no shell defined"));
      }
      this->run(c, s, QStringList() << "-c" << c);
#endif /* Q_OS_WIN */
    }  // end of ShellProcessLineEdit::treatUserInput

    ShellProcessLineEdit::~ShellProcessLineEdit() = default;

  }  // end of namespace gui
}  // end of namespace tfel
