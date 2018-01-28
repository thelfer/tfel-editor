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

  static QStringList
  PipeTestSyntaxHighlighter_buildGetPipeTestKeys()
  {
    mtest::PipeTestParser p;
    auto keys = QStringList{};
    for(const auto& k : p.getKeyWordsList()){
      keys.append(QString::fromStdString(k));
    }
    return keys;
  } // end of PipeTestSyntaxHighlighter_buildGetPipeTestKeys()
  
  QStringList&
  PipeTestSyntaxHighlighter::getPipeTestKeys()
  {
    static QStringList keys(PipeTestSyntaxHighlighter_buildGetPipeTestKeys());
    return keys;
  } // end of PipeTestSyntaxHighlighter::getPipeTestKeys

  PipeTestSyntaxHighlighter::PipeTestSyntaxHighlighter(QTextDocument *p)
    : CxxSyntaxHighlighter(p)
  {
    this->cCharAsString = true;
    this->mtestKeyFormat.setForeground(Qt::blue);
    QStringList keys;
    foreach (const QString &k,PipeTestSyntaxHighlighter::getPipeTestKeys()){
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->mtestKeyFormat;
      this->highlightingRules.push_front(rule);
    }
  }

  QStringList
  PipeTestSyntaxHighlighter::getKeyWordsList() const
  {
    static QStringList keys(PipeTestSyntaxHighlighter_buildGetPipeTestKeys());
    return keys;
  } // end of PipeTestSyntaxHighlighter::getKeyWordsList

  PipeTestSyntaxHighlighter::~PipeTestSyntaxHighlighter() = default;
  
} // end of namespace qemacs

