/*!
 * \file   ProcessOutputMajorModeBase.cxx
 * \brief
 * \author Thomas Helfer
 * \date   7/02/2018
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtGui/QKeyEvent>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"

namespace tfel {

  namespace gui {

    ProcessOutputMajorModeBase::ProcessOutputMajorModeBase(EditorWidget& w,
                                                           Buffer& b,
                                                           TextEditBase& t,
                                                           QWidget* const p)
        : MajorModeBase(w, b, t, p) {}
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

    void ProcessOutputMajorModeBase::completeContextMenu(
        QMenu* const m, const QTextCursor& tc) {
      MajorModeBase::completeContextMenu(m, tc);
      auto* a = m->addAction("Restart");
      a->setStatusTip(tr("Restart the process"));
      a->setIcon(QIcon(":/tfel/editor/restart.png"));
      a->setIconVisibleInMenu(true);
      QObject::connect(a, &QAction::triggered, this,
                       &ProcessOutputMajorModeBase::restart);
      auto* s = m->addAction("Stop");
      s->setStatusTip(tr("Stop the process"));
      s->setIcon(QIcon::fromTheme("process-stop"));
      s->setIconVisibleInMenu(true);
      QObject::connect(s, &QAction::triggered, this,
                       &ProcessOutputMajorModeBase::stop);
    }  // end of void ProcessOutputMajorModeBase::completeContextMenu

    bool ProcessOutputMajorModeBase::keyPressEvent(QKeyEvent* const e) {
      const auto k = e->key();
      const auto m = e->modifiers();
      if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
        this->restart();
        return true;
      }
      return MajorModeBase::keyPressEvent(e);
    }  // end of ProcessOutputMajorModeBase::keyPressEvent

    void ProcessOutputMajorModeBase::restart() {
      auto* po = qobject_cast<ProcessOutputFrame*>(&(this->textEdit));
      if (po == nullptr) {
        return;
      }
      auto& p = po->getProcess();
      if (p.state() != QProcess::NotRunning) {
        this->report("process already running");
        return;
      }
      if (this->command.isEmpty()) {
        this->report("empty command");
        return;
      }
      this->textEdit.clear();
      if (!this->arguments.empty()) {
        p.start(this->command, this->arguments);
      } else {
        p.start(this->command);
      }
      this->editor.setSecondaryTaskIcon(po, QIcon::fromTheme("system-run"));
    }  // end of ProcessOutputMajorModeBase::restart

    void ProcessOutputMajorModeBase::stop() {
      auto* po = qobject_cast<ProcessOutputFrame*>(&(this->textEdit));
      if (po == nullptr) {
        return;
      }
      auto& p = po->getProcess();
      if (p.state() != QProcess::Running) {
        this->report("process not running");
        return;
      }
      p.terminate();
    }  // end of ProcessOutputMajorModeBase::stop

    ProcessOutputMajorModeBase::~ProcessOutputMajorModeBase() = default;

  }  // end of namespace gui
}  // end of namespace tfel
