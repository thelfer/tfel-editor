/*! 
 * \file  CxxSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include"QEmacs/CxxSyntaxHighlighter.hxx"

namespace qemacs
{
  
  CxxSyntaxHighlighter::CxxSyntaxHighlighter(QTextDocument *p)
    : CSyntaxHighlighter(p)
  {
    const unsigned short knbr = 63;
    const QString keys[knbr] = {"asm","auto","bool","break","case",
				"catch","char ","class","const",
				"const_cast","continue","default",
				"delete","do","double","dynamic_cast",
				"else","enum","explicit","export",
				"extern","false","float","for","friend",
				"goto","if","inline","int","long",
				"mutable","namespace","new","operator",
				"private","protected","public","register",
				"reinterpret_cast","return","short",
				"signed","sizeof","static","static_cast",
				"struct","switch","template","this","throw",
				"true","try","typedef","typeid","typename",
				"union","unsigned","using","virtual","void",
				"volatile","wchar_t","while"};

    for(int i=0;i!=knbr;++i){
      HighlightingRule rule;
      rule.key     = keys[i];
      rule.format  = this->keyFormat;
      highlightingRules.push_front(rule);
    }
  }

} // end of namespace qemacs

