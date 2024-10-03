/*!
 * \file  PipeTestSyntaxHighlighter.PipeTest
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#include "MTest/PipeTestParser.hxx"
#include "TFEL/GUI/PipeTestSyntaxHighlighter.hxx"

namespace tfel::gui {

  static std::vector<std::string>
  PipeTestSyntaxHighlighter_buildGetPipeTestKeys() {
    return mtest::PipeTestParser().getKeyWordsList();
  }  // end of PipeTestSyntaxHighlighter_buildGetPipeTestKeys()

  const std::vector<std::string>& PipeTestSyntaxHighlighter::getPipeTestKeys() {
    static auto keys = PipeTestSyntaxHighlighter_buildGetPipeTestKeys();
    return keys;
  }  // end of PipeTestSyntaxHighlighter::getPipeTestKeys

  PipeTestSyntaxHighlighter::PipeTestSyntaxHighlighter(QTextDocument* p)
      : CxxSyntaxHighlighter(p) {
    this->options.charAsString = true;
    this->mtestKeyFormat.setForeground(Qt::blue);
    for (const std::string& k : PipeTestSyntaxHighlighter::getPipeTestKeys()) {
      HighlightingRule rule;
      rule.key = k;
      rule.format = this->mtestKeyFormat;
      this->highlightingRules.push_back(rule);
    }
  }

  const std::vector<std::string>& PipeTestSyntaxHighlighter::getKeyWordsList()
      const {
    static auto keys = PipeTestSyntaxHighlighter_buildGetPipeTestKeys();
    return keys;
  }  // end of PipeTestSyntaxHighlighter::getKeyWordsList

  PipeTestSyntaxHighlighter::~PipeTestSyntaxHighlighter() = default;

}  // end of namespace tfel::gui
