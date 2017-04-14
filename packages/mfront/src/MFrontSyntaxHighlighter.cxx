/*! 
 * \file  MFrontSyntaxHighlighter.MFront
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include<stdexcept>

#include"MFront/DSLFactory.hxx"
#include"MFront/AbstractDSL.hxx"
#include"QEmacs/MFrontSyntaxHighlighter.hxx"

namespace qemacs
{
  
  MFrontSyntaxHighlighter::MFrontSyntaxHighlighter(QTextDocument *p,
						   const QString& n)
    : CxxSyntaxHighlighter(p)
  {
    QStringList keys;
    try{
      auto& f = mfront::DSLFactory::getDSLFactory();
      std::shared_ptr<mfront::AbstractDSL> dsl{f.createNewParser(n.toStdString())};
      std::vector<std::string> mkeys;
      dsl->getKeywordsList(mkeys);
      for(const auto& k:mkeys){
	keys.append(QString::fromStdString(k));
      }
    } catch(std::exception& e){
      return;
    }
    this->mfrontKeyFormat.setForeground(Qt::blue);
    for(const auto &k: keys) {
      HighlightingRule rule;
      rule.key     = k;
      rule.format  = this->mfrontKeyFormat;
      this->highlightingRules.push_front(rule);
    }
  }

} // end of namespace qemacs

