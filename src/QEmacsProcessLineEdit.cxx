/*!
 * \file   QEmacsProcessLineEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#include <QtCore/QDir>
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/ProcessOutputMajorModeBase.hxx"
#include "QEmacs/QEmacsProcessLineEdit.hxx"

namespace qemacs {

  QEmacsProcessLineEdit::QEmacsProcessLineEdit(const QString& l,
                                               const QString& c,
                                               const QString& m,
                                               QEmacsWidget& p)
      : QEmacsLineEdit(l, p), mode(m) {
    if (!c.isEmpty()) {
      this->input->setText(c);
    }
  }  // end of QEmacsProcessLineEdit::QEmacsProcessLineEdit

  void QEmacsProcessLineEdit::treatUserInput() {
    const auto c = this->input->text();
    auto args = c.split(" ", QString::SkipEmptyParts);
    if (args.empty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("empty command"));
      return;
    }
    const auto c0 = args.front();
    args.pop_front();
    this->run(c0, c0, args);
    return;
  }

  void QEmacsProcessLineEdit::run(const QString& on,
                                  const QString& c,
                                  const QStringList& args) {
    if (c.isEmpty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("empty command"));
      return;
    }
    auto& b = this->qemacs.getCurrentBuffer();
    auto& t = b.getMainFrame();
    auto* po = new ProcessOutputFrame(this->qemacs, b);
    const auto dn = t.getDirectory();
    QDir d(dn);
    const auto de = d.exists();
    QStringList nargs;
    for (const auto& a : args) {
      const auto sargs =
          a.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      for (const auto& a2 : sargs) {
        auto na = a2.trimmed();
        if(de){
          if ((na == ".") || (na == "$(pwd)") || (na == "%cd%")) {
            na = dn;
          }
        }
        nargs << na;
      }
    }
    auto& p = po->getProcess();
    if (de) {
      p.setWorkingDirectory(d.absolutePath());
    } else {
      p.setWorkingDirectory(QDir::current().absolutePath());
    }
    if (!this->mode.isEmpty()) {
      auto* m = qobject_cast<ProcessOutputMajorModeBase*>(
          po->setMajorMode(this->mode));
      if (m != nullptr) {
        m->setDirectory(p.workingDirectory());
        m->setCommand(c);
        m->setArguments(nargs);
        m->setMajorMode(this->mode);
      }
    }
    p.start(c, nargs);
    b.attachSecondaryTask("*" + on + "* ouput", po);
    b.setSecondaryTaskIcon(po,QIcon::fromTheme("system-run"));
  }  // end of QEmacsProcessLineEdit::run

  QEmacsProcessLineEdit::~QEmacsProcessLineEdit() = default;

}  // end of namespace qemacs
