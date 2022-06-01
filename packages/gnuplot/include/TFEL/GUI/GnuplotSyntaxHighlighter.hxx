/*!
 * \file   GnuplotSyntaxHiglighter.hxx
 * \brief
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

  struct TFELEDITOR_GNUPLOTMODE_VISIBILITY_EXPORT GnuplotSyntaxHighlighter
      : public CSyntaxHighlighterBase {
    GnuplotSyntaxHighlighter(QTextDocument *);
    void highlightBlock(const QString &) override;
    ~GnuplotSyntaxHighlighter() override;
  };  // end of struct GnuplotSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GNUPLOTSYNTAXHIGLIGHTER_HXX_ */
