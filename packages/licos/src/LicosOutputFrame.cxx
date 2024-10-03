/*!
 * \file  LicosOutputFrame.cxx
 * \brief
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#include <QtCore/QDebug>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/LicosStudyThread.hxx"
#include "TFEL/GUI/LicosOutputFrame.hxx"

namespace tfel::gui {

  LicosOutputFrame::LicosOutputFrame(EditorWidget &w,
                                     Buffer &b,
                                     const QString &f,
                                     const LicosStudyOptions &options)
      : QWidget(&w), textEdit(new PlainTextEdit(w, b)) {
    auto *hl = new QHBoxLayout;
    auto *vl = new QVBoxLayout;
    hl->addWidget(this->textEdit);
    this->pbar = new QProgressBar;
    this->pbar->setMaximum(100);
    this->textEdit->setReadOnly(true);
    //    vl->setMargin(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    vl->addLayout(hl);
    vl->addWidget(pbar);
    this->setLayout(vl);
    this->licos = new LicosStudyThread(f, options, QStringList(), this);
    QObject::connect(this->licos, &LicosStudyThread::newProcessOutput, this,
                     &LicosOutputFrame::displayProcessOutput);
    QObject::connect(this->licos, &LicosStudyThread::newPeriod, this,
                     &LicosOutputFrame::newPeriod);
    QObject::connect(this->licos, &LicosStudyThread::finished, this,
                     &LicosOutputFrame::studyFinished);
    this->licos->start();
  }  // end of LicosOutputFrame::LicosOutputFrame

  void LicosOutputFrame::displayProcessOutput(QString o) {
    this->textEdit->appendPlainText(o);
  }

  void LicosOutputFrame::newPeriod(int s) { this->pbar->setValue(s); }

  void LicosOutputFrame::studyFinished(bool s, QString e) {
    if (s) {
      this->textEdit->appendHtml("<font color=blue><b>success</b></font>");
    }
    if (!e.isEmpty()) {
      QStringList errors = e.split("\n");
      foreach (const QString &error, errors) {
        this->textEdit->appendHtml("<font color=red><b>" + error +
                                   "</b></font>");
      }
    }
    this->pbar->hide();
    this->licos->deleteLater();
    this->licos = nullptr;
    emit finished(s, e);
  }

  void LicosOutputFrame::closeEvent(QCloseEvent *) {
    if (this->licos != nullptr) {
      this->licos->deleteLater();
    }
  }  // end of LicosOutputFrame::closeEvent

}  // end of namespace tfel::gui
