/*! 
 * \file  CSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include"QEmacs/CSyntaxHighlighter.hxx"

namespace qemacs
{
  
  CSyntaxHighlighter::CSyntaxHighlighter(QTextDocument *p)
    : CSyntaxHighlighterBase(p)
  {
    // QStringList keys;
    // for(const QString &k: keys) {
    //   HighlightingRule rule;
    //   rule.key     = k;
    //   rule.format  = this->keyFormat;
    //   highlightingRules.append(rule);
    // }
  }

} // end of namespace qemacs

