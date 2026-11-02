/*!
 * \file   GnuplotSyntaxHiglighter.hxx
 * \brief GnuplotSyntaxHighlighter: a syntax highlighter for gnuplot files.
 * \author Thomas Helfer
 * \date   23/01/2016
 */

#ifndef LIB_TFEL_GUI_GNUPLOTSYNTAXHIGLIGHTER_HXX_
#define LIB_TFEL_GUI_GNUPLOTSYNTAXHIGLIGHTER_HXX_

#include "TFEL/GUI/CSyntaxHighlighterBase.hxx"

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#if defined TFELEditorGnuplotMode_EXPORTS
#define TFELEDITOR_GNUPLOTMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#else
#if not defined TFEL_STATIC_BUILD
#define TFELEDITOR_GNUPLOTMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#else
#define TFELEDITOR_GNUPLOTMODE_VISIBILITY_EXPORT
#endif
#endif
#else
#define TFELEDITOR_GNUPLOTMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

namespace tfel::gui {

  /*!
   * \brief syntax highlighter class for `gnuplot` files
   *
   * The highlighter reuses the tokenizer of \c CSyntaxHighlighterBase with
   * options tailored to `gnuplot`: `#` starts a comment, `C` style
   * comments and preprocessor directives are not treated and single quotes
   * are string delimiters.
   */
  struct TFELEDITOR_GNUPLOTMODE_VISIBILITY_EXPORT GnuplotSyntaxHighlighter
      : public CSyntaxHighlighterBase {
    /*!
     * \brief constructor
     * \param[in] p : document to be highlighted
     *
     * \note the commands \c set, \c plot and \c splot are registered as
     * keywords, highlighted with \c keyFormat
     */
    GnuplotSyntaxHighlighter(QTextDocument *);
    /*!
     * \brief highlight the current line
     * \param[in] l : current line
     *
     * A line starting with `#` is highlighted as a comment and nothing
     * else is done; any other line is handed to
     * \c CSyntaxHighlighterBase::highlightBlock
     */
    void highlightBlock(const QString &) override;
    //! destructor
    ~GnuplotSyntaxHighlighter() override;
  };  // end of struct GnuplotSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GNUPLOTSYNTAXHIGLIGHTER_HXX_ */
