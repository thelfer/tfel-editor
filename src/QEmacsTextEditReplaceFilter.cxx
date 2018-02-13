/*! 
 * \file  QEmacsTextEditReplaceFilter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   06/08/2012
 */

#include<QtCore/QDebug>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsTextEditQueryReplace.hxx"
#include"QEmacs/QEmacsTextEditReplaceFilter.hxx"

namespace qemacs
{

  QEmacsTextEditReplaceFilter::QEmacsTextEditReplaceFilter(QEmacsWidget& w,
							   QEmacsTextEditBase& t,
							   QEmacsTextEditQueryReplace& qr_,
							   const QString& s1_,
							   const QString& s2_)
    : QEmacsTextEditKeyPressEventFilter(t),
      qemacs(w),
      textEdit(t),
      qr(qr_),
      s1(s1_),
      s2(s2_),
      nb(0),
      hasSelection(false)
  {
    QTextCursor tc = this->textEdit.textCursor();
    if(tc.hasSelection()){
      this->hasSelection = true;
      this->bc = tc;
      this->ec = tc;
      if(tc.selectionStart()>tc.selectionEnd()){
	this->bc.setPosition(tc.selectionEnd());
	this->ec.setPosition(tc.selectionStart());
      } else {
	this->bc.setPosition(tc.selectionStart());
	this->ec.setPosition(tc.selectionEnd());
      }
      this->bc.clearSelection();
      this->ec.clearSelection();
      this->textEdit.setTextCursor(this->bc);
    }
    this->bOK = this->findNext();
    if(!this->bOK){
      this->qemacs.displayInformativeMessage(QObject::tr("'%1' not found").arg(this->s1));
    }
  }

  bool
  QEmacsTextEditReplaceFilter::isOk() const
  {
    return this->bOK;
  } // end of QEmacsTextEditReplaceFilter::isOk
    
  bool
  QEmacsTextEditReplaceFilter::findNext()
  {
    QTextCursor tc = this->textEdit.textCursor();
    QTextCursor nc = this->textEdit.document()->find(s1,tc);
    bool c = true;
    if(this->hasSelection){
      c = (!nc.isNull()) && (!nc.atEnd()) && (nc<this->ec);
    } else {
      c = (!nc.isNull()) && (!nc.atEnd());
    }
    if(c){
      this->textEdit.setTextCursor(nc);
    }
    return c;
  } // end of QEmacsTextEditReplaceFilter::findNext

  bool
  QEmacsTextEditReplaceFilter::filterKeyPressEvent(QKeyEvent * const ev)
  {
    if((ev->modifiers() == Qt::ControlModifier)&&(ev->key()==Qt::Key_G)){
      QTextCursor tc = this->textEdit.textCursor();
      tc.clearSelection();
      this->textEdit.setTextCursor(tc);
      this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
      this->textEdit.removeKeyPressEventFilter();
    }
    if(((ev->modifiers() == Qt::NoModifier)&&(ev->key()==Qt::Key_Y))||
       ((ev->modifiers() == Qt::NoModifier)&&(ev->key()==Qt::Key_N))||
       ((ev->modifiers() == Qt::NoModifier)&&(ev->key()==Qt::Key_Exclam))){
      if((ev->key()==Qt::Key_Y)||
	 (ev->key()==Qt::Key_N)){
	if(ev->key()==Qt::Key_Y){
	  QTextCursor tc = this->textEdit.textCursor();
	  tc.beginEditBlock();
	  tc.movePosition(QTextCursor::NoMove,
			  QTextCursor::MoveAnchor);
	  tc.movePosition(QTextCursor::NextCharacter,
			  QTextCursor::MoveAnchor,
			  -s1.size());
	  tc.removeSelectedText();
	  tc.insertText(s2);
	  tc.endEditBlock();
	  tc.clearSelection();
	  this->textEdit.setTextCursor(tc);
	  ++nb;
	}
	if(!this->findNext()){
	  QTextCursor tc = this->textEdit.textCursor();
	  tc.clearSelection();
	  this->textEdit.setTextCursor(tc);
	  this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
	  this->textEdit.removeKeyPressEventFilter();
	}
      }
      if(ev->key()==Qt::Key_Exclam){
	bool c = true;
	QTextCursor tc = this->textEdit.textCursor();
	QTextDocument *document = this->textEdit.document();
	tc.beginEditBlock();
	while(c){
	  tc.movePosition(QTextCursor::NoMove,
			  QTextCursor::MoveAnchor);
	  tc.movePosition(QTextCursor::NextCharacter,
			  QTextCursor::KeepAnchor,
			  -s1.size());
	  tc.removeSelectedText();
	  tc.insertText(s2);
	  QTextCursor nc = document->find(s1,tc);
	  if(this->hasSelection){
	    c = (!nc.isNull()) && (!nc.atEnd())  && (nc<this->ec);
	  } else {
	    c = (!nc.isNull()) && (!nc.atEnd());
	  }
	  if(c){
	    tc = nc;
	  }
	  ++nb;
	}
	tc.endEditBlock();
	tc.clearSelection();
	this->textEdit.setTextCursor(tc);
	this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
	this->textEdit.removeKeyPressEventFilter();
      }
    } else {
      this->qemacs.displayInformativeMessage(QObject::tr("type either 'y', 'n' or '!'"));
    }
    return true;
  }
  
  QEmacsTextEditReplaceFilter::~QEmacsTextEditReplaceFilter()
  {
    this->qemacs.removeUserInput(&qr);
    if((this->nb==0)||(this->nb==1)){
      this->qemacs.displayInformativeMessage(QObject::tr("replaced %1 occurrence")
					     .arg(QString::number(nb)));
    } else {
      this->qemacs.displayInformativeMessage(QObject::tr("replaced %1 occurrences")
      					     .arg(QString::number(nb)));
    }
  }
  
} // end of namespace qemacs
