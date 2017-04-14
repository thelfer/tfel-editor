/*! 
 * \file  CSyntaxHighlighterBase.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include<stdexcept>

#include<QtCore/QDebug>

#include"QEmacs/CxxTokenizer.hxx"
#include"QEmacs/CSyntaxHighlighterBase.hxx"

namespace qemacs
{
  
  CSyntaxHighlighterBase::CSyntaxHighlighterBase(QTextDocument *p)
    : QSyntaxHighlighter(p),
      cCharAsString(false)
  {
    this->preprocessorFormat.setForeground(Qt::darkMagenta);
    this->keyFormat.setForeground(Qt::darkMagenta);
    this->numberFormat.setForeground(Qt::darkRed);
    this->quotationFormat.setForeground(Qt::darkGreen);
    this->commentFormat.setForeground(Qt::red);
  }

  void
  CSyntaxHighlighterBase::highlightBlock(const QString &text)
  {
    CxxTokenizer t;
    CxxTokenizer::const_iterator p;
    CxxTokenizer::const_iterator pe;
    t.treatCharAsString(this->cCharAsString);
    t.setCCommentOpened(this->previousBlockState()==1);
    try{
      t.parseString(text);
    } catch(std::exception&){}
    for(p=t.begin();p!=t.end();++p){
      QTextCharFormat f;
      bool b = false;
      if((p->flag==Token::String)||
	 (p->flag==Token::Char)){
	f = this->quotationFormat;
	b = true;
      } else if(p->flag==Token::Number){
	f = this->numberFormat;
	b = true;
      } else if (p->flag==Token::Comment){
	f = this->commentFormat;
	b = true;
      } else {
	foreach (const HighlightingRule &rule, highlightingRules){
	  if(rule.key==p->value){
	    f = rule.format;
	    b = true;
	  }
	}
      }
      if(b){
	this->setFormat(p->pos,p->value.size(),f);
      }
    }
    if(t.isCCommentOpened()){
      this->setCurrentBlockState(1);
    } else {
      this->setCurrentBlockState(0);
    }
  }

} // end of namespace qemacs

