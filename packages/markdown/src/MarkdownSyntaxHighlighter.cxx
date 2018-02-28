/*!
 * \file  MarkdownSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include <tuple>
#include <cctype>
#include <utility>
#include <cstdlib>
#include <stdexcept>
#include <QtCore/QDebug>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "QEmacs/MarkdownMajorMode.hxx"
#include "QEmacs/MarkdownSyntaxHighlighter.hxx"

namespace qemacs {

  MarkdownSyntaxHighlighter::MarkdownSyntaxHighlighter(
      MarkdownMajorMode &m, QTextDocument *p)
      : QSyntaxHighlighter(p), mode(m) {
    // parsing options
    this->options.treatPreprocessorDirectives = false;
    this->options.allowStrayBackSlash = true;
    this->options.allowStrayHashCharacter = true;
    this->options.treatNumbers = false;
    this->options.treatStrings = false;
    this->options.treatComments = false;
    this->options.graveAccentAsSeparator = true;
    this->options.joinCxxTwoCharactersSeparators = true;
    // formating options
    this->md.setForeground(Qt::red);
    this->md.setFontStretch(120);
    this->md.setFontWeight(QFont::Bold);
    this->h1.setForeground(Qt::black);
    this->h1.setFontWeight(QFont::Bold);
    this->h1.setFontStretch(115);
    this->h2.setForeground(Qt::darkGray);
    this->h2.setFontWeight(QFont::Bold);
    this->h2.setFontStretch(110);
    this->h3.setForeground(Qt::gray);
    this->h3.setFontWeight(QFont::Bold);
    this->h3.setFontStretch(105);
    this->h4.setForeground(Qt::lightGray);
    this->h4.setFontWeight(QFont::Bold);
  }  // end of MarkdownSyntaxHighlighter::MarkdownSyntaxHighlighter

  void MarkdownSyntaxHighlighter::highlightBlock(const QString &l) {
    using namespace tfel::utilities;
    constexpr const int valid_flag = 1 << 0;
    constexpr const int bold_flag = 1 << 1;
    constexpr const int emph_flag = 1 << 2;
    // previous state + validity flag
    const auto ipstate = [this] {
      const auto pbs = this->previousBlockState();
      return pbs != -1 ? pbs : valid_flag;
    }();
    auto set_invalid_state = [this] { this->setCurrentBlockState(0); };
    auto set_state = [this](const int s) {
      const auto ns = (s << 1) + valid_flag;
      this->setCurrentBlockState(ns);
    };
    if (!(ipstate & valid_flag)) {
      this->setCurrentBlockState(0);
      return;
    }
    const auto pstate = ipstate >> 1;
    int m, pr, s;
    std::tie(m, pr, s) = [this, pstate] {
      const auto r = div(pstate, 100);
      const auto r2 = div(r.quot, 10);
      return std::make_tuple(r.rem, r2.rem, r2.quot);
    }();
    bool bold = (m == 0 && (s & bold_flag));
    bool emph = (m == 0 && (s & emph_flag));
    // tokenize the line
    CxxTokenizer tokens(this->options);
    try {
      tokens.parseString(l.toStdString());
    } catch (std::exception &e) {
      set_invalid_state();
      return;
    } catch (...) {
      set_invalid_state();
      return;
    }
    auto &sc = this->mode.getSpellChecker();
    auto spell_check = [this, &sc](const Token &t) {
      if (t.value.empty()) {
        return;
      }
      if (std::isalpha(t.value[0]) == 0) {
        return;
      }
      const auto w = QString::fromStdString(t.value);
      if (!sc.spell(w)) {
        QTextCharFormat f;
        f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        this->setFormat(t.offset, t.value.size(), f);
      }
    };
    auto spell_check_all_tokens = [&tokens, &spell_check] {
      for (const auto &t : tokens) {
        spell_check(t);
      }
    };
    // m is the current mode
    // - 0: standard text
    // - 1: LaTeX equation
    if (pr < 3) {
      if (tokens.empty()) {
        pr = 3;
        set_state(300);
        return;
      }
      // here we are in the firt three lines of the file and the
      // preamble can still be opened.
      if ((tokens[0].value[0] == '%') && (tokens[0].offset == 0)) {
        // title, author, date
        this->setFormat(0, l.size(), this->md);
        spell_check_all_tokens();
        ++pr;
        set_state(100*pr);
        return;
      } else {
        pr = 3;
      }
    }
    // the preamble is over, looking for headers
    if (m == 0) {
      if (tokens.empty()) {
        if (bold || emph) {
          set_invalid_state();
        } else {
          set_state(300);
        }
        return;
      }
      if ((tokens[0].value[0] == '#') && (tokens[0].offset == 0)) {
        // in a header
        if (bold || emph) {
          set_invalid_state();
          return;
        }
        const auto lvl = [&tokens] {
          short i = 0;
          for (const auto &t : tokens) {
            if (t.value[0] == '#') {
              ++i;
            } else {
              return i;
            }
          }
          return i;
        }();
        switch (lvl) {
          case 1:
            this->setFormat(0, l.size(), this->h1);
            break;
          case 2:
            this->setFormat(0, l.size(), this->h2);
            break;
          case 3:
            this->setFormat(0, l.size(), this->h3);
            break;
          default:
            this->setFormat(0, l.size(), this->h4);
            break;
        }
        spell_check_all_tokens();
        set_state(300);
        return;
      }
    }
    set_state(300);
  }  // end of MarkdownSyntaxHighlighter::highlight

  MarkdownSyntaxHighlighter::~MarkdownSyntaxHighlighter() = default;

}  // end of namespace qemacs
