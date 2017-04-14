/*! 
 * \file  GrepOutputMajorMode.cxx
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

  struct GrepUserData
    : public QTextBlockUserData
  {
    QString file;
    int line;
    ~GrepUserData()
    {}
  }; // end of GrepUserData

  struct GrepOutputSyntaxHighlighter
    : public QSyntaxHighlighter
  {
    GrepOutputSyntaxHighlighter(QTextDocument *p)
      : QSyntaxHighlighter(p)
    {
      this->line.setForeground(Qt::darkGreen);
    } // end of GrepOutputSyntaxHighlighter

    virtual void
    highlightBlock(const QString &text) override
    {
      QRegExp expr("^([/-\\w|\\.]+):(\\d+):");
      expr.setMinimal(true);
      int index = expr.indexIn(text);
      if((index >= 0)&&(expr.captureCount()==2)){
	GrepUserData * d = new GrepUserData;
	QString n = expr.cap(2);
	bool b;
	d->file = expr.cap(1);
	d->line = n.toInt(&b);
	if(!b){
	  delete d;
	  return;
	}
	int l = d->file.size();
	this->setFormat(0,l,this->line);
	this->setFormat(l+1,n.size(),this->line);
	this->setCurrentBlockUserData(d);
      }
    }

  protected:

    QTextCharFormat line;

  }; // end of struct GrepOutputSyntaxHighlighter


  /*!
   * A major mode to display the results of the grepOutput unix command
   */
  struct GrepOutputMajorMode
    : public QEmacsMajorModeBase
  {
    
    GrepOutputMajorMode(QEmacsWidget& w,
			QEmacsBuffer& b,
			QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w,b,t,&t)
    {}

    virtual QString
    getName(void) const override
    {
      return "grep-output";
    } // end of getName

    virtual QString
    getDescription(void) const override
    {
      return "major mode dedicated to the output of the grep utility";
    } // end of getDescription

    virtual void
    setSyntaxHighlighter(QTextDocument* d) override
    {
      new GrepOutputSyntaxHighlighter(d);
    } // end of setSyntaxHighlighter

    virtual bool
    mousePressEvent(QMouseEvent *e) override
    {
      if(e->buttons()==Qt::LeftButton){
	ProcessOutputFrame *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
	if(po==nullptr){
	  return true;
	}
	QTextCursor c = this->textEdit.cursorForPosition(e->pos()); 
	QTextBlockUserData *ud = c.block().userData();
	if(ud==nullptr){
	  return false;
	}
	GrepUserData *d = static_cast<GrepUserData *>(ud);
	const QString wd = po->getProcess().workingDirectory();
	if(!wd.isEmpty()){
	  this->qemacs.openFile(wd+QDir::separator()+d->file);
	} else {
	  this->qemacs.openFile(d->file);
	}
	QEmacsPlainTextEdit& t = this->qemacs.getCurrentBuffer().getMainFrame();
	t.gotoLine(d->line);
      }
      return false;
    }

    virtual bool
    keyPressEvent(QKeyEvent *e) override
    {
      if(((e->modifiers()==Qt::AltModifier)&&(e->key()==Qt::Key_M))||
	 ((e->modifiers()==Qt::NoModifier)&&(e->key()==Qt::Key_Return))){
	ProcessOutputFrame *po = qobject_cast<ProcessOutputFrame *>(&(this->textEdit));
	if(po==nullptr){
	  return false;
	}
	QTextCursor c = this->textEdit.textCursor(); 
	QTextBlockUserData *ud = c.block().userData();
	if(ud==nullptr){
	  return false;
	}
	GrepUserData *d = static_cast<GrepUserData *>(ud);
	const QString wd = po->getProcess().workingDirectory();
	if(!wd.isEmpty()){
	  this->qemacs.openFile(wd+QDir::separator()+d->file);
	} else {
	  this->qemacs.openFile(d->file);
	}
	QEmacsPlainTextEdit& t = this->qemacs.getCurrentBuffer().getMainFrame();
	t.gotoLine(d->line);
	return true;
      }
      return false;
    }

    virtual void format(void) override
    {}

    virtual void comment() override
    {}

    virtual ~GrepOutputMajorMode()
    {}

  }; // end of GrepOutputMajorMode
  
  static StandardQEmacsMajorModeProxy<GrepOutputMajorMode> proxy("grep output");

} // end of namespace qemacs
