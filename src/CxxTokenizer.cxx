/*! 
 * \file  CxxTokenizer.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#include<limits>
#include<sstream>
#include<algorithm>
#include<stdexcept>

#include<QtCore/QDebug>
#include<QtCore/QFile>
#include<QtCore/QRegExp>
#include<QtCore/QTextStream>

#include"QEmacs/CxxTokenizer.hxx"

namespace qemacs
{

  template<typename T>
  static std::string toString(const T& src)
  {
    std::ostringstream os;
    os << src;
    return os.str();
  }

  static const QString 
  reservedCxxKeywords[63] = {"asm","auto","bool","break","case",
			     "catch","char ","class","const",
			     "const_cast","continue","default",
			     "delete","do","double","dynamic_cast",
			     "else","enum","explicit","export",
			     "extern","false","float","for","friend",
			     "goto","if","inline","int","long",
			     "mutable","namespace","new","operator",
			     "private","protected","public","register",
			     "reinterpret_cast","return","short",
			     "signed","sizeof","static","static_cast",
			     "struct","switch","template","this","throw",
			     "true","try","typedef","typeid","typename",
			     "union","unsigned","using","virtual","void",
			     "volatile","wchar_t","while"};

  unsigned short getNumberOfCxxKeywords(void)
  {
    return sizeof(reservedCxxKeywords)/sizeof(char*);
  }

  bool isReservedCxxKeywords(const QString& s)
  {
    unsigned short i;
    for(i=0;i<getNumberOfCxxKeywords();++i){
      if(s==reservedCxxKeywords[i]){
	return true;
      }
    }
    return false;
  }

  const QChar CxxTokenizer::Separator[23] = {'?',';','/','!','&','*',
					     '|','{','}','[',']','(',
					     ')','%','=','^',',',':',
					     '<','>','+','-','#'};

  static int findStringBeginning(const QString& s,
				 const QChar b)
  {
    using namespace std;
    int p;
    if(s.isEmpty()){
      return -1;
    }
    if(s[0]==b){
      return 0;
    }
    p = s.indexOf(b);
    while(p!=-1){
      if(s[p-1]!='\\'){
	return p;
      }
      p = s.indexOf(b,p+1);
    }
    return -1;
  } // end of findQStringBeginning

  static int normalizePosition(int i)
  {
    using namespace std;
    if(i==-1){
      return numeric_limits<int>::max();
    }
    return i;
  }

  bool CxxTokenizer::isCCommentOpened(void) const
  {
    return this->cStyleCommentOpened;
  } // end of CxxTokenizer::isCCommentOpened

  void CxxTokenizer::setCCommentOpened(const bool b)
  {
    this->cStyleCommentOpened = b;
  } // end of CxxTokenizer::setCCommentOpened

  void CxxTokenizer::splitLine(QString line,
			       const unsigned short lineNumber)
  {

    using namespace std;
    unsigned short cpos = 0;
    // left trimming
    if(line.isEmpty()){
      return;
    }
    while(line[cpos].isSpace()){
      ++cpos;
    }
    if(cpos!=0){
      line.remove(0,cpos);
    }
    if(this->cStyleCommentOpened){
      int pos3;
      pos3 = line.indexOf("*/");
      if(pos3==-1){
	this->fileTokens.push_back(Token(lineNumber,cpos,
					 line,Token::Comment));
	return;
      }
      this->fileTokens.push_back(Token(lineNumber,cpos,
				       line.mid(0,pos3+2),
				       Token::Comment));
      line.remove(0,pos3+2);
      cpos = pos3+2;
      this->cStyleCommentOpened=false;
    }

    while(!line.isEmpty()){
      bool treatString=false;
      bool treatCppComment=false;
      bool treatCComment=false;
      bool treatChar=false;
      int pos[4];
      const int * min_pos;
      pos[0] = normalizePosition(findStringBeginning(line,'"'));
      pos[1] = normalizePosition(line.indexOf("//"));
      pos[2] = normalizePosition(line.indexOf("/*"));
      pos[3] = normalizePosition(findStringBeginning(line,'\''));
	
      min_pos=min_element(pos,pos+4);
      if(*min_pos!=numeric_limits<int>::max()){
	ptrdiff_t diff = min_pos-pos;
	if(diff==0){
	  treatString=true;
	}
	if(diff==1){
	  treatCppComment=true;
	}
	if(diff==2){
	  treatCComment=true;
	}
	if(diff==3){
	  treatChar = true;
	}
      }

      if(treatString){
	if(pos[0]!=0){
	  this->splitStandardToken(line.mid(0,pos[0]),
				   lineNumber,cpos);
	  line.remove(0,pos[0]);
	  cpos += pos[0];
	}
	int ps = 0;
	++ps;
	bool found=false;
	for(;(ps!=line.size())&&(!found);++ps){
	  if((line[ps]=='\"')&&(line[ps-1]!='\\')){
	    found=true;
	  }
	}
	if(!found){
	  string msg("CxxTokenizer::splitLine : ");
	  msg += "found no matching \" to close string\n";
	  msg += "Error at line ";
	  msg += toString(lineNumber);
	  msg += " ("+line.toStdString()+")";
	  throw(runtime_error(msg));
	}
	this->fileTokens.push_back(Token(lineNumber,cpos,
					 line.mid(0,ps),
					 Token::String));
	line.remove(0,ps);
	cpos += ps;
      } else if (treatCppComment){
	if(pos[1]!=0){
	  this->splitStandardToken(line.mid(0,pos[1]),lineNumber,cpos);
	  line.remove(0,pos[1]);
	  cpos += pos[1];
	}
	this->fileTokens.push_back(Token(lineNumber,cpos,
					 line,Token::Comment));
	line.clear();
      } else if(treatCComment){
	if(pos[2]!=0){
	  this->splitStandardToken(line.mid(0,pos[2]),lineNumber,cpos);
	  line.remove(0,pos[2]);
	  cpos += pos[2];
	}
	pos[2]=line.indexOf("*/");
	if(pos[2]==-1){
	  this->fileTokens.push_back(Token(lineNumber,cpos,
					   line,Token::Comment));
	  line.clear();
	  this->cStyleCommentOpened=true;  
	} else {
	  this->fileTokens.push_back(Token(lineNumber,cpos,
					   line.mid(0,pos[2]+2),
					   Token::Comment));
	  line.remove(0,pos[2]+2);
	  cpos += pos[2]+2;
	}
      } else if(treatChar){
	if(this->charAsString){
	  if(pos[3]!=0){
	    this->splitStandardToken(line.mid(0,pos[3]),lineNumber,cpos);
	    line.remove(0,pos[3]);
	    cpos += pos[3];
	  }
	  int ps = 0;
	  ++ps;
	  bool found=false;
	  for(;(ps!=line.size())&&(!found);++ps){
	    if((line[ps]=='\'')&&(line[ps-1]!='\\')){
	      found=true;
	    }
	  }
	  if(!found){
	    string msg("CxxTokenizer::splitLine : ");
	    msg += "found no matching \' to close string\n";
	    msg += "Error at line : ";
	    msg += toString(lineNumber);
	    throw(runtime_error(msg));
	  }
	  this->fileTokens.push_back(Token(lineNumber,cpos,
					   line.mid(0,ps),
					   Token::String));
	  line.remove(0,ps);
	  cpos += ps;
	} else {
	  if(pos[3]!=0){
	    this->splitStandardToken(line.mid(0,pos[3]),lineNumber,cpos);
	    line.remove(0,pos[3]);
	    cpos += pos[3];
	  }
	  if(line.length()<3){
	    string msg("CxxTokenizer::splitLine : ");
	    msg += "error while reading char (1)\n";
	    msg += "Error at line : ";
	    msg += toString(lineNumber);
	    throw(runtime_error(msg));
	  }
	  if(line[1]=='\\'){
	    if(line.length()<4){
	      string msg("CxxTokenizer::splitLine : ");
	      msg += "error while reading char (2)\n";
	      msg += "Error at line : ";
	      msg += toString(lineNumber);
	      throw(runtime_error(msg));
	    }
	    if(line[3]!='\''){
	      string msg("CxxTokenizer::splitLine : ");
	      msg += "error while reading char (3)\n";
	      msg += "Error at line : ";
	      msg += toString(lineNumber);
	      throw(runtime_error(msg));	      
	    }
	    this->fileTokens.push_back(Token(lineNumber,cpos,
					     line.mid(0,4),
					     Token::Char));
	    line.remove(0,4);
	    cpos += 4;
	  } else {
	    if(line[2]!='\''){
	      string msg("CxxTokenizer::splitLine : ");
	      msg += "error while reading char (expected to read ', read '";
	      msg += line[2].toLatin1();
	      msg += "')\n";
	      msg += "Error at line : ";
	      msg += toString(lineNumber);
	      throw(runtime_error(msg));
	    }
	    this->fileTokens.push_back(Token(lineNumber,cpos,
					     line.mid(0,3),Token::Char));
	    line.remove(0,3);
	    cpos += 3;
	  }
	}
      } else {
	this->splitStandardToken(line,lineNumber,cpos);
	line.clear();
      }
    }
  }

  void
  CxxTokenizer::splitStandardToken(const QString& w,
				   const unsigned short lineNumber,
				   const unsigned short pos)
  {
    using namespace std;
    unsigned short cpos = 0;
    while(cpos<w.size()){
      while((cpos<w.size())&&(w[cpos].isSpace())){
	++cpos;
      }
      if(cpos!=w.size()){
	unsigned short cposb = cpos;
	while((cpos<w.size())&&(!w[cpos].isSpace())){
	  ++cpos;
	}
	this->splitStandardTokenAtCxxSeparator(w.mid(cposb,cpos-cposb),
					       lineNumber,cposb+pos);
      }
    }
  }

  void
  CxxTokenizer::stripComments(void){
    using namespace std;
    TokensContainer::iterator p;    
    for(p=this->fileTokens.begin();p!=this->fileTokens.end();++p){
      if(p->flag==Token::Comment){
	p = --(this->fileTokens.erase(p));
      }
    }
  } // end of CxxTokenizer::stripComments

  static bool
  nextCharacter(const QString& w,
		const unsigned short p,
		const QChar c)
  {
    if((p+1<w.size())&&
       (w[p+1]==c)){
      return true;
    }
    return false;
  }

  static bool
  nextCharacterIsDigit(const QString& w,
		       const unsigned short p)
  {
    if(p+1<w.size()){
      return w[p+1].isDigit();
    }
    return false;
  }

  static bool
  isTokensContainerEmptyOrIsLastTokenLastCharacterASeparator(const CxxTokenizer::TokensContainer& c)
  {
    if(c.isEmpty()){
      return true;
    }
    const auto& t = c.back();
    if(t.flag!=Token::Standard){
      return false;
    }
    if(t.value.isEmpty()){
      return false;
    }
    auto l = t.value[t.value.size()-1];
    for(unsigned short i=0;i<sizeof(CxxTokenizer::Separator)/sizeof(QChar);++i){
      if(l==CxxTokenizer::Separator[i]){
	return true;
      }
    }
    return false;
  }

  void
  CxxTokenizer::splitStandardTokenAtCxxSeparator(const QString& w,
						 const unsigned short line,
						 const unsigned short pos)
  {
    QRegExp number("^(\\+|-)?(\\d+\\.?\\d*|\\.\\d+)([eE](\\+|-)?[\\d]+)?");
    if(w.isEmpty()){
      return;
    }
    unsigned short cpos  = 0;
    unsigned short cposb = 0;
    while(cpos!=w.size()){
      bool found = false;
      QChar c = w[cpos];
      if((this->bExtractNumbers)&&(c.isDigit())&&((cposb==cpos))){
	found = true;
	number.indexIn(w.mid(cposb,w.size()-cposb));
	this->fileTokens.push_back(Token(line,pos+cposb,
					 w.mid(cposb,number.matchedLength()),
					 Token::Number));
	cpos += number.matchedLength();
	cposb=cpos;
      }
      if(!found){
	for(unsigned short i=0;(i<sizeof(Separator)/sizeof(QChar))&&(!found);++i){
	  if(c==Separator[i]){
	    found = true;
	    if(cposb!=cpos){
	      this->fileTokens.push_back(Token(line,pos+cposb,
					       w.mid(cposb,cpos-cposb)));
	      cposb = cpos;
	    }
	    if(c==':'){
	      if(nextCharacter(w,cpos,':')){
		this->fileTokens.push_back(Token(line,pos+cpos,"::"));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,":"));
	      }
	    } else if(c=='<'){
	      if(nextCharacter(w,cpos,'<')){
		this->fileTokens.push_back(Token(line,pos+cpos,"<<"));
		++cpos;
	      } else if(nextCharacter(w,cpos,'<')){
		this->fileTokens.push_back(Token(line,pos+cpos,"<="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"<"));
	      }
	    } else if(c=='>'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,">="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,">"));
	      }
	    } else if(c=='+'){
	      if((isTokensContainerEmptyOrIsLastTokenLastCharacterASeparator(this->fileTokens))&&
		 (nextCharacterIsDigit(w,cpos))&&
		 (number.indexIn(w.mid(cposb,w.size()-cposb))!=-1)){
		this->fileTokens.push_back(Token(line,pos+cposb,
						 w.mid(cposb,number.matchedLength()),
						 Token::Number));
		cpos += number.matchedLength()-1;
	      } else if(nextCharacter(w,cpos,'+')){
		this->fileTokens.push_back(Token(line,pos+cpos,"++"));
		++cpos;
	      } else if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"+="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"+"));
	      }
	    } else if(c=='-'){
	      if((isTokensContainerEmptyOrIsLastTokenLastCharacterASeparator(this->fileTokens))&&
		 (nextCharacterIsDigit(w,cpos))&&
		 (number.indexIn(w.mid(cposb,w.size()-cposb))!=-1)){
		this->fileTokens.push_back(Token(line,pos+cposb,
						 w.mid(cposb,number.matchedLength()),
						 Token::Number));
		cpos += number.matchedLength()-1;
	      } else if(nextCharacter(w,cpos,'-')){
		this->fileTokens.push_back(Token(line,pos+cpos,"--"));
		++cpos;
	      } else if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"-="));
		++cpos;
	      } else if(nextCharacter(w,cpos,'>')){
		if(nextCharacter(w,cpos+1,'*')){
		  this->fileTokens.push_back(Token(line,pos+cpos,"->*"));
		  ++cpos;
		  ++cpos;
		} else {
		  this->fileTokens.push_back(Token(line,pos+cpos,"->"));
		  ++cpos;
		}
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"-"));
	      }
	    } else if(c=='*'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"*="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"*"));
	      }
	    } else if(c=='/'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"/="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"/"));
	      }
	    } else if(c=='%'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"%="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"%"));
	      }
	    } else if(c=='|'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"|="));
		++cpos;
	      } else if(nextCharacter(w,cpos,'|')){
		this->fileTokens.push_back(Token(line,pos+cpos,"||"));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"|"));
	      }
	    } else if(c=='!'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"!="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"!"));
	      }
	    } else if(c=='&'){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"&="));
		++cpos;
	      } else if(nextCharacter(w,cpos,'&')){
		this->fileTokens.push_back(Token(line,pos+cpos,"&&"));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"&"));
	      }
	    } else if(c=='='){
	      if(nextCharacter(w,cpos,'=')){
		this->fileTokens.push_back(Token(line,pos+cpos,"=="));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"="));
	      }
	    } else if(c=='.'){
	      if(nextCharacter(w,cpos,'*')){
		this->fileTokens.push_back(Token(line,pos+cpos,".*"));
		++cpos;
	      } else {
		this->fileTokens.push_back(Token(line,pos+cpos,"."));
	      }
	    } else {
	      QString t;
	      t.append(c);
	      this->fileTokens.push_back(Token(line,pos+cpos,t));
	    }
	    cposb=cpos+1;
	  }
	}
	++cpos;
      }
    }
    if(cposb!=cpos){
      this->fileTokens.push_back(Token(line,pos+cposb,
				       w.mid(cposb,cpos-cposb)));
    }
  } // end of CxxTokenizer::splitToken
  
  void
  CxxTokenizer::openFile(const QString& f)
  {
    using namespace std;
    this->fileTokens.clear();
    QFile file(f);
    if (!file.open(QIODevice::ReadOnly |
		   QIODevice::Text)){
      return;
    }
    QTextStream in(&file);
    unsigned short lineNumber;
    lineNumber=0;
    while(!in.atEnd()){
      QString line = in.readLine();
      ++lineNumber;
      if(!line.isEmpty()){
	this->splitLine(line,lineNumber);
      }
    }
    file.close();
  }

  void
  CxxTokenizer::parseString(const QString& s,
			    const unsigned short l)
  {
    this->fileTokens.clear();
    this->splitLine(s,l);
  } // end of CxxTokenizer::parseOneString

  bool
  CxxTokenizer::isComment(const Token& t)
  {
    return t.flag==Token::Comment;
  }

  CxxTokenizer::CxxTokenizer()
    : cStyleCommentOpened(false),
      bExtractNumbers(true),
      charAsString(false)
  {}
  
  CxxTokenizer::CxxTokenizer(const QString& f)
    : cStyleCommentOpened(false),
      bExtractNumbers(true),
      charAsString(false)
  {
    this->openFile(f);
  }

  void
  CxxTokenizer::treatCharAsString(const bool b){
    this->charAsString = b;
  } // end of CxxTokenizer::treatCharAsString

  void
  CxxTokenizer::extractNumbers(const bool b){
    this->bExtractNumbers = b;
  } // end of CxxTokenizer::extractNumbers

  bool
  CxxTokenizer::isValidIdentifier(const QString& s, 
				  const bool b)
  {
    using namespace std;
    if(s.isEmpty()){
      return false;
    }
    QString::const_iterator p = s.begin();
    if(p->isDigit()){
      return false;
    }
    for(;p!=s.end();++p){
      if((!p->isLetter())&&
	 (!(p->isDigit()))&&
	 (*p!='_')){
	return false;
      }
      if(p->isSpace()){
	return false;
      }
    }
    if(b){
      if(isReservedCxxKeywords(s)){
	return false;
      }
    }
    return true;
  } // end of CxxTokenizer::isValidIdentifier

  bool
  CxxTokenizer::isValidFunctionIdentifier(const QString& s, 
					  const bool checkCxxKeywords)
  {
    using namespace std;
    if(s.isEmpty()){
      return false;
    }
    QString::const_iterator p = s.begin();
    if(p->isDigit()){
      return false;
    }
    for(;p!=s.end();++p){
      if((!p->isLetter())&&
	 (!(p->isDigit()))&&
	 (*p!='_')&&(*p!=':')){
	return false;
      }
      if(p->isLetter()){
	return false;
      }
      if(*p==':'){
	++p;
	if(p==s.end()){
	  return false;
	}
	if(*p!=':'){
	  return false;
	}
	++p;
	if(p==s.end()){
	  return false;
	}
	if((!p->isLetter())&&
	   (!(p->isDigit()))&&
	   (*p!='_')){
	  return false;
	}
      }
    }

    if(checkCxxKeywords){
      if(isReservedCxxKeywords(s)){
	return false;
      }
    }

    return true;

  } // end of CxxTokenizer::isValidFunctionIdentifier

  const Token&
  CxxTokenizer::back(void) const
  {
    return this->fileTokens.back();
  } // end of CxxTokenizer::back

  CxxTokenizer::const_iterator
  CxxTokenizer::begin(void) const
  {
    return this->fileTokens.begin();
  } // end of CxxTokenizer::begin

  CxxTokenizer::const_iterator
  CxxTokenizer::end(void) const
  {
    return this->fileTokens.end();
  } // end of CxxTokenizer::end

  void
  CxxTokenizer::clear(void)
  {
    this->cStyleCommentOpened = false;
    this->fileTokens.clear();
  } // end of CxxTokenizer::clear

  void
  CxxTokenizer::reset(void)
  {
    this->clear();
  } // end of CxxTokenizer::reset

  void
  CxxTokenizer::checkNotEndOfLine(const QString& method,
				  CxxTokenizer::const_iterator& p, 
				  const CxxTokenizer::const_iterator pe)
  {
    using namespace std;
    if(p==pe){
      string msg(method.toStdString());
      msg += " : unexpected end of line";
      throw(runtime_error(msg));
    }
  } // end of CxxTokenizer::checkNotEndOfLine
    
  void
  CxxTokenizer::checkNotEndOfLine(const QString& method,
				  const QString& error,
				  CxxTokenizer::const_iterator& p, 
				  const CxxTokenizer::const_iterator pe)
  {
    using namespace std;
    if(p==pe){
      string msg(method.toStdString());
      msg += " : unexpected end of line";
      if(!error.isEmpty()){
	msg += " ("+error.toStdString()+")";
      }
      throw(runtime_error(msg));
    }
  } // end of CxxTokenizer::checkNotEndOfLine
    
  void
  CxxTokenizer::readSpecifiedToken(const QString& method,
				   const QString& value,
				   CxxTokenizer::const_iterator& p, 
				   const CxxTokenizer::const_iterator pe)
  {
    using namespace std;
    CxxTokenizer::checkNotEndOfLine(method,"expected '"+value+"'",p,pe);
    if(p->value!=value){
      string msg(method.toStdString());
      msg += " : unexpected token '"+p->value.toStdString()+"'";
      msg += " (expected '"+value.toStdString()+"')";
      throw(runtime_error(msg));
    }
    ++p;
  } // end of CxxTokenizer::readSpecifiedToken

  CxxTokenizer::size_type
  CxxTokenizer::size() const
  {
    return this->fileTokens.size();
  } // end of CxxTokenizer::size

  CxxTokenizer::~CxxTokenizer()
  {}

} // end of namespace qemacs


  // CxxTokenizer::TokensContainer::iterator
  // CxxTokenizer::joinPreviousCurrentNext(CxxTokenizer::TokensContainer& res,
  // 					CxxTokenizer::TokensContainer& src,
  // 					CxxTokenizer::TokensContainer::iterator p)
  // {
  //   using namespace std;
  //   TokensContainer::iterator p2;
  //   TokensContainer::iterator a;
  //   TokensContainer::iterator b;
  //   bool previous=false;
  //   bool next=false;
  //   if(p==src.end()){
  //     return p;
  //   }
  //   b=p;
  //   ++b;
  //   if(p!=src.begin()){
  //     a=p;
  //     --a;
  //     if(a->line==p->line){
  // 	previous=true;
  //     }
  //   }
  //   if(b!=src.end()){
  //     if(b->line==p->line){
  // 	next=true;
  //     }
  //   }
  //   if(res.empty()){
  //     string msg("CxxTokenizer::joinPreviousCurrentNext : ");
  //     msg += "internal error \"::\"\n";
  //     msg += "Error at line : ";
  //     msg += toString(p->line);
  //     throw(runtime_error(msg));
  //   }
  //   if(previous&&next){
  //     res.back().value.append(p->value+b->value);
  //     return b;
  //   }
  //   if(previous){
  //     res.back().value.append(p->value);
  //     return p;
  //   }
  //   if(next){
  //     res.push_back(*p);
  //     res.back().value.append(b->value);
  //     return b;
  //   }
  //   res.push_back(*p);
  //   return p;    
  // } // end of CxxTokenizer::joinPreviousCurrentNext

// QString
// CxxTokenizer::readString(CxxTokenizer::const_iterator& p, 
// 			     const CxxTokenizer::const_iterator pe)
// {
//   using namespace std;
//   using namespace tfel::utilities;
//   CxxTokenizer::checkNotEndOfLine("CxxTokenizer::readString","",p,pe);
//   if(p->flag!=Token::String){
// 	string msg("CxxTokenizer::readString : ");
// 	msg += "expected to read a string (read '"+p->value.toStdString()+"').\n";
// 	throw(runtime_error(msg));
//   }
//   if(p->value.size()<2){
// 	string msg("CxxTokenizer::readString : ");
// 	msg += "internal error (invalid string size)";
// 	throw(runtime_error(msg));
//   }
//   QString value = p->value.mid(1,p->value.size()-2);
//   ++p;
//   return value;
// } // end of CxxTokenizer::readString

// double
// CxxTokenizer::readDouble(CxxTokenizer::const_iterator& p, 
// 			     const CxxTokenizer::const_iterator pe)
      
// {
//   using namespace std;
//   double res;
//   bool b;
//   CxxTokenizer::checkNotEndOfLine("CxxTokenizer::readDouble","expected number",p,pe);
//   res = p->value.toDouble(&b);
//   if(!b){
// 	ostringstream msg;
// 	msg << "CxxTokenizer::readDouble : ";
// 	msg << "could not read value from token '"+p->value.toStdString()+"'.\n";
// 	throw(runtime_error(msg.str()));
//   }
//   ++p;
//   return res;
// } // end of CxxTokenizer::readDouble

// unsigned int
// CxxTokenizer::readUnsignedInt(CxxTokenizer::const_iterator& p, 
// 				  const CxxTokenizer::const_iterator pe)
// {
//   using namespace std;
//   unsigned int res;
//   bool b;
//   CxxTokenizer::checkNotEndOfLine("CxxTokenizer::readUnsignedInt","expected number",p,pe);
//   res = p->value.toUInt(&b);
//   if(!b){
// 	ostringstream msg;
// 	msg << "CxxTokenizer::readDouble : ";
// 	msg << "could not read value from token '"+p->value.toStdString()+"'.\n";
// 	throw(runtime_error(msg.str()));
//   }
//   ++p;
//   return res;
// } // end of CxxTokenizer::readUnsignedInt

// QString
// CxxTokenizer::readNumber(QString::const_iterator& p,
// 			   const QString::const_iterator  pe)
// {
//   using namespace std;
//   QString word;
//   QString::const_iterator b=p;
//   if((*p=='-')||(*p=='+')){
//     ++p;
//   }
//   //reading decimal part
//   while((p->isDigit())&&(p!=pe)){
//     ++p;
//   }
//   if(p!=pe){
//     if(*p=='.'){
// 	++p;
// 	while((p->isDigit())&&(p!=pe)){
// 	  ++p;
// 	}
//     }
//   }
//   if(p!=pe){
//     if((*p=='e')||(*p=='E')){
// 	++p;
// 	if(p==pe){
// 	  --p;
// 	} else {
// 	  if((*p=='+')||(*p=='-')){
// 	    ++p;
// 	    if(p==pe){
// 	      --(--p);
// 	    } else if (p->isDigit()){
// 	      while((p->isDigit())&&(p!=pe)){
// 		++p;
// 	      }
// 	    } else {
// 	      --(--p);
// 	    }
// 	  } else if (p->isDigit()){
// 	    while((p->isDigit())&&(p!=pe)){
// 	      ++p;
// 	    }
// 	  } else {
// 	    --p;
// 	  }
// 	}
//     }
//   }
//   if(p!=pe){
//     if(*p=='f'){
// 	++p;
//     }
//   }
//   while(b!=p){
//     word.append(*b);
//     ++b;
//   }
//   return word;
// } // end of CxxTokenizer::readNumber

// void
// CxxTokenizer::extractNumbers(QStringList& res,
// 			       const QString& s)
// {
//   using namespace std;
//   QString::const_iterator p = s.begin();
//   const QString::const_iterator pe = s.end();
//   QChar buf[3];
//   buf[1] = '\0';
//   buf[2] = '\0';
//   while(p!=pe){
//     if(*p=='.'){
// 	if(p==s.begin()){
// 	  if(s.size()!=1){
// 	    ++p;
// 	    if(p->isDigit()){
// 	      --p;
// 	      res.push_back(CxxTokenizer::readNumber(p,pe));
// 	    } else {
// 	      --p;
// 	      buf[0] = *p;
// 	      res.push_back(QString(buf[0]));
// 	      ++p;
// 	    }
// 	  } else {
// 	    buf[0] = *p;
// 	    res.push_back(QString(buf[0]));
// 	    ++p;
// 	  }
// 	}else{
// 	  buf[0] = *p;
// 	  res.push_back(QString(buf[0]));
// 	  ++p;
// 	}
//     } else if((*p=='+')||(*p=='-')){
// 	if(p==s.begin()){
// 	  if(s.size()!=1){
// 	    ++p;
// 	    if(p->isDigit()){
// 	      --p;
// 	      res.push_back(CxxTokenizer::readNumber(p,pe));
		
// 	    } else if (*p=='='){
// 	      --p;
// 	      buf[0] = *p;
// 	      buf[1] = '=';
// 	      res.push_back(QString(buf[0])+QString(buf[1]));
// 	      buf[1] = '\0';
// 	      ++(++p);
// 	    }else {
// 	      --p;
// 	      buf[0] = *p;
// 	      res.push_back(QString(buf[0]));
// 	      ++p;
// 	    }
// 	  } else {
// 	    buf[0] = *p;
// 	    res.push_back(QString(buf[0]));
// 	    ++p;
// 	  }
// 	}else{
// 	  buf[0] = *p;
// 	  res.push_back(QString(buf[0]));
// 	  ++p;
// 	}
//     } else if(p->isDigit()){
// 	res.push_back(CxxTokenizer::readNumber(p,pe));
//     } else {
// 	QString::const_iterator pp = find(p,pe,'+');
// 	QString::const_iterator pm = find(p,pe,'-');
// 	QString::const_iterator pd = find(p,pe,'.');
// 	QString word;
// 	if((pp==pe)&&(pm==pe)&&(pd==pe)){
// 	  // this is the end
// 	  while(p!=pe){
// 	    word.append(*p);
// 	    ++p;
// 	  }
// 	  res.push_back(word);
// 	} else {
// 	  if((pp-p<pm-p)&&(pp-p<pd-p)){
// 	    // next one is '+'
// 	    while(p!=pp){
// 	      word.append(*p);
// 	      ++p;
// 	    }
// 	  } else if((pm-p<pp-p)&&(pm-p<pd-p)){
// 	    // next one is '-'
// 	    while(p!=pm){
// 	      word.append(*p);
// 	      ++p;
// 	    }
// 	  } else {
// 	    // next one is '.'
// 	    while(p!=pd){
// 	      word.append(*p);
// 	      ++p;
// 	    }
// 	  }
// 	  res.push_back(word);
// 	  buf[0] = *p;
// 	  res.push_back(QString(buf[0]));
// 	  ++p;
// 	}
//     }
//   }
// } // end of CxxTokenizer::extractNumbers

// void
// CxxTokenizer::splitString(QStringList& res,
// 			    QString s, const QString delim)
// {
//   using namespace std;
//   QString tmp;
//   int posb = 0;
//   while((!s.isEmpty())&&(posb!=-1)){
//     posb = s.indexOf(delim);
//     if(posb!=0){
// 	res.push_back(s.mid(0,posb));
//     }
//     if(posb!=-1){
// 	res.push_back(delim);
// 	s.remove(0,posb+delim.length());
//     } else {
// 	s.remove(0,posb);
//     }
//   }
// }

// void
// CxxTokenizer::splitString2(QStringList& res,QString s)
// {
//   using namespace std;
//   QString tmp;
//   int posb = 0;
//   while((!s.isEmpty())&&(posb!=-1)){
//     posb = s.indexOf(">");
//     if(posb!=0){
// 	res.push_back(s.mid(0,posb));
//     }
//     if(posb!=-1){
// 	if((posb+1<s.length())&&(s[posb+1]=='>')){
// 	  res.push_back(">>");
// 	  s.remove(0,posb+2);
// 	  ++posb;
// 	} else {
// 	  res.push_back(">");
// 	  s.remove(0,posb+1);
// 	}
//     } else {
// 	s.remove(0,posb);
//     }
//   }
// }

// void
// CxxTokenizer::splitString3(QStringList& res,QString s)
// {
//   using namespace std;
//   QString tmp;
//   int posb = 0;
//   while((!s.isEmpty())&&(posb!=-1)){
//     posb = s.indexOf("<");
//     if(posb!=0){
// 	res.push_back(s.mid(0,posb));
//     }
//     if(posb!=-1){
// 	if((posb+1<s.length())&&(s[posb+1]=='<')){
// 	  res.push_back("<<");
// 	  s.remove(0,posb+2);
// 	  ++posb;
// 	} else {
// 	  res.push_back("<");
// 	  s.remove(0,posb+1);
// 	}
//     } else {
// 	s.remove(0,posb);
//     }
//   }
// }

// void 
// CxxTokenizer::join(QStringList& src,
// 		     const QString& s1,
// 		     const QString& s2)
// {
//   using namespace std;
//   QStringList res;
//   QStringList::iterator p;

//   if(src.isEmpty()){
//     return;
//   }
    
//   p=src.begin();
//   res.push_back(*p);
//   ++p;
//   for(;p!=src.end();++p){
//     if((res.back()==s1)&&(*p==s2)){
// 	res.back().append(s2);
//     } else {
// 	res.push_back(*p);
//     }
//   }
//   QStringList tmp(src);
//   src = res;
//   res = tmp;

// } // end of CxxTokenizer::join

// void
// CxxTokenizer::splitAtCxxTokenSperator(QStringList& tokens)
// {
//   using namespace std;
    
//   QStringList res(tokens);
//   QStringList init;
//   QStringList tmp;
//   QStringList::const_iterator p;
//   unsigned short i;

//   for(i=0;i<sizeof(Separator)/sizeof(char);++i){
//     if(Separator[i]=='<'){
// 	// Special care is taken for '<' as "< <" and "<<" are both 
// 	// valid keywords 
// 	tmp  = init;
// 	init = res;
// 	res  = tmp; 
// 	res.clear();    
// 	for(p=init.begin();p!=init.end();++p){
// 	  if(p->indexOf("<")!=-1){
// 	    QStringList v;
// 	    splitString3(v,*p);
// 	    res << v;
// 	  } else {
// 	    res.push_back(*p);
// 	  }
// 	}
//     } else if(Separator[i]=='>'){
// 	tmp  = init;
// 	init = res;
// 	res  = tmp;
// 	res.clear();    
// 	for(p=init.begin();p!=init.end();++p){
// 	  if(p->indexOf(">")!=-1){
// 	    QStringList v;
// 	    splitString2(v,*p);
// 	    res << v;
// 	  } else {
// 	    res.push_back(*p);
// 	  }
// 	}
//     } else {
// 	tmp  = init;
// 	init = res;
// 	res  = tmp;
// 	res.clear();
// 	for(p=init.begin();p!=init.end();++p){
// 	  if(p->indexOf(Separator[i])!=-1){
// 	    QStringList v;
// 	    QString s;
// 	    s.append(Separator[i]);
// 	    splitString(v,*p,s);
// 	    res << v;
// 	  } else {
// 	    res.push_back(*p);
// 	  }
// 	}
//     }

//   }

//   if(this->bExtractNumbers){
//     tmp  = init;
//     init = res;
//     res  = tmp;
//     res.clear();
//     for(p=init.begin();p!=init.end();++p){
// 	if((p->indexOf("+")!=-1)||
// 	   (p->indexOf("-")!=-1)||
// 	   (p->indexOf(".")!=-1)){
// 	  QStringList v;
// 	  CxxTokenizer::extractNumbers(v,*p);
// 	  res << v;
// 	} else {
// 	  res.push_back(*p);
// 	}
//     }
//   }

//   join(res,"/","/"); 
//   join(res,"/","*");    
//   join(res,"*","/");
//   join(res,":",":");
//   join(res,"-",">");
//   join(res,"<","<");
//   join(res,"<","=");
//   join(res,">","=");
//   join(res,"+","+");
//   join(res,"-","-");
//   join(res,"+","=");
//   join(res,"-","=");
//   join(res,"*","=");
//   join(res,"/","=");
//   join(res,"%","=");
//   join(res,"|","=");
//   join(res,"!","=");
//   join(res,"&","&");
//   join(res,"=","=");
//   join(res,".","*");
//   join(res,"->","*");    
//   join(res,"|","|");

//   swap(res,tokens);

// }


// void 
// CxxTokenizer::splitTokens(void)
// {
//   using namespace std;
//   TokensContainer::iterator p;
//   for(p=this->fileTokens.begin();p!=this->fileTokens.end();++p){
// 	if(p->flag==Token::Standard){
// 	  unsigned short cpos = p->pos;	  
// 	  QStringList splittedTokens;
// 	  splittedTokens.push_back(p->value);
// 	  splitAtCxxTokenSperator(splittedTokens);
// 	  unsigned short line = p->line;
// 	  QStringList::iterator p2 = splittedTokens.begin();
// 	  TokensContainer::iterator current=p;
// 	  for(;p2!=splittedTokens.end();++p2){
// 	    p = this->fileTokens.insert(current,Token(line,cpos,*p2));
// 	  }
// 	  this->fileTokens.erase(current);
// 	}
//   }
// } // end of CxxTokenizer::splitStandardTokens

// void
// CxxTokenizer::treatPreprocessorDirectives(void)
// {
//   using namespace std;

//   static const char * const CppKeywords[11] = {"#define","#undef","#include",
// 						 "#line","#error","#if","#ifdef",
// 						 "#ifndef","elif","#else","#endif"};
//   static const unsigned short CppKeywordsNumber = 11;

//   unsigned short i;
//   TokensContainer::iterator p;
//   int pos;
//   for(i=0;i<CppKeywordsNumber;++i){
//     for(p=this->fileTokens.begin();p!=this->fileTokens.end();++p){
// 	if(p->flag==Token::Standard){
// 	  pos=p->value.indexOf(CppKeywords[i]);
// 	  if(pos!=-1){
// 	    TokensContainer::iterator b=p;
// 	    TokensContainer::iterator current;
// 	    b->flag=Token::Preprocessor;
// 	    ++p;
// 	    if(p==this->fileTokens.end()){
// 	      --p;
// 	    } else if(p->line!=b->line){
// 	      --p;
// 	    } else {
// 	      while((p->line==b->line)&&
// 		    (p!=this->fileTokens.end())){
// 		current = p;
// 		++p;
// 		if(current->flag!=Token::Comment){
// 		  b->value.append(" ");
// 		  b->value.append(current->value);
// 		  this->fileTokens.erase(current);
// 		}
// 	      }
// 	      --p;
// 	    }
// 	  }
// 	}
//     }
//   }
// } // end of CxxTokenizer::treatPreprocessorDirectives

