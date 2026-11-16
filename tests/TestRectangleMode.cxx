/*!
 * \file   tests/TestRectangleMode.cxx
 * \brief  tests of the rectangle commands of `C-x r`: which lines of the buffer
 *         a rectangle covers, what the three commands do to them, and where they
 *         leave the cursor.
 *
 * Every expectation of this file is the answer GNU Emacs 27.1 gives to the same
 * shape, measured with its own `rect.el` (`string-rectangle`, `kill-rectangle`,
 * `yank-rectangle`) rather than read off a description: the commands are old and
 * their behaviour on the short lines of a buffer, on a rectangle of no width and
 * on the empty last line a file ends with is not something a prose rule pins
 * down. Two differences with Emacs are deliberate and are named where they show:
 * a tab counts as one character of column here, where Emacs moves to a tab stop,
 * and the editor has no `C-x SPC` (`rectangle-mark-mode`), so the rectangle is
 * always what a linear selection spans.
 * \author Thomas Helfer
 * \date   16/11/2026
 */

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/RectangleMode.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  void pump(int n = 4) {
    for (int k = 0; k != n; ++k) {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
  }  // end of pump()

  //! an editor which records what it displays in the minibuffer
  struct ReportingEditor : tfel::gui::EditorWidget {
    ReportingEditor() : EditorWidget(nullptr) {}
    QStringList messages;
    void displayInformativeMessage(const QString& m) override {
      this->messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct ReportingEditor

  /*!
   * \brief an editor of its own, shown
   *
   * \note the editor is shown because a buffer is only fully built when it is
   * visible, which is what the modes themselves rely on
   */
  struct Editor {
    ReportingEditor ed;
    Editor() {
      ed.setMinimumSize(600, 400);
      ed.resize(800, 600);
      ed.show();
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
  tfel::gui::CommandLine* liveInput(tfel::gui::EditorWidget& ed) {
    for (auto* const input : ed.findChildren<tfel::gui::CommandLine*>()) {
      if (isLaidOut(*input)) {
        return input;
      }
    }
    return nullptr;
  }  // end of liveInput()

  //! \return the entry of the given question
  tfel::gui::LineEdit* entryOf(tfel::gui::CommandLine& cl) {
    for (auto* const line : cl.findChildren<tfel::gui::LineEdit*>()) {
      if (isLaidOut(*line)) {
        return line;
      }
    }
    return nullptr;
  }  // end of entryOf()

  void press(QWidget* const w, const int k, const Qt::KeyboardModifiers m) {
    if (w != nullptr) {
      QTest::keyClick(w, static_cast<Qt::Key>(k), m);
    }
  }  // end of press()

  //! \return the lines of the document of the buffer on the screen
  QStringList blocks(tfel::gui::TextEditBase& frame) {
    auto l = QStringList();
    auto* const d = frame.document();
    for (auto b = d->begin(); b != d->end(); b = b.next()) {
      l << b.text();
    }
    return l;
  }  // end of blocks()

  /*!
   * \brief the line and the column the cursor of the buffer stands on
   *
   * \note the line comes from the document, not from `QTextCursor::blockNumber`,
   * which asks the layout of the document: that one is only brought up to date
   * when the text is drawn, so after a command which rewrote lines programmatically
   * it can still name the line the cursor stood on the last time the buffer was
   * painted
   */
  QString cursorWhere(tfel::gui::TextEditBase& frame) {
    const auto tc = frame.textCursor();
    const auto b = frame.document()->findBlock(tc.position());
    return QString("%1 %2").arg(b.blockNumber()).arg(tc.positionInBlock());
  }  // end of cursorWhere()

  /*!
   * \brief places the selection between the two corners of a rectangle
   *
   * The editor has no `C-x SPC`, so a rectangle is what a linear selection
   * between its top-left and bottom-right corners spans, and that is what the
   * commands of `C-x r` read.
   */
  void select(tfel::gui::TextEditBase& frame,
              const int n,
              const int left,
              const int n1,
              const int right) {
    auto* const d = frame.document();
    auto tc = QTextCursor(d);
    const auto positionOf = [d](const int line, const int column) {
      const auto b = d->findBlockByNumber(line);
      return b.position() + qMin(column, b.length() - 1);
    };
    tc.setPosition(positionOf(n, left));
    tc.setPosition(positionOf(n1, right), QTextCursor::KeepAnchor);
    frame.setTextCursor(tc);
    pump();
  }  // end of select()

  /*!
   * \brief presses `C-x r t`, `C-x r k` or `C-x r y`
   *
   * `C-x r` is itself a question of the minibuffer, which waits for the single
   * key of the command, so the key is typed into its entry and not into the
   * buffer. `t` uncovers a second question, the string to insert.
   *
   * \note the string is set on the entry rather than typed: a newline of a
   * multi-line answer is the `Return` of the minibuffer, which is what a paste
   * of several lines into the entry stands for, and `QTest` has no key for that
   * character anyway
   */
  void pressRectangle(ReportingEditor& ed,
                      tfel::gui::TextEditBase& frame,
                      const char command,
                      const QString& answer = QStringLiteral("XY")) {
    press(frame.widget(), Qt::Key_X, Qt::ControlModifier);
    press(frame.widget(), Qt::Key_R, Qt::NoModifier);
    pump();
    auto* const mode = liveInput(ed);
    QVERIFY2(mode != nullptr, "`C-x r` opened no question in the minibuffer");
    press(entryOf(*mode), command == 't'   ? Qt::Key_T
                          : (command == 'k') ? Qt::Key_K
                                             : Qt::Key_Y,
          Qt::NoModifier);
    pump();
    if (command == 't') {
      auto* const question = liveInput(ed);
      QVERIFY2(question != nullptr,
               "`C-x r t` asked for no string in the minibuffer");
      entryOf(*question)->setText(answer);
      question->userEditingFinished();
      pump();
    }
  }  // end of pressRectangle()

  /*!
   * \brief places the cursor of the buffer on a line and a column, selection cleared
   *
   * The column is clamped to the end of the line, as the reference harness of
   * Emacs does with \c move-to-column, and a line which the document does not
   * have is the end of its last line: the two rules of the harness which decides
   * where a paste stands, so that a case names a place and gets that place on
   * both sides of the comparison.
   */
  void gotoWhere(tfel::gui::TextEditBase& frame,
                 const int line,
                 const int column) {
    auto* const d = frame.document();
    auto tc = QTextCursor(d);
    const auto b = d->findBlockByNumber(line);
    if (b.isValid()) {
      tc.setPosition(b.position() + qMin(column, b.length() - 1));
    } else {
      const auto last = d->lastBlock();
      tc.setPosition(last.position() + last.length() - 1);
    }
    frame.setTextCursor(tc);
    pump();
  }  // end of gotoWhere()

}  // end of anonymous namespace

//! tests of the rectangle commands of `C-x r t`, `C-x r k` and `C-x r y`
struct TestRectangleMode : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();

  // ---- what a paste with nothing to paste is ------------------------
  // the killed rectangle is a static of the mode which nothing clears, so this
  // case has to run before any rectangle of the run has been killed
  void pastingWithNoRectangleKilledSaysSo();

  // ---- insert rectangle ---------------------------------------------
  void theRectangleReachesTheLineTheEndOfTheSelectionStandsOn();
  void theInsertedTextStartsAtTheLeftEdgeOfEveryLine();
  void aLineShorterThanTheRectangleIsPaddedWithBlanks();
  void anUpwardSelectionInsertsTheSameRectangle();
  void aRectangleOfNoWidthOnlyInserts();
  void aRectangleOverTheWholeBufferLeavesTheEmptyLastLineAlone();
  void aBufferWithoutAFinalNewlineHasNoEmptyLastLine();
  void anAnswerOfSeveralLinesGoesOnEveryLineOfTheRectangle();
  void theCursorEndsBehindTheTextInsertedOnTheLastLine();

  // ---- kill rectangle -----------------------------------------------
  void killingTheWholeBufferLeavesTheLinesEmpty();
  void aKilledLineShorterThanTheRectangleIsStoredPadded();
  void thePointOfAKillStaysOnTheLineItWasOn();
  void aRectangleOfNoWidthKillsNothing();
  void aKillLeavesALineItCanNotReachAlone();
  void aRectangleKillDoesNotTouchTheKillRing();

  // ---- yank rectangle -----------------------------------------------
  void aYankPutsTheRectangleBackWhereItWasKilled();
  void aYankUsesTheColumnOfTheCursorAndPadsTheLinesToIt();
  void aYankOnTheEmptyLastLineOpensOnlyTheLinesItNeeds();
  void aYankBelowTheLastLineOfAFileWithoutAFinalNewlineOpensLines();
  void aRectangleOfNoWidthYankedOnTheEmptyLastLineOpensNothing();
  void aRectangleKilledInOneBufferIsPastedInAnother();

 private:
  QTemporaryDir dir;
};

void TestRectangleMode::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-rectangle-mode");
  QVERIFY2(dir.isValid(), "no temporary directory to write the files in");
}  // end of TestRectangleMode::initTestCase

void TestRectangleMode::pastingWithNoRectangleKilledSaysSo() {
  // the shape of the report of a command which has nothing to work with.
  //
  // This case must stay the first of the list: what `C-x r y` pastes is a static
  // of the mode, shared by every buffer and never cleared, so the run has to
  // reach it before any other case has killed a rectangle.
  Editor e;
  e.ed.openFile(writeFile(dir, "yank-nothing.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  pressRectangle(e.ed, frame, 'y');
  QVERIFY2(e.ed.messages.contains(QStringLiteral("no rectangle killed")),
           qPrintable(QStringLiteral("the paste of nothing reported: %1")
                          .arg(e.ed.messages.join(QStringLiteral(" | ")))));
  QCOMPARE(blocks(frame), (QStringList() << "one" << "two" << "three" << ""));
}  // end of pastingWithNoRectangleKilledSaysSo()

void TestRectangleMode::theRectangleReachesTheLineTheEndOfTheSelectionStandsOn() {
  // the report this file was written for: the selection of a whole file ends on
  // the empty line its final newline opens, and a rectangle which stopped one
  // line above that line reads as a rectangle which does not reach where the
  // cursor is. Emacs writes the four lines, and the empty last line of the
  // buffer takes the string on its own place instead of staying empty, which is
  // where the defect shows.
  Editor e;
  e.ed.openFile(writeFile(dir, "reaches.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 0, 3, 5);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame), (QStringList()
                           << "XYone" << "XYtwo" << "XYthree" << "XY"));
  QCOMPARE(cursorWhere(frame), QStringLiteral("3 2"));
}  // end of theRectangleReachesTheLineTheEndOfTheSelectionStandsOn()

void TestRectangleMode::theInsertedTextStartsAtTheLeftEdgeOfEveryLine() {
  // the rectangle of lines 2 and 3 from column 0 to column 2: each of them keeps
  // what it holds after the right edge, which is the `o` of `two` and the `ree`
  // of `three`, and the line above the rectangle is left alone
  Editor e;
  e.ed.openFile(writeFile(dir, "edges.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 0, 2, 2);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "XYo" << "XYree" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
}  // end of theInsertedTextStartsAtTheLeftEdgeOfEveryLine()

void TestRectangleMode::aLineShorterThanTheRectangleIsPaddedWithBlanks() {
  // a rectangle from column 2 to column 10 of five lines, one of which is empty
  // and two of which stop before the left edge of the rectangle: the empty line
  // becomes blanks up to that edge plus the string, and a line which stops short
  // of the right edge keeps nothing of itself
  Editor e;
  e.ed.openFile(writeFile(dir, "padded.txt",
                          "abcdefghi\nab\ncdefgh\n\nghi\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 2, 4, 10);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame), (QStringList() << "abXYdefghi" << "abXY" << "cdXYfgh"
                                         << "  XY" << "ghXY" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("4 4"));
}  // end of aLineShorterThanTheRectangleIsPaddedWithBlanks()

void TestRectangleMode::anUpwardSelectionInsertsTheSameRectangle() {
  // the same rectangle as the case above, dragged the other way: what the
  // commands work on are the two corners, not which end of the selection the
  // cursor holds
  Editor e;
  e.ed.openFile(writeFile(dir, "upward.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 2, 2, 1, 0);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "XYo" << "XYree" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
}  // end of anUpwardSelectionInsertsTheSameRectangle()

void TestRectangleMode::aRectangleOfNoWidthOnlyInserts() {
  // both ends on column 3: nothing of the lines is lost and the string goes in at
  // that column, which is what makes the empty rectangle a way of putting the
  // same text on a column of several lines
  Editor e;
  e.ed.openFile(writeFile(dir, "no-width.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 3, 2, 3);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame),
           (QStringList() << "one" << "twoXY" << "thrXYee" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 5"));
}  // end of aRectangleOfNoWidthOnlyInserts()

void TestRectangleMode::aRectangleOverTheWholeBufferLeavesTheEmptyLastLineAlone() {
  // the columns of the selection are what the rectangle covers: the end of a
  // selection can not stand past the end of a line, so a selection dragged to the
  // end of the buffer names the columns of the last *text* line and covers no
  // column at all of the empty line the final newline opens. Every line of text
  // is therefore replaced by the string, and the empty last line stays empty.
  Editor e;
  e.ed.openFile(writeFile(dir, "whole.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 0, 2, 9);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame), (QStringList() << "XY" << "XY" << "XY" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
}  // end of aRectangleOverTheWholeBufferLeavesTheEmptyLastLineAlone()

void TestRectangleMode::aBufferWithoutAFinalNewlineHasNoEmptyLastLine() {
  // the same shape on a file which does not end with a newline: there is no empty
  // last line to leave alone, and its last line has no text behind the rectangle
  Editor e;
  e.ed.openFile(writeFile(dir, "no-final.txt", "one\ntwo\nthree"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 0, 2, 9);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame), (QStringList() << "XY" << "XY" << "XY"));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
}  // end of aBufferWithoutAFinalNewlineHasNoEmptyLastLine()

void TestRectangleMode::anAnswerOfSeveralLinesGoesOnEveryLineOfTheRectangle() {
  // an answer of two lines written on the three lines of a buffer opens six lines
  // where they were three: Emacs writes the whole answer on every line of the
  // rectangle, and the last line of the rectangle keeps what it held after its
  // right edge behind the last line of the answer, which is the `ee` of `three`
  // standing on the line the cursor ends on
  Editor e;
  e.ed.openFile(writeFile(dir, "multi.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 0, 2, 3);
  pressRectangle(e.ed, frame, 't', QStringLiteral("11\n22"));
  QCOMPARE(blocks(frame), (QStringList() << "11" << "22" << "11" << "22"
                                         << "11" << "22ee" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("5 2"));
}  // end of anAnswerOfSeveralLinesGoesOnEveryLineOfTheRectangle()

void TestRectangleMode::theCursorEndsBehindTheTextInsertedOnTheLastLine() {
  // a rectangle of a single line: the cursor goes behind the string written on
  // that line, wherever the ends of the selection stood
  Editor e;
  e.ed.openFile(writeFile(dir, "one-line.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 1, 0, 3);
  pressRectangle(e.ed, frame, 't');
  QCOMPARE(blocks(frame),
           (QStringList() << "oXY" << "two" << "three" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("0 3"));
}  // end of theCursorEndsBehindTheTextInsertedOnTheLastLine()

void TestRectangleMode::killingTheWholeBufferLeavesTheLinesEmpty() {
  // what a rectangle covers are the columns of the selection, not the lines: each
  // line loses what it holds between the edges and stays a line, and the cursor
  // goes back by the width of the text taken in front of it
  Editor e;
  e.ed.openFile(writeFile(dir, "kill-whole.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 0, 2, 9);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "" << "" << "" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 0"));
}  // end of killingTheWholeBufferLeavesTheLinesEmpty()

void TestRectangleMode::aKilledLineShorterThanTheRectangleIsStoredPadded() {
  // a rectangle from column 2 to column 9, over lines of 8, 2 and 8 characters:
  // the pieces of the killed rectangle are all seven columns wide, so the line
  // which held nothing there gives a piece of blanks and a paste at the left edge
  // of the rectangle puts each line back where it was, the short one behind the
  // blanks the rectangle was made of on that line
  Editor e;
  e.ed.openFile(writeFile(dir, "kill-padded.txt", "abcdefgh\nab\ncdefgh\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 2, 2, 9);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "abgh" << "ab" << "cd" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
  gotoWhere(frame, 0, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "cdefabgh" << "    ab"
                                         << "efghcd" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 4"));
}  // end of aKilledLineShorterThanTheRectangleIsStoredPadded()

void TestRectangleMode::thePointOfAKillStaysOnTheLineItWasOn() {
  // an upward selection: the kill takes the same rectangle, and its cursor does
  // not go to the other corner of it — it keeps the line the selection left it on
  // and goes back by the two characters the kill took in front of it
  Editor e;
  e.ed.openFile(writeFile(dir, "kill-upward.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 2, 2, 1, 0);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "o" << "ree" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("1 0"));
}  // end of thePointOfAKillStaysOnTheLineItWasOn()

void TestRectangleMode::aRectangleOfNoWidthKillsNothing() {
  // both ends on column 2 of two lines: the rectangle covers no column, so the
  // buffer is left as it was and the cursor, which stands at the edge of the
  // rectangle already, does not move. What is stored is a piece of no width per
  // line, so the paste which follows writes nothing on the two lines it goes on
  // and leaves the cursor at the end of the last of them.
  Editor e;
  e.ed.openFile(writeFile(dir, "kill-no-width.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 2, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "two" << "three" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
  gotoWhere(frame, 0, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "two" << "three" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("1 0"));
}  // end of aRectangleOfNoWidthKillsNothing()

void TestRectangleMode::aKillLeavesALineItCanNotReachAlone() {
  // a rectangle whose left edge stands at column 5, over a buffer with a line of
  // two characters and an empty line: a line which holds nothing at that column
  // loses nothing, so only the lines which did reach into the rectangle are
  // touched, and an untouched line is not even made modified
  Editor e;
  e.ed.openFile(writeFile(dir, "kill-short.txt",
                          "abcdefghi\nab\ncdefgh\n\nghi\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 0, 5, 2, 7);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "abcdeghi" << "ab" << "cdefg" << ""
                                         << "ghi" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 5"));
}  // end of aKillLeavesALineItCanNotReachAlone()

void TestRectangleMode::aRectangleKillDoesNotTouchTheKillRing() {
  // a killed rectangle is *not* a killed text: Emacs keeps it in the register
  // `yank-rectangle` reads (`killed-rectangle` of `rect.el`) and never pushes it
  // on the kill ring, so `Ctrl-y` after `C-x r k` must not bring the rectangle
  // back and `C-x r y` is the only way of pasting it. What the kill leaves on the
  // kill ring is what a selection being replaced by a typed key puts there, which
  // here is nothing at all.
  Editor e;
  e.ed.openFile(writeFile(dir, "kill-ring.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  QVERIFY2(e.ed.getKillRing().isEmpty(),
           "the buffer of a fresh file left something on the kill ring");
  select(frame, 1, 0, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "o" << "ree" << ""));
  QVERIFY2(e.ed.getKillRing().isEmpty(),
           qPrintable(QStringLiteral("the rectangle kill pushed %1 entry/-entries"
                                      " on the kill ring: %2")
                          .arg(e.ed.getKillRing().size())
                          .arg(e.ed.getKillRing().join(QStringLiteral(" | ")))));
}  // end of aRectangleKillDoesNotTouchTheKillRing()

void TestRectangleMode::aYankPutsTheRectangleBackWhereItWasKilled() {
  // the round trip: a paste at the left edge of the first line of the rectangle
  // gives the buffer back as it was, which is what a killed piece padded to the
  // width of the rectangle is for, and the cursor ends behind the last piece, on
  // the last line the rectangle covered
  Editor e;
  e.ed.openFile(writeFile(dir, "yank-back.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 0, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "o" << "ree" << ""));
  gotoWhere(frame, 1, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "two" << "three" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("2 2"));
}  // end of aYankPutsTheRectangleBackWhereItWasKilled()

void TestRectangleMode::aYankUsesTheColumnOfTheCursorAndPadsTheLinesToIt() {
  // the paste asks nothing of the selection and overwrites nothing: it inserts at
  // the column the cursor stands on and keeps what the lines hold from there on,
  // and a line which stops short of that column is padded with blanks up to it,
  // so the piece of the rectangle stands on the column and not behind the blanks
  // of a shorter line. The cursor ends behind the last piece written.
  Editor e;
  e.ed.openFile(writeFile(dir, "yank-column.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 0, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "o" << "ree" << ""));
  gotoWhere(frame, 0, 2);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "ontwe" << "o th" << "ree" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("1 4"));
}  // end of aYankUsesTheColumnOfTheCursorAndPadsTheLinesToIt()

void TestRectangleMode::aYankOnTheEmptyLastLineOpensOnlyTheLinesItNeeds() {
  // the two pieces of a rectangle pasted on the empty line the final newline of a
  // file opens: that line is already the start of a line, so the first piece goes
  // on its own place and only the second one needs a line, which is opened below
  // it. The buffer ends without a newline, as Emacs leaves it.
  Editor e;
  e.ed.openFile(writeFile(dir, "yank-empty.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 0, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "o" << "ree" << ""));
  gotoWhere(frame, 3, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame),
           (QStringList() << "one" << "o" << "ree" << "tw" << "th"));
  QCOMPARE(cursorWhere(frame), QStringLiteral("4 2"));
}  // end of aYankOnTheEmptyLastLineOpensOnlyTheLinesItNeeds()

void TestRectangleMode::
    aYankBelowTheLastLineOfAFileWithoutAFinalNewlineOpensLines() {
  // the same paste on a file which has no final newline: there is no empty line to
  // take the first piece, so it goes at the end of the last line and the line
  // below is opened for the second one, padded with blanks up to the column the
  // paste works on
  Editor e;
  e.ed.openFile(writeFile(dir, "yank-below.txt", "one\ntwo\nthree"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 0, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "o" << "ree"));
  gotoWhere(frame, 3, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame),
           (QStringList() << "one" << "o" << "reetw" << "   th"));
  QCOMPARE(cursorWhere(frame), QStringLiteral("3 5"));
}  // end of aYankBelowTheLastLineOfAFileWithoutAFinalNewlineOpensLines()

void TestRectangleMode::
    aRectangleOfNoWidthYankedOnTheEmptyLastLineOpensNothing() {
  // the paste of a rectangle of no width where the buffer has one line left to
  // show: the pieces are empty, the line the cursor stands on takes them as it
  // stands, and no line is opened. This is the shape of a command which reaches
  // the end of a buffer and must leave it exactly as it was.
  Editor e;
  e.ed.openFile(writeFile(dir, "yank-no-width.txt", "one\ntwo\nthree\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  select(frame, 1, 2, 2, 2);
  pressRectangle(e.ed, frame, 'k');
  gotoWhere(frame, 3, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame), (QStringList() << "one" << "two" << "three" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("3 0"));
}  // end of aRectangleOfNoWidthYankedOnTheEmptyLastLineOpensNothing()

void TestRectangleMode::aRectangleKilledInOneBufferIsPastedInAnother() {
  // what `C-x r y` pastes is one register for the whole editor, not a thing of the
  // buffer it was killed in: the rectangle of two lines of a buffer, pasted at
  // column 0 of another one, goes on its two lines and inserts there as it does at
  // home, which is the rule the case above measures.
  Editor e;
  e.ed.openFile(writeFile(dir, "register-home.txt", "one\ntwo\nthree\n"));
  pump();
  auto& home = e.ed.getCurrentBuffer().getMainFrame();
  select(home, 1, 0, 2, 2);
  pressRectangle(e.ed, home, 'k');
  e.ed.openFile(writeFile(dir, "register-here.txt", "aaa\nbbb\nccc\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  gotoWhere(frame, 0, 0);
  pressRectangle(e.ed, frame, 'y');
  QCOMPARE(blocks(frame),
           (QStringList() << "twaaa" << "thbbb" << "ccc" << ""));
  QCOMPARE(cursorWhere(frame), QStringLiteral("1 2"));
}  // end of aRectangleKilledInOneBufferIsPastedInAnother()

QTEST_MAIN(TestRectangleMode)

#include "TestRectangleMode.moc"
