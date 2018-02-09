/*!
 * \file  MarkdownSyntaxHighlighter.Markdown
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include <tuple>
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
    using tfel::utilities::CxxTokenizer;
    using tfel::utilities::CxxTokenizerOptions;
    int pr, s;
    std::tie(pr, s) = [this] {
      const auto state = [this] {
        const auto cs = this->previousBlockState();
        return cs != -1 ? cs : 0;
      }();
      const auto r = div(state, 10);
      return std::make_pair(r.rem, r.quot);
    }();
    if (l.isEmpty()) {
      pr = 3;
      this->setCurrentBlockState(pr + 10 * s);
      return;
    }
    CxxTokenizer tokens(this->options);
    try {
      tokens.parseString(l.toStdString());
    } catch (std::exception &e) {
      qDebug() << e.what();
      pr = 3;
      this->setCurrentBlockState(pr + 10 * s);
      return;
    } catch (...) {
      pr = 3;
      this->setCurrentBlockState(pr + 10 * s);
      return;
    }
    if (tokens.empty()) {
      pr = 3;
      this->setCurrentBlockState(pr + 10 * s);
      return;
    }
    if (pr < 3) {
      // here we are in the firt three lines of the file and the
      // preamble can still be opened.
      if ((tokens[0].value[0] == '%')&&(tokens[0].offset==0)) {
        // title, author, date
        this->setFormat(0, l.size(), this->md);
        ++pr;
      } else {
        pr = 3;
      }
    } else {
      // the preamble is over
      if ((tokens[0].value[0] == '#')&&(tokens[0].offset==0)) {
        // in a header
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
            return;
          case 2:
            this->setFormat(0, l.size(), this->h2);
            return;
          case 3:
            this->setFormat(0, l.size(), this->h3);
            return;
          default:
            this->setFormat(0, l.size(), this->h4);
            return;
        }
      }
    }
    this->setCurrentBlockState(pr + 10 * s);
  }  // end of MarkdownSyntaxHighlighter::highlight

  void MarkdownSyntaxHighlighter::highLightMispellWords(
      const QString &w, const int p) {
    auto &sc = this->mode.getSpellChecker();
    QTextCharFormat f;
    f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    int pos = 0;
    while (pos != w.size()) {
      if (w[pos].isLetter()) {
        const auto npos = pos;
        ++pos;
        while ((pos != w.size()) && (w[pos].isLetter())) {
          ++pos;
        }
        if (!sc.spell(w.mid(npos, pos - npos))) {
          this->setFormat(p + npos, pos - npos, f);
        }
      } else {
        ++pos;
      }
    }
  }  // end of MarkdownSyntaxHighlighter::highLightMispellWords

  MarkdownSyntaxHighlighter::~MarkdownSyntaxHighlighter() = default;

}  // end of namespace qemacs