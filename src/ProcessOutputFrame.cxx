/*!
 * \file  ProcessOutputFrame.cxx
 * \brief
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#include <QtCore/QDebug>
#include <QtGui/QTextCursor>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/Utilities.hxx"

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
    // the bytes are appended as a *continuation* of the text, never as a new
    // paragraph. A program writes what it has to say and the places Qt reads the
    // pipe are none of its business: a program which ends a piece with something
    // which has no line break — an interpreter which wrote its `>>> ' and waits
    // for a line — says of that line that what comes next belongs to it. Cutting
    // a paragraph there moves the answer of the next thing sent to a line of its
    // own, under the prompt, which reads as the answer being the input.
    // `appendPlainText' is a new paragraph whatever the text before ends with.
    auto c = this->textCursor();
    // a reader who moved the cursor to a line of the output, to jump to a file
    // from a grep match for instance, is not dragged to the end by the next
    // piece; a reader whom the end of the output had on screen keeps having it
    const auto follows = c.atEnd();
    c.movePosition(QTextCursor::End);
    c.insertText(normalizeLineSeparators(QString::fromUtf8(out)));
    if (follows) {
      this->setTextCursor(c);
    }
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
