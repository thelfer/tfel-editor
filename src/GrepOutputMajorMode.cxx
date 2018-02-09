/*!
 * \file  GrepOutputMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <QtGui/QSyntaxHighlighter>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"

#include "QEmacs/QEmacsMajorMode.hxx"
#include "QEmacs/QEmacsMajorModeBase.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs {

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
      if ((index==0) && (expr.captureCount() == 2)) {
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
   * A major mode to display the results of the grepOutput unix command
   */
  struct GrepOutputMajorMode : public QEmacsMajorModeBase {
    GrepOutputMajorMode(QEmacsWidget &w,
                        QEmacsBuffer &b,
                        QEmacsTextEditBase &t)
        : QEmacsMajorModeBase(w, b, t, &t) {}

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
        auto *po =
            qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
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
        b.movePosition(QTextCursor::StartOfLine,
                       QTextCursor::MoveAnchor);
        const auto pos = c.position()-b.position();
        if(pos<d->file.size()){
          const auto wd = po->getProcess().workingDirectory();
          if (!wd.isEmpty()) {
            this->qemacs.openFile(wd + QDir::separator() + d->file);
          } else {
            this->qemacs.openFile(d->file);
          }
          auto &t = this->qemacs.getCurrentBuffer().getMainFrame();
          t.gotoLine(d->line);
          return true;
        }
      }
      return false;
    }

    bool keyPressEvent(QKeyEvent *e) override {
      if (((e->modifiers() == Qt::AltModifier) &&
           (e->key() == Qt::Key_M)) ||
          ((e->modifiers() == Qt::NoModifier) &&
           (e->key() == Qt::Key_Return))) {
        auto *po =
            qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
        if (po == nullptr) {
          return false;
        }
        auto c = this->textEdit.textCursor();
        auto *ud = c.block().userData();
        if (ud == nullptr) {
          return false;
        }
        auto *d = static_cast<GrepUserData *>(ud);
        const auto wd = po->getProcess().workingDirectory();
        if (!wd.isEmpty()) {
          this->qemacs.openFile(wd + QDir::separator() + d->file);
        } else {
          this->qemacs.openFile(d->file);
        }
        auto &t = this->qemacs.getCurrentBuffer().getMainFrame();
        t.gotoLine(d->line);
        return true;
      }
      return false;
    }

    void format() override {}

    void comment() override {}

    ~GrepOutputMajorMode() override = default;

  };  // end of GrepOutputMajorMode

  static StandardQEmacsMajorModeProxy<GrepOutputMajorMode> proxy(
      "grep output");

}  // end of namespace qemacs
