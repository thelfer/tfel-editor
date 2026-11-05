/*!
 * \file  LspTransport.cxx
 * \brief process of a language server and the framed byte stream it speaks.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtCore/QProcess>

#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/LspTransport.hxx"

namespace tfel::gui {

  LspTransport::LspTransport(QObject *const p)
      : QObject(p), process(new QProcess(this)) {
    this->process->setProcessChannelMode(QProcess::SeparateChannels);
    QObject::connect(this->process, &QProcess::readyReadStandardOutput, this,
                     &LspTransport::handleReadyReadStandardOutput);
    QObject::connect(this->process, &QProcess::readyReadStandardError, this,
                     &LspTransport::handleReadyReadStandardError);
    QObject::connect(this->process,
                     qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
                     this, &LspTransport::handleFinished);
    QObject::connect(this->process, &QProcess::errorOccurred, this,
                     [this](const QProcess::ProcessError e) {
                       if (e == QProcess::FailedToStart) {
                         emit failedToStart();
                       }
                     });
  }  // end of LspTransport::LspTransport

  LspTransport::~LspTransport() { this->terminate(); }  // end of ~LspTransport

  bool LspTransport::start(const QString &program,
                           const QStringList &arguments) {
    if (this->process->state() != QProcess::NotRunning) {
      return false;
    }
    this->in_buffer.clear();
    this->err_buffer.clear();
    this->process->start(program, arguments);
    return this->process->waitForStarted(5000);
  }  // end of LspTransport::start

  bool LspTransport::isRunning() const {
    return this->process->state() != QProcess::NotRunning;
  }  // end of LspTransport::isRunning

  void LspTransport::send(const QJsonObject &m) {
    if (!this->isRunning()) {
      return;
    }
    const auto framed = LspProtocol::encode(m);
    this->process->write(framed);
  }  // end of LspTransport::send

  void LspTransport::terminate(const int grace_period_ms) {
    if (this->process->state() == QProcess::NotRunning) {
      return;
    }
    this->process->terminate();
    if (!this->process->waitForFinished(grace_period_ms)) {
      this->process->kill();
      this->process->waitForFinished(1000);
    }
  }  // end of LspTransport::terminate

  void LspTransport::handleReadyReadStandardOutput() {
    this->in_buffer += this->process->readAllStandardOutput();
    for (const auto &m : LspProtocol::decode(this->in_buffer)) {
      emit messageReceived(m);
    }
  }  // end of LspTransport::handleReadyReadStandardOutput

  void LspTransport::handleReadyReadStandardError() {
    this->err_buffer += this->process->readAllStandardError();
    auto i = this->err_buffer.indexOf('\n');
    while (i >= 0) {
      auto line = this->err_buffer.left(i);
      this->err_buffer.remove(0, i + 1);
      if (line.endsWith('\r')) {
        line.chop(1);
      }
      emit serverMessage(QString::fromLocal8Bit(line));
      i = this->err_buffer.indexOf('\n');
    }
  }  // end of LspTransport::handleReadyReadStandardError

  void LspTransport::handleFinished(const int exit_code, QProcess::ExitStatus) {
    // the bytes the server wrote before exiting are not gone from the socket
    // yet: a last read lets the pending requests be answered, or at least
    // tells the connection that the server died with something on its hands
    this->handleReadyReadStandardOutput();
    emit finished(exit_code);
  }  // end of LspTransport::handleFinished

}  // end of namespace tfel::gui
