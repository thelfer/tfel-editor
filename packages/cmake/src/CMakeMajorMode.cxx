/*! 
 * \file  CMakeMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/QEmacsShellProcessLineEdit.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/CMakeSyntaxHighlighter.hxx"
#include "QEmacs/CMakeMajorMode.hxx"

namespace qemacs {

  CMakeMajorMode::CMakeMajorMode(QEmacsWidget& w,
                                 QEmacsBuffer& b,
                                 QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w, b, t, &t) {
  }  // end of CMakeMajorMode::CMakeMajorMode

  QString CMakeMajorMode::getName() const {
    return "CMake";
  } // end of CMakeMajorMode::CMakeMajorMode

  QString CMakeMajorMode::getDescription() const {
    return "major mode dedicated to CMake";
  } // end of CMakeMajorMode::CMakeMajorMode

  bool CMakeMajorMode::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      this->runCompilation();
      return true;
    }
    return false;
  } // end of CMakeMajorMode::keyPressEvent

  void CMakeMajorMode::completeContextMenu(QMenu *const m,
                                            const QTextCursor &tc) {
    QEmacsMajorModeBase::completeContextMenu(m, tc);
    const auto cmds = CMakeSyntaxHighlighter::getCMakeCommandsList();
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    b.select(QTextCursor::LineUnderCursor);
    QString l = b.selectedText();
    QRegExp r("^\\s*(\\w+)");
    if(r.indexIn(l) >= 0) {
      QString k = r.cap(1);
      if(cmds.indexOf(k) != -1) {
        delete this->ha;
        this->ha = new QAction(QObject::tr("Help on %1").arg(k), this);
        this->ha->setIcon(QIcon::fromTheme("dialog-question"));
        this->ha->setData(k);
        const auto cactions = m->actions();
        if(cactions.isEmpty()) {
          m->addAction(this->ha);
        } else {
          m->insertSeparator(*(cactions.begin()));
          m->insertAction(*(cactions.begin()), this->ha);
        }
        QObject::connect(m, &QMenu::triggered, this,
                         &CMakeMajorMode::actionTriggered);
      }
    }
  } // end of CMakeMajorMode::completeContextMenu

  void CMakeMajorMode::actionTriggered(QAction *a) {
    if(a == this->ha) {
      const auto k = this->ha->data().toString();
      auto nf = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.attachSecondaryTask(QObject::tr("help on '%1'").arg(k), nf);
      auto &p = nf->getProcess();
      if(p.state() != QProcess::Running) {
        p.start("cmake", QStringList() << "--help-command" << k);
        p.waitForStarted();
        p.waitForFinished(1000);
      }
      nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    }
  } // end of CMakeMajorMode::actionTriggered

  QString CMakeMajorMode::getCommentSyntax() {
    return "#";
  } // end of CMakeMajorMode::getCommentSyntax

  void CMakeMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    new CMakeSyntaxHighlighter(d);
  } // end of CMakeMajorMode::setSyntaxHighlighter

  void CMakeMajorMode::format() {}

  void CMakeMajorMode::runCompilation() {
    //     QSettings s;
    //     const auto ch =
    //     s.value("cmake/compilation/history").toStringList();
    //     const auto d = [&ch]() -> QString {
    //       if (ch.isEmpty()) {
    //         return "make -k";
    //       }
    //       return ch.back();
    //     }();
    //     auto* l = new QEmacsShellProcessLineEdit(
    //         "compilation command :", d, "compilation-output",
    //         this->qemacs);
    //     l->setInputHistorySettingAddress("cmake/compilation/history");
    //     this->qemacs.setUserInput(l);
  }  // end of CMakeMajorMode::runCompilation

  CMakeMajorMode::~CMakeMajorMode() = default;

  static StandardQEmacsMajorModeProxy<CMakeMajorMode> proxy(
      "CMake",
      QVector<QRegExp>() << QRegExp("^CMakeLists\\.txt$")
                         << QRegExp(".*\\.cmake$"));

} // end of namespace qemacs
