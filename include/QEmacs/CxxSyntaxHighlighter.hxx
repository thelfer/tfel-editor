/*! 
 * \file  CxxSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_QEMACS_CXXSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_CXXSYNTAXHIGHLIGHTER_HXX 

#include"QEmacs/Config.hxx"
#include"QEmacs/CSyntaxHighlighter.hxx"

namespace qemacs {

  /*!
   * a syntaxt highlighter for the C++ language
   */
  struct QEMACS_VISIBILITY_EXPORT CxxSyntaxHighlighter
      : public CSyntaxHighlighter {

    /*!
     * \param[in] t : text document to be highlighted
     */
    CxxSyntaxHighlighter(QTextDocument *const);
    //! destructor
    ~CxxSyntaxHighlighter() override;

   private:
    Q_OBJECT

  }; // end of struct CxxSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_CXXSYNTAXHIGHLIGHTER_HXX */

