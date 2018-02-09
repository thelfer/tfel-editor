/*!
 * \file   GnuplotSyntaxHighlighter.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   23/01/2016
 */

#include"QEmacs/GnuplotSyntaxHighlighter.hxx"

namespace qemacs{

  GnuplotSyntaxHighlighter::GnuplotSyntaxHighlighter(QTextDocument *p)
    : CSyntaxHighlighterBase(p)
  {
    for(const std::string& key : {"set","plot","splot"}){
      HighlightingRule rule;
      rule.key     = key;
      rule.format  = this->keyFormat;
      highlightingRules.push_front(rule);
    }
    this->options.charAsString = true;
    this->options.treatComments = false;
    this->options.joinCxxTwoCharactersSeparators = true;
    this->options.joinCxxTwoCharactersSeparators = true;
    this->options.allowStrayHashCharacter = true;
    this->options.allowStrayBackSlash = true;
    this->options.treatPreprocessorDirectives = false;
  } // end of GnuplotSyntaxHighlighter

  void GnuplotSyntaxHighlighter::highlightBlock(const QString & l){
    if (l.isEmpty()) { return; }
    if (l[0] == '#') {
      this->setFormat(0, l.size(), this->commentFormat);
      return;
    }
    CSyntaxHighlighterBase::highlightBlock(l);
  } // end of GnuplotSyntaxHighlighter::highlightBlock

  GnuplotSyntaxHighlighter::~GnuplotSyntaxHighlighter() = default;

} // end of namespace qemacs
