/*! 
 * \file  LicosStudy.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 20 mai 2012
 */

#include<QtCore/QDir>
#include<QtCore/QDebug>
#include<QtCore/QFileInfo>
#include<QtCore/QTextCodec>
#include<QtCore/QTextDecoder>

#include"QEmacs/LicosStudy.hxx"

namespace qemacs
{

  LicosStudy::LicosStudy(const QString& f,
			 const LicosStudyOptions& o,
			 const QStringList& a)
    : server(new QLocalServer(this)),
      in(new QLocalSocket(this)),
      out(nullptr),
      process(new QProcess(this)),
      inputFile(f),
      args(a),
      options(o)
  {}

  void
  LicosStudy::run()
  {
    QObject::connect(this->in, SIGNAL(error(QLocalSocket::LocalSocketError)),
		     this, SLOT(displayInputSocketError(QLocalSocket::LocalSocketError)));
    QObject::connect(this->process,SIGNAL(error(QProcess::ProcessError)),
		     this,SLOT(processError(QProcess::ProcessError)));
    QObject::connect(this->process,SIGNAL(finished(int,QProcess::ExitStatus)),
		     this,SLOT(processFinished(int,QProcess::ExitStatus)));
    QObject::connect(this->server, SIGNAL(newConnection()),
    		     this, SLOT(processInitialised()));
    QObject::connect(this->process,SIGNAL(readyReadStandardOutput()),
		     this,SLOT(displayProcessOutput()));
    // this is the input for the sub-process (and the output of
    // this...), hence the name
    QString sname("licos.in");
    if(!this->server->listen(sname)){
      this->fails(tr("Unable to start the server '%1': %2.")
    		  .arg(sname)
    		  .arg(this->server->errorString()));
      return;
    } 
    QFileInfo fn(this->inputFile);
    this->process->setWorkingDirectory(fn.dir().absolutePath());
    this->process->start("xlicos-client",QStringList() << "licos");
  } // end of LicosStudy::run

  void
  LicosStudy::processInitialised(){
    QObject::disconnect(this->server, SIGNAL(newConnection()),
			this, SLOT(processInitialised()));
    QString sname("licos.out");
    this->out = this->server->nextPendingConnection();
    QObject::connect(this->out, SIGNAL(error(QLocalSocket::LocalSocketError)),
		     this, SLOT(displayOutputSocketError(QLocalSocket::LocalSocketError)));
    this->in->connectToServer(sname,QIODevice::ReadOnly);
    if(this->in->state()!=QLocalSocket::ConnectedState){
      this->fails(tr("Unable to connect to socket '%1'")
    		  .arg(sname));
      return;
    }
    QString m = this->receive<QString>();
    if(m!="ok"){
      this->fails(QObject::tr("unexpected child message (expected 'ok', "
    			      "read '%1')").arg(m));
      return;
    }
    this->inServer = this->receive<QString>();
    // treating options
    QObject::connect(this->in, SIGNAL(readyRead()),
		     this, SLOT(processReachedNextStage()));
    this->send("initialize");
    this->sendOption("VerboseLevel",this->options.vlvl);
    this->sendOption("WarningLevel",this->options.wlvl);
    this->sendOption("allowMissingDependencies",
		     this->options.allowMissingDependencies);
    this->sendOption("fpe",this->options.fpe);
    this->sendOption("printBackTrace",this->options.printBackTrace);
    this->sendOption("debugMode",this->options.debugMode);
    this->send("inputFile");
    this->send(this->inputFile);
  }

  void
  LicosStudy::processReachedNextStage(){
    QObject::disconnect(this->in, SIGNAL(readyRead()),
			this, SLOT(processReachedNextStage()));
    QString s = this->receive<QString>();
    if(s=="readInputFile"){
      this->send("run");
    } else if(s=="newPeriod"){
      // current state
      const int cs = this->receive<QString>().toInt();
      emit newPeriod(cs);
      this->send("continue");
    } else if(s=="finished"){
      emit newPeriod(100);
      this->send("end");
      this->success = true;
      return this->quit();
    } else if(s=="fails"){
      QString msg = this->receive<QString>();
      this->fails(msg);
    } else {
      this->fails(tr("Unknonw process stage '%1'").arg(s));
    }
    QObject::connect(this->in, SIGNAL(readyRead()),
		     this, SLOT(processReachedNextStage()));
  }

  void
  LicosStudy::sendOption(const QString& o,
			 const QString& v)
  {
    this->send("option");
    this->send(o);
    this->send(v);
  }

  void
  LicosStudy::send(const char *const m)
  {
    QString msg(m);
    this->send(msg);
  }

  void
  LicosStudy::send(const QString& msg)
  {
    QByteArray block;
    QDataStream o(&block, QIODevice::WriteOnly);
    o.setVersion(QDataStream::Qt_4_0);
    o << static_cast<quint16>(0);
    o << msg;
    o.device()->seek(0);
    o << static_cast<quint16>(block.size() - sizeof(quint16));
    this->out->write(block);
    this->out->flush();
  }
  
  template<>
  QString
  LicosStudy::receive<QString>()
  {
    QDataStream id(this->in);
    id.setVersion(QDataStream::Qt_4_0);
    quint16 blockSize;
    while(this->in->bytesAvailable()==0){
      this->in->waitForReadyRead(10);
    }
    if (this->in->bytesAvailable() < static_cast<int>(sizeof(quint16))){
      this->fails("invalid message");
    }
    id >> blockSize;
    if (id.atEnd()){
      this->fails("invalid message");
    }
    QString m;
    id >> m;
    return m;
  }

  void
  LicosStudy::displayProcessOutput()
  {
    QByteArray data = this->process->readAllStandardOutput();
    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextDecoder *decoder = codec->makeDecoder();
    emit newProcessOutput(decoder->toUnicode(data));
    delete decoder;
  } // end of LicosStudy::displayProcessOutput

  void
  LicosStudy::displayInputSocketError(QLocalSocket::LocalSocketError e)
  {
    this->displaySocketError(this->in,e);
  }

  void
  LicosStudy::displayOutputSocketError(QLocalSocket::LocalSocketError e)
  {
    this->displaySocketError(this->out,e);
  }

  void
  LicosStudy::displaySocketError(QLocalSocket *s,
				 QLocalSocket::LocalSocketError e)
  {
    QString msg;
    if(e== QLocalSocket::SocketTimeoutError){
      return;
    }
    switch (e) {
    case QLocalSocket::ServerNotFoundError:
      msg = tr("The host was not found. Please check the "
	       "host name and port settings.");
      break;
    case QLocalSocket::ConnectionRefusedError:
      msg = tr("The connection was refused by the peer. "
	       "Make sure the fortune server is running, "
	       "and check that the host name and port "
	       "settings are correct.");
      break;
    default:
      msg = tr("The following error occurred: %1.")
	.arg(s->errorString());
    }
    this->fails(msg);
  }

  void
  LicosStudy::processError(QProcess::ProcessError e)
  {
    QString m;
    switch(e){
    case QProcess::FailedToStart:
      this->fails(tr("Unable to start the process '%1'.")
		  .arg("xlicos-client"));
      break;
    case QProcess::Crashed:
      if(m.isEmpty()){
	this->fails(tr("Process '%1' crashed.")
		    .arg("xlicos-client"));
      } else {
	this->fails(tr("Process '%1' crashed : %2")
		    .arg("xlicos-client")
		    .arg(m));
      }
      break;
    case QProcess::Timedout:
      this->fails(tr("Process '%1' is not responding.")
		  .arg("xlicos-client"));
      break;
    case QProcess::WriteError:
    case QProcess::ReadError:
    case QProcess::UnknownError:
    default:
      this->fails(tr("Process '%1' failed.")
		  .arg("xlicos-client"));
    }
  }

  void
  LicosStudy::fails(const QString& msg)
  {
    this->success = false;
    this->errorMessage = msg;
    QString m = this->process->readAllStandardError();
    if(!m.isEmpty()){
      this->errorMessage += "\n"+m;
    }
    return this->quit();
  } // end of LicosStudy::fails

  void
  LicosStudy::quit()
  {
    if(this->in!=nullptr){
      QObject::disconnect(this->in, SIGNAL(error(QLocalSocket::LocalSocketError)),
			  this, SLOT(displayInputSocketError(QLocalSocket::LocalSocketError)));
    }
    if(this->out!=nullptr){
      QObject::disconnect(this->out, SIGNAL(error(QLocalSocket::LocalSocketError)),
			  this, SLOT(displayOutputSocketError(QLocalSocket::LocalSocketError)));
    }
    if(this->process!=nullptr){
      QObject::disconnect(this->process,SIGNAL(error(QProcess::ProcessError)),
			  this,SLOT(processError(QProcess::ProcessError)));
      QObject::disconnect(this->process,SIGNAL(finished(int,QProcess::ExitStatus)),
			  this,SLOT(processFinished(int,QProcess::ExitStatus)));
    }

    emit finished();
    return;
  }

  void
  LicosStudy::processFinished(int s,
			      QProcess::ExitStatus)
  {
    if(s!=0){
      this->fails(tr("xlicos-client failed"));
      this->success = false;
    } else {
      this->success = true;
    }
    
  }

  QString
  LicosStudy::getErrorMessage() const
  {
    return this->errorMessage;
  }

  bool
  LicosStudy::succeed() const
  {
    return this->success;
  } // end of LicosStudy::succeed

  void
  LicosStudy::stopComputations()
  {
    this->in->disconnectFromServer();
    if(this->process->state()==QProcess::Running){
      this->send("end");
      this->process->waitForFinished(300);
      if(this->process->state()==QProcess::Running){
	this->process->terminate();
      }
    }
    if(this->out!=nullptr){
      this->out->disconnectFromServer();
    }
    if(!this->inServer.isEmpty()){
      QFile f(this->inServer);
      f.remove();
    }
    this->server->close();
  }

  LicosStudy::~LicosStudy()
  {
    this->stopComputations();
  } // end of LicosStudy::~LicosStudy

} // end of namespace qemacs
