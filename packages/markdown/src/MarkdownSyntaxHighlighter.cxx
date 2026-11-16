/*!
 * \file  MarkdownSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#include <tuple>
#include <utility>
#include <cstdlib>
#include <stdexcept>
#include <QtCore/QDebug>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/GUI/MarkdownMajorMode.hxx"
#include "TFEL/GUI/MarkdownSyntaxHighlighter.hxx"

namespace tfel::gui {

  namespace {

    /*!
     * \return the number of bytes the UTF-8 encoding of the character which
     * starts at the given index of the given text takes
     * \param[in] t : text
     * \param[in] i : index of the character, which shall be a valid one
     *
     * \note a pair of surrogates is one character of four bytes, which is the
     * only case where the number of bytes and the number of `QChar`s of a
     * character disagree
     */
    int utf8SizeOfCharacter(const QString& t, const int i) {
      const auto c = t[i].unicode();
      if (QChar::isHighSurrogate(c) && ((i + 1) != t.size()) &&
          QChar::isLowSurrogate(t[i + 1].unicode())) {
        return 4;
      }
      if (c < 0x80) {
        return 1;
      }
      if (c < 0x800) {
        return 2;
      }
      return 3;
    }  // end of utf8SizeOfCharacter()

    /*!
     * \return the index, in the given text, of the character which starts at
     * the given byte of its UTF-8 encoding, the size of the text when that byte
     * is what closes it, -1 when it stands inside a character
     * \param[in] t    : text
     * \param[in] byte : byte of the UTF-8 encoding of \c t
     */
    int characterAtByte(const QString& t, const int byte) {
      if (byte < 0) {
        return -1;
      }
      auto bytes = 0;
      auto i = 0;
      while (i != t.size()) {
        if (bytes == byte) {
          return i;
        }
        const auto n = utf8SizeOfCharacter(t, i);
        if (bytes + n > byte) {
          return -1;
        }
        bytes += n;
        i += (n == 4) ? 2 : 1;
      }
      return (bytes == byte) ? t.size() : -1;
    }  // end of characterAtByte()

  }  // end of anonymous namespace

  tfel::utilities::CxxTokenizerOptions
  MarkdownSyntaxHighlighter::tokenizingOptions() {
    auto o = tfel::utilities::CxxTokenizerOptions();
    o.treatPreprocessorDirectives = false;
    o.allowStrayBackSlash = true;
    o.allowStrayHashCharacter = true;
    o.treatNumbers = false;
    o.treatStrings = false;
    o.treatCComments = false;
    o.treatCxxComments = false;
    o.graveAccentAsSeparator = true;
    o.joinCxxTwoCharactersSeparators = true;
    return o;
  }  // end of MarkdownSyntaxHighlighter::tokenizingOptions

  std::vector<std::pair<int, int>>
  MarkdownSyntaxHighlighter::spellCheckedWords(const QString& l) {
    using namespace tfel::utilities;
    auto words = std::vector<std::pair<int, int>>();
    auto tokens = CxxTokenizer(tokenizingOptions());
    try {
      tokens.parseString(l.toStdString());
    } catch (std::exception&) {
      return words;
    } catch (...) {
      return words;
    }
    for (const auto& t : tokens) {
      if (t.value.empty()) {
        continue;
      }
      const auto first = characterAtByte(l, static_cast<int>(t.offset));
      const auto last = characterAtByte(
          l, static_cast<int>(t.offset) + static_cast<int>(t.value.size()));
      if ((first < 0) || (last <= first)) {
        // a token whose place the encoding of the line does not answer is
        // nothing the checker is asked about
        continue;
      }
      const auto word = l.mid(first, last - first);
      // issue #71: isalpha() on the (signed) char of a UTF-8 byte is undefined
      // behaviour and, on glibc, silently skips every word starting with a
      // non-ASCII letter; QChar::isLetter on the decoded word is the test
      // actually meant here
      if (word.isEmpty() || !word[0].isLetter()) {
        continue;
      }
      words.push_back({first, last});
    }
    return words;
  }  // end of MarkdownSyntaxHighlighter::spellCheckedWords

  MarkdownSyntaxHighlighter::MarkdownSyntaxHighlighter(MarkdownMajorMode &m,
                                                       QTextDocument *p)
      : QSyntaxHighlighter(p), mode(m) {
    // parsing options
    this->options = tokenizingOptions();
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
    const auto ipstate = [this, valid_flag] {
      const auto pbs = this->previousBlockState();
      return pbs != -1 ? pbs : valid_flag;
    }();
    auto set_invalid_state = [this] { this->setCurrentBlockState(0); };
    auto set_state = [this, valid_flag](const int s) {
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
    // the words of the line the checker is asked about, drawn over the face of
    // the line they stand in: it is the one split \c spellCheckedWords answers,
    // which the context menu of the mode asks its suggestions of too, so that
    // the underline and the suggestions cannot disagree about what a misspelled
    // word is
    auto spell_check_all_tokens = [this, &sc, &l](const QTextCharFormat &f) {
      for (const auto &w : MarkdownSyntaxHighlighter::spellCheckedWords(l)) {
        if (sc.spell(l.mid(w.first, w.second - w.first))) {
          continue;
        }
        auto nf = QTextCharFormat(f);
        nf.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        this->setFormat(w.first, w.second - w.first, nf);
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
        spell_check_all_tokens(this->md);
        ++pr;
        set_state(100 * pr);
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
            spell_check_all_tokens(this->h1);
            break;
          case 2:
            this->setFormat(0, l.size(), this->h2);
            // the underline must reuse *this* level's format: passing h1
            // here restyled the whole heading under the user's cursor
            // (issue #71)
            spell_check_all_tokens(this->h2);
            break;
          case 3:
            this->setFormat(0, l.size(), this->h3);
            spell_check_all_tokens(this->h3);
            break;
          default:
            this->setFormat(0, l.size(), this->h4);
            spell_check_all_tokens(this->h4);
            break;
        }
        set_state(300);
        return;
      }
      // standard line at last
      //       // testing latex mode
      //       if ((tokens.size() > 2) && (tokens[0].value == "\\") &&
      //           (tokens[1] == "[")) {
      //         // starting latex formula
      //         if (bold || emph) {
      //           set_invalid_state();
      //           return;
      //         }
      //         set_state();
      //       }
      spell_check_all_tokens(QTextCharFormat());
    }
    set_state(300);
  }  // end of MarkdownSyntaxHighlighter::highlight

  MarkdownSyntaxHighlighter::~MarkdownSyntaxHighlighter() = default;

}  // end of namespace tfel::gui
