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

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"

#include "TFEL/GUI/ProcessOutputMajorModeBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel{

  namespace gui{

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
      this->backtrace.setForeground(Qt::darkRed);
      this->backtrace.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      this->note.setForeground(Qt::blue);
      this->note.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      this->number.setForeground(Qt::darkBlue);
    }  // end of CompilationOutputSyntaxHighlighter

    void highlightBlock(const QString &l) override {
      static auto e1 = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): fatal error:");
        r.setMinimal(true);
        return r;
      }();
      static auto e2 = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): error:");
        r.setMinimal(true);
        return r;
      }();
      // qt-moc error
      static auto e3 = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+): Error:");
        r.setMinimal(true);
        return r;
      }();
      static auto b = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+):   required from here");
        r.setMinimal(true);
        return r;
      }();
      static auto w = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): warning:");
        r.setMinimal(true);
        return r;
      }();
      static auto n1 = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+):(\\d+): note:");
        r.setMinimal(true);
        return r;
      }();
      // qt-moc note
      static auto n2 = [] {
        QRegExp r("^([/-\\w|\\.]+):(\\d+): Note:");
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
      auto apply2 = [this, &l](const QRegExp &r,
                              const QTextCharFormat &f) {
        const auto index = r.indexIn(l);
        if ((index == 0) && (r.captureCount() == 2)) {
          auto *d = new CompilationOutputUserData;
          bool b1;
          d->file = r.cap(1);
          d->line = r.cap(2).toInt(&b1);
          d->column = 0;
          if (!b1) {
            delete d;
            return false;
          }
          const auto l1 = d->file.size();
          const auto l2 = r.cap(2).size();
          this->setFormat(0, l1, f);
          this->setFormat(l1 + 1, l2, this->number);
          this->setCurrentBlockUserData(d);
          return true;
        }
        return false;
      };
      if (apply(e1, this->error)) {
        return;
      }
      if (apply(e2, this->error)) {
        return;
      }
      if (apply2(e3, this->error)) {
        return;
      }
      if (apply(w, this->warning)) {
        return;
      }
      if (apply(b, this->backtrace)) {
        return;
      }
      if (apply(n1, this->note)) {
        return;
      }
      if (apply2(n2, this->note)) {
        return;
      }
    }

   protected:
    QTextCharFormat error;
    QTextCharFormat warning;
    QTextCharFormat note;
    QTextCharFormat number;
    QTextCharFormat backtrace;

  };  // end of struct CompilationOutputSyntaxHighlighter

  /*!
   * \brief a major mode to display the results of the a compilation
   * process.
   */
  struct CompilationOutputMajorMode : public ProcessOutputMajorModeBase {
    CompilationOutputMajorMode(EditorWidget &w,
                               Buffer &b,
                               TextEditBase &t)
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
          auto &cb = this->editor.getCurrentBuffer();
          auto *const self = cb.getCurrentSecondaryTask();
          this->editor.openFile(d->file);
          auto &nb = this->editor.getCurrentBuffer();
          nb.getMainFrame().gotoPosition(d->line, d->column);
          if (self != nullptr) {
            nb.attachSecondaryTask(self);
          }
          return true;
        }
      }
      return false;
    }  // end of mousePressEvent

    bool keyPressEvent(QKeyEvent *e) override {
      const auto m = e->modifiers();
      const auto k = e->key();
      if (((m == Qt::NoModifier) && (k == Qt::Key_Enter)) ||
          ((m == Qt::NoModifier) && (k == Qt::Key_Return))) {
        auto c = this->textEdit.textCursor();
        auto *ud = c.block().userData();
        if (ud == nullptr) {
          return false;
        }
        auto *d = static_cast<CompilationOutputUserData *>(ud);
        auto b = c;
        b.movePosition(QTextCursor::StartOfLine,
                       QTextCursor::MoveAnchor);
        const auto pos = c.position() - b.position();
        if (pos < d->file.size()) {
          auto &cb = this->editor.getCurrentBuffer();
          const auto self = cb.getCurrentSecondaryTask();
          this->editor.openFile(d->file);
          auto &nb = this->editor.getCurrentBuffer();
          if (self != nullptr) {
            nb.attachSecondaryTask(self);
          }
          nb.getMainFrame().gotoPosition(d->line, d->column);
          return true;
        }
      }
      return ProcessOutputMajorModeBase::keyPressEvent(e);
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

  static StandardMajorModeProxy<CompilationOutputMajorMode> proxy(
      "compilation-output");

}  // end of namespace gui
}// end of namespace tfel
