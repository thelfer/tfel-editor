/*!
 * \file  MTestSyntaxHighlighter.MTest
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include"MTest/MTestParser.hxx"
#include"QEmacs/MTestSyntaxHighlighter.hxx"

namespace qemacs
{

  static std::vector<std::string>
  MTestSyntaxHighlighter_buildGetMTestKeys()
  {
    return mtest::MTestParser().getKeyWordsList();
  } // end of MTestSyntaxHighlighter_buildGetMTestKeys()
  
  const std::vector<std::string>& MTestSyntaxHighlighter::getMTestKeys()
  {
    static const auto keys = MTestSyntaxHighlighter_buildGetMTestKeys();
    return keys;
  } // end of MTestSyntaxHighlighter::getMTestKeys

  MTestSyntaxHighlighter::MTestSyntaxHighlighter(QTextDocument *p)
    : CxxSyntaxHighlighter(p)
  {
    this->options.charAsString = true;
    this->mtestKeyFormat.setForeground(Qt::blue);
    for(const std::string &k : MTestSyntaxHighlighter::getMTestKeys()){
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->mtestKeyFormat;
      this->highlightingRules.push_back(rule);
    }
  }

  const std::vector<std::string>&
  MTestSyntaxHighlighter::getKeyWordsList() const
  {
    static auto keys = MTestSyntaxHighlighter_buildGetMTestKeys();
    return keys;
  } // end of MTestSyntaxHighlighter::getKeyWordsList

  MTestSyntaxHighlighter::~MTestSyntaxHighlighter() = default;
  
} // end of namespace qemacs

