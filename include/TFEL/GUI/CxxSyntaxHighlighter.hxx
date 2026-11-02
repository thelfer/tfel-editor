/*!
 * \file  CxxSyntaxHighlighter.hxx
 * \brief syntax highlighter for the C++ language.
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_CXXSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_CXXSYNTAXHIGHLIGHTER_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CSyntaxHighlighter.hxx"

namespace tfel::gui {

  /*!
   * \brief a syntaxt highlighter for the C++ language
   *
   * \note the highlighter registers the standard C++ keywords as highlighting
   * rules, all displayed with the bold dark magenta format of the keywords
   *
   * \note the keywords are matched on the exact value of the tokens, which
   * holds no surrounding space: the rule declared with the "char " key, whose
   * value ends with a space, is therefore never used
   *
   * \note this is the highlighter the modes of the languages derived from C++,
   * such as `MFront` and `MTest`, build upon
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CxxSyntaxHighlighter
      : public CSyntaxHighlighter {
    /*!
     * \param[in] t : text document to be highlighted
     *
     * \note the highlighter registers itself on the document, which owns it
     */
    CxxSyntaxHighlighter(QTextDocument *const);
    //! \brief destructor
    ~CxxSyntaxHighlighter() override;

   private:
    Q_OBJECT
  };  // end of struct CxxSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CXXSYNTAXHIGHLIGHTER_HXX */
