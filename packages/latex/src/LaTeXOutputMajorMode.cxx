/*! 
 * \file  LaTeXOutputMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#include<QtCore/QDir>
#include<QtCore/QDebug>

#include<QtGui/QSyntaxHighlighter>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"

#include"QEmacs/QEmacsMajorMode.hxx"
#include"QEmacs/QEmacsMajorModeBase.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{

  struct LaTeXUserData
    : public QTextBlockUserData
  {
    ~LaTeXUserData() override = default;
    QString file;
    int line;
  }; // end of LaTeXUserData

  /*!
   * structure in charge of highlighting the latex output
   *
   * Block state :
   * - 0 normal state
   * - 1 warning
   */
  struct LaTeXOutputSyntaxHighlighter
    : public QSyntaxHighlighter
  {

    LaTeXOutputSyntaxHighlighter(QTextDocument *p)
      : QSyntaxHighlighter(p),
	err("^! "),
	err2("^l.(\\d+) "),
	err3("^([\\./][\\w-0-9_/\\.]+):(\\d+): "),
	warning("^[\\w ]+Warning: "),
	warning2("^No file ([\\w-0-9_/\\.]+).")
    {
      this->latexFile.setForeground(Qt::blue);
      this->latexWarning.setForeground(Qt::darkYellow);
      this->latexError.setForeground(Qt::red);
      this->latexLineError.setForeground(Qt::darkRed);
      err.setMinimal(true);
      err2.setMinimal(true);
      err3.setMinimal(true);
      warning.setMinimal(true);
      warning2.setMinimal(true);
    } // end of LaTeXOutputSyntaxHighlighter

    void highlightBlock(const QString &text) override
    {
      this->setCurrentBlockState(0);
      if(this->previousBlockState()==1){
	if(!(text.trimmed().size()==0)){
	  this->setFormat(0,text.size(),this->latexWarning);
	  this->setCurrentBlockState(1);
	}
      } else {
	// warnings
	if(warning.indexIn(text)!=-1){
	  this->setFormat(0,text.size(),this->latexWarning);
	  this->setCurrentBlockState(1);
	} 
	if(warning2.indexIn(text)!=-1){
	  this->setFormat(0,text.size(),this->latexWarning);
	} 
	// errors
	if(err.indexIn(text)!=-1){
	  this->setFormat(0,text.size(),this->latexError);
	}
	if(err2.indexIn(text)!=-1){
	  this->setFormat(0,text.size(),this->latexLineError);
	}
	if(err3.indexIn(text)!=-1){
	  auto *d = new LaTeXUserData;
	  const auto f = err3.cap(1);
	  const auto l = err3.cap(2);
	  bool b;
	  d->file = f;
	  d->line = l.toInt(&b);
	  if(!b){
	    delete d;
	  } else {
	    this->setFormat(0,text.size(),this->latexLineError);
	    this->setCurrentBlockUserData(d);
	  }
	}
	QRegExp file("\\(([\\./][\\w-0-9_/\\.]+)");
	int pos = 0;
	while ((pos = file.indexIn(text, pos)) != -1) {
	  // removing the first character
	  this->setFormat(pos+1,file.cap(1).size(),this->latexFile);
	  pos += file.matchedLength();
	}
      }
    }

  protected:

    QRegExp err;
    QRegExp err2;
    QRegExp err3;
    QRegExp warning;
    QRegExp warning2;
    
    QTextCharFormat latexWarning;
    QTextCharFormat latexError;
    QTextCharFormat latexLineError;
    QTextCharFormat latexFile;

  }; // end of struct LaTeXOutputSyntaxHighlighter


  /*!
   * A major mode to display the results of the grepOutput unix command
   */
  struct LaTeXOutputMajorMode
    : public QEmacsMajorModeBase
  {
    
    LaTeXOutputMajorMode(QEmacsWidget& w,
			QEmacsBuffer& b,
			QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w,b,t,&t)
    {}

    QString getName() const override
    {
      return "latex-output";
    } // end of LicosMajorMode

    QString getDescription() const override
    {
      return "major mode dedicated to the output of the grep utility";
    } // end of getDescription

    void setSyntaxHighlighter(QTextDocument* d) override
    {
      new LaTeXOutputSyntaxHighlighter(d);
    } // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent * e) override
    {
      if(e->buttons()==Qt::LeftButton){
	auto *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
	if(po==nullptr){
	  return true;
	}
	auto c = this->textEdit.cursorForPosition(e->pos()); 
	auto *ud = c.block().userData();
	if(ud==nullptr){
	  return false;
	}
	auto *d = static_cast<LaTeXUserData *>(ud);
	QFileInfo fi(d->file);
	QString file;
	if(fi.isRelative()){
	  const auto wd = po->getProcess().workingDirectory();
	  if(!wd.isEmpty()){
	    file = wd+QDir::separator()+d->file;
	  } else {
	    file = d->file;
	  }
	} else {
	  file = d->file;
	}
	fi.setFile(file);
	if(fi.exists()){
	  this->qemacs.openFile(file);
	  auto& t = this->qemacs.getCurrentBuffer().getMainFrame();
	  t.gotoLine(d->line);
	} else {
	  this->qemacs.displayInformativeMessage(QObject::tr("file named '%1'").arg(file));
	}
      }
      return false;
    }

    bool keyPressEvent(QKeyEvent *) override
    {
      // if(((e->modifiers()==Qt::AltModifier)&&(e->key()==Qt::Key_M))||
      // 	 ((e->modifiers()==Qt::NoModifier)&&(e->key()==Qt::Key_Return))){
      // 	ProcessOutputFrame *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
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
      // 	  this->qemacs.openFile(wd+QDir::separator()+d->file);
      // 	} else {
      // 	  this->qemacs.openFile(d->file);
      // 	}
      // 	QEmacsPlainTextEdit& t = this->qemacs.getCurrentBuffer().getMainFrame();
      // 	t.gotoLine(d->line);
      // 	return true;
      // }
      return false;
    }

    void format() override
    {}

    void comment() override
    {}

    ~LaTeXOutputMajorMode() override;

  }; // end of LaTeXOutputMajorMode
  
  static StandardQEmacsMajorModeProxy<LaTeXOutputMajorMode> proxy("latex output");

} // end of namespace qemacs
