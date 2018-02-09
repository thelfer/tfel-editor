/*! 
 * \file  CSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/07/2012
 */

#include"QEmacs/CSyntaxHighlighter.hxx"

namespace qemacs {

  CSyntaxHighlighter::CSyntaxHighlighter(QTextDocument *p)
      : CSyntaxHighlighterBase(p) {
    // QStringList keys;
    // for(const QString &k: keys) {
    //   HighlightingRule rule;
    //   rule.key     = k;
    //   rule.format  = this->keyFormat;
    //   highlightingRules.append(rule);
    // }
  }

  CSyntaxHighlighter::~CSyntaxHighlighter() = default;

} // end of namespace qemacs

