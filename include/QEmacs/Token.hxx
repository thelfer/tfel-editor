/*! 
 * \file  Token.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#ifndef _LIB_QEMACS_TOKEN_H_
#define _LIB_QEMACS_TOKEN_H_ 

#include<QtCore/QString>

#include"QEmacs/Config.hxx"

namespace qemacs
{

  /*!
   * \brief class representing a token in a C++-like file
   */
  struct QEMACS_VISIBILITY_EXPORT Token
  {

    //! possible type of the token
    enum TokenFlag{Standard,Number,Comment,
		   String,Char,Preprocessor};

    //! line number
    unsigned short line;
    //! position
    unsigned short pos;
    //! string holded by the token
    QString value;
    //! type of the token
    TokenFlag flag;

    /*!
     * \brief default constructor
     */
    Token();

    /*!
     * \brief constructor
     * \param line_  : line number
     * \param pos_   : position in line
     * \param token_ : token value
     * \param flag_  : token type
     */
    Token(const unsigned short,
	  const unsigned short,
	  const QString&,
	  const TokenFlag = Standard);

    /*!
     * \brief copy constructor
     * \param src : token to be copied
     */
    Token(const Token&);

    /*!
     * \brief assignement operator
     * \param src : token to be assigned
     * \return this token
     */
    Token&
    operator=(const Token&);

  }; // end of struct Token

} // end of namespace qemacs

#endif /* _LIB_QEMACS_TOKEN_H */

