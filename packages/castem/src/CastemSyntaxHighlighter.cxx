/*!
 * \file  CastemSyntaxHighlighter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   05/08/2012
 */

#include "TFEL/GUI/CastemMajorMode.hxx"
#include "TFEL/GUI/CastemSyntaxHighlighter.hxx"

namespace tfel::gui {

  CastemSyntaxHighlighter::CastemSyntaxHighlighter(QTextDocument *p)
      : QSyntaxHighlighter(p) {
    this->keyFormat.setForeground(Qt::blue);
    this->quotationFormat.setForeground(Qt::darkGreen);
    this->commentFormat.setForeground(Qt::red);
    this->outOfBoundsFormat.setForeground(Qt::darkRed);
    HighlightingRule rq;
    rq.pattern = QRegularExpression("'.*'");
    rq.pattern.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    rq.format = this->quotationFormat;
    this->highlightingRules.append(rq);
    // castem keys
    const QStringList &keys = CastemMajorMode::getKeysList();
    foreach (const QString &pattern, keys) {
      HighlightingRule rule;
      rule.pattern = QRegularExpression("\\b" + pattern.toUpper() + "\\w*\\b");
      rule.format = this->keyFormat;
      this->highlightingRules.append(rule);
    }
  }  // end of CastemSyntaxHighlighter

  void CastemSyntaxHighlighter::highlightBlock(const QString &text) {
    if (text.isEmpty()) {
      return;
    }
    if (text[0] == '*') {
      this->setFormat(0, text.length(), this->commentFormat);
      return;
    }
    foreach (const HighlightingRule &rule, highlightingRules) {
      QRegularExpression expression(rule.pattern);
      auto match = expression.match(text);
      while (match.hasMatch()) {
        const auto index = match.lastCapturedIndex();
        const auto length = match.capturedLength();
        this->setFormat(index, length, rule.format);
        match = expression.match(text, index + length);
      }
    }
    // if text is longer than 80 characters
    if (text.size() > 80) {
      this->setFormat(80, text.length() - 80, this->outOfBoundsFormat);
    }
  }

}  // end of namespace tfel::gui
