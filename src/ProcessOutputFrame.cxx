/*!
 * \file  ProcessOutputFrame.cxx
 * \brief
 * \author Helfer Thomas
 * \date   01/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextDecoder>
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
#ifdef Q_OS_WIN
    auto* codec = QTextCodec::codecForName("cp850");
#else  /*  Q_OS_WIN */
    auto* codec = QTextCodec::codecForLocale();
#endif /*  Q_OS_WIN */
    auto* decoder = codec->makeDecoder();
    this->appendPlainText(decoder->toUnicode(out));
    delete decoder;
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
