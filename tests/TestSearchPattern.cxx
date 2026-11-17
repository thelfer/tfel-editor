/*!
 * \file   tests/TestSearchPattern.cxx
 * \brief  tests of the one search primitive the dialog and the query replace
 *         share: what a match is, which place it holds, and what a match is
 *         rewritten to.
 *
 * The whole words option is asserted against the Qt option this class exists to
 * reproduce: `QTextDocument::FindWholeWords` answers it for a plain text and
 * plays no part at all in a search with a regular expression, so the parity of
 * the two answers on a plain text is the evidence that the rule of
 * `SearchPattern::standsAlone` is the rule of Qt, and not a rule this file
 * invented. The rest is measured on the shapes a replaced text can take: an
 * expression whose replacement is longer than the match it replaces, an
 * expression satisfied by nothing, an alternative which leaves a group unfilled,
 * an answer asked at the end of a selection.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtTest/QtTest>

#include "TFEL/GUI/SearchPattern.hxx"

using tfel::gui::SearchPattern;

struct TestSearchPattern : QObject {
  Q_OBJECT

  private slots:

  /*!
   * \brief the whole words option answers the same places as the Qt option of
   *        the same name
   *
   * A search with `QTextDocument::FindWholeWords` names the places the option
   * accepts; the same search through this class has to name the same ones, on the
   * shapes which are the point of the option: the head or the tail of a longer
   * word, an underscore, a digit, punctuation, and the two ends of the buffer.
   */
  void theWholeWordsOptionAnswersWhatTheQtOptionAnswers() {
    const auto texts = QStringList()
                       << "beta betagam beta"      // the head of a longer word
                       << "foo_bar foo foo2"        // an underscore, a digit
                       << "(beta) beta! _beta"      // punctuation on either side
                       << "beta2 xbeta beta"        // the tail of a longer word
                       << "un beta, deux beta"      // a word between two words
                       << "beta\nbeta\tbeta-beta"   // the separators of a buffer
                       << "beta at the end of the beta"  // and the last place
                       << "BETA beta Beta";              // and the letter case
    for (const auto& t : texts) {
      QTextDocument d;
      d.setPlainText(t);
      auto byQt = QStringList();
      for (auto at = 0;;) {
        /* the case flag is what makes the two questions the same question:
           QTextDocument::find is case-INsensitive unless it is told otherwise,
           which is the opposite of what the dialog asks of its pattern. */
        const auto c = d.find("beta", at,
                              QTextDocument::FindWholeWords |
                                  QTextDocument::FindCaseSensitively);
        if (c.isNull()) {
          break;
        }
        byQt << QString("%1-%2").arg(c.selectionStart()).arg(c.selectionEnd());
        at = c.selectionEnd();
      }
      const auto p = SearchPattern("beta", false, true, true);
      QVERIFY2(places(p, d) == byQt.join(','),
               qPrintable(QString("on '%1': Qt answers [%2], the pattern "
                                  "answers [%3]")
                            .arg(t, byQt.join(','), places(p, d))));
    }
  }  // end of theWholeWordsOptionAnswersWhatTheQtOptionAnswers()

  //! \brief the three options decide what is found, one at a time
  void theOptionsDecideWhatIsFound() {
    QTextDocument d;
    d.setPlainText("BETA beta Beta beta");
    // the letter case, which the dialog asks by default
    QCOMPARE(places(SearchPattern("beta", false, true, false), d),
             QString("5-9,15-19"));
    QCOMPARE(places(SearchPattern("beta", false, false, false), d),
             QString("0-4,5-9,10-14,15-19"));
    // the whole words, which all four entries of that text satisfy
    QCOMPARE(places(SearchPattern("beta", false, true, true), d),
             QString("5-9,15-19"));
    QCOMPARE(places(SearchPattern("beta", false, false, true), d),
             QString("0-4,5-9,10-14,15-19"));
    // the expression, which reads the field as an expression and not as a text.
    // `b.ta` is the shape of a pattern with a metacharacter where the word has
    // its `t`: four characters either way, and the dot stands for what is there
    QCOMPARE(places(SearchPattern("b.ta", false, true, false), d), QString());
    QCOMPARE(places(SearchPattern("b.ta", true, true, false), d),
             QString("5-9,15-19"));
    // the whole words option of an expression reads the same boundaries
    QCOMPARE(places(SearchPattern("b.ta", true, false, true), d),
             QString("0-4,5-9,10-14,15-19"));
    QCOMPARE(places(SearchPattern("beta", true, true, true), d),
             QString("5-9,15-19"));
    // the metacharacters of a pattern of a plain search are characters: what a
    // reader who does not tick the option types is what is looked for
    QCOMPARE(places(SearchPattern("beta|BETA", false, true, false), d),
             QString());
    QCOMPARE(places(SearchPattern("beta|BETA", true, false, false), d),
             QString("0-4,5-9,10-14,15-19"));
  }  // end of theOptionsDecideWhatIsFound()

  /*!
   * \brief an expression is asked of the whole text of the document, so a
   *        pattern of several lines is a pattern of the buffer
   *
   * The places of a document and the places of its whole text are the same
   * numbers, which is what lets an answer of the expression be applied to the
   * document without a conversion. `^` and `$` therefore name the ends of a line
   * of the buffer, the multiline option being what makes them do so against the
   * newline the whole text spells the end of a block with.
   */
  void anExpressionSpeaksOfTheLinesOfTheBuffer() {
    QTextDocument d;
    d.setPlainText("int a = 1;\nint b = 22;\nfloat c = 3;\n");
    const auto assignments = SearchPattern("int\\s+(\\w+)\\s*=\\s*(\\d+);",
                                           true, true, false, "\\1 := \\2");
    QVERIFY(assignments.isValid());
    QCOMPARE(places(assignments, d), QString("0-10,11-22"));
    auto hits = QStringList();
    for (auto m = assignments.find(d, 0); not m.isNull();
         m = assignments.find(d, m.end)) {
      hits << QString("%1->%2").arg(d.toPlainText().mid(m.start, m.size()),
                                    assignments.replacementOf(m));
    }
    QCOMPARE(hits, (QStringList() << "int a = 1;->a := 1"
                                  << "int b = 22;->b := 22"));
    // the ends of a line: `^` and `$` of the multiline option, which a search
    // of a single line could not answer at all
    QTextDocument e;
    e.setPlainText("one\ntwo\nthree\n");
    const auto lineStart =
      SearchPattern("^(t\\w+)$", true, true, false, ">\\1<");
    QCOMPARE(places(lineStart, e), QString("4-7,8-13"));
    QCOMPARE(places(SearchPattern("^t", true, true, false), e),
             QString("4-5,8-9"));
    // and an expression which names no line break is kept inside a line: the two
    // matches below are the two lines of the text, and a match of the whole text
    // would name 0-18
    QCOMPARE(places(SearchPattern("\\w+ \\w+", true, true, false),
                    QTextDocument("one two\nthree four")),
             QString("0-7,8-18"));
  }  // end of anExpressionSpeaksOfTheLinesOfTheBuffer()

  //! \brief what a match is rewritten to
  void aMatchIsRewrittenToWhatTheReplacementSays() {
    QTextDocument d;
    d.setPlainText("foo(2) bar foo(33)");
    const auto re =
      SearchPattern("foo\\((\\d+)\\)", true, true, false, "<\\1>");
    const auto first = re.find(d, 0);
    const auto second = re.find(d, first.end);
    QCOMPARE(first.start, 0);
    QCOMPARE(second.start, 11);
    QCOMPARE(re.replacementOf(first), QString("<2>"));
    QCOMPARE(re.replacementOf(second), QString("<33>"));
    // `\0` is the whole match, beside the groups the expression answers
    QCOMPARE(SearchPattern("foo\\((\\d+)\\)", true, true, false, "\\0/\\0")
               .replacementOf(first),
             QString("foo(2)/foo(2)"));
    // a backslash before anything else stands for that character alone, which
    // is how a replacement writes a backslash it means literally; a trailing
    // backslash has nothing to stand for and is itself
    QCOMPARE(SearchPattern("foo\\((\\d+)\\)", true, true, false, "a\\nb\\")
               .replacementOf(first),
             QString("anb\\"));
    // a group an alternative leaves unfilled writes nothing
    const auto alternative =
      SearchPattern("(a)|(b)", true, true, false, "[\\1|\\2]");
    QCOMPARE(alternative.replacementOf(alternative.find(QTextDocument("b"), 0)),
             QString("[|b]"));
    QCOMPARE(alternative.replacementOf(alternative.find(QTextDocument("a"), 0)),
             QString("[a|]"));
    // a number names one group, so `\12` is the first group then a two
    const auto twelfth = SearchPattern("(a)", true, true, false, "[\\12]");
    QCOMPARE(twelfth.replacementOf(twelfth.find(QTextDocument("a"), 0)),
             QString("[a2]"));
    // a plain search is no expression: the digits of its replacement are what
    // the reader typed, since a text search has no groups to answer
    const auto plain = SearchPattern("beta", false, true, false, "x\\1y");
    QCOMPARE(plain.replacementOf(plain.find(QTextDocument("beta"), 0)),
             QString("x\\1y"));
  }  // end of aMatchIsRewrittenToWhatTheReplacementSays()

  /*!
   * \brief the two directions walk from the place they are given, and the place
   *        of a selection is the place which passes that selection over
   *
   * This is the whole of what lets `Find next` find the *next* match: the dialog
   * gives the end of the selection it just made, and the answer is not the match
   * the screen already holds.
   */
  void aSearchStartsWhereItIsAskedTo() {
    QTextDocument d;
    d.setPlainText("beta gamma beta delta beta");
    const auto p = SearchPattern("beta", false, true, false, "x");
    QCOMPARE(p.find(d, 0).start, 0);
    QCOMPARE(p.find(d, 5).start, 11);
    QCOMPARE(p.find(d, 12).start, 22);
    // a match which starts exactly where the search is asked from is an answer,
    // which is what makes the end of a selection pass that selection over
    QCOMPARE(p.find(d, 11).start, 11);
    QCOMPARE(p.find(d, 4).start, 11);
    // the end of the document, and a place past it, hold nothing ahead
    QVERIFY(p.find(d, static_cast<int>(d.toPlainText().size())).isNull());
    QVERIFY(p.find(d, 400).isNull());
    // backward: the last match which ends at or before the place
    QCOMPARE(p.find(d, 22, true).start, 11);
    QCOMPARE(p.find(d, 26, true).start, 22);
    QCOMPARE(p.find(d, 15, true).start, 11);
    QCOMPARE(p.find(d, 5, true).start, 0);
    QVERIFY(p.find(d, 1, true).isNull());
    // startingAt: the question of a replacement of what the screen holds
    QCOMPARE(p.startingAt(d, 0).end, 4);
    QCOMPARE(p.startingAt(d, 11).end, 15);
    QVERIFY(p.startingAt(d, 1).isNull());
    QVERIFY(p.startingAt(d, 5).isNull());
  }  // end of aSearchStartsWhereItIsAskedTo()

  /*!
   * \brief what a search is asked not to answer: nothing at all
   *
   * The four shapes which have no answer, and the report each owes. A pattern of
   * no text is a valid pattern which finds nothing; an expression which does not
   * compile is not a valid pattern and says why; a match of no character is
   * never an answer, since rewriting nothing in the place of nothing is a walk
   * which never ends; and an empty document holds no match of anything.
   */
  void nothingIsAnsweredToTheQuestionsWhichHaveNoAnswer() {
    const auto text = QTextDocument("one\ntwo\nthree\n");
    // an empty pattern: a valid pattern which asks for nothing
    const auto empty = SearchPattern("", false, true, false, "x");
    QVERIFY(empty.isValid());
    QVERIFY(empty.errorString().isEmpty());
    QVERIFY(empty.find(text, 0).isNull());
    QVERIFY(empty.startingAt(text, 0).isNull());
    // an expression which does not compile
    const auto broken = SearchPattern("foo((\\d+)", true, true, false, "x");
    QVERIFY(not broken.isValid());
    QVERIFY(not broken.errorString().isEmpty());
    QVERIFY(broken.find(text, 0).isNull());
    QVERIFY(broken.startingAt(text, 0).isNull());
    // the same characters as a plain text are a valid pattern, parentheses
    // and all: only an expression has to compile
    const auto literal = SearchPattern("foo((\\d+)", false, true, false, "x");
    QVERIFY(literal.isValid());
    QVERIFY(literal.errorString().isEmpty());
    // a match of no character names no text of the document: `x*` is satisfied
    // everywhere and would be rewritten on nothing, forever
    const auto star = SearchPattern("x*", true, true, false, "-");
    const auto where = star.find(QTextDocument("a x b"), 0);
    QVERIFY(not where.isNull());
    QCOMPARE(where.size(), 1);
    QVERIFY(star.find(QTextDocument("abc"), 0).isNull());
    // an expression the optional group makes satisfiable by nothing is answered
    // by the shape which holds a character
    const auto optional = SearchPattern("(x)?y", true, true, false, "[\\1]");
    const auto first = optional.find(QTextDocument("xy y"), 0);
    QCOMPARE(first.start, 0);
    QCOMPARE(optional.replacementOf(first), QString("[x]"));
    const auto second = optional.find(QTextDocument("xy y"), first.end);
    QCOMPARE(second.start, 3);
    QCOMPARE(optional.replacementOf(second), QString("[]"));
    // an empty document
    const auto nothing = QTextDocument();
    const auto beta = SearchPattern("beta", false, true, false, "x");
    QVERIFY(beta.find(nothing, 0).isNull());
    QVERIFY(beta.startingAt(nothing, 0).isNull());
    // what no line of a document holds
    QVERIFY(SearchPattern("zebra", false, true, false).find(text, 0).isNull());
    QVERIFY(SearchPattern("^z", true, true, false).find(text, 0).isNull());
  }  // end of nothingIsAnsweredToTheQuestionsWhichHaveNoAnswer()

  /*!
   * \brief the walk of a replace-all over one text rewritten as it goes
   *
   * The walk is the one thing a search and a replacement have to agree on: the
   * place the next search starts from is computed from the match as it was found,
   * so a replacement longer than the match it stands for must not make the next
   * search read the text the previous one wrote. The two shapes here are the
   * replacement shorter than its match and the replacement longer than it, which
   * is the only one of the two a rewind of a fixed length breaks.
   */
  void aWalkOfReplacementsReachesEveryMatchOnce() {
    const auto walk = [](const SearchPattern& p, QString text) {
      auto at = 0;
      auto count = 0;
      while (count < 20) {
        QTextDocument d;
        d.setPlainText(text);
        const auto m = p.find(d, at);
        if (m.isNull()) {
          break;
        }
        const auto written = p.replacementOf(m);
        text = text.left(m.start) + written + text.mid(m.end);
        at = m.start + static_cast<int>(written.size());
        ++count;
      }
      return QStringList() << QString::number(count) << text;
    };
    QCOMPARE(walk(SearchPattern("foo\\((\\d+)\\)", true, true, false, "[\\1]"),
                  "foo(1) foo(22) bar foo(333)"),
             (QStringList() << "3" << "[1] [22] bar [333]"));
    // the replacement longer than the match: the next search has to start behind
    // the text written, not behind the place the match held
    QCOMPARE(walk(SearchPattern("ab", false, true, false, "LONGER"), "ab ab ab"),
             (QStringList() << "3" << "LONGER LONGER LONGER"));
    // a replacement which writes what it replaces is a walk which ends: the
    // search moves on whatever the replacement wrote
    QCOMPARE(walk(SearchPattern("aa", false, true, false, "aa"), "aa aa aa"),
             (QStringList() << "3" << "aa aa aa"));
    // a replacement of nothing: the two words are taken and the one space which
    // stood between them is all the buffer is left with
    QCOMPARE(walk(SearchPattern("bb", false, true, false, QString()), "bb bb"),
             (QStringList() << "2" << " "));
  }  // end of aWalkOfReplacementsReachesEveryMatchOnce()

  //! \brief the answers of a pattern are what it was built with
  void thePatternKeepsWhatItWasBuiltWith() {
    const auto p = SearchPattern("beta", true, false, true, "x");
    QCOMPARE(p.pattern(), QString("beta"));
    QCOMPARE(p.isRegexp(), true);
    QCOMPARE(p.isCaseSensitive(), false);
    QCOMPARE(p.isWholeWords(), true);
    QCOMPARE(p.replacement(), QString("x"));
    const auto q = SearchPattern("beta", false, true, false);
    QCOMPARE(q.pattern(), QString("beta"));
    QCOMPARE(q.isRegexp(), false);
    QCOMPARE(q.isCaseSensitive(), true);
    QCOMPARE(q.isWholeWords(), false);
    QCOMPARE(q.replacement(), QString());
    // the replacement is the one thing a pattern is asked to forget, which is
    // what lets the query replace ask the two questions one after the other
    auto r = SearchPattern("beta", false, true, false);
    r.setReplacement("<&>");
    QCOMPARE(r.replacement(), QString("<&>"));
    QCOMPARE(r.replacementOf(r.find(QTextDocument("beta"), 0)), QString("<&>"));
    // the groups of an answer: the whole match first, then the expression's
    const auto groups = SearchPattern("(b)(et)(a)", true, true, false);
    QCOMPARE(groups.find(QTextDocument("beta"), 0).groups,
             (QStringList() << "beta"
                            << "b"
                            << "et"
                            << "a"));
    QCOMPARE(SearchPattern("beta", false, true, false)
               .find(QTextDocument("beta"), 0)
               .groups,
             QStringList() << "beta");
  }  // end of thePatternKeepsWhatItWasBuiltWith()

  private:
  /*!
   * \return the places of the matches of a pattern, as `start-end` entries
   * \param[in] p : the pattern
   * \param[in] d : the document searched
   * \param[in] backward : walk the document from its end instead
   *
   * \note the walk is bounded: `find` does not wrap, so the number of matches of
   * a text is not something the caller of a probe has to know, and a walk of the
   * whole document is what the two directions answer here.
   */
  static QString places(const SearchPattern& p,
                        const QTextDocument& d,
                        const bool backward = false) {
    auto hits = QStringList();
    auto at = (backward) ? static_cast<int>(d.toPlainText().size()) : 0;
    for (auto guard = 0; guard != 100; ++guard) {
      const auto m = p.find(d, at, backward);
      if (m.isNull()) {
        break;
      }
      hits << QString("%1-%2").arg(m.start).arg(m.end);
      at = (backward) ? m.start : m.end;
    }
    return hits.join(',');
  }  // end of places()
};  // end of struct TestSearchPattern

QTEST_GUILESS_MAIN(TestSearchPattern)

#include "TestSearchPattern.moc"
