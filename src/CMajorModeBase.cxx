/*! 
 * \file  CMajorModeBase.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 août 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QProcess>
#include<QtCore/QTemporaryFile>
#include<QtGui/QTextBlock>
#include<QtGui/QTextCursor>
#include<QtGui/QTextDocumentWriter>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/CMajorModeBase.hxx"

namespace qemacs
{

  CMajorModeBase::CMajorModeBase(QEmacsWidget& w,
				 QEmacsBuffer& b,
				 QEmacsTextEditBase& t)
    : CompiledLanguageMajorModeBase(w,b,t)
  {} // end of CMajorModeBase::CMajorModeBase

  void
  CMajorModeBase::format()
  {
    auto c = this->textEdit.textCursor();
    auto b = c;
    auto e = c;
    b.movePosition(QTextCursor::Start);
    e.movePosition(QTextCursor::End);
    this->indent(b,e,c);
  } // end of CMajorModeBase::format

  void
  CMajorModeBase::comment()
  {} // end of CMajorModeBase::comment

  void
  CMajorModeBase::indent(const QTextCursor& b,
			 const QTextCursor& e,
			 const QTextCursor& c)
  {
    static QRegExp rcursor("(\\d+)");
    QTemporaryFile tmp;
    if(!tmp.open()){
      this->qemacs.displayInformativeMessage(QObject::tr("can't open temporary file"));
      return;
    }
    QTextDocumentWriter writer;
    writer.setFormat("plaintext");
    QFile tmp2(tmp.fileName());
    writer.setDevice(&tmp2);
    if (!writer.write(this->textEdit.document())){
      this->qemacs.displayInformativeMessage(QObject::tr("can't write buffer temporary file"));
      return;
    }
    tmp2.close();
    QProcess cformat;
    const auto ln = b.blockNumber();
    const auto lb = QString::number(ln+1);
    const auto le = QString::number(e.blockNumber()+1);
    cformat.start("clang-format-3.5",
                  QStringList() << "-assume-filename=cxx" 
		  << ("-lines=" + lb + ':' + le)
		  <<  ("-style=file")
		  << ("-cursor=" + QString::number(c.position()))
		  << tmp.fileName());
    if (!cformat.waitForStarted()){
      this->qemacs.displayInformativeMessage(QObject::tr("call to 'clang-format' failed"));
      return;
    }
    cformat.closeWriteChannel();
    if(!cformat.waitForFinished()){
      this->qemacs.displayInformativeMessage(QObject::tr("call to 'clang-format' failed"));
      return;
    }
    QTextStream in(cformat.readAll());
    auto pr = in.readLine();
    if(rcursor.indexIn(pr,0)==-1){
      this->qemacs.displayInformativeMessage(QObject::tr("analysis of 'clang-format' output failed"));
      return;
    }
    const auto cpos = rcursor.cap(1).toInt();
    // skipping the first lines
    for(int i=0;i!=ln;++i){
      in.readLine();
    }
    auto tc = b;
    tc.beginEditBlock();
    tc.movePosition(QTextCursor::StartOfBlock,
    		   QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::End,
		    QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    tc.insertText(in.readAll());
    tc.setPosition(cpos);
    tc.endEditBlock();
    this->textEdit.setTextCursor(tc);
  }
  
  void
  CMajorModeBase::indentLine(const QTextCursor& c)
  {
    this->indent(c,c,c);
  }
  
  void
  CMajorModeBase::indentRegion(const QTextCursor& tc)
  {
    if(!tc.hasSelection()){
      return;
    }
    QTextCursor b(tc);
    QTextCursor e(tc);
    QTextCursor c(tc);
    c.setPosition(tc.selectionEnd());
    if(tc.selectionStart()>tc.selectionEnd()){
      b.setPosition(tc.selectionEnd());
      e.setPosition(tc.selectionStart());
    } else {
      b.setPosition(tc.selectionStart());
      e.setPosition(tc.selectionEnd());
    }
    this->indent(b,e,c);
  } // end of QEmacsMajorModeBase::indentRegion

  CMajorModeBase::~CMajorModeBase() = default;
  
} // end of namespace qemacs
