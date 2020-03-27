/*!
 * \file  CxxSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"

namespace tfel{

  namespace gui{

  CxxSyntaxHighlighter::CxxSyntaxHighlighter(QTextDocument *p)
      : CSyntaxHighlighter(p) {
    constexpr const unsigned short knbr = 70;
    const std::string keys[knbr] = {
        "asm",          "auto",         "bool",
        "break",        "case",         "catch",
        "char ",        "class",        "const",
        "const_cast",   "continue",     "default",
        "delete",       "do",           "double",
        "dynamic_cast", "else",         "enum",
        "explicit",     "export",       "extern",
        "false",        "float",        "for",
        "friend",       "goto",         "if",
        "inline",       "int",          "long",
        "mutable",      "namespace",    "new",
        "operator",     "private",      "protected",
        "public",       "register",     "reinterpret_cast",
        "return",       "short",        "signed",
        "sizeof",       "static",       "static_cast",
        "struct",       "switch",       "template",
        "this",         "throw",        "true",
        "try",          "typedef",      "typeid",
        "typename",     "union",        "unsigned",
        "using",        "virtual",      "void",
        "volatile",     "wchar_t",      "while",
        "nullptr",      "constexpr",    "override",
        "final",        "static_assert", "noexcept",
        "size_t"};
    for (int i = 0; i != knbr; ++i) {
      HighlightingRule rule;
      rule.key = keys[i];
      rule.format = this->keyFormat;
      rule.format.setFontWeight(QFont::Bold);
      highlightingRules.push_back(rule);
    }
  }

  CxxSyntaxHighlighter::~CxxSyntaxHighlighter() = default;

}  // end of namespace gui
}// end of namespace tfel
