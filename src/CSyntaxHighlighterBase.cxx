/*!
 * \file  CSyntaxHighlighterBase.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include <stdexcept>
#include <QtCore/QDebug>
#include "TFEL/GUI/CSyntaxHighlighterBase.hxx"
#include "TFEL/Utilities/CxxTokenizer.hxx"

namespace tfel::gui {

  std::vector<std::string> &CSyntaxHighlighterBase::getRawStringDelimiters() {
    static std::vector<std::string> delimiters;
    return delimiters;
  }  // end of getRawStringDelimiters

  const std::string &CSyntaxHighlighterBase::getRawStringDelimiter(
      const int i) {
    static const std::string empty;
    const auto &delimiters = getRawStringDelimiters();
    if ((i < 0) || (i >= delimiters.size())) {
      return empty;
    }
    return delimiters[i];
  }  // end of getRawStringDelimiter

  int CSyntaxHighlighterBase::getRawStringDelimiterId(const std::string &n) {
    auto &delimiters = getRawStringDelimiters();
    const auto p = std::find(delimiters.begin(), delimiters.end(), n);
    if (p == delimiters.end()) {
      const auto s = static_cast<int>(delimiters.size());
      delimiters.push_back(n);
      return s;
    }
    return static_cast<int>(p - delimiters.begin());
  }  // end of getRawStringDelimiterId

  CSyntaxHighlighterBase::CSyntaxHighlighterBase(QTextDocument *p)
      : QSyntaxHighlighter(p) {
    this->options.bKeepCommentBoundaries = true;
    this->preprocessorFormat.setForeground(Qt::darkMagenta);
    this->preprocessorFormat.setFontWeight(QFont::Bold);
    this->preprocessorFormat.setForeground(Qt::darkMagenta);
    this->keyFormat.setForeground(Qt::darkMagenta);
    this->keyFormat.setFontWeight(QFont::Bold);
    this->numberFormat.setForeground(Qt::darkRed);
    this->stringFormat.setForeground(Qt::darkGreen);
    this->commentFormat.setForeground(Qt::red);
  }

  void CSyntaxHighlighterBase::highlightBlock(const QString &text) {
    using tfel::utilities::CxxTokenizer;
    using tfel::utilities::Token;
    if (this->previousBlockState() == invalidState) {
      this->setCurrentBlockState(invalidState);
      return;
    }
    CxxTokenizer tokenizer(this->options);
    if (this->previousBlockState() >= rawStringOpenedState) {
      tokenizer.setRawStringDelimiter(getRawStringDelimiter(
          this->previousBlockState() - rawStringOpenedState));
    }
    tokenizer.setCStyleCommentOpened(this->previousBlockState() ==
                                     cStyleCommentOpenedState);
    try {
      tokenizer.parseString(text.toStdString());
    } catch (std::exception &) {
      this->setCurrentBlockState(invalidState);
      return;
    }
    for (auto pt = tokenizer.begin(); pt != tokenizer.end(); ++pt) {
      QTextCharFormat f;
      bool b = false;
      if ((pt->flag == Token::String) || (pt->flag == Token::Char)) {
        f = this->stringFormat;
        b = true;
      } else if (pt->flag == Token::Preprocessor) {
        f = this->preprocessorFormat;
        b = true;
      } else if (pt->flag == Token::Number) {
        f = this->numberFormat;
        b = true;
      } else if (tfel::utilities::isComment(*pt)) {
        f = this->commentFormat;
        b = true;
      } else {
        // standard C++ keywords
        for (const auto &r : this->highlightingRules) {
          if (r.key == pt->value) {
            f = r.format;
            b = true;
          }
        }
      }
      if (b) {
        this->setFormat(pt->offset, pt->value.size(), f);
      }
    }
    // special cases
    if ((tokenizer.size() > 3) && (tokenizer[0].value == "#") &&
        (tokenizer[1].flag == Token::Preprocessor) &&
        (tokenizer[1].value == "include") && (!tokenizer[2].value.empty())) {
      const auto &t = tokenizer[2];
      if (t.value.front() == '<') {
        auto pt = tokenizer.begin();
        std::advance(pt, 2);
        while (pt != tokenizer.end()) {
          this->setFormat(pt->offset, pt->value.size(), this->stringFormat);
          if (pt->value == ">") {
            break;
          }
          ++pt;
        }
      }
    }
    if (tokenizer.isCStyleCommentOpened()) {
      this->setCurrentBlockState(cStyleCommentOpenedState);
    } else if (tokenizer.isRawStringOpened()) {
      this->setCurrentBlockState(
          rawStringOpenedState +
          getRawStringDelimiterId(tokenizer.getCurrentRawStringDelimiter()));
    } else {
      this->setCurrentBlockState(defaultState);
    }
  }

}  // end of namespace tfel::gui
