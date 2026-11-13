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

  bool ProcessOutputFrame::displaysCompilationOutput() const {
    // the name of the mode is the only thing which tells the output of a
    // compilation from the output of any other command: the modes which display
    // a process output are not exported and the frame holds none of them
    return this->hasMajorMode() &&
           (this->getMajorMode().getName() == "compilation-output");
  }  // end of displaysCompilationOutput

  ProcessOutputFrame::~ProcessOutputFrame() {
    using QProcessFinished = void (QProcess::*)(int, QProcess::ExitStatus);
    QObject::disconnect(this->process,
                        static_cast<QProcessFinished>(&QProcess::finished),
                        this, &ProcessOutputFrame::processFinished);
    if (this->process->state() != QProcess::NotRunning) {
      // a process which was only asked to start is `Starting`, and it is as
      // much this frame's child as one which is up already: left alone, it is
      // killed by the destructor of the process itself, with a complaint on the
      // standard error. `terminate` reaches it in that state too.
      this->process->terminate();
      if (!this->process->waitForFinished()) {
        this->process->kill();
      }
    }
  }  // end of ~ProcessOutputFrame()

}  // end of namespace tfel::gui
