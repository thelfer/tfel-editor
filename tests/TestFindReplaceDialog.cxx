/*!
 * \file   tests/TestFindReplaceDialog.cxx
 * \brief  tests of the modeless Find/Replace dialog: that an editor holds one of
 *         them, what the two directions answer, and what the three options and
 *         the two ways of replacing do to the buffer.
 *
 * The dialog is reached through its public entry, `showFor`, and driven through
 * the buttons and the fields it shows, which are named so that this file and
 * whoever scripts the editor reach them the same way: `find-field`, `find-next`,
 * `replace-all`. Nothing here asks the buffer what it holds by hand alone: the
 * status line of the dialog is the answer of every question the dialog is asked,
 * so a case which did not read it asserted a button did something without
 * asserting what it said, which is half of what a reader of the dialog has.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>
#include <QtGui/QTextDocument>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandFactory.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FindReplaceDialog.hxx"
#include "TFEL/GUI/MainWindow.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/RectangleEditing.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::EditorWidget;
  using tfel::gui::FindReplaceDialog;
  using tfel::gui::TextEditBase;

  void pump(int n = 4) {
    for (int k = 0; k != n; ++k) {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
  }  // end of pump()

  /*!
   * \brief an editor which is shown, and which records the minibuffer
   *
   * \note the editor is shown because a buffer is only fully built when it is
   * visible. The recording is what proves the dialog does *not* report in the
   * minibuffer: the answers of its buttons belong to the dialog, which is modeless
   * and stands where the reader is already looking
   */
  struct ShownEditor : EditorWidget {
    ShownEditor() : EditorWidget(nullptr) {}
    QStringList messages;
    void displayInformativeMessage(const QString& m) override {
      this->messages << m;
      EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct ShownEditor

  struct Editor {
    ShownEditor ed;
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

  //! \return the lines of the document of the buffer on the screen
  QStringList blocks(TextEditBase& frame) {
    auto l = QStringList();
    auto* const d = frame.document();
    for (auto b = d->begin(); b != d->end(); b = b.next()) {
      l << b.text();
    }
    return l;
  }  // end of blocks()

  /*!
   * \brief the places the selection of the buffer holds, with the line it starts
   *        on
   * \return `line-start-end`
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

  //! \return the dialog of an editor, which is null until one was shown
  FindReplaceDialog* dialogOf(EditorWidget& ed) {
    return ed.findChild<FindReplaceDialog*>("find-replace");
  }  // end of dialogOf()

  //! \return the named part of a widget, which every part of the dialog is
  template <typename W>
  W* partOf(QObject& o, const char* name) {
    return o.findChild<W*>(QString::fromUtf8(name));
  }  // end of partOf()

  //! \brief writes a text in one of the two fields
  void setField(QObject& dialog, const char* name, const QString& text) {
    auto* const field = partOf<QLineEdit>(dialog, name);
    QVERIFY2(field != nullptr, name);
    field->setText(text);
  }  // end of setField()

  //! \brief ticks or unticks one of the three options
  void tick(QObject& dialog, const char* name, const bool on) {
    auto* const option = partOf<QCheckBox>(dialog, name);
    QVERIFY2(option != nullptr, name);
    option->setChecked(on);
  }  // end of tick()

  //! \brief presses a button of the dialog
  void pressButton(QObject& dialog, const char* name) {
    auto* const button = partOf<QAbstractButton>(dialog, name);
    QVERIFY2(button != nullptr, name);
    button->click();
    pump();
  }  // end of pressButton()

  /*!
   * \return what the status line of the dialog says
   *
   * \note this helper answers what it finds, since `QVERIFY` expands to a bare
   * `return` and is no use to a function which has an answer; the name of a
   * missing status line is what the caller then compares against, and the case
   * about the parts of the dialog is what names the line itself
   */
  QString statusOf(QObject& dialog) {
    auto* const status = partOf<QLabel>(dialog, "status");
    return (status != nullptr) ? status->text()
                               : QString("<no status line>");
  }  // end of statusOf()
  /*!
   * \brief triggers the entry of a menu which shows a given text
   *
   * \note the text of an entry carries the marker of its mnemonic, which is not
   * part of what the menu shows: `Cu&t Rectangle` reads `Cut Rectangle` to the
   * reader and to this file
   */
  void trigger(QMenu& menu, const QString& text) {
    for (auto* const a : menu.actions()) {
      if (a->text().remove(QLatin1Char('&')) == text) {
        a->activate(QAction::Trigger);
        return;
      }
    }
    QFAIL(qPrintable(QString("the menu holds no entry '%1'").arg(text)));
  }  // end of trigger()

  /*!
   * \brief the submenu of a menu whose entry shows a given text, or null
   *
   * \note an entry which leads a submenu carries that submenu as its own menu:
   * what a test walks in order to name the entries of a submenu is the answer of
   * this helper, not the parent, whose actions hold the submenu entry and none of
   * what stands inside it
   */
  QMenu* menuNamed(QMenu& menu, const QString& text) {
    for (auto* const a : menu.actions()) {
      if (a->text().remove(QLatin1Char('&')) == text) {
        return a->menu();
      }
    }
    return nullptr;
  }  // end of menuNamed()
}  // end of anonymous namespace

struct TestFindReplaceDialog : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void anEditorHoldsOneDialog();
  void theDialogStartsFromWhatIsOnTheScreen();
  void theTwoDirectionsWalkTheMatches();
  void aQuestionWhichFindsNothingSaysSo();
  void replacingWalksTheBufferOnePressAtATime();
  void replaceAllReachesTheWholeBuffer();
  void theOptionsDecideWhatIsReplaced();
  void theDialogFollowsTheBuffer();
  void aBufferWhichCanNotBeWrittenToIsRefusedBothButtonsWhichWrite();
  void theMenuAndTheCommandsReachTheDialog();
};

void TestFindReplaceDialog::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-find-replace");
}  // end of initTestCase()

/*!
 * \brief an editor holds one dialog, which is shown again rather than built again
 *
 * The dialog is modeless, so a second call which built a second one would leave a
 * first one on the screen asking questions of the same buffer. What a person
 * expects of the second call is the dialog they left, holding what they typed.
 */
void TestFindReplaceDialog::anEditorHoldsOneDialog() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "one.txt", "alpha\n"));
  pump();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const first = dialogOf(e.ed);
  QVERIFY2(first != nullptr, "showFor built no dialog");
  QVERIFY(first->isVisible());
  QCOMPARE(first->objectName(), QString("find-replace"));
  // the whole set of parts is there, each under the name a test or a script
  // reaches it by
  for (const auto name : {"find-field", "replace-field", "match-case",
                          "whole-words", "regular-expression", "find-next",
                          "find-previous", "replace", "replace-all", "close",
                          "status"}) {
    QVERIFY2(first->findChild<QWidget*>(QString::fromUtf8(name)) != nullptr,
             name);
  }
  // the letter case is the one option the dialog asks by default
  QVERIFY(partOf<QCheckBox>(*first, "match-case")->isChecked());
  QVERIFY(not partOf<QCheckBox>(*first, "whole-words")->isChecked());
  QVERIFY(not partOf<QCheckBox>(*first, "regular-expression")->isChecked());
  setField(*first, "find-field", "alpha");
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const second = dialogOf(e.ed);
  QVERIFY2(second == first,
           "the second call built another dialog instead of reusing the one "
           "which was there");
  QCOMPARE(partOf<QLineEdit>(*second, "find-field")->text(), QString("alpha"));
  // `Close` hides the dialog and leaves what it selected where it is
  pressButton(*first, "find-next");
  const auto kept = selectionWhere(e.ed.getCurrentBuffer().getMainFrame());
  pressButton(*first, "close");
  QVERIFY(not first->isVisible());
  QCOMPARE(selectionWhere(e.ed.getCurrentBuffer().getMainFrame()), kept);
  // and the hidden dialog is the one which comes back
  FindReplaceDialog::showFor(e.ed);
  pump();
  QVERIFY(dialogOf(e.ed) == first);
  QVERIFY(first->isVisible());
}  // end of anEditorHoldsOneDialog()

/*!
 * \brief the find field is filled with what the screen holds, and only with a
 *        text of one line
 *
 * A selection dragged over a paragraph is no text to search for; what the reader
 * who asks for the dialog after that drag wants is what the line the cursor
 * stands on holds, which such a selection does not name. And what is already
 * written in the field is never replaced by the selection: the dialog comes back
 * with what was typed in it.
 */
void TestFindReplaceDialog::theDialogStartsFromWhatIsOnTheScreen() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "two.txt", "first line\nsecond line\nthird\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  auto* const doc = frame.document();
  const auto second = doc->findBlockByNumber(1);
  auto tc = QTextCursor(doc);
  tc.setPosition(second.position());
  tc.setPosition(second.position() + 6, QTextCursor::KeepAnchor);
  frame.setTextCursor(tc);
  pump();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  auto* const field = partOf<QLineEdit>(*dlg, "find-field");
  QCOMPARE(field->text(), QString("second"));
  // a selection which reaches two lines prefill nothing: the text of such a
  // selection holds a line break, which no plain text of the find field is
  field->clear();
  auto wide = QTextCursor(doc);
  wide.setPosition(second.position() - 6);
  wide.setPosition(second.position() + 8, QTextCursor::KeepAnchor);
  frame.setTextCursor(wide);
  pump();
  FindReplaceDialog::showFor(e.ed);
  pump();
  QCOMPARE(field->text(), QString());
  // what was typed stays what was typed, selection or none
  field->setText("kept");
  auto narrow = QTextCursor(doc);
  narrow.setPosition(second.position());
  narrow.setPosition(second.position() + 3, QTextCursor::KeepAnchor);
  frame.setTextCursor(narrow);
  FindReplaceDialog::showFor(e.ed);
  pump();
  QCOMPARE(field->text(), QString("kept"));
}  // end of theDialogStartsFromWhatIsOnTheScreen()

/*!
 * \brief the two directions walk the matches of the buffer, and the match on the
 *        screen is passed over
 *
 * This is the pair of behaviour the whole dialog rests on and the reason the
 * search is asked from the end of the selection: a `Find next` which answered the
 * match the selection already holds is a button which does nothing.
 */
void TestFindReplaceDialog::theTwoDirectionsWalkTheMatches() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "three.txt", "beta one\nbeta two\nbeta three\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  // what opening the file said in the minibuffer is no answer of the dialog: the
  // recording is taken up again with the buffer and the dialog in place, so that
  // the assertion at the end of this case is about the presses alone
  e.ed.messages.clear();
  setField(*dlg, "find-field", "beta");
  // the three matches of the buffer, in the order its lines hold them
  pressButton(*dlg, "find-next");
  QCOMPARE(selectionWhere(frame), QString("0-0-4"));
  pressButton(*dlg, "find-next");
  QCOMPARE(selectionWhere(frame), QString("1-9-13"));
  pressButton(*dlg, "find-next");
  QCOMPARE(selectionWhere(frame), QString("2-18-22"));
  QVERIFY(statusOf(*dlg).isEmpty());
  // the fourth press wraps, says so, and lands on the first match again
  pressButton(*dlg, "find-next");
  QCOMPARE(selectionWhere(frame), QString("0-0-4"));
  QCOMPARE(statusOf(*dlg),
           QString("'beta' searched from the beginning of the buffer"));
  // and back down, which wraps the other way
  pressButton(*dlg, "find-previous");
  QCOMPARE(selectionWhere(frame), QString("2-18-22"));
  QCOMPARE(statusOf(*dlg), QString("'beta' searched from the end of the buffer"));
  pressButton(*dlg, "find-previous");
  QCOMPARE(selectionWhere(frame), QString("1-9-13"));
  // the answers of a modeless dialog never reach the minibuffer: the dialog holds
  // its own line, and a modeless window which reported in the space of the
  // commands would take an answer away from whatever the reader does next
  QVERIFY2(e.ed.messages.isEmpty(), qPrintable(e.ed.messages.join(" | ")));
}  // end of theTwoDirectionsWalkTheMatches()

//! \brief what a question which found nothing, or asked nothing, answers
void TestFindReplaceDialog::aQuestionWhichFindsNothingSaysSo() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "four.txt", "beta one\nbeta two\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  // a text which is not there
  setField(*dlg, "find-field", "zebra");
  const auto kept = selectionWhere(frame);
  pressButton(*dlg, "find-next");
  QCOMPARE(statusOf(*dlg), QString("'zebra' not found"));
  QCOMPARE(selectionWhere(frame), kept);
  // an empty field, which is no question at all
  setField(*dlg, "find-field", QString());
  pressButton(*dlg, "find-next");
  QCOMPARE(statusOf(*dlg), QString("empty search"));
  // an expression which does not compile: the report is what the expression
  // engine says of it, which is what tells the reader which parenthesis of
  // theirs is missing — and not that the text was not found, which would send
  // them looking at the buffer instead of at their pattern
  setField(*dlg, "find-field", "beta((\\d+)");
  tick(*dlg, "regular-expression", true);
  pressButton(*dlg, "find-next");
  QVERIFY2(not statusOf(*dlg).isEmpty(), "an invalid expression said nothing");
  QVERIFY2(not statusOf(*dlg).contains("not found"), qPrintable(statusOf(*dlg)));
  // the same characters as a plain text are a valid pattern, and are a text
  // which the buffer does not hold
  tick(*dlg, "regular-expression", false);
  pressButton(*dlg, "find-next");
  QCOMPARE(statusOf(*dlg), QString("'beta((\\d+)' not found"));
  // the same three answers of the two buttons which write
  setField(*dlg, "find-field", QString());
  pressButton(*dlg, "replace");
  QCOMPARE(statusOf(*dlg), QString("empty search"));
  pressButton(*dlg, "replace-all");
  QCOMPARE(statusOf(*dlg), QString("empty search"));
  setField(*dlg, "find-field", "beta((\\d+)");
  tick(*dlg, "regular-expression", true);
  pressButton(*dlg, "replace");
  QVERIFY2(not statusOf(*dlg).contains("not found"),
           qPrintable(statusOf(*dlg)));
  pressButton(*dlg, "replace-all");
  QVERIFY2(not statusOf(*dlg).contains("not found"),
           qPrintable(statusOf(*dlg)));
}  // end of aQuestionWhichFindsNothingSaysSo()

/*!
 * \brief `Replace` walks the buffer one press at a time
 *
 * A press replaces the match the screen holds and, when the screen holds none,
 * finds the next one. A buffer is therefore walked with the one button, two presses
 * per match, which is what makes it worth having beside `Replace all`: the reader
 * sees every match of the buffer before it is written over, and the text between
 * two matches never moves under the answer of an expression.
 */
void TestFindReplaceDialog::replacingWalksTheBufferOnePressAtATime() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "five.txt", "beta one\nbeta two\nbeta three\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  setField(*dlg, "find-field", "beta");
  setField(*dlg, "replace-field", "gamma");
  const auto everyMatch = QStringList() << "beta one"
                                        << "beta two"
                                        << "beta three"
                                        << "";
  // the first press finds: what the screen held was no match
  pressButton(*dlg, "replace");
  QCOMPARE(blocks(frame), everyMatch);
  QCOMPARE(selectionWhere(frame), QString("0-0-4"));
  // the second replaces that match and puts the next one on the screen, so one
  // press of the button is one match of the buffer
  pressButton(*dlg, "replace");
  QCOMPARE(blocks(frame), (QStringList() << "gamma one"
                                         << "beta two"
                                         << "beta three"
                                         << ""));
  QCOMPARE(statusOf(*dlg), QString("replaced 'beta' with 'gamma'"));
  QCOMPARE(selectionWhere(frame), QString("1-10-14"));
  pressButton(*dlg, "replace");
  QCOMPARE(blocks(frame), (QStringList() << "gamma one"
                                         << "gamma two"
                                         << "beta three"
                                         << ""));
  QCOMPARE(selectionWhere(frame), QString("2-20-24"));
  pressButton(*dlg, "replace");
  QCOMPARE(blocks(frame), (QStringList() << "gamma one"
                                         << "gamma two"
                                         << "gamma three"
                                         << ""));
  // and the buffer holds no more match: the press after the last one says so,
  // and the report of the replacement it follows is not mistaken for an answer
  pressButton(*dlg, "replace");
  QCOMPARE(statusOf(*dlg), QString("'beta' not found"));
  // the report of a replacement names the text as it was written, which for an
  // expression is the replacement with its groups filled in. The pattern is
  // anchored at the start of a line, which the dialog answers only because the
  // expression is asked of the buffer and not of a line of it.
  tick(*dlg, "regular-expression", true);
  setField(*dlg, "find-field", "^(\\w+) three");
  setField(*dlg, "replace-field", "<\\1> three");
  pressButton(*dlg, "replace");
  pressButton(*dlg, "replace");
  QCOMPARE(blocks(frame), (QStringList() << "gamma one"
                                         << "gamma two"
                                         << "<gamma> three"
                                         << ""));
  QCOMPARE(statusOf(*dlg),
           QString("replaced '^(\\w+) three' with '<gamma> three'"));
}  // end of replacingWalksTheBufferOnePressAtATime()

/*!
 * \brief `Replace all` reaches the whole buffer and says what it did
 *
 * The walk starts where the cursor stands, goes to the end of the buffer and
 * comes back to the cursor, which is what lets a replacement cover a match behind
 * the place the reader was reading at. The report is the evidence the walk was
 * bounded: the number of matches says itself, and the singular of one occurrence
 * is a string of its own.
 */
void TestFindReplaceDialog::replaceAllReachesTheWholeBuffer() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "six.txt", "beta\nbeta\nbeta\nbeta\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  setField(*dlg, "find-field", "beta");
  setField(*dlg, "replace-field", "x");
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "x" << "x" << "x" << "x" << ""));
  QCOMPARE(statusOf(*dlg), QString("replaced 4 occurrences of 'beta'"));
  // one occurrence is reported in the singular
  frame.setPlainText("beta alone\n");
  pump();
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "x alone" << ""));
  QCOMPARE(statusOf(*dlg), QString("replaced 1 occurrence of 'beta'"));
  // and nothing to replace says what is not there
  pressButton(*dlg, "replace-all");
  QCOMPARE(statusOf(*dlg), QString("'beta' not found"));
  // the walk reaches the lines above the cursor, which is the point of starting
  // it there and of ending it there
  frame.setPlainText("beta up\nbeta up two\nbeta down\n");
  pump();
  auto tc = QTextCursor(frame.document());
  tc.setPosition(frame.document()->findBlockByNumber(2).position());
  frame.setTextCursor(tc);
  pump();
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "x up"
                                         << "x up two"
                                         << "x down"
                                         << ""));
  QCOMPARE(statusOf(*dlg), QString("replaced 3 occurrences of 'beta'"));
  // an expression whose answer is the text it matched is a walk which ends
  tick(*dlg, "regular-expression", true);
  setField(*dlg, "find-field", "(b)(eta)");
  setField(*dlg, "replace-field", "\\1\\2");
  frame.setPlainText("beta beta\n");
  pump();
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "beta beta" << ""));
  QCOMPARE(statusOf(*dlg), QString("replaced 2 occurrences of '(b)(eta)'"));
  // a replacement longer than its match, which is the shape a walk of a fixed
  // length cut would go wrong on
  setField(*dlg, "find-field", "beta");
  setField(*dlg, "replace-field", "beta beta");
  frame.setPlainText("beta beta\n");
  pump();
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "beta beta beta beta" << ""));
  QCOMPARE(statusOf(*dlg), QString("replaced 2 occurrences of 'beta'"));
}  // end of replaceAllReachesTheWholeBuffer()

/*!
 * \brief the three options decide what a match is, on the buffer and not only on
 *        the fields
 *
 * The pattern is built again on every button, which is what makes a tick taken off
 * reach the next press with nothing else to do: the options are neither read when
 * the dialog was built nor remembered by a search.
 */
void TestFindReplaceDialog::theOptionsDecideWhatIsReplaced() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  e.ed.openFile(writeFile(d, "seven.txt", "beta betagam Beta\nfoo_bar foo\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  setField(*dlg, "find-field", "beta");
  setField(*dlg, "replace-field", "X");
  // the letter case, which the dialog asks by default: the capitalised word
  // stands for something else, and the substring inside `betagam` is the same
  // characters, which the whole words option is for
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "X Xgam Beta"
                                         << "foo_bar foo"
                                         << ""));
  // the letter case off: the three shapes of the word are one word
  frame.setPlainText("beta betagam Beta\nfoo_bar foo\n");
  pump();
  tick(*dlg, "match-case", false);
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame),
           (QStringList() << "X Xgam X"
                          << "foo_bar foo"
                          << ""));
  // the whole words, which leave the substring of a longer word alone
  frame.setPlainText("beta betagam beta\n");
  pump();
  tick(*dlg, "match-case", true);
  tick(*dlg, "whole-words", true);
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "X betagam X" << ""));
  // and off, which is what the same two presses answer then: the shape of the
  // answer with the option on is the one which could only come from the option
  tick(*dlg, "whole-words", false);
  frame.setPlainText("beta betagam beta\n");
  pump();
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "X Xgam X" << ""));
  // the expression, and the groups of its answer
  frame.setPlainText("foo(2) bar foo(33)\n");
  pump();
  tick(*dlg, "regular-expression", true);
  setField(*dlg, "find-field", "foo\\((\\d+)\\)");
  setField(*dlg, "replace-field", "[\\1]");
  pressButton(*dlg, "replace-all");
  QCOMPARE(blocks(frame), (QStringList() << "[2] bar [33]" << ""));
  // the same pattern without the option is the text, characters and all
  tick(*dlg, "regular-expression", false);
  pressButton(*dlg, "replace-all");
  QCOMPARE(statusOf(*dlg), QString("'foo\\((\\d+)\\)' not found"));
}  // end of theOptionsDecideWhatIsReplaced()

/*!
 * \brief the dialog asks the buffer worked on, so changing the buffer reaches the
 *        dialog which is already open
 *
 * A dialog which held on to the text edit it was first asked about would go on
 * searching a buffer the reader has left behind, which is what a `Ctrl-X b` of an
 * editor holding two files open is.
 */
void TestFindReplaceDialog::theDialogFollowsTheBuffer() {
  Editor e;
  QTemporaryDir d;
  QVERIFY(d.isValid());
  const auto one = writeFile(d, "one.side", "shared word\n");
  const auto two = writeFile(d, "two.side", "shared word\nshared word\n");
  e.ed.openFile(one);
  pump();
  e.ed.openFile(two);
  pump();
  // a buffer is named by the file it visits, which is the name `Ctrl-X b` answers
  // and the name `changeBuffer` takes: the absolute path is what the buffer holds,
  // the two are not interchangeable here
  auto names = e.ed.getBuffersNames();
  auto nameOf = [&names](const QString& file) {
    const auto base = QFileInfo(file).fileName();
    for (const auto& n : names) {
      if (n.contains(base)) {
        return n;
      }
    }
    return QString();
  };
  const auto firstName = nameOf(one);
  const auto secondName = nameOf(two);
  QVERIFY2((not firstName.isEmpty()) and (not secondName.isEmpty()),
           qPrintable(names.join(" | ")));
  QVERIFY2(firstName != secondName,
           "the two files of the same name in two directories are one buffer");
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  setField(*dlg, "find-field", "shared");
  // the buffer which is current holds the text twice, so the third press is the
  // first which has nothing ahead of it and wraps
  pressButton(*dlg, "find-next");
  pressButton(*dlg, "find-next");
  QVERIFY(statusOf(*dlg).isEmpty());
  pressButton(*dlg, "find-next");
  QCOMPARE(statusOf(*dlg),
           QString("'shared' searched from the beginning of the buffer"));
  // the other buffer, which holds the text once: the second press wraps there,
  // which is the dialog asking the buffer worked on and not the one the dialog
  // was first raised on
  e.ed.changeBuffer(firstName);
  pump();
  pressButton(*dlg, "find-next");
  pressButton(*dlg, "find-next");
  QCOMPARE(statusOf(*dlg),
           QString("'shared' searched from the beginning of the buffer"));
  pressButton(*dlg, "replace-all");
  // the replace field was never filled, which is a replacement of nothing: what
  // the match held is taken and the space behind it stays where it stands
  QCOMPARE(statusOf(*dlg), QString("replaced 1 occurrence of 'shared'"));
  QCOMPARE(blocks(e.ed.getCurrentBuffer().getMainFrame()),
           (QStringList() << " word" << ""));
  // the buffer left behind holds what it held
  e.ed.changeBuffer(secondName);
  pump();
  QCOMPARE(blocks(e.ed.getCurrentBuffer().getMainFrame()),
           (QStringList() << "shared word"
                          << "shared word"
                          << ""));
}  // end of theDialogFollowsTheBuffer()

/*!brief a buffer which is frozen is refused the two buttons which write, and
 *        answered the two which only ask
 *
 * The freeze of a file modified on disk — and any other read-only frame — keeps
 * a key from writing, and that is all it keeps: a `QTextCursor` of a program
 * writes through a read-only text edit (measured,
 * `/tmp/opencode/frd/rocheck.cxx`), so the two buttons which replace have to ask
 * the same question the editing commands of the buffer ask. The two which only
 * search are answered, which is what a reader of a frozen buffer is left to do
 * with it while the banner waits.
 *
 * \note the refusal is said by the line of the dialog and not in the minibuffer,
 * which is the contract of every answer of these buttons and what the recorded
 * messages are here to catch
 */
void TestFindReplaceDialog::
    aBufferWhichCanNotBeWrittenToIsRefusedBothButtonsWhichWrite() {
  QTemporaryDir d;
  QVERIFY(d.isValid());
  Editor e;
  e.ed.openFile(writeFile(d, "frozen.txt", "beta one\nbeta two\n"));
  pump();
  auto& frame = e.ed.getCurrentBuffer().getMainFrame();
  FindReplaceDialog::showFor(e.ed);
  pump();
  auto* const dlg = dialogOf(e.ed);
  QVERIFY(dlg != nullptr);
  setField(*dlg, "find-field", "beta");
  setField(*dlg, "replace-field", "X");
  frame.setReadOnly(true);
  e.ed.messages.clear();
  // the search of a frozen buffer is answered: it writes nothing
  pressButton(*dlg, "find-next");
  QCOMPARE(selectionWhere(frame), QString("0-0-4"));
  // and the two buttons which write name the reason they do not write
  pressButton(*dlg, "replace");
  QCOMPARE(statusOf(*dlg), QString("read only buffer"));
  pressButton(*dlg, "replace-all");
  QCOMPARE(statusOf(*dlg), QString("read only buffer"));
  QVERIFY2(e.ed.messages.isEmpty(),
           qPrintable(QString("the dialog answered in the minibuffer: '") +
                      e.ed.messages.join(" | ") + "'"));
  QCOMPARE(blocks(frame), (QStringList() << "beta one"
                                         << "beta two"
                                         << ""));
  frame.setReadOnly(false);
}  // end of aBufferWhichCanNotBeWrittenToIsRefusedBothButtonsWhichWrite()

/*!
 * \brief the Edit menu and the commands reach the dialog and the rectangle
 *        register of the window they stand in
 *
 * The menu entries are what a person who does not know a command name has, and
 * the commands are what the `Alt-X` question answers. Both must reach the one
 * dialog of the window, which is the object the keys reach, and the rectangle
 * entries must write the register the commands of `Ctrl-X R` read.
 *
 * \note `launchCommand` answers nothing: what an unknown command does is report
 * it, so the recording editor is what tells a command which ran from a name which
 * stands for nothing
 */
void TestFindReplaceDialog::theMenuAndTheCommandsReachTheDialog() {
  QTemporaryDir d;
  QVERIFY(d.isValid());
  auto window = tfel::gui::MainWindow(QStringList()
                                      << writeFile(d, "eight.txt",
                                                   "beta one\nbeta two\n"));
  window.resize(900, 700);
  window.show();
  pump(8);
  auto* const ed = window.findChild<EditorWidget*>();
  QVERIFY(ed != nullptr);
  auto& frame = ed->getCurrentBuffer().getMainFrame();
  // the Edit menu names the six entries this issue added
  auto* const bar = window.menuBar();
  QVERIFY(bar != nullptr);
  auto* const edit = bar->actions().at(1)->menu();
  QVERIFY2((edit != nullptr) and (edit->title() == "Edit"),
           qPrintable(bar->actions().at(1)->text()));
  auto named = QStringList();
  for (const auto* const a : edit->actions()) {
    // what the menu shows is the text without the marker of its mnemonic
    named << a->text().remove(QLatin1Char('&'));
  }
  for (const auto entry : {"Find/Replace..."}) {
    QVERIFY2(named.contains(QString::fromUtf8(entry)),
             qPrintable(named.join(" | ")));
  }
  // the four entries stand after the six the menu already held, and the three
  // rectangle commands stand in a submenu of their own behind a separator: they
  // work on columns and keep what they take in a register the kill ring knows
  // nothing of, which is a question different enough from the three clipboard
  // entries above to be asked in a place of its own rather than beside them
  QCOMPARE(named, (QStringList() << "Undo"
                                 << "Redo"
                                 << ""
                                 << "Cut"
                                 << "Copy"
                                 << "Paste"
                                 << "Select All"
                                 << "Find/Replace..."
                                 << ""
                                 << "Rectangle Edit"));
  auto* const rect = menuNamed(*edit, "Rectangle Edit");
  QVERIFY2(rect != nullptr, "the Edit menu holds no Rectangle Edit submenu");
  auto rect_entries = QStringList();
  for (const auto* const a : rect->actions()) {
    rect_entries << a->text().remove(QLatin1Char('&'));
  }
  QCOMPARE(rect_entries, (QStringList() << "Cut Rectangle"
                                        << "Copy Rectangle"
                                        << "Paste Rectangle"));
  // the entry builds the dialog of the window it stands in
  QCOMPARE(dialogOf(*ed), nullptr);
  trigger(*edit, "Find/Replace...");
  pump();
  auto* const dlg = dialogOf(*ed);
  QVERIFY2(dlg != nullptr, "the Edit menu entry built no dialog");
  // the command reaches the same one
  ed->launchCommand("find-replace");
  pump();
  QVERIFY2(dialogOf(*ed) == dlg, "the `find-replace` command is not the menu");
  // and the dialog is still empty of a pattern: the entry which asks for the
  // dialog is the only way to it, and a selection of the whole first line is no
  // prefill of its own since nothing was selected here
  QCOMPARE(partOf<QLineEdit>(*dlg, "find-field")->text(), QString());
  // the rectangle entries work on the columns of the selection: what is cut is
  // the four characters of the first line the selection spans, and the line
  // below, which the selection does not reach, is left alone
  auto tc = QTextCursor(frame.document());
  tc.setPosition(0);
  tc.setPosition(4, QTextCursor::KeepAnchor);
  frame.setTextCursor(tc);
  pump();
  trigger(*rect, "Cut Rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << " one"
                                         << "beta two"
                                         << ""));
  QVERIFY2(tfel::gui::RectangleEditing::hasRectangle(),
           "the menu entry which cuts a rectangle kept nothing");
  trigger(*rect, "Paste Rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "beta one"
                                         << "beta two"
                                         << ""));
  // `Copy Rectangle` is the same question with the buffer left as it was. The
  // selection is made again first: the paste which follows is drawn from the
  // column the cursor stands on, which the copy does not move and which is
  // therefore the far end of the columns that were copied
  tc.setPosition(0);
  tc.setPosition(4, QTextCursor::KeepAnchor);
  frame.setTextCursor(tc);
  pump();
  trigger(*rect, "Copy Rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "beta one"
                                         << "beta two"
                                         << ""));
  trigger(*rect, "Paste Rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "betabeta one"
                                         << "beta two"
                                         << ""));
  // the commands of the same three names, which is what the `Alt-X` question
  // answers: undoing the paste above with the three of them in turn brings the
  // buffer back to what it was, which is the evidence that each command ran
  frame.document()->undo();
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "beta one"
                                         << "beta two"
                                         << ""));
  // the three commands of the `Alt-X` question are the three entries of the menu.
  // The selection is put back and the commands asked by name: cutting the columns
  // takes them from the buffer, pasting them puts them back, and the answers are
  // the ones the same entries made through the menu, which is the evidence that
  // each name reached a command and not a report about a missing one
  tc.setPosition(0);
  tc.setPosition(4, QTextCursor::KeepAnchor);
  frame.setTextCursor(tc);
  pump();
  ed->launchCommand("cut-rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << " one"
                                         << "beta two"
                                         << ""));
  ed->launchCommand("paste-rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "beta one"
                                         << "beta two"
                                         << ""));
  ed->launchCommand("copy-rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << "beta one"
                                         << "beta two"
                                         << ""));
  QVERIFY(tfel::gui::RectangleEditing::hasRectangle());
  // `kill-rectangle`, the name GNU Emacs gives to the first of the three, is
  // registered as an alias of it: a second registration of the same function,
  // which the `Alt-X` question lists and answers as the name it was registered
  // under. The columns are taken again, which is what says the alias reached the
  // command rather than a report about a name which stands for nothing
  tc.setPosition(0);
  tc.setPosition(4, QTextCursor::KeepAnchor);
  frame.setTextCursor(tc);
  pump();
  QVERIFY(tfel::gui::CommandFactory::getCommandFactory()
              .getAvailableCommandsNames()
              .contains("kill-rectangle"));
  ed->launchCommand("kill-rectangle");
  pump();
  QCOMPARE(blocks(frame), (QStringList() << " one"
                                         << "beta two"
                                         << ""));
}  // end of theMenuAndTheCommandsReachTheDialog()

QTEST_MAIN(TestFindReplaceDialog)

#include "TestFindReplaceDialog.moc"
