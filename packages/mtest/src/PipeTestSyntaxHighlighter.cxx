/*!
 * \file  PipeTestSyntaxHighlighter.PipeTest
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include"MTest/PipeTestParser.hxx"
#include"QEmacs/PipeTestSyntaxHighlighter.hxx"

namespace qemacs
{

  static std::vector<std::string>
  PipeTestSyntaxHighlighter_buildGetPipeTestKeys()
  {
    return mtest::PipeTestParser().getKeyWordsList();
  } // end of PipeTestSyntaxHighlighter_buildGetPipeTestKeys()
  
  const std::vector<std::string>& PipeTestSyntaxHighlighter::getPipeTestKeys()
  {
    static auto keys = PipeTestSyntaxHighlighter_buildGetPipeTestKeys();
    return keys;
  } // end of PipeTestSyntaxHighlighter::getPipeTestKeys

  PipeTestSyntaxHighlighter::PipeTestSyntaxHighlighter(QTextDocument *p)
    : CxxSyntaxHighlighter(p)
  {
    this->options.charAsString = true;
    this->mtestKeyFormat.setForeground(Qt::blue);
    for(const std::string &k : PipeTestSyntaxHighlighter::getPipeTestKeys()){
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->mtestKeyFormat;
      this->highlightingRules.push_front(rule);
    }
  }

  const std::vector<std::string>&
  PipeTestSyntaxHighlighter::getKeyWordsList() const
  {
    static auto keys = PipeTestSyntaxHighlighter_buildGetPipeTestKeys();
    return keys;
  } // end of PipeTestSyntaxHighlighter::getKeyWordsList

  PipeTestSyntaxHighlighter::~PipeTestSyntaxHighlighter() = default;
  
} // end of namespace qemacs

