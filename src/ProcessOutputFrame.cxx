/*! 
 * \file  ProcessOutputFrame.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#include<QtCore/QDebug>

#include<QtCore/QTextCodec>
#include<QtCore/QTextDecoder>

#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"

namespace qemacs
{
  
  ProcessOutputFrame::ProcessOutputFrame(QEmacsWidget& w,
					 QEmacsBuffer& b)
    : QEmacsPlainTextEdit(w,b),
      process(new QProcess(this))
  {
    QEmacsPlainTextEdit::setReadOnly(true); 
    this->process->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(this->process,SIGNAL(readyReadStandardOutput()),
		     this,SLOT(displayProcessOutput()));
    QObject::connect(this->process,SIGNAL(finished(int,QProcess::ExitStatus)),
		     this,SLOT(processFinished(int,QProcess::ExitStatus)));
  }

  void 
  ProcessOutputFrame::setReadOnly(bool)
  {} // end of ProcessOutputFrame::setReadOnly

  void
  ProcessOutputFrame::displayProcessOutput()
  {
    QObject::disconnect(this->process,SIGNAL(readyReadStandardOutput()),
			this,SLOT(displayProcessOutput()));
    QByteArray out = this->process->readAll();
    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextDecoder *decoder = codec->makeDecoder();
    this->appendPlainText(decoder->toUnicode(out));
    delete decoder;
    QObject::connect(this->process,SIGNAL(readyReadStandardOutput()),
		     this,SLOT(displayProcessOutput()));
  } // end of ProcessOutputFrame::displayProcessOutput

  void
  ProcessOutputFrame::processFinished(int s,QProcess::ExitStatus)
  {
    if(s==0){
      this->buffer.setSlaveIcon(this, QIcon(":/QEmacsSuccessIcon.png"));
    } else {
      this->buffer.setSlaveIcon(this, QIcon(":/QEmacsFailureIcon.png"));
    }
  } // end of ProcessOutputFrame::processFinished

  QProcess&
  ProcessOutputFrame::getProcess()
  {
    return *(this->process);
  } // end of ProcessOutputFrame::getProcess

  ProcessOutputFrame::~ProcessOutputFrame()
  {
    if(this->process->state()==QProcess::Running){
      this->process->terminate();
      if(!this->process->waitForFinished()){
	this->process->kill();
      }
  }
  } // end of ProcessOutputFrame::~ProcessOutputFrame()

} // end of namespace qemacs