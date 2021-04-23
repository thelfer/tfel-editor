/*!
 * \file  GrepOutputMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel {

  namespace gui {

    struct GrepUserData : public QTextBlockUserData {
      //! file name
      QString file;
      //! line number
      int line;
      //! destructor
      ~GrepUserData() override = default;
    };  // end of GrepUserData

    struct GrepOutputSyntaxHighlighter : public QSyntaxHighlighter {
      explicit GrepOutputSyntaxHighlighter(QTextDocument *p)
          : QSyntaxHighlighter(p) {
        this->line.setForeground(Qt::darkGreen);
        this->line.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        this->number.setForeground(Qt::darkGreen);
      }  // end of GrepOutputSyntaxHighlighter

      void highlightBlock(const QString &text) override {
        QRegExp expr("^([/-\\w|\\.]+):(\\d+):");
        expr.setMinimal(true);
        int index = expr.indexIn(text);
        if ((index == 0) && (expr.captureCount() == 2)) {
          auto *d = new GrepUserData;
          auto n = expr.cap(2);
          bool b;
          d->file = expr.cap(1);
          d->line = n.toInt(&b);
          if (!b) {
            delete d;
            return;
          }
          const auto l = d->file.size();
          this->setFormat(0, l, this->line);
          this->setFormat(l + 1, n.size(), this->number);
          this->setCurrentBlockUserData(d);
        }
      }

     protected:
      QTextCharFormat line;
      QTextCharFormat number;

    };  // end of struct GrepOutputSyntaxHighlighter

    /*!
     * A major mode to display the results of the grep unix command
     */
    struct GrepOutputMajorMode : public ProcessOutputMajorModeBase {
      /*!
       * \brief constructor
       */
      GrepOutputMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
          : ProcessOutputMajorModeBase(w, b, t, &t) {
      }  // end of GrepOutputMajorMode

      QString getName() const override {
        return "grep-output";
      }  // end of getName

      QString getDescription() const override {
        return "major mode dedicated to the output of the grep utility";
      }  // end of getDescription

      void setSyntaxHighlighter(QTextDocument *d) override {
        new GrepOutputSyntaxHighlighter(d);
      }  // end of setSyntaxHighlighter

      bool mousePressEvent(QMouseEvent *e) override {
        if (e->buttons() == Qt::LeftButton) {
          auto *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
          if (po == nullptr) {
            return false;
          }
          auto c = this->textEdit.cursorForPosition(e->pos());
          auto *ud = c.block().userData();
          if (ud == nullptr) {
            return false;
          }
          auto *d = static_cast<GrepUserData *>(ud);
          auto b = c;
          b.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
          const auto pos = c.position() - b.position();
          if (pos < d->file.size()) {
            auto &cb = this->editor.getCurrentBuffer();
            const auto self = cb.getCurrentSecondaryTask();
            const auto wd = po->getProcess().workingDirectory();
            if (!wd.isEmpty()) {
              this->editor.openFile(wd + QDir::separator() + d->file);
            } else {
              this->editor.openFile(d->file);
            }
            auto &nb = this->editor.getCurrentBuffer();
            if (self != nullptr) {
              nb.attachSecondaryTask(self);
            }
            nb.getMainFrame().gotoLine(d->line);
            return true;
          }
        }
        return ProcessOutputMajorModeBase::mousePressEvent(e);
      }

      bool keyPressEvent(QKeyEvent *e) override {
        const auto k = e->key();
        const auto m = e->modifiers();
        if (((m == Qt::NoModifier) && (k == Qt::Key_Enter)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Return))) {
          auto *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
          if (po == nullptr) {
            return false;
          }
          auto c = this->textEdit.textCursor();
          auto *ud = c.block().userData();
          if (ud == nullptr) {
            return false;
          }
          auto *d = static_cast<GrepUserData *>(ud);
          auto b = c;
          b.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
          const auto pos = c.position() - b.position();
          if (pos < d->file.size()) {
            auto &cb = this->editor.getCurrentBuffer();
            const auto self = cb.getCurrentSecondaryTask();
            const auto wd = po->getProcess().workingDirectory();
            if (!wd.isEmpty()) {
              this->editor.openFile(wd + QDir::separator() + d->file);
            } else {
              this->editor.openFile(d->file);
            }
            auto &nb = this->editor.getCurrentBuffer();
            nb.attachSecondaryTask(self);
            nb.getMainFrame().gotoLine(d->line);
          }
        }
        return ProcessOutputMajorModeBase::keyPressEvent(e);
      }

      void processFinished(int, QProcess::ExitStatus) override {
      }  // end of processFinished

      void format() override {}

      void comment() override {}

      ~GrepOutputMajorMode() override = default;

    };  // end of GrepOutputMajorMode

    static StandardMajorModeProxy<GrepOutputMajorMode> proxy("grep-output");

  }  // end of namespace gui
}  // end of namespace tfel
