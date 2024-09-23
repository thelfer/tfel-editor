/*!
 * \file  LaTeXOutputMajorMode.cxx
 * \brief
 * \author Thomas Helfer
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
#include "TFEL/GUI/MajorModeFactory.hxx"

namespace tfel::gui {

  struct LaTeXUserData : public QTextBlockUserData {
    ~LaTeXUserData() override = default;
    QString file;
    int line;
  };  // end of LaTeXUserData

  /*!
   * structure in charge of highlighting the latex output
   *
   * Block state :
   * - 0 normal state
   * - 1 warning
   */
  struct LaTeXOutputSyntaxHighlighter : public QSyntaxHighlighter {
    LaTeXOutputSyntaxHighlighter(QTextDocument *p)
        : QSyntaxHighlighter(p),
          err("^! "),
          err2("^l.(\\d+) "),
          err3("^([\\./][\\w-0-9_/\\.]+):(\\d+): "),
          warning("^[\\w ]+Warning: "),
          warning2("^No file ([\\w-0-9_/\\.]+).") {
      this->latexFile.setForeground(Qt::blue);
      this->latexWarning.setForeground(Qt::darkYellow);
      this->latexError.setForeground(Qt::red);
      this->latexLineError.setForeground(Qt::darkRed);
      err.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
      err2.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
      err3.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
      warning.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
      warning2.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    }  // end of LaTeXOutputSyntaxHighlighter

    void highlightBlock(const QString &text) override {
      this->setCurrentBlockState(0);
      if (this->previousBlockState() == 1) {
        if (!(text.trimmed().size() == 0)) {
          this->setFormat(0, text.size(), this->latexWarning);
          this->setCurrentBlockState(1);
        }
      } else {
        // warnings
        if (warning.match(text).hasMatch()) {
          this->setFormat(0, text.size(), this->latexWarning);
          this->setCurrentBlockState(1);
        }
        if (warning2.match(text).hasMatch()) {
          this->setFormat(0, text.size(), this->latexWarning);
        }
        // errors
        if (err.match(text).hasMatch()) {
          this->setFormat(0, text.size(), this->latexError);
        }
        if (err2.match(text).hasMatch()) {
          this->setFormat(0, text.size(), this->latexLineError);
        }
        const auto match3 = err3.match(text);
        if (match3.hasMatch()) {
          auto *d = new LaTeXUserData;
          const auto f = match3.captured(1);
          const auto l = match3.captured(2);
          bool b;
          d->file = f;
          d->line = l.toInt(&b);
          if (!b) {
            delete d;
          } else {
            this->setFormat(0, text.size(), this->latexLineError);
            this->setCurrentBlockUserData(d);
          }
        }
        QRegularExpression file("\\(([\\./][\\w-0-9_/\\.]+)");
        auto match = file.match(text);
        while (match.hasMatch()) {
          const auto pos = match.lastCapturedIndex();
          // removing the first character
          const auto l = match.capturedLength();
          this->setFormat(pos + 1, l, this->latexFile);
          match = file.match(text, pos + l);
        }
      }
    }

   protected:
    QRegularExpression err;
    QRegularExpression err2;
    QRegularExpression err3;
    QRegularExpression warning;
    QRegularExpression warning2;

    QTextCharFormat latexWarning;
    QTextCharFormat latexError;
    QTextCharFormat latexLineError;
    QTextCharFormat latexFile;

  };  // end of struct LaTeXOutputSyntaxHighlighter

  /*!
   * A major mode to display the results of the grepOutput unix command
   */
  struct LaTeXOutputMajorMode : public MajorModeBase {
    LaTeXOutputMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
        : MajorModeBase(w, b, t, &t) {}

    QString getName() const override {
      return "latex-output";
    }  // end of LicosMajorMode

    QString getDescription() const override {
      return "major mode dedicated to the output of the grep utility";
    }  // end of getDescription

    void setSyntaxHighlighter(QTextDocument *d) override {
      new LaTeXOutputSyntaxHighlighter(d);
    }  // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent *e) override {
      if (e->buttons() == Qt::LeftButton) {
        auto *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
        if (po == nullptr) {
          return true;
        }
        auto c = this->textEdit.cursorForPosition(e->pos());
        auto *ud = c.block().userData();
        if (ud == nullptr) {
          return false;
        }
        auto *d = static_cast<LaTeXUserData *>(ud);
        QFileInfo fi(d->file);
        QString file;
        if (fi.isRelative()) {
          const auto wd = po->getProcess().workingDirectory();
          if (!wd.isEmpty()) {
            file = wd + QDir::separator() + d->file;
          } else {
            file = d->file;
          }
        } else {
          file = d->file;
        }
        fi.setFile(file);
        if (fi.exists()) {
          this->editor.openFile(file);
          auto &t = this->editor.getCurrentBuffer().getMainFrame();
          t.gotoLine(d->line);
        } else {
          this->editor.displayInformativeMessage(
              QObject::tr("file named '%1'").arg(file));
        }
      }
      return false;
    }

    bool keyPressEvent(QKeyEvent *) override {
      // if(((e->modifiers()==Qt::AltModifier)&&(e->key()==Qt::Key_M))||
      // 	 ((e->modifiers()==Qt::NoModifier)&&(e->key()==Qt::Key_Return))){
      // 	ProcessOutputFrame *po = qobject_cast<ProcessOutputFrame
      // *>(&(this->textEdit));
      // 	if(po==0){
      // 	  return false;
      // 	}
      // 	QTextCursor c = this->textEdit.textCursor();
      // 	QTextBlockUserData *ud = c.block().userData();
      // 	if(ud==0){
      // 	  return false;
      // 	}
      // 	LaTeXUserData *d = static_cast<LaTeXUserData *>(ud);
      // 	const QString wd = po->getProcess().workingDirectory();
      // 	if(!wd.isEmpty()){
      // 	  this->editor.openFile(wd+QDir::separator()+d->file);
      // 	} else {
      // 	  this->editor.openFile(d->file);
      // 	}
      // 	PlainTextEdit& t =
      // this->editor.getCurrentBuffer().getMainFrame();
      // 	t.gotoLine(d->line);
      // 	return true;
      // }
      return false;
    }

    void format() override {}

    void comment() override {}

    ~LaTeXOutputMajorMode() override = default;

  };  // end of LaTeXOutputMajorMode

  static StandardMajorModeProxy<LaTeXOutputMajorMode> proxy("latex-output");

}  // end of namespace tfel::gui
