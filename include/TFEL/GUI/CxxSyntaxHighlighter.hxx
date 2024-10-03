/*!
 * \file  CxxSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_CXXSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_CXXSYNTAXHIGHLIGHTER_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CSyntaxHighlighter.hxx"

namespace tfel::gui {

  //! \brief a syntaxt highlighter for the C++ language
  struct TFEL_GUI_VISIBILITY_EXPORT CxxSyntaxHighlighter
      : public CSyntaxHighlighter {
    /*!
     * \param[in] t : text document to be highlighted
     */
    CxxSyntaxHighlighter(QTextDocument *const);
    //! \brief destructor
    ~CxxSyntaxHighlighter() override;

   private:
    Q_OBJECT
  };  // end of struct CxxSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CXXSYNTAXHIGHLIGHTER_HXX */
