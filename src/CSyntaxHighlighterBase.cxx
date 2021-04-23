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

namespace tfel {

  namespace gui {

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
      if (this->previousBlockState() == -2) {
        this->setCurrentBlockState(-2);
        return;
      }
      CxxTokenizer tokenizer(this->options);
      tokenizer.setCStyleCommentOpened(this->previousBlockState() == 1);
      try {
        tokenizer.parseString(text.toStdString());
      } catch (std::exception &) {
        this->setCurrentBlockState(-2);
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
        this->setCurrentBlockState(1);
      } else {
        this->setCurrentBlockState(0);
      }
    }

  }  // end of namespace gui
}  // end of namespace tfel
