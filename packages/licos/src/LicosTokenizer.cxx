/*! 
 * \file  LicosTokenizer.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 18 sept. 2012
 */

#include<stdexcept>

#include<QtCore/QDebug>

#include"QEmacs/LicosTokenizer.hxx"

namespace qemacs
{
  
  LicosTokenizer::LicosTokenizer()
    : state(LicosTokenizer::STANDARD)
  {
    this->treatCharAsString(true);
    this->keepCommentBoundaries(true);
  }

  void LicosTokenizer::parseString(const QString& s)
  {
    using tfel::utilities::Token;
    using CxxTokenizer = tfel::utilities::CxxTokenizer;
    if(this->getState()==FAILED){
      return;
    }
    try{
      const auto b = this->isCStyleCommentOpened();
      this->clear();
      this->setCStyleCommentOpened(b);
      CxxTokenizer::parseString(s.trimmed().toStdString());
    } catch(std::exception& e){
      this->tokens.clear();
      this->state = FAILED;
      this->error = e.what();
    }
    if(this->state!=FAILED){
      auto p=this->begin();
      while(p!=this->end()){
	if(p->flag==Token::Standard){
	  if(p->value=="{"){
	    this->state = ARRAYOPENED;
	    this->apos << p->offset;
	  } else if(p->value=="}"){
	    if(this->apos.empty()){
	      this->state = FAILED;
	      this->error =  "closing unoppened array at line '";
	      this->error += QString::number(p->line)+"'";
	    } else {
	      this->apos.pop_back();
	      if(this->apos.isEmpty()){
		this->state = STANDARD;
	      }
	    }
	  }
	}
	++p;
      }
    }
  }

  LicosTokenizer::State LicosTokenizer::getState() const
  {
    return this->state;
  }

  QVector<unsigned short>
  LicosTokenizer::getBeginningOfArrays() const
  {
    return this->apos;
  }

  QString LicosTokenizer::getErrorMessage()
  {
    return this->error;
  }

  void LicosTokenizer::reset()
  {
    this->state = STANDARD;
    this->apos.clear();
    this->error.clear();
  } // end of LicosTokenizer::reset

  void LicosTokenizer::setFailedState(const QString& e)
  {
    this->state = FAILED;
    this->error = e;
  } // end of LicosTokenizer::setFailedState

} // end of namespace qemacs

