/*! 
 * \file  CSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_QEMACS_CSYNTAXHIGHLIGHTER_HXX
#define LIB_QEMACS_CSYNTAXHIGHLIGHTER_HXX 

#include"QEmacs/CSyntaxHighlighterBase.hxx"

namespace qemacs {

  /*!
   * \brief a syntaxt highlighter for C-based languages
   */
  struct QEMACS_VISIBILITY_EXPORT CSyntaxHighlighter
      : public CSyntaxHighlighterBase {
    /*!
     * \param[in] t : text document to be highlighted
     */
    CSyntaxHighlighter(QTextDocument *const);
    //! destructor
    ~CSyntaxHighlighter();

   private:
    Q_OBJECT
  }; // end of struct CSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_CSYNTAXHIGHLIGHTER_HXX */

