/*!
 * \file  LicosStudyThread.cxx
 * \brief
 * \author Thomas Helfer
 * \date   23/05/2012
 */

#include <QtCore/QDebug>

#include "TFEL/GUI/LicosStudy.hxx"
#include "TFEL/GUI/LicosStudyThread.hxx"
#include "TFEL/GUI/LicosOutputFrame.hxx"

namespace tfel::gui {

  LicosStudyThread::LicosStudyThread(const QString& f,
                                     const LicosStudyOptions& o,
                                     const QStringList& a,
                                     LicosOutputFrame* const p)
      : QThread(p), inputFile(f), args(a), options(o) {}

  void LicosStudyThread::run() {
    LicosStudy s(this->inputFile, this->options, this->args);
    this->study = &s;
    QObject::connect(&s, &LicosStudy::newProcessOutput, this,
                     &LicosStudyThread::forwardProcessOutput);
    QObject::connect(&s, &LicosStudy::newPeriod, this,
                     &LicosStudyThread::forwardNewPeriod);
    QObject::connect(&s, &LicosStudy::finished, this,
                     &LicosStudyThread::studyFinished);
    QObject::connect(this, &LicosStudyThread::killProcess, &s,
                     &LicosStudy::stopComputations);
    s.run();
    this->exec();
  }

  void LicosStudyThread::forwardProcessOutput(QString o) {
    emit newProcessOutput(o);
  }

  void LicosStudyThread::forwardNewPeriod(int s) { emit newPeriod(s); }

  void LicosStudyThread::studyFinished() {
    this->success = this->study->succeed();
    this->errorMessage = this->study->getErrorMessage();
    this->quit();
    emit finished(this->success, this->errorMessage);
  }

  bool LicosStudyThread::succeed() const { return this->success; }

  QString LicosStudyThread::getErrorMessage() const {
    return this->errorMessage;
  }  // end of LicosStudyThread::getErrorMessage

  LicosStudyThread::~LicosStudyThread() {
    if (!this->isFinished()) {
      this->quit();
      this->wait();
      emit killProcess();
    }
  }  // end of LicosStudyThread::~LicosStudyThread()

}  // end of namespace tfel::gui
