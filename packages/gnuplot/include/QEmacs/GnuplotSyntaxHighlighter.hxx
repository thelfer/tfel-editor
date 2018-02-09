/*!
 * \file   GnuplotSyntaxHiglighter.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   23 janv. 2016
 */

#ifndef LIB_QEMACS_GNUPLOTSYNTAXHIGLIGHTER_HXX_
#define LIB_QEMACS_GNUPLOTSYNTAXHIGLIGHTER_HXX_

#include"QEmacs/CSyntaxHighlighterBase.hxx"

#if defined _WIN32 || defined _WIN64 ||defined __CYGWIN__
#  if defined GnuplotQEmacsPackage_EXPORTS
#    define GNUPLOTQEMACSPACKAGE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#  else
#  if not defined TFEL_STATIC_BUILD
#    define GNUPLOTQEMACSPACKAGE_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#  else 
#    define GNUPLOTQEMACSPACKAGE_VISIBILITY_EXPORT 
#  endif
#  endif
#else
#  define GNUPLOTQEMACSPACKAGE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

namespace qemacs{

  struct GNUPLOTQEMACSPACKAGE_VISIBILITY_EXPORT GnuplotSyntaxHighlighter
    : public CSyntaxHighlighterBase
  {
    GnuplotSyntaxHighlighter(QTextDocument *);
    void highlightBlock(const QString &) override;
    ~GnuplotSyntaxHighlighter() override;
  }; // end of struct GnuplotSyntaxHighlighter
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_GNUPLOTSYNTAXHIGLIGHTER_HXX_ */
