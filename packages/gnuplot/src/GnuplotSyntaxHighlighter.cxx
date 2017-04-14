/*!
 * \file   GnuplotSyntaxHighlighter.cxx
 * \brief    
 * \author THOMAS HELFER
 * \date   23 janv. 2016
 */

#include"QEmacs/GnuplotSyntaxHighlighter.hxx"

namespace qemacs{

  GnuplotSyntaxHighlighter::GnuplotSyntaxHighlighter(QTextDocument *p)
    : CSyntaxHighlighterBase(p)
  {
    QStringList keys;
    keys << "set"     << "plot"   << "splot";
    foreach(const QString& key,keys){
      HighlightingRule rule;
      rule.key     = key;
      rule.format  = this->keyFormat;
      highlightingRules.push_front(rule);
    }
    this->cCharAsString = true;
  } // end of GnuplotSyntaxHighlighter

}
