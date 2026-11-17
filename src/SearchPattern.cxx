/*!
 * \file  SearchPattern.cxx
 * \brief
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtGui/QTextDocument>

#include "TFEL/GUI/SearchPattern.hxx"

namespace tfel::gui {

  bool SearchMatch::isNull() const {
    return this->start < 0;
  }  // end of SearchMatch::isNull

  int SearchMatch::size() const {
    return this->end - this->start;
  }  // end of SearchMatch::size

  SearchPattern::SearchPattern(const QString& p,
                               const bool isRegexp,
                               const bool withCase,
                               const bool wholeWordsOnly,
                               const QString& with)
      : text(p),
        regexp(isRegexp),
        caseSensitive(withCase),
        wholeWords(wholeWordsOnly),
        replaceWith(with) {
    if (!this->regexp) {
      return;
    }
    auto options = QRegularExpression::MultilineOption |
                   QRegularExpression::UseUnicodePropertiesOption;
    if (!this->caseSensitive) {
      options |= QRegularExpression::CaseInsensitiveOption;
    }
    this->expression.setPatternOptions(options);
    this->expression.setPattern(p);
    if (!this->expression.isValid()) {
      this->error = this->expression.errorString();
    }
  }  // end of SearchPattern::SearchPattern

  const QString& SearchPattern::pattern() const {
    return this->text;
  }  // end of SearchPattern::pattern

  bool SearchPattern::isRegexp() const {
    return this->regexp;
  }  // end of SearchPattern::isRegexp

  bool SearchPattern::isCaseSensitive() const {
    return this->caseSensitive;
  }  // end of SearchPattern::isCaseSensitive

  bool SearchPattern::isWholeWords() const {
    return this->wholeWords;
  }  // end of SearchPattern::isWholeWords

  bool SearchPattern::isValid() const {
    return (not this->regexp) or this->expression.isValid();
  }  // end of SearchPattern::isValid

  QString SearchPattern::errorString() const {
    return this->error;
  }  // end of SearchPattern::errorString

  void SearchPattern::setReplacement(const QString& with) {
    this->replaceWith = with;
  }  // end of SearchPattern::setReplacement

  const QString& SearchPattern::replacement() const {
    return this->replaceWith;
  }  // end of SearchPattern::replacement

  QString SearchPattern::replacementOf(const SearchMatch& m) const {
    if (not this->regexp) {
      return this->replaceWith;
    }
    QString r;
    int k = 0;
    while (k != this->replaceWith.size()) {
      const QChar c = this->replaceWith.at(k);
      if (c != QLatin1Char('\\')) {
        r += c;
        ++k;
        continue;
      }
      ++k;
      if (k == this->replaceWith.size()) {
        // a replacement which ends on a backslash means that backslash
        r += c;
        break;
      }
      const QChar d = this->replaceWith.at(k);
      ++k;
      if ((d >= QLatin1Char('0')) and (d <= QLatin1Char('9'))) {
        const int g = d.unicode() - QLatin1Char('0').unicode();
        if ((g < m.groups.size()) and (not m.groups.at(g).isNull())) {
          r += m.groups.at(g);
        }
        continue;
      }
      r += d;
    }
    return r;
  }  // end of SearchPattern::replacementOf

  bool SearchPattern::standsAlone(const QString& t,
                                  const int start,
                                  const int end) const {
    const auto wordCharacter = [](const QChar c) {
      return c.isLetterOrNumber();
    };
    if ((start > 0) and wordCharacter(t.at(start - 1))) {
      return false;
    }
    if ((end < t.size()) and wordCharacter(t.at(end))) {
      return false;
    }
    return true;
  }  // end of SearchPattern::standsAlone

  SearchMatch SearchPattern::matchStartingAt(const QString& t,
                                            const int position) const {
    SearchMatch m;
    if (this->regexp) {
      /* the match options are the fourth argument of this overload in Qt 6.2:
         the third one is the type of the match. */
      const auto r = this->expression.match(
          t, position, QRegularExpression::NormalMatch,
          QRegularExpression::AnchorAtOffsetMatchOption);
      if (!r.hasMatch()) {
        return m;
      }
      /* capturedEnd is the place one past the last character of the match, as
         capturedStart is the place of its first: `int\s+(\w+)\s*=\s*(\d+);` on
         `int a = 1;` answers 0 and 10 for the ten characters of that line
         (measured on Qt 6.2.4). An expression satisfied by no character at all,
         which `x*` is everywhere, names no text of the document. */
      m.start = r.capturedStart();
      m.end = r.capturedEnd();
      if (m.end <= m.start) {
        // a match of no character names no text of the document
        return SearchMatch{};
      }
      m.groups = r.capturedTexts();
    } else {
      const auto sensitivity = (this->caseSensitive) ? Qt::CaseSensitive
                                                     : Qt::CaseInsensitive;
      const QString at = t.mid(position, this->text.size());
      if (at.compare(this->text, sensitivity) != 0) {
        return m;
      }
      m.start = position;
      m.end = position + this->text.size();
      m.groups << at;
    }
    if ((this->wholeWords) and (not standsAlone(t, m.start, m.end))) {
      return SearchMatch{};
    }
    return m;
  }  // end of SearchPattern::matchStartingAt

  SearchMatch SearchPattern::findNextMatch(const QString& t,
                                          const int position) const {
    if (this->regexp) {
      /* the two ways a match is passed over are retried one character further:
         a match of no character, which a pattern like `x*` answers everywhere
         without naming any text and which an endless walk would replace nothing
         on, and a match the whole words option rejects, behind which another
         match of the same expression may stand satisfied. */
      int at = position;
      while (at < t.size()) {
        const auto r = this->expression.match(t, at);
        if (!r.hasMatch()) {
          return SearchMatch{};
        }
        const int start = r.capturedStart();
        const int end = r.capturedEnd();
        if (end <= start) {
          at = start + 1;
          continue;
        }
        if ((this->wholeWords) and (not standsAlone(t, start, end))) {
          at = start + 1;
          continue;
        }
        SearchMatch m;
        m.start = start;
        m.end = end;
        m.groups = r.capturedTexts();
        return m;
      }
      return SearchMatch{};
    }
    const auto sensitivity = (this->caseSensitive) ? Qt::CaseSensitive
                                                   : Qt::CaseInsensitive;
    int at = position;
    while (true) {
      const int index = t.indexOf(this->text, at, sensitivity);
      if (index < 0) {
        return SearchMatch{};
      }
      const int end = index + this->text.size();
      if ((this->wholeWords) and (not standsAlone(t, index, end))) {
        at = index + 1;
        continue;
      }
      SearchMatch m;
      m.start = index;
      m.end = end;
      m.groups << t.mid(index, end - index);
      return m;
    }
  }  // end of SearchPattern::findNextMatch

  SearchMatch SearchPattern::find(const QTextDocument& document,
                                  const int from,
                                  const bool backward) const {
    if ((not isValid()) or (this->text.isEmpty())) {
      return SearchMatch{};
    }
    const QString t = document.toPlainText();
    if ((t.isEmpty())) {
      return SearchMatch{};
    }
    if (not backward) {
      if (from >= t.size()) {
        // a match cannot start past the last character of the document
        return SearchMatch{};
      }
      return findNextMatch(t, qMax(from, 0));
    }
    /* a backward search asks for the last match which ends at or before the
       place: the matches have to be walked from the start of the document,
       since an expression says nothing of what stands before a place. */
    SearchMatch best;
    for (SearchMatch m = findNextMatch(t, 0); !m.isNull();
         m = findNextMatch(t, m.end)) {
      if (m.end > from) {
        break;
      }
      best = m;
    }
    return best;
  }  // end of SearchPattern::find

  SearchMatch SearchPattern::startingAt(const QTextDocument& document,
                                        const int position) const {
    if ((not isValid()) or (this->text.isEmpty()) or (position < 0)) {
      return SearchMatch{};
    }
    const QString t = document.toPlainText();
    if (position >= t.size()) {
      return SearchMatch{};
    }
    return matchStartingAt(t, position);
  }  // end of SearchPattern::startingAt

}  // end of namespace tfel::gui
