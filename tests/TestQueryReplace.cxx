/*!
 * \file   tests/TestQueryReplace.cxx
 * \brief  tests of the query replace of the minibuffer and of its regular
 *         expression variant: which questions the two ask, what an answer of an
 *         expression is written, and where the two commands part company.
 *
 * The query replace is three questions and a walk: the pattern, its replacement,
 * and then the buffer walked one match at a time with `y`, `n` and `!`. The plain
 * command and the one of a regular expression are the same three questions, and
 * they differ in exactly two things, which is what this file is about: the first
 * answer of the second is read as an expression, and its second answer may name
 * the groups of that expression. Everything else is asserted to be the same, the
 * plain command included, since the whole feature was rebuilt on the one search
 * primitive the dialog uses.
 *
 * \note the shapes of the buffers hold no `y`, `n`, `q` or `!` of their own, so
 * an answer which leaked into the text instead of walking the matches is a
 * difference in the lines, not something a reader has to tell apart
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::CommandLine;
  using tfel::gui::EditorWidget;
  using tfel::gui::TextEditBase;

  void pump(int n = 4) {
    for (int k = 0; k != n; ++k) {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
  }  // end of pump()

  //! an editor which records what it displays in the minibuffer
  struct ReportingEditor : EditorWidget {
    ReportingEditor() : EditorWidget(nullptr) {}
    QStringList messages;
    void displayInformativeMessage(const QString& m) override {
      this->messages << m;
      EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct ReportingEditor

  /*!
   * \brief an editor of its own, shown
   *
   * \note the editor is shown because a buffer is only fully built when it is
   * visible
   */
  struct Editor {
    ReportingEditor ed;
    Editor() {
      this->ed.setMinimumSize(600, 400);
      this->ed.resize(800, 600);
      this->ed.show();
      pump(8);
    }
    ~Editor() { pump(4); }
  };  // end of struct Editor

  //! \return the absolute name of a file written in the given directory
  QString writeFile(QTemporaryDir& d, const QString& n, const QString& body) {
    const auto p = d.filePath(n);
    QFile f(p);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return {};
    }
    f.write(body.toUtf8());
    f.close();
    return QFileInfo(p).absoluteFilePath();
  }  // end of writeFile()

  /*!
   * \brief true when the widget is still laid out
   *
   * \note an input which was answered lingers as a live child of the minibuffer
   * until the `deleteLater` its removal asked for runs, so looking for a child
   * finds the previous question too: what the layout still holds is the question
   * on the screen
   */
  bool isLaidOut(QWidget& w) {
    auto* const parent = w.parentWidget();
    auto* const layout = (parent != nullptr) ? parent->layout() : nullptr;
    return (layout != nullptr) && (layout->indexOf(&w) >= 0);
  }  // end of isLaidOut()

  //! \return the question the editor waits an answer from
  CommandLine* liveInput(EditorWidget& ed) {
    for (auto* const input : ed.findChildren<CommandLine*>()) {
      if (isLaidOut(*input)) {
        return input;
      }
    }
    return nullptr;
  }  // end of liveInput()

  //! \return the entry of the given question
  tfel::gui::LineEdit* entryOf(CommandLine& cl) {
    for (auto* const line : cl.findChildren<tfel::gui::LineEdit*>()) {
      if (isLaidOut(*line)) {
        return line;
      }
    }
    return nullptr;
  }  // end of entryOf()

  /*!\brief the prompt of a question
   *
   * The prompt is drawn as HTML, so a test asks whether it names a thing rather
   * than comparing it to a string: what the reader reads is the words, and the
   * markup around them is the color of the minibuffer.
   */
  QString labelOf(CommandLine& cl) {
    auto* const l = cl.findChild<QLabel*>();
    return (l != nullptr) ? l->text() : QString("<no prompt>");
  }  // end of labelOf()

  void press(QWidget* const w, const Qt::Key k, const Qt::KeyboardModifiers m) {
    if (w != nullptr) {
      // the argument is a Qt::Key and not an int: QTest::keyClick is overloaded
      // on char and an int selects that overload, which maps its argument
      // through asciiToKey and asserts
      QTest::keyClick(w, k, m);
    }
  }  // end of press()

  /*!\brief presses `Alt-%` or `Ctrl-Alt-%`
   *
   * The second of the two is what the branch of the editor answers to the
   * modifier pair, and on the layout of this machine the pair arrives as
   * `Alt+Ctrl` with no shift. The branch also accepts the shift of the `%` of a
   * US keyboard, which the key of that keyboard reaches it with; what is tested
   * here is the pair the editor is documented with.
   */
  void startQueryReplace(TextEditBase& frame, const bool regexp) {
    press(frame.widget(), Qt::Key_Percent,
          (regexp) ? Qt::KeyboardModifiers(Qt::AltModifier | Qt::ControlModifier)
                   : Qt::KeyboardModifiers(Qt::AltModifier));
    pump();
  }  // end of startQueryReplace()

  //! \brief answers the question the minibuffer holds
  void answer(EditorWidget& ed, const QString& text) {
    auto* const question = liveInput(ed);
    QVERIFY2(question != nullptr, "the query replace asked no question");
    auto* const entry = entryOf(*question);
    QVERIFY2(entry != nullptr, "the question holds no entry to answer it with");
    entry->setText(text);
    question->userEditingFinished();
    pump();
  }  // end of answer()

  /*!\brief answers the walk of the matches with `y`, `n` or `!`
   *
   * The answers are typed into the buffer, not into the minibuffer: the stage of
   * the walk hands the keys to a filter of the text edit, which is what makes a
   * query replace a conversation with the buffer rather than with the prompt.
   */
  void reply(TextEditBase& frame, const QChar a) {
    press(frame.widget(), (a == 'y') ? Qt::Key_Y
                          : (a == 'n') ? Qt::Key_N
                                       : Qt::Key_Exclam,
          Qt::NoModifier);
    pump();
  }  // end of reply()

  /*!\brief the places the selection of the buffer holds, with the line it starts
   *        on
   *
   * The line is taken from the document and not from `QTextCursor::blockNumber`,
   * which asks the layout of the document and is only brought up to date when the
   * text is drawn.
   */
  QString selectionWhere(TextEditBase& frame) {
    const auto tc = frame.textCursor();
    const auto b = frame.document()->findBlock(tc.selectionStart());
    return QString("%1-%2-%3").arg(b.blockNumber()).arg(tc.selectionStart())
                             .arg(tc.selectionEnd());
  }  // end of selectionWhere()

  //! \return the lines of the document of the buffer on the screen
  QStringList blocks(TextEditBase& frame) {
    auto l = QStringList();
    auto* const d = frame.document();
    for (auto b = d->begin(); b != d->end(); b = b.next()) {
      l << b.text();
    }
    return l;
  }  // end of blocks()

  //! \brief what the two commands remember of their answers
  struct Histories {
    QStringList plain;
    QStringList regexp;
  };  // end of struct Histories

  /*!\return the two histories of the query replace
   *
   * The two lists are the two addresses the command writes, read here rather than
   * through the class: what a test asserts on is the file the settings dialog
   * reads, and the addresses are the contract.
   */
  Histories readHistories() {
    auto s = QSettings();
    return {s.value("user input/query replace/history").toStringList(),
            s.value("user input/query replace regexp/history").toStringList()};
  }  // end of readHistories()

  //! \brief forgets what the two commands remember
  void forgetHistories() {
    auto s = QSettings();
    s.remove("user input/query replace/history");
    s.remove("user input/query replace regexp/history");
  }  // end of forgetHistories()
}  // end of anonymous namespace

//! tests of the query replace and of the query replace of a regular expression
struct TestQueryReplace : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void aPlainQueryReplaceWalksTheBufferOneMatchAtATime();
  void theRegularExpressionVariantNamesItselfInBothQuestions();
  void theSameCharactersAreAnExpressionToOneCommandAndATextToTheOther();
  void theGroupsOfAnAnswerAreWrittenInPlaceOfTheMatch();
  void theWholeMatchIsAvailableToItsOwnReplacement();
  void anInvalidExpressionIsReportedAndAskedAgainHoldingWhatWasTyped();
  void anInvalidPatternOfThePlainCommandIsATextLikeAnyOther();
  void aPatternWhichNamesNothingEndsTheCommandAtOnce();
  void anAnswerOfTheWalkWhichIsNotOneOfTheThreeIsReported();
  void controlGEndsTheWalkWithNothingWritten();
  void theTwoCommandsKeepTwoHistories();
  void anEmptyFirstAnswerWithNoHistorySaysSo();
  void aBufferWhichIsFrozenIsRefusedBothCommands();

 private:
  QTemporaryDir dir;
};

void TestQueryReplace::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-query-replace");
  QVERIFY2(this->dir.isValid(), "no temporary directory to write the files in");
  // the two histories decide the wording of the first question, and the cases
  // which name that wording would otherwise be answered by whichever case of an
  // earlier run wrote last
  forgetHistories();
}  // end of TestQueryReplace::initTestCase

/*!
 * \brief the query replace of a text walks the matches of the buffer and reports
 *        what it wrote
 *
 * The control case of the whole file: the three questions, the walk, and the
 * report, asserted on the command which existed before the regular expression one
 * was added, so that the rest of this file can name what the two commands do not
 * have in common and nothing else.
 */
void TestQueryReplace::aPlainQueryReplaceWalksTheBufferOneMatchAtATime() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-plain.txt", "abc abc\nabc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, false);
  auto* const first = liveInput(e.ed);
  QVERIFY2(first != nullptr, "`Alt-%` asked nothing in the minibuffer");
  QVERIFY(labelOf(*first).contains("query replace"));
  // the plain command names no expression: a reader who ticks nothing reads what
  // they wrote as the characters they wrote
  QVERIFY(not labelOf(*first).contains("regexp"));
  answer(e.ed, "abc");
  // the second question names what it was given to replace: the reader who is
  // asked what to write with sees what they are writing it to
  QVERIFY2(labelOf(*liveInput(e.ed)).contains("query replace abc with"),
           qPrintable(labelOf(*liveInput(e.ed))));
  answer(e.ed, "X");
  // the first match of the buffer is put on the screen
  QCOMPARE(selectionWhere(frame), QString("0-0-3"));
  // `n` passes it over, `y` takes the next one and brings the one behind it
  reply(frame, 'n');
  QCOMPARE(selectionWhere(frame), QString("0-4-7"));
  reply(frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "abc X"
                                         << "abc abc"
                                         << ""));
  /* the cursor of the walk stands on the separator of the line it replaced, and a
     place on a line boundary is the first place of the block behind it: the
     selection is the second `abc` of the buffer, which is 6..9 and on the first
     line, while the place 6 itself belongs to the second. */
  QCOMPARE(selectionWhere(frame), QString("1-6-9"));
  // `!` takes the match on the screen and every one behind it
  reply(frame, '!');
  QCOMPARE(blocks(frame),
           (QStringList() << "abc X"
                          << "X X"
                          << ""));
  QVERIFY2(e.ed.messages.contains("replaced 3 occurrences"),
           qPrintable(e.ed.messages.join(" | ")));
  QVERIFY2(not e.ed.hasUserInput(),
           "the command which replaced the last match left a question open");
  // no answer of the walk leaked into the buffer as a typed character
  QVERIFY2(not blocks(frame).join("").contains('y'),
           qPrintable(blocks(frame).join("")));
}  // end of aPlainQueryReplaceWalksTheBufferOneMatchAtATime()

//! \brief the second command says which of the two it is, in both questions
void TestQueryReplace::theRegularExpressionVariantNamesItselfInBothQuestions() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-names.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  auto* const first = liveInput(e.ed);
  QVERIFY2(first != nullptr, "`Ctrl-Alt-%` asked nothing in the minibuffer");
  QVERIFY2(labelOf(*first).contains("query replace regexp"),
           qPrintable(labelOf(*first)));
  answer(e.ed, "a.c");
  // the second question names the expression it was given, so that the reader
  // who is asked what to write with sees what they are writing it to
  const auto second = labelOf(*liveInput(e.ed));
  QVERIFY2(second.contains("query replace regexp"), qPrintable(second));
  QVERIFY2(second.contains("a.c"), qPrintable(second));
  answer(e.ed, "X");
  reply(frame, '!');
  QCOMPARE(blocks(frame), (QStringList() << "X X"
                                         << ""));
  QVERIFY2(e.ed.messages.contains("replaced 2 occurrences"),
           qPrintable(e.ed.messages.join(" | ")));
}  // end of theRegularExpressionVariantNamesItselfInBothQuestions()

/*!\brief the two commands are asked the same characters and answer two different
 *        questions
 *
 * This is the whole of what the two commands have apart: `a.c` is a text to one
 * and an expression to the other. The assertion is the pair of reports, which is
 * what the reader of a buffer which holds neither answer has — and the expression
 * is what answers the buffer, which is the half of the pair a broken option would
 * get wrong in silence.
 */
void TestQueryReplace::
    theSameCharactersAreAnExpressionToOneCommandAndATextToTheOther() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-either.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  // the expression: the buffer holds what the expression names
  startQueryReplace(frame, true);
  answer(e.ed, "a.c");
  answer(e.ed, "X");
  QCOMPARE(selectionWhere(frame), QString("0-0-3"));
  reply(frame, '!');
  QCOMPARE(blocks(frame), (QStringList() << "X X"
                                         << ""));
  // the text: the same characters are five of them, which no line of the buffer
  // holds — and the command says so rather than walking nothing
  frame.setPlainText("abc abc\n");
  pump();
  e.ed.messages.clear();
  startQueryReplace(frame, false);
  auto* const plain = liveInput(e.ed);
  QVERIFY(plain != nullptr);
  QVERIFY(not labelOf(*plain).contains("regexp"));
  answer(e.ed, "a.c");
  answer(e.ed, "X");
  QVERIFY2(e.ed.messages.contains("'a.c' not found"),
           qPrintable(e.ed.messages.join(" | ")));
  QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                         << ""));
  QVERIFY2(not e.ed.hasUserInput(),
           "a text which is not there left a question open");
}  // end of theSameCharactersAreAnExpressionToOneCommandAndATextToTheOther()

/*!\brief the groups of an expression go into the replacement, and the walk writes
 *        them per match
 *
 * Two groups and their answer in the other order: what a match is rewritten to is
 * computed from that match, so the two matches of the buffer must answer two
 * different texts from the same replacement. A replacement which ignored the
 * groups would leave the walk looking right on the first press and wrong on the
 * second, which is why both are asserted.
 */
void TestQueryReplace::theGroupsOfAnAnswerAreWrittenInPlaceOfTheMatch() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-groups.txt", "a-1 b-2\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "([a-z]+)-([0-9]+)");
  answer(e.ed, "\\2:\\1");
  QCOMPARE(selectionWhere(frame), QString("0-0-3"));
  reply(frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "1:a b-2"
                                         << ""));
  QCOMPARE(selectionWhere(frame), QString("0-4-7"));
  reply(frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "1:a 2:b"
                                         << ""));
  QVERIFY2(e.ed.messages.contains("replaced 2 occurrences"),
           qPrintable(e.ed.messages.join(" | ")));
  QVERIFY2(not e.ed.hasUserInput(), "the last match left a question open");
}  // end of theGroupsOfAnAnswerAreWrittenInPlaceOfTheMatch()

/*!\brief `\0` is the whole match and an unfilled group writes nothing
 *
 * The shape a plain text of the replacement is answered by: a command which
 * treated a backslash as a character would leave the backslash standing in the
 * buffer, and a command which read one of a plain text would answer `a\1b` to a
 * search of a text.
 */
void TestQueryReplace::theWholeMatchIsAvailableToItsOwnReplacement() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-whole.txt", "q q\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "(q)");
  answer(e.ed, "<\\0|\\1>");
  reply(frame, '!');
  QCOMPARE(blocks(frame), (QStringList() << "<q|q> <q|q>"
                                         << ""));
  // the digits of the replacement of a plain search are characters: that command
  // has no groups to answer with, and a reader who writes `a\1b` means a, a
  // backslash, a one and a b
  frame.setPlainText("q q\n");
  pump();
  startQueryReplace(frame, false);
  answer(e.ed, "q");
  answer(e.ed, "<\\1>");
  reply(frame, '!');
  QCOMPARE(blocks(frame), (QStringList() << "<\\1> <\\1>"
                                         << ""));
}  // end of theWholeMatchIsAvailableToItsOwnReplacement()

/*!\brief an expression which does not compile is no question about a buffer
 *
 * The report the reader of a missing parenthesis has to be given is the one the
 * expression engine makes of their answer, and the question has to come back
 * holding what they typed: a replacement of a character is a character to put
 * back, and an empty field would send them through the whole expression again.
 */
void TestQueryReplace::anInvalidExpressionIsReportedAndAskedAgainHoldingWhatWasTyped() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-invalid.txt", "b b\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "a((b");
  const auto told = e.ed.messages;
  QVERIFY2(not told.isEmpty(), "an expression which does not compile said nothing");
  QVERIFY2(not told.last().isEmpty(),
           "the report of an invalid expression was an empty line");
  QVERIFY2(not told.last().contains("not found"),
           qPrintable(QStringLiteral("a broken expression was answered as a "
                                      "buffer holding nothing: %1")
                          .arg(told.last())));
  // the question of the expression, asked again and holding the answer
  auto* const again = liveInput(e.ed);
  QVERIFY2(again != nullptr, "an invalid expression ended the command");
  QVERIFY2(labelOf(*again).contains("query replace regexp"),
           qPrintable(labelOf(*again)));
  auto* const entry = entryOf(*again);
  QVERIFY(entry != nullptr);
  QCOMPARE(entry->text(), QString("a((b"));
  // and the retry reaches the walk it was always meant to reach
  answer(e.ed, "b");
  answer(e.ed, "Z");
  QCOMPARE(selectionWhere(frame), QString("0-0-1"));
  reply(frame, '!');
  QCOMPARE(blocks(frame), (QStringList() << "Z Z"
                                         << ""));
}  // end of anInvalidExpressionIsReportedAndAskedAgainHoldingWhatWasTyped()

/*!\brief the same characters are a valid pattern for the plain command
 *
 * The other side of the case above: nothing may refuse parentheses to a command
 * that reads them as characters. `a((b` is a text no buffer of these files holds,
 * and the report is the one of a text which is not there.
 */
void TestQueryReplace::anInvalidPatternOfThePlainCommandIsATextLikeAnyOther() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-literal.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, false);
  answer(e.ed, "a((b");
  answer(e.ed, "Z");
  QVERIFY2(e.ed.messages.contains("'a((b' not found"),
           qPrintable(e.ed.messages.join(" | ")));
  QVERIFY2(not e.ed.hasUserInput(),
           "a plain text asked to be compiled before it was looked for");
  QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                         << ""));
}  // end of anInvalidPatternOfThePlainCommandIsATextLikeAnyOther()

//! \brief a pattern the buffer holds nothing of ends the command where it began
void TestQueryReplace::aPatternWhichNamesNothingEndsTheCommandAtOnce() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-nothing.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "z+");
  answer(e.ed, "X");
  QVERIFY2(e.ed.messages.contains("'z+' not found"),
           qPrintable(e.ed.messages.join(" | ")));
  QVERIFY2(not e.ed.hasUserInput(),
           "a pattern with no match kept the minibuffer for a walk that never "
           "began");
  QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                         << ""));
}  // end of aPatternWhichNamesNothingEndsTheCommandAtOnce()

/*!\brief a key which is none of the three answers is reported and does nothing
 *
 * The walk takes the keys of the buffer, so a key which is not an answer can not
 * be allowed to write in it: the reader who typed a `q` by mistake has to be told
 * what the keys of the moment are, and the buffer has to hold what it held.
 */
void TestQueryReplace::anAnswerOfTheWalkWhichIsNotOneOfTheThreeIsReported() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-stray.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "a.c");
  answer(e.ed, "X");
  e.ed.messages.clear();
  press(frame.widget(), Qt::Key_Q, Qt::NoModifier);
  pump();
  QVERIFY2(e.ed.messages.contains("type either 'y', 'n' or '!'"),
           qPrintable(e.ed.messages.join(" | ")));
  QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                         << ""));
  QCOMPARE(selectionWhere(frame), QString("0-0-3"));
}  // end of anAnswerOfTheWalkWhichIsNotOneOfTheThreeIsReported()

/*!\brief `Ctrl-G` ends the walk with nothing written
 *
 * The way out of a command which reached a buffer nobody meant, and the reason a
 * query replace is not a fate: the report is the one of a walk which replaced
 * nothing, which is what tells the reader that the keys they pressed before were
 * only moving the cursor through the matches.
 */
void TestQueryReplace::controlGEndsTheWalkWithNothingWritten() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-quit.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "a.c");
  answer(e.ed, "X");
  reply(frame, 'n');
  e.ed.messages.clear();
  press(frame.widget(), Qt::Key_G, Qt::ControlModifier);
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                         << ""));
  QVERIFY2(e.ed.messages.contains("replaced 0 occurrences"),
           qPrintable(e.ed.messages.join(" | ")));
  QVERIFY2(not e.ed.hasUserInput(), "`Ctrl-G` left a question in the minibuffer");
  // and the keys of the walk are spent: what is typed now goes in the buffer
  press(frame.widget(), Qt::Key_Q, Qt::NoModifier);
  pump();
  QVERIFY2(blocks(frame).first().contains('q'),
           qPrintable(blocks(frame).join(" | ")));
}  // end of controlGEndsTheWalkWithNothingWritten()

/*!\brief the two commands keep two histories
 *
 * What one asks with an expression is no answer to remember for a search of plain
 * text, whose answers are read as they stand: `\\1` in the middle of a text is a
 * backslash and a one to that command. The two lists are therefore two addresses,
 * and the assertion is the pair of lists after a run of one of the two commands.
 */
void TestQueryReplace::theTwoCommandsKeepTwoHistories() {
  const auto before = readHistories();
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-history.txt", "hist hist\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  startQueryReplace(frame, true);
  answer(e.ed, "hi[t]");
  answer(e.ed, "st");
  reply(frame, '!');
  const auto after = readHistories();
  QVERIFY2(after.regexp.size() >= before.regexp.size() + 2,
           qPrintable(QStringLiteral("the expression command kept %1 entries "
                                     "where it was asked to keep 2 more")
                          .arg(after.regexp.size() - before.regexp.size())));
  QCOMPARE(after.regexp.at(after.regexp.size() - 2), QString("hi[t]"));
  QCOMPARE(after.regexp.last(), QString("st"));
  QCOMPARE(after.plain, before.plain);
  // and the plain command keeps its own list, which the answers above are not in
  startQueryReplace(frame, false);
  answer(e.ed, "abc");
  answer(e.ed, "X");
  reply(frame, 'n');  // the walk ends at once: nothing to replace
  const auto ended = readHistories();
  QVERIFY(ended.plain.contains("abc"));
  QVERIFY(ended.plain.contains("X"));
  QVERIFY(not ended.regexp.contains("abc"));
}  // end of theTwoCommandsKeepTwoHistories()

/*!\brief a first answer of nothing, with nothing remembered, says so
 *
 * The empty answer of the first question is the reuse of the last pair of
 * answers, and with no pair to reuse there is nothing to do but say so and leave
 * the minibuffer back to whatever was waiting there. The two commands are asked
 * the same way and answer the same way, which is why both are asked here.
 */
void TestQueryReplace::anEmptyFirstAnswerWithNoHistorySaysSo() {
  forgetHistories();
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-empty.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  for (const auto regexp : {false, true}) {
    e.ed.messages.clear();
    startQueryReplace(frame, regexp);
    QVERIFY2(liveInput(e.ed) != nullptr,
             "the first question of the query replace was not asked");
    answer(e.ed, QString());
    QVERIFY2(e.ed.messages.contains("empty entry"),
             qPrintable(e.ed.messages.join(" | ")));
    QVERIFY2(not e.ed.hasUserInput(),
             "an empty first answer left a question waiting");
    QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                           << ""));
  }
}  // end of anEmptyFirstAnswerWithNoHistorySaysSo()

/*!brief a buffer which is frozen is refused both walks, and asked no question
 *
 * The two commands write, so `isReadOnly`, the gate every editing command of
 * `TextEditBase` asks, refuses them as it refuses the yank and the kill: the
 * freeze of a file modified on disk exists to keep a key from writing into a
 * buffer whose file holds something else, and a walk which asked its three
 * questions and then wrote over it would be the hole in that. What the two
 * buttons of the dialog which only search do instead is the business of the
 * dialog's own test.
 */
void TestQueryReplace::aBufferWhichIsFrozenIsRefusedBothCommands() {
  Editor e;
  e.ed.openFile(writeFile(this->dir, "qr-frozen.txt", "abc abc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  frame.setReadOnly(true);
  for (const auto regexp : {false, true}) {
    e.ed.messages.clear();
    startQueryReplace(frame, regexp);
    QVERIFY2(e.ed.messages.contains("read only buffer"),
             qPrintable(e.ed.messages.join(" | ")));
    QVERIFY2(not e.ed.hasUserInput(),
             "a frozen buffer was asked the first question of a walk");
    QCOMPARE(blocks(frame), (QStringList() << "abc abc"
                                           << ""));
  }
  frame.setReadOnly(false);
}  // end of aBufferWhichIsFrozenIsRefusedBothCommands()

QTEST_MAIN(TestQueryReplace)

#include "TestQueryReplace.moc"
