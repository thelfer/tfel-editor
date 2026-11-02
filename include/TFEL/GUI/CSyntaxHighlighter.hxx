/*!
 * \file  CSyntaxHighlighter.hxx
 * \brief syntax highlighter for C-based languages.
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_CSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_CSYNTAXHIGHLIGHTER_HXX

#include "TFEL/GUI/CSyntaxHighlighterBase.hxx"

namespace tfel::gui {

  /*!
   * \brief a syntaxt highlighter for C-based languages
   *
   * \note no highlighting rule is registered: only the strings, the numbers,
   * the comments and the preprocessor directives are highlighted, with the
   * formats defined by `CSyntaxHighlighterBase`
   *
   * \note no major mode uses this highlighter directly: `CxxSyntaxHighlighter`
   * adds the keywords of the C++ language to it
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CSyntaxHighlighter
      : public CSyntaxHighlighterBase {
    /*!
     * \param[in] t : text document to be highlighted
     *
     * \note the highlighter registers itself on the document, which owns it
     */
    CSyntaxHighlighter(QTextDocument *const);
    //! destructor
    ~CSyntaxHighlighter();

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CSYNTAXHIGHLIGHTER_HXX */
