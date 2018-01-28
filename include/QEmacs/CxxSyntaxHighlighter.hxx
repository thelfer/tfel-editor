/*! 
 * \file  CxxSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifndef LIB_QEMACS_CXXSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_CXXSYNTAXHIGHLIGHTER_HXX 

#include"QEmacs/Config.hxx"
#include"QEmacs/CSyntaxHighlighter.hxx"

namespace qemacs
{

  /*!
   * a syntaxt highlighter for the C++ language
   */
  class QEMACS_VISIBILITY_EXPORT CxxSyntaxHighlighter
    : public CSyntaxHighlighter
  {

    Q_OBJECT

  public:

    /*!
     * \param[in] t : text document to be highlighted
     */
    CxxSyntaxHighlighter(QTextDocument *const);
    
  }; // end of struct CxxSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_CXXSYNTAXHIGHLIGHTER_HXX */

