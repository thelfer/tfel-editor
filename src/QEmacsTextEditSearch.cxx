/*! 
 * \file  QEmacsTextEditSearch.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#include<QtCore/QDebug>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsTextEditSearch.hxx"

namespace qemacs{

  
  QEmacsTextEditSearchFilter::QEmacsTextEditSearchFilter(QEmacsTextEditBase& t,
							 QEmacsTextEditSearch& p)
    : QEmacsTextEditKeyPressEventFilter(t),
      textEdit(t),
      s(p)
  {
    QObject::connect(&p,SIGNAL(destroyed()),
		     this,SLOT(lineEditDestroyed()));
  }
  
  void
  QEmacsTextEditSearchFilter::lineEditDestroyed()
  {
    delete this;
  }
  
  bool
  QEmacsTextEditSearchFilter::filterKeyPressEvent(QKeyEvent * const e)
  {
    const int k                   = e->key();
    const Qt::KeyboardModifiers m = e->modifiers(); 
    if((m==Qt::NoModifier)&&(k==Qt::Key_Escape)){
      this->textEdit.removeKeyPressEventFilter();
      return true;
    }
    if(k==Qt::Key_Control){
      return true;
    }
    if((m==Qt::NoModifier)&&((k==Qt::Key_Up)||(k==Qt::Key_Down)||
			     (k==Qt::Key_Left)||(k==Qt::Key_Right)||
			     (k==Qt::Key_Return))){
      this->textEdit.removeKeyPressEventFilter();
      return false;
    }
    if((m==Qt::ControlModifier)&&(k==Qt::Key_S)){
      if(this->s.getFlag()==0){
	this->s.findNext();
      } else if(this->s.getFlag()==QTextDocument::FindBackward){
	this->s.setFlag(nullptr);
	this->s.findNext();
      }
      return true;
    }
    if((m==Qt::ControlModifier)&&(k==Qt::Key_R)){
      if(this->s.getFlag()==QTextDocument::FindBackward){
	this->s.findNext();
      } else if(this->s.getFlag()==0){
	this->s.setFlag(QTextDocument::FindBackward);
	this->s.findNext();
      }
      return true;
    }
    // if(!stdkey){
    //   this->textEdit.removeKeyPressEventFilter();
    //   return false;
    // }
    this->s.keyPressEvent(e);
    return true;
  } // end of QEmacsTextEditSearchFilter::filterKeyPressEvent

  bool QEmacsTextEditSearchFilter::isOk() const
  {
    return true;
  } // end of QEmacsTextEditSearch::isOk

  QEmacsTextEditSearchFilter::~QEmacsTextEditSearchFilter() = default;

  QEmacsTextEditSearch::QEmacsTextEditSearch(QEmacsTextEditBase& t,
					     QEmacsWidget& p,
					     const QTextDocument::FindFlags f)
      : QEmacsLineEdit("",p),
	textEdit(t),
	sf(new QEmacsTextEditSearchFilter(t,*this)),
	flag(nullptr)
  {
    this->cursor = this->textEdit.textCursor();
    QObject::connect(this->sf,SIGNAL(destroyed()),
		     this,SLOT(searchFilterDestroyed()));
    QObject::connect(this,SIGNAL(textChanged(const QString&)),
		     this,SLOT(search(const QString&)));
    this->textEdit.setKeyPressEventFilter(this->sf);
    this->setFlag(f);
  }
  
  QEmacsTextEditSearch::~QEmacsTextEditSearch() = default;

  void
  QEmacsTextEditSearch::search(const QString& s)
  {
    this->textEdit.setTextCursor(this->cursor);
    if(!this->textEdit.find(s,this->flag)){
      this->qemacs.displayInformativeMessage("failing search");
    }
  } // end of QEmacsTextEditSearch::search

  void
  QEmacsTextEditSearch::searchFilterDestroyed()
  {
    this->sf = nullptr;
    this->qemacs.removeUserInput(this);
  }

  void
  QEmacsTextEditSearch::findNext()
  {
    QTextCursor c = this->textEdit.textCursor();
    int p = c.selectionEnd();
    if(c.hasSelection()){
      c.setPosition(p,QTextCursor::KeepAnchor);
    } else {
      c.setPosition(p,QTextCursor::MoveAnchor);
    }
    this->textEdit.setTextCursor(c);
    if(this->textEdit.find(this->input->text(),this->flag)){
      this->cursor = c;
    } else {
      this->qemacs.displayInformativeMessage("failing search");
    }
  } // end of QEmacsTextEditSearch::updateCursor

  void
  QEmacsTextEditSearch::treatUserInput()
  {}

  QTextDocument::FindFlags
  QEmacsTextEditSearch::getFlag() const
  {
    return this->flag;
  }

  void
  QEmacsTextEditSearch::setFlag(const QTextDocument::FindFlags f)
  {
    this->flag = f;
    if(this->flag==0){
      this->setLabel("i-search :");
    } else {
      this->setLabel("i-search backward :");
    }
  }

} // end of namespace qemacs
