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
  
  Token::Token(const Token&) = default;
  Token& Token::operator=(const Token&) = default;
} // end of namespace qemacs
