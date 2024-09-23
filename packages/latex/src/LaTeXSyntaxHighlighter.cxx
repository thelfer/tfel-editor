/*!
 * \file  LaTeXSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include "TFEL/GUI/LaTeXMajorMode.hxx"
#include "TFEL/GUI/LaTeXSyntaxHighlighter.hxx"

namespace tfel::gui {

  int LaTeXSyntaxHighlighter::startOfComment(const QString& l) {
    for (int i = 0; i != l.size(); ++i) {
      if (l[i] == '%') {
        // check if previous character was not a '\'
        int j = i;
        if (j == 0) {
          return 0;
        }
        --j;
        if ((j >= 0) && (l[j] != '\\')) {
          return i;
        }
        bool b(true);  // true if it is a comment
        while ((j >= 0) && (l[j] == '\\')) {
          b = !b;
          --j;
        }
        if (b) {
          return i;
        }
      }
    }
    return -1;
  }

  QString LaTeXSyntaxHighlighter::stripComment(const QString& l) {
    int c = LaTeXSyntaxHighlighter::startOfComment(l);
    if (c != -1) {
      if (c == 0) {
        return QString();
      }
      return l.mid(0, c);
    }
    return l;
  }

  QVector<LaTeXSyntaxHighlighter::HighlightingRule>
  LaTeXSyntaxHighlighter::buildHighlightingRules() {
    QVector<HighlightingRule> rules;
    QTextCharFormat keyFormat;
    QTextCharFormat envFormat;
    // QTextCharFormat boldFormat;
    // QTextCharFormat itFormat;
    envFormat.setForeground(Qt::darkMagenta);
    keyFormat.setForeground(Qt::blue);
    // boldFormat.setFontWeight(QFont::Bold);
    // itFormat.setFontItalic(true);
    HighlightingRule rule;
    // rule.format  = QVector<QTextCharFormat>() << this->keyFormat <<
    // this->boldFormat;
    // rule.pattern = QRegularExpression("(\\\\textbf)\\{(\\w+)\\}");
    // rule.pattern.setMinimal(true);
    // rules.append(rule);
    // rule.format  = QVector<QTextCharFormat>() << this->keyFormat <<
    // this->itFormat;
    // rule.pattern = QRegularExpression("(\\\\textit)\\{(\\w+)\\}");
    // rule.pattern.setMinimal(true);
    // rules.append(rule);
    rule.format = QVector<QTextCharFormat>(1, keyFormat);
    rule.pattern = QRegularExpression("\\\\\\w+");
    rules.append(rule);
    rule.format = QVector<QTextCharFormat>() << envFormat << keyFormat;
    rule.pattern = QRegularExpression("(\\\\begin)\\{(\\w+)\\}");
    rule.pattern.setPatternOptions(
        QRegularExpression::InvertedGreedinessOption);
    rules.append(rule);
    rule.pattern = QRegularExpression("(\\\\end)\\{(\\w+)\\}");
    rule.pattern.setPatternOptions(
        QRegularExpression::InvertedGreedinessOption);
    rules.append(rule);
    return rules;
  }  // end of LaTeXSyntaxHighlighter::buildHighlightingRules

  const QVector<LaTeXSyntaxHighlighter::HighlightingRule>&
  LaTeXSyntaxHighlighter::getHighlightingRules() {
    static auto rules = LaTeXSyntaxHighlighter::buildHighlightingRules();
    return rules;
  }

  LaTeXSyntaxHighlighter::LaTeXSyntaxHighlighter(LaTeXMajorMode& m,
                                                 QTextDocument* p)
      : QSyntaxHighlighter(p), mode(m) {
    this->commentFormat.setForeground(Qt::red);
  }

  void LaTeXSyntaxHighlighter::highlightBlock(const QString& text) {
    const auto& rules = LaTeXSyntaxHighlighter::getHighlightingRules();
    // remove comments
    int c = LaTeXSyntaxHighlighter::startOfComment(text);
    QString l;
    if (c != -1) {
      this->setFormat(c, text.size() - c, this->commentFormat);
      l = text.mid(0, c);
    } else {
      l = text.mid(0, c);
    }
    int pos = 0;
    while (pos != l.size()) {
      int cpos = -1;
      auto mp = rules.end();
      for (auto p = rules.begin(); p != rules.end(); ++p) {
        QRegularExpression e(p->pattern);
        const auto m = e.match(l, pos);
        if (m.hasMatch()) {
          const auto rp = m.lastCapturedIndex();
          if ((cpos == -1) || (rp <= cpos)) {
            mp = p;
            cpos = rp;
          }
        }
      }
      if (cpos != -1) {
        if (cpos != pos) {
          this->highLightMispellWords(l.mid(pos, cpos - pos), pos);
        }
        // treating the LaTeX command
        const auto& rule = *mp;
        QRegularExpression e(rule.pattern);
        const auto m = e.match(l, pos);
        const int length = m.capturedLength();
        if (rule.format.size() == 1) {
          this->setFormat(cpos, length, rule.format[0]);
        } else {
#warning message("HERE")
          if (m.lastCapturedIndex() == rule.format.size()) {
            for (int i = 0; i != m.lastCapturedIndex(); ++i) {
              QString cap = m.captured(i + 1);
              int cs = cap.size();
              cpos = l.indexOf(cap, cpos);
              this->setFormat(cpos, cs, rule.format[i]);
              cpos += cs;
            }
          }
        }
        pos = cpos + length;
      } else {
        this->highLightMispellWords(l.mid(pos, l.size() - pos), pos);
        pos = l.size();
      }
    }
  }

  void LaTeXSyntaxHighlighter::highLightMispellWords(const QString& /* l */,
                                                     const int /* p */) {
    // using namespace std;
    // QTextCharFormat f;
    // auto& spellChecker = this->mode.getSpellChecker();
    // f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    // int pos = 0;
    // while (pos != l.size()) {
    //   if (l[pos].isLetter()) {
    //     const auto npos = pos;
    //     ++pos;
    //     while ((pos != l.size()) && (l[pos].isLetter())) {
    //       ++pos;
    //     }
    //     if (!spellChecker.spell(l.mid(npos, pos - npos))) {
    //       this->setFormat(p + npos, pos - npos, f);
    //     }
    //   } else {
    //     ++pos;
    //   }
    // }
  }  // end of LaTeXSyntaxHighlighter::highLightMispellWords

}  // end of namespace tfel::gui
