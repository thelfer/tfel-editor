/*!
 * \file  CompilationOutputMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QRegExp>

#include <QtGui/QSyntaxHighlighter>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"

#include "QEmacs/ProcessOutputMajorModeBase.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs {

  struct CompilationOutputUserData : public QTextBlockUserData {
    QString file;
    int line;
    int column;
    ~CompilationOutputUserData() override = default;
  };  // end of CompilationOutputUserData

  struct CompilationOutputSyntaxHighlighter
      : public QSyntaxHighlighter {
    explicit CompilationOutputSyntaxHighlighter(QTextDocument *p)
        : QSyntaxHighlighter(p) {
      this->error.setForeground(Qt::red);
      this->error.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      this->warning.setForeground(Qt::darkGreen);
      this->warning.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      this->note.setForeground(Qt::blue);
      this->note.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      this->number.setForeground(Qt::darkBlue);
    }  // end of CompilationOutputSyntaxHighlighter

    void highlightBlock(const QString &l) override {
      static auto e = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): error:");
        r.setMinimal(true);
        return r;
      }();
      static auto w = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): warning:");
        r.setMinimal(true);
        return r;
      }();
      static auto n = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): note:");
        r.setMinimal(true);
        return r;
      }();
      auto apply = [this, &l](const QRegExp &r,
                              const QTextCharFormat &f) {
        const auto index = r.indexIn(l);
        if ((index == 0) && (r.captureCount() == 3)) {
          auto *d = new CompilationOutputUserData;
          bool b1, b2;
          d->file = r.cap(1);
          d->line = r.cap(2).toInt(&b1);
          d->column = r.cap(3).toInt(&b2);
          if (!b1 || !b2) {
            delete d;
            return false;
          }
          const auto l1 = d->file.size();
          const auto l2 = r.cap(2).size();
          const auto l3 = r.cap(3).size();
          this->setFormat(0, l1, f);
          this->setFormat(l1 + 1, l2, this->number);
          this->setFormat(l1 + l2 + 2, l3, this->number);
          this->setCurrentBlockUserData(d);
          return true;
        }
        return false;
      };
      if (apply(e, this->error)) {
        return;
      }
      if (apply(w, this->warning)) {
        return;
      }
      if (apply(n, this->note)) {
        return;
      }
    }

   protected:
    QTextCharFormat error;
    QTextCharFormat warning;
    QTextCharFormat note;
    QTextCharFormat number;

  };  // end of struct CompilationOutputSyntaxHighlighter

  /*!
   * \brief a major mode to display the results of the a compilation
   * process.
   */
  struct CompilationOutputMajorMode : public ProcessOutputMajorModeBase {
    CompilationOutputMajorMode(QEmacsWidget &w,
                               QEmacsBuffer &b,
                               QEmacsTextEditBase &t)
        : ProcessOutputMajorModeBase(w, b, t, &t) {
      this->textEdit.setReadOnly(true);
    }  // end of CompilationOutputMajorMode

    QString getName() const override {
      return "compilation-output";
    }  // end of getName

    QString getDescription() const override {
      return "major mode dedicated to the output of compilation "
             "process";
    }  // end of getDescription

    void setSyntaxHighlighter(QTextDocument *d) override {
      new CompilationOutputSyntaxHighlighter(d);
    }  // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent *e) override {
      if (e->buttons() == Qt::LeftButton) {
        auto *po =
            qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
        if (po == nullptr) {
          return true;
        }
        auto c = this->textEdit.cursorForPosition(e->pos());
        auto *ud = c.block().userData();
        if (ud == nullptr) {
          return false;
        }
        auto *d = static_cast<CompilationOutputUserData *>(ud);
        auto b = c;
        b.movePosition(QTextCursor::StartOfLine,
                       QTextCursor::MoveAnchor);
        const auto pos = c.position()-b.position();
        if (pos < d->file.size()) {
          this->qemacs.openFile(d->file);
          auto &t = this->qemacs.getCurrentBuffer().getMainFrame();
          t.gotoPosition(d->line,d->column);
          return true;
        }
      }
      return false;
    }  // end of mousePressEvent

    bool keyPressEvent(QKeyEvent *e) override {
      if (((e->modifiers() == Qt::AltModifier) &&
           (e->key() == Qt::Key_M)) ||
          ((e->modifiers() == Qt::NoModifier) &&
           (e->key() == Qt::Key_Return))) {
        auto c = this->textEdit.textCursor();
        auto *ud = c.block().userData();
        if (ud == nullptr) {
          return false;
        }
        auto *d = static_cast<CompilationOutputUserData *>(ud);
        auto b = c;
        b.movePosition(QTextCursor::StartOfLine,
                       QTextCursor::MoveAnchor);
        const auto pos = c.position()-b.position();
        if(pos<d->file.size()){
          this->qemacs.openFile(d->file);
          auto &t = this->qemacs.getCurrentBuffer().getMainFrame();
          t.gotoPosition(d->line, d->column);
          return true;
        }
      }
      return false;
    } // end of keyPressEvent

    void format() override {}  // end of format

    void comment() override {}  // end of comment

    void processFinished(int s, QProcess::ExitStatus) override {
      if (s == 0) {
        // success
        return;
      }
      // failure: we are looking for the first error
      auto c = this->textEdit.textCursor();
      c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      while (!c.atEnd()) {
        auto *ud = c.block().userData();
        if (ud != nullptr) {
          this->textEdit.setTextCursor(c);
          break;
        }
        c.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
      };
    } // end of processFinished

    ~CompilationOutputMajorMode() override = default;

  };  // end of CompilationOutputMajorMode

  static StandardQEmacsMajorModeProxy<CompilationOutputMajorMode> proxy(
      "compilation-output");

}  // end of namespace qemacs
