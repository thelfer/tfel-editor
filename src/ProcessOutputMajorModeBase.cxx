/*!
 * \file   ProcessOutputMajorModeBase.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   7/02/2018
 */

#include<QtCore/QDebug>
#include<QtCore/QDir>
#include<QtGui/QKeyEvent>
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"
#include"QEmacs/ProcessOutputMajorModeBase.hxx"

namespace qemacs{

  ProcessOutputMajorModeBase::ProcessOutputMajorModeBase(
      QEmacsWidget& w,
      QEmacsBuffer& b,
      QEmacsTextEditBase& t,
      QWidget* const p)
      : QEmacsMajorModeBase(w, b, t, p) {}
  // end of ProcessOutputMajorModeBase::ProcessOutputMajorModeBase

  void ProcessOutputMajorModeBase::setDirectory(const QString& c) {
    this->directory = c;
  }  // end of ProcessOutputMajorModeBase::setDirectory

  void ProcessOutputMajorModeBase::setCommand(const QString& c) {
    this->command = c;
  }  // end of ProcessOutputMajorModeBase::setCommand

  void ProcessOutputMajorModeBase::setArguments(const QStringList& a) {
    this->arguments = a;
  }  // end of ProcessOutputMajorModeBase::setArguments

  void ProcessOutputMajorModeBase::setMajorMode(const QString& m) {
    this->mode = m;
  }  // end of ProcessOutputMajorModeBase::setMajorMode

  bool ProcessOutputMajorModeBase::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_M) &&
        (!this->command.isEmpty())) {
      auto& b = this->qemacs.getCurrentBuffer();
      auto& t = b.getMainFrame();
      auto* po = new ProcessOutputFrame(this->qemacs, b);
      QDir d(this->directory);
      auto& p = po->getProcess();
      if (d.exists()) {
        p.setWorkingDirectory(d.absolutePath());
      } else {
        p.setWorkingDirectory(QDir::current().absolutePath());
      }
      if (!this->mode.isEmpty()) {
        po->setMajorMode(this->mode);
      }
      p.start(this->command,this->arguments);
      b.attachSecondaryTask("*" + this->command + "* ouput", po);
      return true;
    }
    return false;
  }  // end of ProcessOutputMajorModeBase::keyPressEvent

  ProcessOutputMajorModeBase::~ProcessOutputMajorModeBase() = default;

}  // end of namespace qemacs
