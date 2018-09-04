/*! 
 * \file  CSyntaxHighlighter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_CSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_CSYNTAXHIGHLIGHTER_HXX 

#include"TFEL/GUI/CSyntaxHighlighterBase.hxx"

namespace tfel{

  namespace gui{

  /*!
   * \brief a syntaxt highlighter for C-based languages
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CSyntaxHighlighter
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
  
} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_CSYNTAXHIGHLIGHTER_HXX */

