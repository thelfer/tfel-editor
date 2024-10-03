/*!
 * \file  ProcessOutputFrame.cxx
 * \brief
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#include <QtCore/QDebug>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"

namespace tfel::gui {

  ProcessOutputFrame::ProcessOutputFrame(EditorWidget& w, Buffer& b)
      : PlainTextEdit(w, b), process(new QProcess(this)) {
    using QProcessFinished = void (QProcess::*)(int, QProcess::ExitStatus);
    PlainTextEdit::setReadOnly(true);
    PlainTextEdit::setUndoRedoEnabled(false);
    this->process->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(this->process, &QProcess::readyReadStandardOutput, this,
                     &ProcessOutputFrame::displayProcessOutput);
    QObject::connect(this->process,
                     static_cast<QProcessFinished>(&QProcess::finished), this,
                     &ProcessOutputFrame::processFinished);
  }

  void ProcessOutputFrame::setReadOnly(bool) {}  // end of setReadOnly

  void ProcessOutputFrame::displayProcessOutput() {
    QObject::disconnect(this->process, &QProcess::readyReadStandardOutput, this,
                        &ProcessOutputFrame::displayProcessOutput);
    auto out = this->process->readAll();
    this->appendPlainText(out);
    QObject::connect(this->process, &QProcess::readyReadStandardOutput, this,
                     &ProcessOutputFrame::displayProcessOutput);
  }  // end of displayProcessOutput

  void ProcessOutputFrame::processFinished(int s, QProcess::ExitStatus es) {
    if (s == 0) {
      this->editor.setSecondaryTaskIcon(this,
                                        QIcon(":/tfel/editor/success.png"));
    } else {
      this->editor.setSecondaryTaskIcon(this,
                                        QIcon(":/tfel/editor/failure.png"));
    };
    auto pom = qobject_cast<ProcessOutputMajorModeBase*>(this->mode);
    if (pom != nullptr) {
      pom->processFinished(s, es);
    }
  }  // end of processFinished

  QProcess& ProcessOutputFrame::getProcess() {
    return *(this->process);
  }  // end of getProcess

  ProcessOutputFrame::~ProcessOutputFrame() {
    using QProcessFinished = void (QProcess::*)(int, QProcess::ExitStatus);
    QObject::disconnect(this->process,
                        static_cast<QProcessFinished>(&QProcess::finished),
                        this, &ProcessOutputFrame::processFinished);
    if (this->process->state() == QProcess::Running) {
      this->process->terminate();
      if (!this->process->waitForFinished()) {
        this->process->kill();
      }
    }
  }  // end of ~ProcessOutputFrame()

}  // end of namespace tfel::gui
