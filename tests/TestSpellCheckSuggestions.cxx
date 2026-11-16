/*!
 * \file   TestSpellCheckSuggestions.cxx
 * \brief  tests for the suggestions the context menu of a buffer offers for the
 *         misspelled words the spell checker underlines in it: the entry which
 *         leads to them, the suggestions it names, and the replacement it makes.
 * \author Thomas Helfer
 * \date   16 nov. 2026
 */

#include <QtTest/QtTest>
#include <QtCore/QLibrary>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <memory>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/SpellChecker.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

/*
 * The suggestions of the spell checker are the one public face of hunspell a test
 * can see on a packaged mode: the underlines of the misspelled words are written
 * by the highlighter into the document's private extra selections, which neither
 * `QTextCursor::charFormat` nor `TextEditBase::extraSelections` reports (see
 * `AGENTS.md`).
 *
 * The menu is therefore asked of the mode directly, through the public
 * `MajorMode::completeContextMenu`, and its entries are triggered with
 * `QAction::trigger()`. A real right click can not be driven from a test:
 * `TextEditBase::handleMousePressEvent` ends with `QMenu::exec`, a nested event
 * loop no offscreen test returns from.
 *
 * What the entries of the menu are is compared to what a `SpellChecker` of the
 * test's own answers for the same word on the same dictionary: the assertions are
 * then right whatever the dictionary of the machine happens to say, and the cases
 * skip when it says nothing at all.
 */
namespace {

  //! the word the cases ask the dictionary about: no language spells it that way
  auto needle() {
    return QStringLiteral("mistaked");
  }  // end of needle()

  //! a second misspelled word, to tell which of two words of a line a place names
  auto otherNeedle() {
    return QStringLiteral("wrod");
  }  // end of otherNeedle()

  //! the dictionary the cases are run against, by name
  auto dictionary() {
    return QStringLiteral("en_US");
  }  // end of dictionary()

}  // end of anonymous namespace

struct TestSpellCheckSuggestions : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  /*!
   * \brief the shape of the menu: an entry named `Suggestions`, in front of the
   * entries of the text edit with a separator behind it, leading to a submenu of
   * one entry per suggestion, in the order the dictionary answers them
   */
  void theMenuOfAMisspelledWordHoldsASubmenuOfItsSuggestions();
  /*!
   * \brief choosing a suggestion replaces the word by it and leaves the rest of
   * the line where it was
   */
  void choosingASuggestionReplacesTheWordInTheBuffer();
  /*!
   * \brief two misspelled words on one line: the place the menu was raised at
   * decides which of the two the entry is about
   */
  void theMenuNamesTheWordThePositionStandsIn();
  /*!
   * \brief a place of a document belongs to the word on either side of it, and
   * the place between two words names neither of them
   */
  void aPlaceAtEitherEndOfTheWordNamesItAndAPlaceBetweenTwoWordsNamesNeither();
  //! a word the dictionary knows is offered nothing
  void aWellSpelledWordIsOfferedNothing();
  //! a misspelled word of a comment is none of the menu's business
  void aWordOfACommentOfANoteIsOfferedNothing();
  //! the off switch of the spell checker: no dictionary, no suggestions
  void aBufferWithNoDictionaryIsOfferedNothing();
  //! a frozen buffer shows the underlines and none of the ways out of them
  void aBufferWhichCanNotBeWrittenToIsOfferedNothing();
  // the last case: it asks the `Markdown` mode, whose menu is built by the same
  // helper of the core library as that of the `LaTeX` mode is
  void aNoteOfMarkdownHoldsTheSameSubmenuAndReplacesTheSameWay();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  //! write `content` in a temporary file of the given name and return its path
  QString writeFile(const QString &name, const QString &content);
  /*!
   * \brief an editor widget holding a note of the given mode: the frame is the one
   * which holds the document, the mode the one which completes the menu
   */
  struct Note {
    std::unique_ptr<tfel::gui::EditorWidget> editor;
    tfel::gui::TextEditBase *frame = nullptr;
    tfel::gui::MajorMode *mode = nullptr;
  };  // end of struct Note
  /*!
   * \brief open a buffer of the given content under the given mode, with the
   * dictionary of this machine given to it by name
   *
   * \note the mode is asked for by name although the file name chooses one: the
   * point is to hold an instance of the mode a plugin built, and `setMajorMode` is
   * the one public door to that
   */
  Note openNote(const QString &name,
                const QString &content,
                const QString &mode_name);
  /*!
   * \brief the suggestions the dictionary of this machine answers for a word,
   * empty when it spells the word well or answers nothing for it
   */
  QStringList suggestionsOf(const QString &word);
  /*!
   * \brief the text of the document of a frame, with the end of a block always
   * spelled the same way: the two readings of these documents differ, some spell
   * it `'\n'`, some U+2029, and a whole text is compared character by character
   */
  static QString textOf(tfel::gui::TextEditBase &frame);
  /*!
   * \brief the place of the given occurrence of a word in the document, or -1 when
   * the document holds fewer occurrences than asked for
   *
   * \note a place of a document is between two characters, and that is exactly
   * what the menu is raised at: the index of a word is the place right before its
   * first letter
   */
  static int placeOf(tfel::gui::TextEditBase &frame,
                     const QString &word,
                     const int occurrence);
  /*!
   * \brief fill a menu at the given place as a right click fills it, and return
   * the entry which leads to the suggestions, a null pointer when the menu holds
   * no such entry
   *
   * \note the menu holds an entry of the text edit before the mode is asked to
   * complete it, as the menu of a right click holds the undo, copy and paste
   * entries of the frame: where the suggestions stand among those is part of the
   * behaviour under test
   */
  static QAction *askMenu(tfel::gui::MajorMode &mode,
                          tfel::gui::TextEditBase &frame,
                          const int position,
                          QMenu &menu);
  //! the submenu of a menu, null when the menu holds no entry which leads to one
  static QMenu *suggestionsSubmenu(QMenu &menu);
  /*!
   * \brief the entries of the submenu of a menu, empty when it holds none: what is
   * asserted on is that those entries are the suggestions, and in what order
   */
  static QStringList suggestionsInTheMenu(QMenu &menu);
  //! the directory the files of the cases are written in
  QTemporaryDir directory;
};  // end of struct TestSpellCheckSuggestions

void TestSpellCheckSuggestions::isolateSettings() {
  // an own configuration file: the cases name the dictionary they are run against
  // and must leave the configuration of the user alone
  QApplication::setOrganizationName("tfel-editor-tests");
  QApplication::setApplicationName("test-spell-check-suggestions");
  auto settings = QSettings();
  settings.remove("spelling/checker");
  settings.remove("spelling/language");
  settings.remove("hunspell default dictionary");
  settings.sync();
}  // end of TestSpellCheckSuggestions::isolateSettings

QString TestSpellCheckSuggestions::writeFile(const QString &name,
                                             const QString &content) {
  const auto file = directory.filePath(name);
  auto handle = QFile(file);
  if (!handle.open(QIODevice::WriteOnly)) {
    return QString();
  }
  handle.write(content.toUtf8());
  handle.close();
  return file;
}  // end of TestSpellCheckSuggestions::writeFile

TestSpellCheckSuggestions::Note TestSpellCheckSuggestions::openNote(
    const QString &name,
    const QString &content,
    const QString &mode_name) {
  auto note = Note();
  const auto file = writeFile(name, content);
  if (file.isEmpty()) {
    return note;
  }
  note.editor = std::make_unique<tfel::gui::EditorWidget>();
  note.editor->show();
  note.editor->openFile(file);
  note.frame = &note.editor->getCurrentBuffer().getMainFrame();
  note.mode = note.frame->setMajorMode(mode_name);
  // the dictionary is given by name: a machine which holds no such dictionary is
  // answered nothing by the checker, which is what the preconditions of the cases
  // are about, and a machine whose default dictionary is another language would
  // otherwise answer the words of its own
  note.frame->setSpellCheckLanguage(dictionary());
  QTest::qWait(50);
  return note;
}  // end of TestSpellCheckSuggestions::openNote

QStringList TestSpellCheckSuggestions::suggestionsOf(const QString &word) {
  auto checker = tfel::gui::SpellChecker();
  checker.setSpellCheckLanguage(dictionary());
  if (checker.spell(word)) {
    return QStringList();
  }
  return checker.suggest(word);
}  // end of TestSpellCheckSuggestions::suggestionsOf

QString TestSpellCheckSuggestions::textOf(tfel::gui::TextEditBase &frame) {
  return frame.document()
      ->toPlainText()
      .replace(QChar(0x2029), QChar('\n'));
}  // end of TestSpellCheckSuggestions::textOf

int TestSpellCheckSuggestions::placeOf(tfel::gui::TextEditBase &frame,
                                       const QString &word,
                                       const int occurrence) {
  auto from = 0;
  auto place = -1;
  for (auto n = 0; n != occurrence; ++n) {
    const auto found = frame.document()->find(word, from);
    if (found.isNull()) {
      return -1;
    }
    place = static_cast<int>(found.selectionStart());
    from = place + 1;
  }
  return place;
}  // end of TestSpellCheckSuggestions::placeOf

QAction *TestSpellCheckSuggestions::askMenu(tfel::gui::MajorMode &mode,
                                            tfel::gui::TextEditBase &frame,
                                            const int position,
                                            QMenu &menu) {
  // an entry of the text edit first, as a right click's menu holds the undo, copy
  // and paste entries of the frame before the mode completes it: an entry appended
  // at the end of the menu, or one which pushes those aside, is a shape change
  // this test is meant to see
  menu.addAction(QStringLiteral("stock"));
  auto cursor = QTextCursor(frame.document());
  cursor.setPosition(position);
  mode.completeContextMenu(&menu, cursor);
  auto *const submenu = suggestionsSubmenu(menu);
  return (submenu == nullptr) ? nullptr : submenu->menuAction();
}  // end of TestSpellCheckSuggestions::askMenu

QMenu *TestSpellCheckSuggestions::suggestionsSubmenu(QMenu &menu) {
  return menu.findChild<QMenu *>(QStringLiteral("spell-suggestions"));
}  // end of TestSpellCheckSuggestions::suggestionsSubmenu

QStringList TestSpellCheckSuggestions::suggestionsInTheMenu(QMenu &menu) {
  auto entries = QStringList();
  auto *const submenu = suggestionsSubmenu(menu);
  if (submenu == nullptr) {
    return entries;
  }
  for (const auto *const a : submenu->actions()) {
    entries << a->text();
  }
  return entries;
}  // end of TestSpellCheckSuggestions::suggestionsInTheMenu

void TestSpellCheckSuggestions::initTestCase() {
  isolateSettings();
  QVERIFY(directory.isValid());
  // the two modes which answer the question are packaged plugins, and the point of
  // the cases is to ask a mode such a plugin built
  auto latex = QLibrary(QStringLiteral(TFEL_EDITOR_LATEX_MODE));
  QVERIFY2(latex.load(), qPrintable(QStringLiteral(
                             "the packaged mode is not loaded: ") +
                         latex.errorString()));
  auto markdown = QLibrary(QStringLiteral(TFEL_EDITOR_MARKDOWN_MODE));
  QVERIFY2(markdown.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      markdown.errorString()));
}  // end of TestSpellCheckSuggestions::initTestCase

void TestSpellCheckSuggestions::
    theMenuOfAMisspelledWordHoldsASubmenuOfItsSuggestions() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP(
        "no suggestions for `mistaked`: this build has no hunspell, or this "
        "machine no en_US dictionary");
  }
  auto note = openNote(QStringLiteral("shape.tex"),
                       QStringLiteral("This is a note with a %1 word.\n")
                           .arg(needle()),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  const auto place = placeOf(*note.frame, needle(), 1);
  QVERIFY(place >= 0);
  auto menu = QMenu();
  const auto entry = askMenu(*note.mode, *note.frame, place, menu);
  QVERIFY2(entry != nullptr,
           "the menu of a misspelled word of a note holds no suggestions");
  // the entry is named for what it leads to, and it leads to a submenu
  QCOMPARE(entry->text(), QStringLiteral("Suggestions"));
  auto *const submenu = entry->menu();
  QVERIFY2(submenu != nullptr, "the entry of the suggestions leads nowhere");
  // which is what a test, and whoever else walks the menu, singles it out by
  QCOMPARE(submenu->objectName(), QStringLiteral("spell-suggestions"));
  // the entry stands in front of the entries the menu already held, with a
  // separator behind it: the suggestions are what the click asked about, the
  // entries of the text edit are none of its business
  const auto entries = menu.actions();
  QCOMPARE(static_cast<int>(entries.size()), 3);
  QCOMPARE(entries.at(0), static_cast<QAction *>(entry));
  QVERIFY2(entries.at(1)->isSeparator(),
           "nothing separates the suggestions from the entries of the frame");
  QCOMPARE(entries.at(2)->text(), QStringLiteral("stock"));
  // and the submenu holds one entry per suggestion, in the order the dictionary
  // answered them: the order of hunspell is what puts the likeliest correction
  // first for a reader to see
  QCOMPARE(suggestionsInTheMenu(menu), expected);
  // the status tip of a suggestion says which two words the entry trades
  auto tip_number = 0;
  for (const auto *const a : submenu->actions()) {
    const auto tip = a->statusTip();
    QVERIFY2(tip.contains(needle()) && tip.contains(expected.at(tip_number)),
             qPrintable(QStringLiteral("the status tip of a suggestion names "
                                       "neither the word nor the suggestion: ") +
                        tip));
    ++tip_number;
  }
  QCOMPARE(tip_number, static_cast<int>(expected.size()));
}  // end of TestSpellCheckSuggestions::theMenuOfAMisspelledWordHoldsASubmenu...

void TestSpellCheckSuggestions::
    choosingASuggestionReplacesTheWordInTheBuffer() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP("no suggestions for `mistaked`: nothing can be chosen");
  }
  const auto line = QStringLiteral("This is a note with a %1 word.\n");
  auto note = openNote(QStringLiteral("replace.tex"), line.arg(needle()),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  const auto place = placeOf(*note.frame, needle(), 1);
  QVERIFY(place >= 0);
  auto menu = QMenu();
  const auto entry = askMenu(*note.mode, *note.frame, place, menu);
  QVERIFY2(entry != nullptr,
           "the menu of a misspelled word of a note holds no suggestions");
  const auto suggestions = entry->menu()->actions();
  QVERIFY(!suggestions.isEmpty());
  auto *const chosen = suggestions.at(0);
  QVERIFY2(!chosen->isSeparator(), "the first suggestion of the menu is a rule");
  const auto replacement = chosen->text();
  // what a click on the entry does is emit `triggered`, which is what is emitted
  // here: the menu is never shown and no nested event loop is entered
  auto spy = QSignalSpy(chosen, &QAction::triggered);
  QVERIFY(spy.isValid());
  QVERIFY2(spy.count() == 0, "the entry of a menu never shown was already triggered");
  chosen->trigger();
  QCOMPARE(static_cast<int>(spy.count()), 1);
  // the word is replaced, the rest of the line is where it was, and the buffer
  // reads as modified, which is what a replacement written into a document does
  const auto text = textOf(*note.frame);
  QVERIFY2(!text.contains(needle()),
           qPrintable(QStringLiteral("the word is still in the buffer: ") + text));
  QCOMPARE(text, line.arg(replacement));
  QVERIFY2(note.frame->document()->isModified(),
           "replacing a word left the buffer reading as unmodified");
}  // end of TestSpellCheckSuggestions::choosingASuggestionReplacesTheWord...

void TestSpellCheckSuggestions::theMenuNamesTheWordThePositionStandsIn() {
  const auto first = suggestionsOf(needle());
  const auto second = suggestionsOf(otherNeedle());
  if (first.isEmpty() || second.isEmpty()) {
    QSKIP(
        "one of the two misspelled words is spelled well here or answered "
        "nothing: this case can not tell the two of them apart");
  }
  if (first == second) {
    QSKIP("the dictionary answers the same suggestions for both words");
  }
  auto note = openNote(QStringLiteral("two-words.tex"),
                       QStringLiteral("%1 and %2\n").arg(needle(), otherNeedle()),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  // one menu raised in each of the two words of the line: each names its own word
  const auto first_place = placeOf(*note.frame, needle(), 1);
  const auto second_place = placeOf(*note.frame, otherNeedle(), 1);
  QVERIFY((first_place >= 0) && (second_place >= 0));
  auto first_menu = QMenu();
  const auto first_entry =
      askMenu(*note.mode, *note.frame, first_place, first_menu);
  QVERIFY2(first_entry != nullptr, "the first word of the line is offered none");
  QCOMPARE(suggestionsInTheMenu(first_menu), first);
  auto second_menu = QMenu();
  const auto second_entry =
      askMenu(*note.mode, *note.frame, second_place, second_menu);
  QVERIFY2(second_entry != nullptr, "the second word of the line is offered none");
  QCOMPARE(suggestionsInTheMenu(second_menu), second);
  // and the same answer from the side of the buffer: choosing a suggestion of the
  // second word leaves the first word of the line where it is. The entry to
  // trigger is a leaf of the submenu, never the menu action itself: triggering the
  // action of a submenu does not emit `triggered`, it activates the submenu, which
  // for a menu nobody showed does nothing at all
  const auto leaves = second_entry->menu()->actions();
  QVERIFY(!leaves.isEmpty());
  QVERIFY2(!leaves.at(0)->isSeparator(), "the first suggestion of the menu is a rule");
  leaves.at(0)->trigger();
  const auto text = textOf(*note.frame);
  QVERIFY2(text.contains(needle()),
           qPrintable(QStringLiteral("the word which was not asked about is gone: ") +
                      text));
  QVERIFY2(!text.contains(otherNeedle()),
           qPrintable(
               QStringLiteral("the word which was asked about is still there: ") +
               text));
}  // end of TestSpellCheckSuggestions::theMenuNamesTheWordThePositionStandsIn

void TestSpellCheckSuggestions::
    aPlaceAtEitherEndOfTheWordNamesItAndAPlaceBetweenTwoWordsNamesNeither() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP("no suggestions for `mistaked`: nothing to name either way");
  }
  // two words and two blanks between them: a place of a document is a place
  // between two characters, so the click on the first letter of a word is answered
  // by the place which precedes it and the click on its last letter by the place
  // which follows it as likely as not. Both bounds of a word therefore belong to
  // it, and the only place left over, the one between the two blanks, belongs to
  // neither
  const auto width = static_cast<int>(needle().size());
  auto note = openNote(QStringLiteral("ends.tex"),
                       needle() + QStringLiteral("  ") + needle() +
                           QStringLiteral("\n"),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  for (const auto &place : {0, width / 2, width}) {
    auto menu = QMenu();
    const auto entry = askMenu(*note.mode, *note.frame, place, menu);
    QVERIFY2(entry != nullptr,
             qPrintable(QStringLiteral("the place ") + QString::number(place) +
                        QStringLiteral(" of the first word is offered none")));
    QCOMPARE(suggestionsInTheMenu(menu), expected);
  }
  for (const auto &place : {width + 2, width + 2 + width / 2, 2 * width + 2}) {
    auto menu = QMenu();
    const auto entry = askMenu(*note.mode, *note.frame, place, menu);
    QVERIFY2(entry != nullptr,
             qPrintable(QStringLiteral("the place ") + QString::number(place) +
                        QStringLiteral(" of the second word is offered none")));
    QCOMPARE(suggestionsInTheMenu(menu), expected);
  }
  auto gap = QMenu();
  QVERIFY2(askMenu(*note.mode, *note.frame, width + 1, gap) == nullptr,
           "the place between two misspelled words names one of them");
}  // end of TestSpellCheckSuggestions::aPlaceAtEitherEndOfTheWordNamesIt...

void TestSpellCheckSuggestions::aWellSpelledWordIsOfferedNothing() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP(
        "no suggestions for `mistaked`: the positive half of the case holds for "
        "no dictionary");
  }
  // one line, two words: the menu is asked at each of them and answers the
  // misspelled one and not the other
  auto note = openNote(QStringLiteral("well-spelled.tex"),
                       QStringLiteral("This note is spelled well and %1.\n")
                           .arg(needle()),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  auto well = QMenu();
  const auto well_place = placeOf(*note.frame, QStringLiteral("spelled"), 1);
  QVERIFY(well_place >= 0);
  QVERIFY2(askMenu(*note.mode, *note.frame, well_place, well) == nullptr,
           "the menu of a word the dictionary knows holds suggestions");
  QCOMPARE(static_cast<int>(well.actions().size()), 1);
  auto badly = QMenu();
  const auto badly_place = placeOf(*note.frame, needle(), 1);
  QVERIFY(badly_place >= 0);
  QVERIFY2(askMenu(*note.mode, *note.frame, badly_place, badly) != nullptr,
           "the menu of the misspelled word of the same line holds nothing: the "
           "two menus disagree about one line");
}  // end of TestSpellCheckSuggestions::aWellSpelledWordIsOfferedNothing

void TestSpellCheckSuggestions::aWordOfACommentOfANoteIsOfferedNothing() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP("no suggestions for `mistaked`: nothing to withhold from a comment");
  }
  // the same misspelled word twice, once in the text of the note, once past the
  // `%' which opens a comment: the highlighter of the mode underlines the first
  // and not the second, and the menu has to agree with it, since a suggestion for
  // a word of a comment is a replacement of something the mode never read
  auto note = openNote(
      QStringLiteral("comment.tex"),
      QStringLiteral("\\section{Title}\n\n"
                     "This note is ") +
          needle() + QStringLiteral(".\n\n% a comment holds the same ") +
          needle() + QStringLiteral(" too\n"),
      QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  const auto text_place = placeOf(*note.frame, needle(), 1);
  const auto comment_place = placeOf(*note.frame, needle(), 2);
  QVERIFY((text_place >= 0) && (comment_place >= 0));
  auto in_text = QMenu();
  QVERIFY2(askMenu(*note.mode, *note.frame, text_place, in_text) != nullptr,
           "the misspelled word of the text of the note is offered nothing");
  auto in_comment = QMenu();
  QVERIFY2(
      askMenu(*note.mode, *note.frame, comment_place, in_comment) == nullptr,
      "the menu offers to replace a word of a comment, which the mode never "
      "underlines");
}  // end of TestSpellCheckSuggestions::aWordOfACommentOfANoteIsOfferedNothing

void TestSpellCheckSuggestions::aBufferWithNoDictionaryIsOfferedNothing() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP(
        "no suggestions for `mistaked`: the positive half of the case holds for "
        "no dictionary");
  }
  auto note = openNote(QStringLiteral("no-dictionary.tex"),
                       QStringLiteral("This note is %1.\n").arg(needle()),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  const auto place = placeOf(*note.frame, needle(), 1);
  QVERIFY(place >= 0);
  auto with = QMenu();
  QVERIFY2(askMenu(*note.mode, *note.frame, place, with) != nullptr,
           "the buffer was given no dictionary at all");
  // the empty language is the off switch of the spell checker, and it is what the
  // choices of the proofreader which name no hunspell give a buffer: a checker with
  // no dictionary spells every word, so no word is misspelled and none is offered
  // a suggestion
  note.frame->setSpellCheckLanguage(QString());
  auto without = QMenu();
  QVERIFY2(askMenu(*note.mode, *note.frame, place, without) == nullptr,
           "a buffer with no dictionary is still offered suggestions");
  QCOMPARE(static_cast<int>(without.actions().size()), 1);
}  // end of TestSpellCheckSuggestions::aBufferWithNoDictionaryIsOfferedNothing

void TestSpellCheckSuggestions::
    aBufferWhichCanNotBeWrittenToIsOfferedNothing() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP("no suggestions for `mistaked`: nothing to refuse on a frozen buffer");
  }
  auto note = openNote(QStringLiteral("frozen.tex"),
                       QStringLiteral("This note is %1.\n").arg(needle()),
                       QStringLiteral("LaTeX"));
  QVERIFY(note.mode != nullptr);
  const auto place = placeOf(*note.frame, needle(), 1);
  QVERIFY(place >= 0);
  auto writable = QMenu();
  QVERIFY2(askMenu(*note.mode, *note.frame, place, writable) != nullptr,
           "the buffer was not writable to begin with");
  // what the freeze of a file modified on disk, or the output of a process, is: the
  // underlines of the mode stay on the screen, and a menu which offered to replace
  // a word it can not replace would lie
  note.frame->setReadOnly(true);
  auto frozen = QMenu();
  QVERIFY2(askMenu(*note.mode, *note.frame, place, frozen) == nullptr,
           "a read-only buffer is offered a replacement it can not make");
  QCOMPARE(static_cast<int>(frozen.actions().size()), 1);
}  // end of TestSpellCheckSuggestions::aBufferWhichCanNotBeWrittenTo...

void TestSpellCheckSuggestions::
    aNoteOfMarkdownHoldsTheSameSubmenuAndReplacesTheSameWay() {
  const auto expected = suggestionsOf(needle());
  if (expected.isEmpty()) {
    QSKIP("no suggestions for `mistaked`: the menu of a note has nothing to say");
  }
  // an accented word in front of the misspelled one: the tokenizer a note is split
  // with answers byte offsets of the UTF-8 encoding of the line, while the places
  // of a document and of a `QString` are characters, so a mode which took the
  // answer of the splitter literally would look for the word one character behind
  // where it stands and find nothing at the place the menu was raised at
  const auto line = QStringLiteral("Une rentrée %1 dans un paragraphe.\n");
  auto note = openNote(QStringLiteral("note.md"), line.arg(needle()),
                       QStringLiteral("Markdown"));
  QVERIFY2(note.mode != nullptr,
           "the `Markdown' mode was not built by the loaded plugin");
  const auto place = placeOf(*note.frame, needle(), 1);
  QVERIFY(place >= 0);
  auto menu = QMenu();
  const auto entry = askMenu(*note.mode, *note.frame, place, menu);
  QVERIFY2(entry != nullptr,
           "the menu of a misspelled word of a note holds no suggestions");
  QCOMPARE(entry->text(), QStringLiteral("Suggestions"));
  QCOMPARE(suggestionsInTheMenu(menu), expected);
  // and the replacement lands where the word stands, the accent of the line and the
  // words on either side of it included
  const auto replacement = entry->menu()->actions().at(0)->text();
  entry->menu()->actions().at(0)->trigger();
  QCOMPARE(textOf(*note.frame), line.arg(replacement));
}  // end of TestSpellCheckSuggestions::aNoteOfMarkdownHoldsTheSameSubmenu...

QTEST_MAIN(TestSpellCheckSuggestions)
#include "TestSpellCheckSuggestions.moc"
