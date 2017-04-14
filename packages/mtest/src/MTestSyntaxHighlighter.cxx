/*!
 * \file  MTestSyntaxHighlighter.MTest
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include"MTest/MTestParser.hxx"
#include"QEmacs/MTestSyntaxHighlighter.hxx"

namespace qemacs
{

  static QStringList
  MTestSyntaxHighlighter_buildGetMTestKeys(void)
  {
    mtest::MTestParser p;
    auto keys = QStringList{};
    for(const auto& k : p.getKeyWordsList()){
      keys.append(QString::fromStdString(k));
    }
    return keys;
  } // end of MTestSyntaxHighlighter_buildGetMTestKeys(void)
  
  QStringList&
  MTestSyntaxHighlighter::getMTestKeys(void)
  {
    static QStringList keys(MTestSyntaxHighlighter_buildGetMTestKeys());
    return keys;
  } // end of MTestSyntaxHighlighter::getMTestKeys

  MTestSyntaxHighlighter::MTestSyntaxHighlighter(QTextDocument *p)
    : CxxSyntaxHighlighter(p)
  {
    this->cCharAsString = true;
    this->mtestKeyFormat.setForeground(Qt::blue);
    QStringList keys;
    foreach (const QString &k,MTestSyntaxHighlighter::getMTestKeys()){
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->mtestKeyFormat;
      this->highlightingRules.push_front(rule);
    }
  }

  QStringList
  MTestSyntaxHighlighter::getKeyWordsList(void) const
  {
    static QStringList keys(MTestSyntaxHighlighter_buildGetMTestKeys());
    return keys;
  } // end of MTestSyntaxHighlighter::getKeyWordsList

  MTestSyntaxHighlighter::~MTestSyntaxHighlighter() = default;
  
} // end of namespace qemacs

