/*!
 * \file   TestGrammarCheck.cxx
 * \brief  tests for the grammar checker of a buffer: the settings which decide
 *         whether a buffer takes a session with it, the underlines such a
 *         session draws, and the entries of a menu which steer both.
 * \author Thomas Helfer
 * \date   5 nov. 2026
 */

#include <QtTest/QtTest>
#include <QtCore/QLibrary>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtGui/QColor>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolTip>
#include <functional>
#include <memory>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// The checker is a process, so the cases which ask it anything are worth
// running only where the program is installed; the settings and the shape of
// the menus are asked about either way, since what a person who has no checker
// is shown is exactly that the editor offers nothing.
//
// The cases all go through the `C++` mode: that mode lives in the library this
// test links, whereas the `LaTeX` and `Markdown` modes are plugins the editor
// loads, which the test would have to load itself, see
// `aMajorModeLoadedAsAPluginExportsTheEntriesItUses` of `TestSettings.cxx`.
// What is checked here is the session, not the mode which hosts it, and the two
// textual modes add nothing of their own to it but the note that the checker
// reads English.
namespace {

  /*!
   * \brief the English the checker is asked about, as the comments of a C++
   * file spell it: three sentences, each wrong in a way of its own, and a line
   * of good English which has to stay unmarked
   *
   * \note the checker reads a C++ file as prose where a person wrote prose,
   * which is to say in the comments: the same text out of the comments is code,
   * and the server answers nothing about it
   */
  auto checkedComments() {
    return QStringLiteral(
        "// This is an tests of the grammar.\n"
        "// The colour of the apple are red.\n"
        "// She don't like to eats apples.\n"
        "// The variable is a plain line of the buffer.\n");
  }  // end of checkedComments()

  //! a buffer to check: the English above, and a line of code to leave alone
  auto checkedSource() {
    return checkedComments() + QStringLiteral("int main() { return 0; }\n");
  }  // end of checkedSource()

}  // end of anonymous namespace

struct TestGrammarCheck : QObject {
  Q_OBJECT

 public:
  /*!
   * \brief the words the grammar checker of a frame underlines
   * \param[in] frame : the text edit whose keyed selections are read
   *
   * \note the faces a highlighter writes are held by the document and are none
   * of these selections' business; the keyed contributions are, and their list
   * is flat: the current line, the diagnostics, the colors of the identifiers
   * and the lints of the checker all sit in it together. The face of a lint is
   * an underline in the red of the spelling checker, `170 0 0` being the red of
   * an error of `clangd`, so the color tells the two underlines apart
   */
  static QStringList underlinedWords(tfel::gui::TextEditBase &frame);

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void theSettingsSayWhatIsStoredAndNothingElse();
  void theVarietiesOfferedAreTheOnesTheCheckerReads();
  void theCxxMenuOffersTheCheckerOnlyWhereItIs();
  void theCheckerIsLaunchedByTheSettingAndStopsWithTheMenu();
  void theLintsOfABufferStandUnderTheWordsTheyAreAbout();
  void stoppingTheModeTakesTheUnderlinesAway();
  void aVarietyOfEnglishChangesWhatIsFound();
  void aCheckedTextBufferIsNotReadByBothCheckers();
  // the last case: it loads a packaged mode, which stays registered for the
  // rest of the run
  void theMenuOfTheMarkdownModeLaunchesTheChecker();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  //! write `content` in a temporary file of the given name and return its path
  QString writeFile(const QString &name, const QString &content);
  /*!
   * \brief the words the checker underlines once it stopped publishing
   *
   * \note the checker answers a whole file in pieces, and an early piece is not
   * always the last word: what it says of a sentence is recomputed as it reads
   * the rest of the file, and a test which reads the screen of the first answer
   * would assert on a state the checker had not reached. The list is sampled
   * until it stops changing, which is what the assertions are about
   */
  static QStringList settledWords(tfel::gui::TextEditBase &frame,
                                  const int ms = 10000);
  //! the checkable entry of a menu which turns the checker on and off
  static QAction *grammarEntry(QMenu &menu);
  //! the submenu of the varieties of English a menu offers, null if none
  static QMenu *varietiesOfEnglish(QMenu &menu);
  /*!
   * \brief the `C/C++` menu the current buffer offers, null when the buffer is
   * not a C++ file
   *
   * \note the menus of the buffer are built by the modes, which is the only
   * public door to the entries of a mode: a `MajorMode &` of a frame is handed
   * out as a `const` reference only
   */
  static QMenu *cxxMenu(tfel::gui::EditorWidget &);
  //! spin the event loop until the predicate holds or the time is up
  static bool waitFor(const std::function<bool()> &predicate, const int ms);
  //! the directory the files of the cases are written in
  QTemporaryDir directory;
  //! whether a checker is to be launched at all on this machine
  bool available = false;
};  // end of struct TestGrammarCheck

void TestGrammarCheck::isolateSettings() {
  // an own configuration file: the cases write the addresses the checker reads
  // and must leave the configuration of the user alone
  QApplication::setOrganizationName("tfel-editor-tests");
  QApplication::setApplicationName("test-grammar-check");
  auto settings = QSettings();
  settings.remove("harper/launch-by-default");
  settings.remove("harper/executable");
  settings.remove("harper/dialect");
  settings.sync();
}  // end of TestGrammarCheck::isolateSettings

QString TestGrammarCheck::writeFile(const QString &name,
                                    const QString &content) {
  const auto file = directory.filePath(name);
  auto handle = QFile(file);
  if (!handle.open(QIODevice::WriteOnly)) {
    return QString();
  }
  handle.write(content.toUtf8());
  handle.close();
  return file;
}  // end of TestGrammarCheck::writeFile

QStringList TestGrammarCheck::underlinedWords(tfel::gui::TextEditBase &frame) {
  auto words = QStringList();
  for (const auto &selection : frame.extraSelections()) {
    const auto &format = selection.format;
    if (!format.fontUnderline() ||
        (format.underlineColor() != QColor(200, 0, 0))) {
      continue;
    }
    const auto text = selection.cursor.selectedText();
    if (!text.isEmpty()) {
      words << text;
    }
  }
  return words;
}  // end of TestGrammarCheck::underlinedWords

QStringList TestGrammarCheck::settledWords(tfel::gui::TextEditBase &frame,
                                           const int ms) {
  auto words = underlinedWords(frame);
  auto stable = 0;
  auto timer = QElapsedTimer();
  timer.start();
  while ((stable < 3) && (timer.elapsed() < ms)) {
    QTest::qWait(100);
    auto next = underlinedWords(frame);
    if (next == words) {
      ++stable;
    } else {
      stable = 0;
      words = next;
    }
  }
  return words;
}  // end of TestGrammarCheck::settledWords

QAction *TestGrammarCheck::grammarEntry(QMenu &menu) {
  for (auto *const a : menu.actions()) {
    if (a->text().contains(QStringLiteral("Grammar checking"))) {
      return a;
    }
  }
  return nullptr;
}  // end of TestGrammarCheck::grammarEntry

QMenu *TestGrammarCheck::varietiesOfEnglish(QMenu &menu) {
  for (auto *const a : menu.actions()) {
    if (a->text().contains(QStringLiteral("English variant"))) {
      return a->menu();
    }
  }
  return nullptr;
}  // end of TestGrammarCheck::varietiesOfEnglish

QMenu *TestGrammarCheck::cxxMenu(tfel::gui::EditorWidget &editor) {
  for (auto *const menu : editor.getCurrentBufferSpecificMenus()) {
    if (menu->title() == QStringLiteral("C/C++")) {
      return menu;
    }
  }
  return nullptr;
}  // end of TestGrammarCheck::cxxMenu

bool TestGrammarCheck::waitFor(const std::function<bool()> &predicate,
                               const int ms) {
  auto timer = QElapsedTimer();
  timer.start();
  while (!predicate()) {
    if (timer.elapsed() > ms) {
      return false;
    }
    QTest::qWait(50);
  }
  return true;
}  // end of TestGrammarCheck::waitFor

void TestGrammarCheck::initTestCase() {
  isolateSettings();
  QVERIFY(directory.isValid());
  this->available = tfel::gui::GrammarCheckSession::isAvailable();
}  // end of TestGrammarCheck::initTestCase

void TestGrammarCheck::cleanupTestCase() {
  isolateSettings();
}  // end of TestGrammarCheck::cleanupTestCase

void TestGrammarCheck::theSettingsSayWhatIsStoredAndNothingElse() {
  // what a freshly installed editor answers: no session at all, the program
  // looked for in the directories of the session, and the English of the
  // checker itself
  QVERIFY2(!tfel::gui::LspHarperSupport::launchByDefault(),
           "a buffer is asked to take a grammar session on its own");
  QVERIFY2(!tfel::gui::LspHarperSupport::serverExecutable().isEmpty() ||
               !this->available,
           "the program is missing and the checker says it is there");
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("American"));
  // a session speaks over the pipes the client writes to: without that argument
  // the server waits on a local socket and reads nothing of the buffers
  QCOMPARE(tfel::gui::LspHarperSupport::serverArguments(),
           QStringList() << QStringLiteral("--stdio"));
  // turning the checker on is stored, turning it off again is forgotten: the
  // configuration file holds what was chosen, not what the editor would have
  // answered anyway, as every other entry of the editor does
  tfel::gui::LspHarperSupport::setLaunchByDefault(true);
  QVERIFY(tfel::gui::LspHarperSupport::launchByDefault());
  QCOMPARE(QSettings().value("harper/launch-by-default").toBool(), true);
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  QVERIFY(!tfel::gui::LspHarperSupport::launchByDefault());
  QVERIFY(!QSettings().contains("harper/launch-by-default"));
  // the settings a session is answered with live under the name of the server,
  // which is where it reads them and where it says it reads them; no other key
  // is sent, and the variety is the one the setting names
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("British"));
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("British"));
  const auto settings = tfel::gui::LspHarperSupport::configuration();
  QVERIFY2(settings.isObject(),
           "the settings a session answers with are not an "
           "object held under the name of the server");
  const auto server = settings.toObject().value(QStringLiteral("harper-ls"));
  QVERIFY2(!server.isUndefined(),
           "the settings are not held under the name of the server");
  QCOMPARE(server.toObject().value(QStringLiteral("dialect")).toString(),
           QStringLiteral("British"));
  // a name which is none of the varieties is no choice at all: keeping it would
  // have the next session read the text as a variety nobody can choose again
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("Klingon"));
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("American"));
  // and the variety the checker reads a text by when nothing was chosen is not
  // stored either
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("American"));
  QVERIFY(!QSettings().contains("harper/dialect"));
}  // end of TestGrammarCheck::theSettingsSayWhatIsStoredAndNothingElse

void TestGrammarCheck::theVarietiesOfferedAreTheOnesTheCheckerReads() {
  const auto dialects = tfel::gui::LspHarperSupport::dialects();
  // the varieties of the server, the first of them its own default, which is
  // what `dialect` answers when nothing was chosen
  QCOMPARE(dialects.size(), 5);
  QCOMPARE(dialects.front(), QStringLiteral("American"));
  QVERIFY2(dialects.contains(QStringLiteral("British")),
           "the varieties offered miss the one a document spelled in British "
           "English is written in");
  for (const auto &name : dialects) {
    // a person chooses a variety of English, and the entry of a menu says so
    // rather than the bare name the server reads
    QCOMPARE(tfel::gui::LspHarperSupport::dialectLabel(name),
             name + QStringLiteral(" English"));
  }
  // and the variety which is stored is the one a session is answered with: the
  // list, the setting and the handshake answer all come from one place
  for (const auto &name : dialects) {
    tfel::gui::LspHarperSupport::setDialect(name);
    QCOMPARE(tfel::gui::LspHarperSupport::dialect(), name);
    QCOMPARE(tfel::gui::LspHarperSupport::configuration()
                 .toObject()
                 .value(QStringLiteral("harper-ls"))
                 .toObject()
                 .value(QStringLiteral("dialect"))
                 .toString(),
             name);
  }
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("American"));
}  // end of
   // TestGrammarCheck::theVarietiesOfferedAreTheOnesTheCheckerReads

void TestGrammarCheck::theCxxMenuOffersTheCheckerOnlyWhereItIs() {
  const auto file = writeFile(QStringLiteral("menu.cxx"),
                              QStringLiteral("int main() { return 0; }\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto *const menu = cxxMenu(*editor);
  QVERIFY(menu != nullptr);
  auto *const entry = grammarEntry(*menu);
  auto *const varieties = varietiesOfEnglish(*menu);
  if (!this->available) {
    // a program which is not there is not offered: an entry which could only
    // report that the program is missing is no entry
    QVERIFY2(entry == nullptr,
             "the menu offers a checker where none is to be launched");
    QVERIFY2(
        varieties == nullptr,
        "the menu offers varieties of English with no checker to read them");
    editor.reset();
    return;
  }
  QVERIFY(entry != nullptr);
  QVERIFY(entry->isCheckable());
  QVERIFY2(!entry->isChecked(),
           "a buffer takes a grammar session without anybody asking");
  QVERIFY(varieties != nullptr);
  auto names = QStringList();
  auto checked = QStringList();
  for (auto *const a : varieties->actions()) {
    QVERIFY2(a->isCheckable(), qPrintable(QStringLiteral("the entry '") +
                                          a->text() + "' names no state"));
    names << a->text();
    if (a->isChecked()) {
      checked << a->text();
    }
  }
  auto expected = QStringList();
  for (const auto &name : tfel::gui::LspHarperSupport::dialects()) {
    expected << tfel::gui::LspHarperSupport::dialectLabel(name);
  }
  QCOMPARE(names, expected);
  // exactly one variety is ticked, the one a session reads a text as
  QCOMPARE(checked.size(), 1);
  QCOMPARE(checked.front(), tfel::gui::LspHarperSupport::dialectLabel(
                                tfel::gui::LspHarperSupport::dialect()));
  // and picking another one writes the setting: the submenu is one of the two
  // ways that address is written, the other being the dialog of the
  // configuration
  auto *const british = varieties->actions().at(3);
  QCOMPARE(british->text(), QStringLiteral("British English"));
  british->trigger();
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("British"));
  // a menu built afterwards names the variety the setting chose, which is what
  // keeps the submenus of the buffers on screen from disagreeing with each
  // other
  auto *const again = varietiesOfEnglish(*cxxMenu(*editor));
  QVERIFY(again != nullptr);
  QVERIFY(again->actions().at(3)->isChecked());
  QVERIFY(!again->actions().at(0)->isChecked());
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("American"));
  editor.reset();
}  // end of TestGrammarCheck::theCxxMenuOffersTheCheckerOnlyWhereItIs

void TestGrammarCheck::theCheckerIsLaunchedByTheSettingAndStopsWithTheMenu() {
  if (!this->available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // a buffer opened while the setting says so takes a session with it, and the
  // words the checker has something to say about are underlined: the case is
  // the whole chain, from an address of the configuration to the screen, with
  // no menu touched at all
  tfel::gui::LspHarperSupport::setLaunchByDefault(true);
  const auto file = writeFile(QStringLiteral("by-default.cxx"),
                              QStringLiteral("// an tests of the grammar\n"
                                             "int main() { return 0; }\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 30000),
      "the setting launched no session, or its answer reached no text");
  QVERIFY2(underlinedWords(frame).contains(QStringLiteral("an")),
           qPrintable("the article of `an tests` is not underlined: " +
                      underlinedWords(frame).join(QStringLiteral(", "))));
  // the entry of the menu says what the session the setting started did: it is
  // checked, and triggering it is what ends that session
  auto *const entry = grammarEntry(*cxxMenu(*editor));
  QVERIFY(entry != nullptr);
  QVERIFY2(entry->isChecked(),
           "the entry of the menu ignores the session of the buffer");
  entry->trigger();
  QVERIFY(
      waitFor([&frame] { return underlinedWords(frame).isEmpty(); }, 10000));
  // and the entry wrote the setting which launched the session: a buffer opened
  // afterwards is checked no more, which is what makes the one switch of the
  // editor one switch and not three
  QVERIFY2(!tfel::gui::LspHarperSupport::launchByDefault(),
           "the entry which stopped a session left the setting on");
  editor.reset();
}  // end of
   // TestGrammarCheck::theCheckerIsLaunchedByTheSettingAndStopsWithTheMenu

void TestGrammarCheck::theLintsOfABufferStandUnderTheWordsTheyAreAbout() {
  if (!this->available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // the case drives the entry of the menu, so it starts from the state that
  // entry is unchecked in: a buffer opened while the setting says otherwise
  // takes a session with it, and the first trigger below would end it, not
  // start one
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  const auto file = writeFile(QStringLiteral("lints.cxx"), checkedSource());
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the buffer starts no session of its own: the case before this one ended
  // with the setting off
  QVERIFY(!tfel::gui::LspHarperSupport::launchByDefault());
  QVERIFY(underlinedWords(frame).isEmpty());
  auto *const entry = grammarEntry(*cxxMenu(*editor));
  QVERIFY(entry != nullptr);
  entry->trigger();
  const auto report = [&frame] {
    return qPrintable(QStringLiteral("underlined: ") +
                      underlinedWords(frame).join(QStringLiteral(", ")));
  };
  QVERIFY2(
      waitFor([&frame] { return underlinedWords(frame).size() >= 3; }, 30000),
      report());
  // what the checker says of a sentence stands under the words it said it of:
  // the article which does not fit the word it introduces, the verb which has
  // to be the base form after `to`, the form which has to agree with its
  // pronoun, and the spelling of another variety of English. The list is taken
  // once the publisher is done, which is what the negative assertions below
  // lean on
  const auto words = settledWords(frame);
  QVERIFY2(words.contains(QStringLiteral("an")), report());
  QVERIFY2(words.contains(QStringLiteral("to eats")), report());
  QVERIFY2(words.contains(QStringLiteral("don't")), report());
  QVERIFY2(words.contains(QStringLiteral("colour")), report());
  // a sentence of good English is left alone, which is what makes the
  // underlines worth reading: a checker which inked a whole file says nothing
  // a named list: `QStringList() << a << b` answers a reference to the
  // temporary it built, and a range-`for` does not extend the lifetime of that
  // temporary
  const auto cleans = QStringList()
                      << QStringLiteral("variable") << QStringLiteral("plain")
                      << QStringLiteral("buffer");
  for (const auto &clean : cleans) {
    QVERIFY2(!words.contains(clean), report());
  }
  // and the line of code is none of the checker's business: what it reads of a
  // C++ file is the prose a person wrote in it
  QVERIFY2(!words.contains(QStringLiteral("main")), report());
  // the message of a lint is read where it stands: resting the mouse on an
  // underlined word shows what the checker said of it, with the rule it
  // applied. Qt 6.2 delivers a `ToolTip` event as a contentless `QShowEvent`,
  // so the place the tip is about is the position of the virtual mouse, which
  // the move sets
  auto *const view = frame.widget()->viewport();
  QVERIFY(view != nullptr);
  auto colour = QTextCursor();
  for (const auto &selection : frame.extraSelections()) {
    if (selection.cursor.selectedText() == QStringLiteral("colour")) {
      colour = selection.cursor;
    }
  }
  QVERIFY(!colour.isNull());
  // the middle of the underlined run, and not the rectangle of a selection
  // cursor: the rectangle of a cursor which holds a selection is the whole run,
  // and its right edge is the character the lint stops before
  auto middle = QTextCursor(frame.document());
  middle.setPosition((colour.selectionStart() + colour.selectionEnd()) / 2);
  const auto rect = frame.cursorRect(middle);
  const QPoint over(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
  QToolTip::hideText();
  QTest::mouseMove(view, over);
  auto resting = QEvent(QEvent::ToolTip);
  QApplication::sendEvent(view, &resting);
  auto tip = QString();
  QVERIFY2(
      waitFor(
          [&tip] {
            for (auto *const w : QApplication::topLevelWidgets()) {
              auto *const l = qobject_cast<QLabel *>(w);
              if ((l != nullptr) && l->isVisible() && (!l->text().isEmpty())) {
                tip = l->text();
                return true;
              }
            }
            return false;
          },
          5000),
      "resting the mouse on an underlined word shows no tooltip");
  // the sentence names the rule the checker applied, which is what a person who
  // thinks the rule is wrong looks the name up for
  QVERIFY2(tip.contains(QStringLiteral("SpellCheck")), qPrintable(tip));
  QToolTip::hideText();
  entry->trigger();
  QVERIFY(
      waitFor([&frame] { return underlinedWords(frame).isEmpty(); }, 10000));
  editor.reset();
}  // end of
   // TestGrammarCheck::theLintsOfABufferStandUnderTheWordsTheyAreAbout

void TestGrammarCheck::stoppingTheModeTakesTheUnderlinesAway() {
  if (!this->available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  const auto file = writeFile(QStringLiteral("gone.cxx"),
                              QStringLiteral("// an tests of the grammar\n"
                                             "int main() { return 0; }\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto *const entry = grammarEntry(*cxxMenu(*editor));
  QVERIFY(entry != nullptr);
  entry->trigger();
  QVERIFY(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 30000));
  QVERIFY2(entry->isChecked(), "the entry ignores the session it steers");
  // and the mode is replaced while that session runs, which is the shape of the
  // moment: a mode which is going hands the text edit to the next one, keyed
  // contributions and all, so the checker which is going has taken its own
  // away, or the underlines of a dead session stay on the screen of a buffer
  // which asks nothing of it. `C++` is the only mode this library registers,
  // and a fresh instance of it is as good a next mode as any — but a fresh
  // instance which starts a session of its own would draw the same key, so the
  // switch the entry wrote on its way is put back here: what this case is about
  // is the answer the session which is going leaves behind, not the setting
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  QVERIFY(frame.setMajorMode(QStringLiteral("C++")) != nullptr);
  auto &changed = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(
      waitFor([&changed] { return underlinedWords(changed).isEmpty(); }, 5000),
      "the checker of a mode which is gone went on underlining");
  // and the buffer is not left without its own selections: the current line is
  // still painted
  QVERIFY(changed.extraSelections().size() >= 1);
  editor.reset();
}  // end of TestGrammarCheck::stoppingTheModeTakesTheUnderlinesAway

void TestGrammarCheck::aVarietyOfEnglishChangesWhatIsFound() {
  if (!this->available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  // a document spelled in British English is full of lints until it is read as
  // the variety it is spelled in: the same buffer, the same text, and one word
  // less once the checker knows which English it is looking at, which is the
  // whole use of the choice
  const auto file = writeFile(QStringLiteral("dialect.cxx"), checkedSource());
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto *const entry = grammarEntry(*cxxMenu(*editor));
  QVERIFY(entry != nullptr);
  entry->trigger();
  QVERIFY2(
      waitFor(
          [&frame] {
            return underlinedWords(frame).contains(QStringLiteral("colour"));
          },
          30000),
      qPrintable("the buffer is not read as American English: " +
                 underlinedWords(frame).join(QStringLiteral(", "))));
  auto *const varieties = varietiesOfEnglish(*cxxMenu(*editor));
  QVERIFY(varieties != nullptr);
  varieties->actions().at(3)->trigger();  // British English
  QVERIFY2(
      waitFor(
          [&frame] {
            return !underlinedWords(frame).contains(QStringLiteral("colour"));
          },
          30000),
      qPrintable("the buffer is still read as " +
                 tfel::gui::LspHarperSupport::dialect() + ": " +
                 underlinedWords(frame).join(QStringLiteral(", "))));
  // the variety decides which spellings are somebody's own, not whether a
  // sentence agrees with itself: the other lints of the same text stand
  const auto words = underlinedWords(frame);
  QVERIFY2(words.contains(QStringLiteral("an")),
           qPrintable("the lints of British English are: " +
                      words.join(QStringLiteral(", "))));
  QVERIFY2(words.contains(QStringLiteral("don't")),
           qPrintable("the lints of British English are: " +
                      words.join(QStringLiteral(", "))));
  // back to the variety an editor reads a text by when nothing was chosen, and
  // the session of this buffer goes with it
  varietiesOfEnglish(*cxxMenu(*editor))->actions().at(0)->trigger();
  entry->trigger();
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("American"));
  QVERIFY(!tfel::gui::LspHarperSupport::launchByDefault());
  editor.reset();
}  // end of TestGrammarCheck::aVarietyOfEnglishChangesWhatIsFound

#ifdef TFEL_EDITOR_LATEX_MODE
void TestGrammarCheck::aCheckedTextBufferIsNotReadByBothCheckers() {
  // the two checkers of the editor say what is wrong with a word in the same
  // place, one in the red of a spelling mistake, the other in the red of a
  // grammar mistake, and a buffer which shows both says nothing: while the
  // grammar checker of a text mode runs, hunspell is taken off its text, which
  // is what this case is about
  //
  // the `LaTeX` mode is chosen over `Markdown` because that is the text mode
  // whose context menu asks the spell checker anything: what the suggestions of
  // a misspelled word are is the one public face of hunspell a test can see,
  // the underlines themselves belong to the highlighter and are held by the
  // document
  if (!this->available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // the mode is a plugin, and the editor is what loads it: the build tree holds
  // the absolute path of every library a plugin needs in its own run path, so
  // loading it needs no help from the environment
  auto library = QLibrary(QStringLiteral(TFEL_EDITOR_LATEX_MODE));
  QVERIFY2(library.load(), qPrintable(QStringLiteral("the packaged mode is not "
                                                     "loaded: ") +
                                      library.errorString()));
  // one word hunspell does not know, one article the checker does not like: the
  // first is what the suggestions are asked about, the second what proves a
  // session answers about this buffer
  const auto file = writeFile(
      QStringLiteral("spell.tex"),
      QStringLiteral("\\section{Title}\n\n"
                     "This is an tests with a mistaked word in it.\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the mode of the buffer, in hand and not as the `const MajorMode &` a frame
  // hands out: `setMajorMode` is the one public door to an instance of a mode
  // which a plugin built, and the mode it installs is a fresh one, which is
  // none of this case's business since no session runs yet
  auto *const mode = frame.setMajorMode(QStringLiteral("LaTeX"));
  QVERIFY(mode != nullptr);
  // the dictionary of this machine is asked for by name; a machine which has no
  // English dictionary has nothing to take off the text, which the precondition
  // below is what names
  frame.setSpellCheckLanguage(QStringLiteral("en_US"));
  const auto suggestions = [mode, &frame] {
    auto cursor = frame.document()->find(QStringLiteral("mistaked"));
    if (cursor.isNull()) {
      return -1;  // the word is missing: nothing to ask about
    }
    // the middle of the word: the suggestions are asked about the word the
    // position stands in, and a position on its first character is not in it
    cursor.setPosition(cursor.selectionStart() + 3);
    auto menu = QMenu();
    mode->completeContextMenu(&menu, cursor);
    return static_cast<int>(menu.actions().size());
  };
  // hunspell is on the text before anything else: an empty menu here would say
  // that the case has nothing to observe, not that the checker works
  const auto with_hunspell = suggestions();
  if (with_hunspell == 0) {
    QSKIP(
        "no suggestions for `mistaked`: this build has no hunspell, or this "
        "machine no en_US dictionary");
  }
  // and the menu of the mode names the language it reads, which is the one
  // thing a person choosing the entry has to know: the checker knows no word of
  // French
  auto *const specific = mode->getSpecificMenu();
  QVERIFY(specific != nullptr);
  auto *const entry = grammarEntry(*specific);
  QVERIFY(entry != nullptr);
  QVERIFY2(entry->text().contains(QStringLiteral("English only")),
           qPrintable("the entry of a text mode does not say which English it "
                      "reads: '" +
                      entry->text() + "'"));
  entry->trigger();
  // the session answers, which is what the underline of `an` says; the very
  // same moment takes hunspell off the text, and the suggestions of the
  // misspelled word go away with it
  QVERIFY2(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 30000),
      "the checker of a LaTeX buffer drew no underline");
  QVERIFY2(underlinedWords(frame).contains(QStringLiteral("an")),
           qPrintable("the article of `an tests` is not underlined: " +
                      underlinedWords(frame).join(QStringLiteral(", "))));
  QVERIFY2(suggestions() == 0,
           "a buffer the grammar checker reads is still read by hunspell");
  // the frame has not forgotten the language it was given: what is taken away
  // is the mode's use of it, not the setting of the buffer
  QCOMPARE(frame.getSpellCheckLanguage(), QStringLiteral("en_US"));
  // and turning the checker off hands the text back to hunspell, which is what
  // makes the one entry of the menu a switch and not a decision
  entry->trigger();
  QVERIFY2(
      waitFor([&frame] { return underlinedWords(frame).isEmpty(); }, 10000),
      "the underlines of a stopped session stayed on the buffer");
  QCOMPARE(suggestions(), with_hunspell);
  editor.reset();
}  // end of
   // TestGrammarCheck::aCheckedTextBufferIsNotReadByBothCheckers
#else  /* TFEL_EDITOR_LATEX_MODE */
void TestGrammarCheck::aCheckedTextBufferIsNotReadByBothCheckers() {
  QSKIP(
      "this build does not name the library of the LaTeX mode: the exclusion "
      "of hunspell can not be observed on a packaged mode");
}  // end of
   // TestGrammarCheck::aCheckedTextBufferIsNotReadByBothCheckers
#endif /* TFEL_EDITOR_LATEX_MODE */

#ifdef TFEL_EDITOR_MARKDOWN_MODE
void TestGrammarCheck::theMenuOfTheMarkdownModeLaunchesTheChecker() {
  // two text modes were asked for, not one: the case above is about hunspell
  // being taken off the text of a `LaTeX` buffer, this one is about the other
  // half — that a buffer of Markdown offers the same entry with the same note
  // and gets the same answer from the same program, under the language of its
  // own mode
  if (!this->available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  auto library = QLibrary(QStringLiteral(TFEL_EDITOR_MARKDOWN_MODE));
  QVERIFY2(library.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      library.errorString()));
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  const auto file =
      writeFile(QStringLiteral("lints.md"),
                QStringLiteral("# Title\n\n"
                               "This is an tests of the grammar.\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // `Markdown` is the mode a `.md` file is opened in already; asking for it by
  // name is what hands an instance of the mode in hand, and not the
  // `const MajorMode &` a frame answers with
  auto *const mode = frame.setMajorMode(QStringLiteral("Markdown"));
  QVERIFY(mode != nullptr);
  auto *const specific = mode->getSpecificMenu();
  QVERIFY2(
      specific != nullptr,
      "the Markdown mode offers no menu although the checker is installed");
  auto *const entry = grammarEntry(*specific);
  QVERIFY(entry != nullptr);
  QVERIFY2(
      entry->text().contains(QStringLiteral("English only")),
      qPrintable("the entry of the Markdown mode does not say which English "
                 "it reads: '" +
                 entry->text() + "'"));
  auto *const varieties = varietiesOfEnglish(*specific);
  QVERIFY2(varieties != nullptr,
           "the Markdown mode offers no submenu of varieties of English");
  auto checked = QStringList();
  for (auto *const a : varieties->actions()) {
    if (a->isChecked()) {
      checked << a->text();
    }
  }
  // a text mode names the one variety the whole editor reads a text as, the
  // menu of the C++ mode above included: the submenu is a second hand on one
  // address
  QCOMPARE(checked.size(), 1);
  QCOMPARE(checked.value(0), tfel::gui::LspHarperSupport::dialectLabel(
                                 tfel::gui::LspHarperSupport::dialect()));
  // and the entry does what the LaTeX entry does: a session of the mode's own,
  // which reads the buffer under the language of Markdown and underlines what
  // it finds there
  QVERIFY2(!entry->isChecked(),
           "a buffer of Markdown takes a session without anybody asking");
  entry->trigger();
  QVERIFY2(waitFor(
               [&frame] {
                 return underlinedWords(frame).contains(QStringLiteral("an"));
               },
               30000),
           qPrintable("the checker launched from the menu of the Markdown mode "
                      "drew no underline: " +
                      underlinedWords(frame).join(QStringLiteral(", "))));
  // and the entry is a switch: the session ends with it and the text is left
  // alone
  entry->trigger();
  QVERIFY(
      waitFor([&frame] { return underlinedWords(frame).isEmpty(); }, 10000));
  tfel::gui::LspHarperSupport::setLaunchByDefault(false);
  editor.reset();
}  // end of
   // TestGrammarCheck::theMenuOfTheMarkdownModeLaunchesTheChecker
#else  /* TFEL_EDITOR_MARKDOWN_MODE */
void TestGrammarCheck::theMenuOfTheMarkdownModeLaunchesTheChecker() {
  QSKIP(
      "this build does not name the library of the Markdown mode: the menu of "
      "that mode can not be looked at");
}  // end of
   // TestGrammarCheck::theMenuOfTheMarkdownModeLaunchesTheChecker
#endif /* TFEL_EDITOR_MARKDOWN_MODE */

QTEST_MAIN(TestGrammarCheck)

#include "TestGrammarCheck.moc"
