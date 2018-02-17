/*!
 * \file   QEmacsShellProcessLineEdit.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsShellProcessLineEdit.hxx"

namespace qemacs{

  QEmacsShellProcessLineEdit::QEmacsShellProcessLineEdit(
      const QString& l,
      const QString& c,
      const QString& m,
      QEmacsWidget& p)
      : QEmacsProcessLineEdit(l, c, m, p) {
  }  // end of QEmacsShellProcessLineEdit::QEmacsShellProcessLineEdit

  void QEmacsShellProcessLineEdit::treatUserInput() {
    const auto c = this->input->text();
    if (c.isEmpty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("empty command"));
      return;
    }
#ifdef Q_OS_UNIX
    const auto* s = ::getenv("SHELL");
    if (s == nullptr) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("no shell defined"));
    }
    this->run(c, s, QStringList() << "-c" << c);
#else  /*  Q_OS_UNIX */
    this->run("cmd.exe", s, QStringList() << "/c" << c);
#endif /* Q_OS_UNIX */
  }  // end of QEmacsShellProcessLineEdit::treatUserInput

  QEmacsShellProcessLineEdit::~QEmacsShellProcessLineEdit() = default;

} // end of namespace qemacs
