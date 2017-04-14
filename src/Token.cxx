/*! 
 * \file  Token.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#include"QEmacs/Token.hxx"

namespace qemacs
{

  Token::Token()
    : line(0u),pos(0u),
      value(),flag(Standard)
  {}

  Token::Token(const unsigned short line_,
	       const unsigned short pos_,
	       const QString& token_,
	       const TokenFlag flag_)
    : line(line_),pos(pos_),
      value(token_),flag(flag_)
  {}
  
  Token::Token(const Token& src):
    line(src.line),pos(src.pos),
    value(src.value),flag(src.flag)
  {}
  
  Token& Token::operator=(const Token& src)
  {
    this->line  = src.line;
    this->pos   = src.pos;
    this->value = src.value;
    this->flag  = src.flag;
    return *this;
  }

} // end of namespace qemacs
