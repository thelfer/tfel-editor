/*!
 * \file  LaTeXSyntaxHighlighter.cxx
 * \brief
 * \author Thomas Helfer
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
    //     rule.format = QVector<QTextCharFormat>()
    //                   << this->keyFormat << this->boldFormat;
    //     rule.pattern = QRegularExpression(R"((\textbf)\{(\w+)\})");
    //     rule.pattern.setMinimal(true);
    // rules.append(rule);
    // rule.format  = QVector<QTextCharFormat>() << this->keyFormat <<
    // this->itFormat;
    // rule.pattern = QRegularExpression("(\\\\textit)\\{(\\w+)\\}");
    // rule.pattern.setMinimal(true);
    // rules.append(rule);
    rule.format = QVector<QTextCharFormat>(1, keyFormat);
    rule.pattern = QRegularExpression(R"(\\\w+)");
    if (rule.pattern.isValid()) {
      rules.append(rule);
    }
    rule.format = QVector<QTextCharFormat>() << envFormat << keyFormat;
    rule.pattern = QRegularExpression(R"((\\begin)\{(\w+)\})");
    rule.pattern.setPatternOptions(
        QRegularExpression::InvertedGreedinessOption);
    if (rule.pattern.isValid()) {
      rules.append(rule);
    }
    rule.pattern = QRegularExpression(R"((\\end)\{(\w+)\})");
    rule.pattern.setPatternOptions(
        QRegularExpression::InvertedGreedinessOption);
    if (rule.pattern.isValid()) {
      rules.append(rule);
    }
    return rules;
  }  // end of LaTeXSyntaxHighlighter::buildHighlightingRules

  const QVector<LaTeXSyntaxHighlighter::HighlightingRule>&
  LaTeXSyntaxHighlighter::getHighlightingRules() {
    static auto rules = LaTeXSyntaxHighlighter::buildHighlightingRules();
    return rules;
  }

  const QTextCharFormat& LaTeXSyntaxHighlighter::getCommentFormat() {
    static const auto f = [] {
      auto r = QTextCharFormat();
      r.setForeground(Qt::red);
      return r;
    }();
    return f;
  }  // end of LaTeXSyntaxHighlighter::getCommentFormat

  LaTeXSyntaxHighlighter::Line LaTeXSyntaxHighlighter::parseLine(
      const QString& text) {
    auto r = Line();
    // everything past the '%' which opens a comment is a comment, whatever the
    // rules may find in it
    r.comment = LaTeXSyntaxHighlighter::startOfComment(text);
    const auto e = (r.comment == -1) ? text.size() : r.comment;
    const auto& rules = LaTeXSyntaxHighlighter::getHighlightingRules();
    auto pos = 0;
    while (pos < e) {
      // looking for the leftmost command, the longest match winning among the
      // rules starting at the same position
      auto start = e;
      auto length = 0;
      auto best = -1;
      auto match = QRegularExpressionMatch();
      for (auto i = 0; i != rules.size(); ++i) {
        const auto& pattern = rules[i].pattern;
        if (!pattern.isValid()) {
          continue;
        }
        const auto m = pattern.match(text, pos);
        // a command which starts in the comment is a part of the comment
        if (!m.hasMatch() || (m.capturedStart() >= e)) {
          continue;
        }
        if ((m.capturedStart() < start) ||
            ((m.capturedStart() == start) && (m.capturedLength() > length))) {
          start = m.capturedStart();
          length = m.capturedLength();
          best = i;
          match = m;
        }
      }
      if ((best < 0) || (length < 1)) {
        // no command left in the line: the rest of it is plain text
        r.plainText.emplace_back(pos, e - pos);
        break;
      }
      if (start > pos) {
        r.plainText.emplace_back(pos, start - pos);
      }
      r.commands.push_back({match, best});
      // the match is not empty and starts at or after pos, so the command is
      // skipped and the loop always moves forward
      pos = start + length;
    }
    return r;
  }  // end of LaTeXSyntaxHighlighter::parseLine

  LaTeXSyntaxHighlighter::LaTeXSyntaxHighlighter(LaTeXMajorMode& m,
                                                 QTextDocument* p)
      : QSyntaxHighlighter(p), mode(m) {}

  void LaTeXSyntaxHighlighter::highlightBlock(const QString& text) {
    const auto l = LaTeXSyntaxHighlighter::parseLine(text);
    const auto& rules = LaTeXSyntaxHighlighter::getHighlightingRules();
    if (l.comment != -1) {
      this->setFormat(l.comment, text.size() - l.comment,
                      LaTeXSyntaxHighlighter::getCommentFormat());
    }
    for (const auto& c : l.commands) {
      if ((c.rule < 0) || (c.rule >= rules.size())) {
        continue;
      }
      this->highLightCommand(c, rules[c.rule]);
    }
    for (const auto& p : l.plainText) {
      this->highLightMispellWords(text.mid(p.first, p.second), p.first);
    }
  }  // end of LaTeXSyntaxHighlighter::highlightBlock

  void LaTeXSyntaxHighlighter::highLightCommand(const Command& c,
                                                const HighlightingRule& r) {
    if (r.format.isEmpty()) {
      return;
    }
    const auto& m = c.match;
    if (m.lastCapturedIndex() != r.format.size()) {
      // a single format for the whole match, which is the case of the rule
      // matching any command and, should it ever happen, of a rule whose
      // capture groups and formats do not match
      this->setFormat(m.capturedStart(), m.capturedLength(), r.format[0]);
      return;
    }
    // each group gets its own format, which is how the `\begin` keyword and
    // the name of the environment are told apart
    for (auto i = 1; i <= m.lastCapturedIndex(); ++i) {
      // an optional group which took no part in the match holds no position
      const auto s = m.capturedStart(i);
      if (s < 0) {
        continue;
      }
      this->setFormat(s, m.capturedLength(i), r.format[i - 1]);
    }
  }  // end of LaTeXSyntaxHighlighter::highLightCommand

  void LaTeXSyntaxHighlighter::highLightMispellWords(const QString& l,
                                                     const int p) {
    auto& spellChecker = this->mode.getSpellChecker();
    auto f = QTextCharFormat();
    f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    // the spell checker gives positions in the given portion of text, the
    // highlighter wants them in the whole line
    for (const auto& [b, e] : spellChecker.spellLine(l)) {
      this->setFormat(p + b, e - b, f);
    }
  }  // end of LaTeXSyntaxHighlighter::highLightMispellWords

}  // end of namespace tfel::gui
