/*! 
 * \file  GnuplotMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 sept. 2012
 */

#include<QtCore/QDir>
#include<QtCore/QDebug>

#include<QtGui/QSyntaxHighlighter>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"

#include"QEmacs/QEmacsMajorMode.hxx"
#include"QEmacs/QEmacsMajorModeBase.hxx"
#include"QEmacs/ProcessInteractionFrame.hxx"
#include"QEmacs/GnuplotSyntaxHighlighter.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{

  /*!
   * A major mode to handle the gnuplot utility
   */
  struct GnuplotMajorMode
    : public QEmacsMajorModeBase
  {
    
    GnuplotMajorMode(QEmacsWidget& w,
		     QEmacsBuffer& b,
		     QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w,b,t,&t),
	go(new ProcessInteractionFrame(w,b))
    {
      QFileInfo fn(t.getCompleteFileName());
      QDir d(fn.dir());
      QProcess& p = go->getProcess();
      if(d.exists()){
	p.setWorkingDirectory(d.absolutePath());
      } else {
	p.setWorkingDirectory(QDir::current().absolutePath());
      }
      p.start("gnuplot",QStringList());
      b.addSlave("* gnuplot *",go);
    }

    QString
    getName(void) const override
    {
      return "gnuplot";
    } // end of LicosMajorMode

    virtual QString
    getDescription(void) const override
    {
      return "major mode dedicated to gnuplot";
    } // end of getDescription

    virtual void
    setSyntaxHighlighter(QTextDocument* d) override
    {
      new GnuplotSyntaxHighlighter(d);
    } // end of setSyntaxHighlighter

    virtual bool
    mousePressEvent(QMouseEvent *) override
    {
      return false;
    }

    virtual bool
    keyPressEvent(QKeyEvent *) override
    {
      return false;
    }

    virtual void
    format(void) override
    {}

    virtual void
    comment() override
    {}

    virtual bool
    handleShortCut(const int k1,
		   const Qt::KeyboardModifiers m,
		   const int k2) override
    {
      if((k1==Qt::Key_C)&&(k2==Qt::Key_L)&&
	 (m==Qt::ControlModifier)){
	QTextCursor c = this->textEdit.textCursor();
	c.select(QTextCursor::LineUnderCursor);
	QString l   = c.selectedText();
	QProcess& p = this->go->getProcess();
#ifdef QEMACS_QT4
	p.write((l+"\n").toAscii());
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
	p.write((l+"\n").toLatin1());
#endif /* QEMACS_QT5 */
	return true;
      }
      return false;
    } // end of QEmacsMajorModeBase::handleShortCut

    ~GnuplotMajorMode()
    {}

  protected:

    ProcessInteractionFrame *go;

  }; // end of GnuplotMajorMode
  
  static StandardQEmacsMajorModeProxy<GnuplotMajorMode> proxy("gnuplot",QVector<QRegExp>() 
							      << QRegExp("^[\\w-\\.0-9]+\\.gp")
							      << QRegExp("^[\\w-\\.0-9]+\\.gnuplot"));

} // end of namespace qemacs

