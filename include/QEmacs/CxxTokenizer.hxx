/*! 
 * \file  CxxTokenizer.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#ifndef _LIB_QEMACS_CXXTOKENIZER_H_
#define _LIB_QEMACS_CXXTOKENIZER_H_ 

#include<QtCore/QString>
#include<QtCore/QList>

#include"QEmacs/Config.hxx"
#include"QEmacs/Token.hxx"

namespace qemacs
{

  /*!
   * \return the number of c++ keywords
   */
  QEMACS_VISIBILITY_EXPORT unsigned short
  getNumberOfCxxKeywords(void);
  
  /*!
   * \return true if the specified QString is a of c++ keyword
   * \param[in] s : QString to be tested
   */
  QEMACS_VISIBILITY_EXPORT bool
  isReservedCxxKeywords(const QString&);

  /*!
   * class in charge of reading a C++-style file and converting it
   * in tokens.
   */
  struct QEMACS_VISIBILITY_EXPORT CxxTokenizer
  {

    static const QChar Separator[23];
    
    //! a simple alias
    typedef QList<Token> TokensContainer;
    //! a simple alias
    typedef TokensContainer::const_iterator const_iterator;
    //! a simple alias
    typedef TokensContainer::size_type size_type;

    /*!
     * \return true if the given string is a valid variable name.
     * \param[in] s : variable name
     * \param[in] b : boolean telling if C++ keywords are allowed.
     */
    static bool
    isValidIdentifier(const QString&,const bool=true);

    /*!
     * \return true if the given token is a comment
     */
    static bool
    isComment(const Token&);

    /*!
     * \brief default constructor
     */
    CxxTokenizer();

    /*!
     * \brief constructor
     * Open the given file.
     * \param[in] f : file name
     */
    CxxTokenizer(const QString&);

    /*!
     * \brief analyse the string given
     * all previous tokens are erased
     * \param[in] s : string to be parsed
     * \param[in] l : line number
     */ 
    virtual void
    parseString(const QString&,
		const unsigned short = 0u);

    /*!
     * \brief import the specified file
     * \param[in] f : file name
     */ 
    virtual void
    openFile(const QString&);

    /*!
     * \brief remove all comment tokens
     */ 
    virtual void
    stripComments(void);

    /*!
     * \brief clear the object
     */ 
    virtual void
    clear(void);

    /*!
     * \brief clear the object
     */ 
    virtual void
    reset(void);

    /*!
     * \brief set if char tokens shall be treated as string tokens
     * \param[in] b : boolean telling if char tokens shall be
     * treated as string tokens
     */ 
    virtual void
    treatCharAsString(const bool);

    /*!
     * \brief set if tokens shall be split to extract numbers
     * \param[in] b : boolean telling numbers shall be extracted
     * from tokens
     */ 
    virtual void
    extractNumbers(const bool);

    /*!
     * \param[in] b : true if a ccomment is opened at the beginning of
     * processing
     */
    virtual void
    setCCommentOpened(const bool);

    /*!
     * \return true if a CComment is still opened at the end of
     * processing
     */
    virtual bool
    isCCommentOpened(void) const;

    /*!
     * \brief an helper method throwing an exception if the given
     * iterator is egal to the third argument (which shall point
     * after the last token of the line)
     * \param[in] method : calling method name
     * \param[in] p  : iterator to the current token
     * \param[in] pe : iterator pointing after the last token of the
     * line
     * \throw std::runtime_error if the given iterator is egal to
     * the third argument
     */ 
    static void
    checkNotEndOfLine(const QString&,
		      const_iterator&, 
		      const const_iterator);

    /*!
     * \brief an helper method throwing an exception if the given
     * iterator is egal to the fourth argument (which shall point
     * after the last token of the line)
     * \param[in] method : calling method name
     * \param[in] error  : error message
     * \param[in] p  : iterator to the current token
     * \param[in] pe : iterator pointing after the last token of the
     * line
     * \throw std::runtime_error if the given iterator is egal to
     * the fourth argument
     */ 
    static void
    checkNotEndOfLine(const QString&,
		      const QString&,
		      const_iterator&, 
		      const const_iterator);

    /*!
     * \brief an helper method throwing an exception if the given
     * iterator is egal to the fourth argument (which shall point
     * after the last token of the line) or if the given token is
     * not equal to the expected value
     * \param[in] method : calling method name
     * \param[in] value  : expected value
     * \param[in] p  : iterator to the current token
     * \param[in] pe : iterator pointing after the last token of the
     * line
     * \throw std::runtime_error if the given iterator is egal to
     * the fourth argument
     */ 
    static void
    readSpecifiedToken(const QString&,
		       const QString&,
		       const_iterator&, 
		       const const_iterator);

    /*!
     * \return a reference to the last object of the list
     */ 
    const Token&
    back(void) const;

    /*!
     * \return an iterator to the first token
     */ 
    const_iterator
    begin(void) const;

    /*!
     * \return an iterator past the last token
     */ 
    const_iterator
    end(void) const;

    /*!
     * \return the number of token
     */ 
    size_type
    size() const;

    /*!
     * desctructor
     */
    virtual ~CxxTokenizer();

  protected:

    static bool
    isValidFunctionIdentifier(const QString&,const bool=true);

    void
    splitLine(QString,const unsigned short);

    void
    splitStandardToken(const QString&,
		       const unsigned short,
		       const unsigned short);

    void
    splitStandardTokenAtCxxSeparator(const QString&,
				     const unsigned short,
				     const unsigned short);

    TokensContainer fileTokens;

    bool cStyleCommentOpened;

    bool bExtractNumbers;

    bool charAsString;

  }; // end of struct CxxTokenizer

} // end of namespace qemacs

#endif /* _LIB_QEMACS_CXXTOKENIZER_H */

