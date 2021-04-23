/*!
 * \file  CMakeMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/CMakeSyntaxHighlighter.hxx"
#include "TFEL/GUI/CMakeMajorMode.hxx"

namespace tfel {

  namespace gui {

    CMakeMajorMode::CMakeMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
        : MajorModeBase(w, b, t, &t) {
    }  // end of CMakeMajorMode::CMakeMajorMode

    QString CMakeMajorMode::getName() const {
      return "CMake";
    }  // end of CMakeMajorMode::CMakeMajorMode

    QString CMakeMajorMode::getDescription() const {
      return "major mode dedicated to CMake";
    }  // end of CMakeMajorMode::CMakeMajorMode

    bool CMakeMajorMode::keyPressEvent(QKeyEvent* const e) {
      const auto k = e->key();
      const auto m = e->modifiers();
      if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
        this->runCompilation();
        return true;
      }
      return false;
    }  // end of CMakeMajorMode::keyPressEvent

    void CMakeMajorMode::completeContextMenu(QMenu* const m,
                                             const QTextCursor& tc) {
      MajorModeBase::completeContextMenu(m, tc);
      const auto cmds = CMakeSyntaxHighlighter::getCMakeCommandsList();
      QTextCursor b(tc);
      b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      b.select(QTextCursor::LineUnderCursor);
      QString l = b.selectedText();
      QRegExp r("^\\s*(\\w+)");
      if (r.indexIn(l) >= 0) {
        QString k = r.cap(1).toLower();
        if (cmds.indexOf(k) != -1) {
          auto* const ha = new QAction(QObject::tr("Help on %1").arg(k), m);
          ha->setIcon(QIcon::fromTheme("dialog-question"));
          const auto cactions = m->actions();
          if (cactions.isEmpty()) {
            m->addAction(ha);
          } else {
            m->insertSeparator(*(cactions.begin()));
            m->insertAction(*(cactions.begin()), ha);
          }
          QObject::connect(ha, &QAction::triggered, this, [k, this] {
            auto nf = new ProcessOutputFrame(this->editor, this->buffer);
            this->buffer.attachSecondaryTask(QObject::tr("help on '%1'").arg(k),
                                             nf);
            auto& p = nf->getProcess();
            if (p.state() != QProcess::Running) {
              p.start("cmake", QStringList() << "--help-command" << k);
              p.waitForStarted();
              p.waitForFinished(1000);
            }
            nf->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
          });
        }
      }
    }  // end of CMakeMajorMode::completeContextMenu

    QString CMakeMajorMode::getCommentSyntax() {
      return "#";
    }  // end of CMakeMajorMode::getCommentSyntax

    void CMakeMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
      new CMakeSyntaxHighlighter(d);
    }  // end of CMakeMajorMode::setSyntaxHighlighter

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
      //     auto* l = new ShellProcessLineEdit(
      //         "compilation command :", d, "compilation-output",
      //         this->editor);
      //     l->setInputHistorySettingAddress("cmake/compilation/history");
      //     this->editor.setUserInput(l);
    }  // end of CMakeMajorMode::runCompilation

    CMakeMajorMode::~CMakeMajorMode() = default;

    static StandardMajorModeProxy<CMakeMajorMode> proxy(
        "CMake",
        QVector<QRegExp>() << QRegExp("^CMakeLists\\.txt$")
                           << QRegExp(".*\\.cmake$"));

  }  // end of namespace gui
}  // end of namespace tfel
