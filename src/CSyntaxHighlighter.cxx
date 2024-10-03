/*!
 * \file  CSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   30/07/2012
 */

#include "TFEL/GUI/CSyntaxHighlighter.hxx"

namespace tfel::gui {

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

}  // end of namespace tfel::gui
