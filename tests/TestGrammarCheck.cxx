/*!
 * \file   TestGrammarCheck.cxx
 * \brief  tests for the proofreader of a buffer: the choice which decides what
 *         reads the words of a buffer, the languages each of the choices reads,
 *         the underlines such a session draws, and the menu which steers both.
 * \author Thomas Helfer
 * \date   5 nov. 2026
 */

#include <QtTest/QtTest>
#include <QtCore/QLibrary>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
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
#include "TFEL/GUI/LanguageToolSupport.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
#include "TFEL/GUI/HunspellDictionariesManager.hxx"
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
#include "TFEL/GUI/SpellingAndGrammar.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

/*
 * The two proofreaders are programs of their own: `harper-ls` is a binary a
 * person installs with a package manager, LanguageTool a Java archive they
 * download where they like. The cases which ask either of them anything are
 * therefore skipped when the machine has nothing to run, and the cases about
 * the settings, the menu and the shape of an answer are asked about either way:
 * what a person with no checker is shown is exactly that the editor offers
 * nothing, and that is a behaviour too.
 *
 * The cases go through the `C++` mode, which lives in the library this test
 * links, or through a packaged text mode loaded as the editor loads its plugins,
 * see the last case. The `LaTeX` mode is the one text mode whose hunspell
 * behaviour a test can see, the underlines of the misspelled words being held by
 * the document and invisible to anything but the suggestions of the context menu.
 */
namespace {

  using Checker = tfel::gui::SpellingAndGrammar::Checker;

  /*!
   * \brief the English the proofreaders are asked about, as the comments of a C++
   * file: what a checker of sentences reads of a source file is exactly this
   *
   * The four lines are three lints and one line the checker has no reason to
   * touch: `an tests` and `to eats` are the agreement rules, `the colour … are`
   * is the agreement of a subject with its verb and the spelling of another
   * variety of English, `don't` is the contraction another variety spells
   * differently.
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

  // an editor widget which remembers what the minibuffer was told: which
  // proofreader a choice ended up reading a buffer with is part of the behaviour
  // under test, and overriding this public virtual is the only door to the
  // messages
  struct RecordingEditor : tfel::gui::EditorWidget {
    QStringList messages;
    void displayInformativeMessage(const QString &m) override {
      messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct RecordingEditor

  /*!
   * \brief the LanguageTool installation the cases are run against, empty when
   * this build names none and the machine holds none either
   *
   * \note the program is a download, not a package: the build names it with
   * `-DLANGUAGE_TOOL_JAR=…` when the person who configured the tests has it, and
   * \c LanguageToolSupport::serverJar answers what the machine holds by itself
   * otherwise, which is what both `isAvailable` and the cases lean on
   */
  QString languageToolInstallation() {
#ifdef TFEL_EDITOR_LANGUAGE_TOOL_JAR
    return QStringLiteral(TFEL_EDITOR_LANGUAGE_TOOL_JAR);
#else  /* TFEL_EDITOR_LANGUAGE_TOOL_JAR */
    return tfel::gui::LanguageToolSupport::serverJar();
#endif /* TFEL_EDITOR_LANGUAGE_TOOL_JAR */
  }  // end of languageToolInstallation

}  // end of anonymous namespace

struct TestGrammarCheck : QObject {
  Q_OBJECT

 public:
  /*!
   * \brief the words a proofreader underlines in a frame
   *
   * \note the faces a highlighter writes are held by the document and are none
   * of these selections' business; the keyed contributions are, and their list
   * is flat: the current line, the diagnostics, the colors of the identifiers
   * and the lints of the checker all sit in it together. The face of a lint is an
   * underline in the red of the spelling checker, `170 0 0` being the red of an
   * error of `clangd`, so the color tells the two underlines apart
   */
  static QStringList underlinedWords(tfel::gui::TextEditBase &frame);

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void theProofreaderIsStoredAndForgotten();
  void theProofreaderDecidesWhatTheBuffersDo();
  void theMenuOffersTheFourChoicesAndTheirLanguages();
  void theHarperSettingsSayWhatIsStoredAndNothingElse();
  void theVarietiesOfferedAreTheOnesTheCheckerReads();
  void theLanguageToolSettingsSayWhatIsStoredAndNothingElse();
  void aLanguageToolRequestAsksAboutTheTextItWasGiven();
  void aLanguageToolAnswerIsDrawnWhereTheBufferHoldsTheWords();
  void aReportAboutTheBlanksOfAModeIsNotADetailOfTheBuffer();
  // the three cases below ask a real `harper-ls`: the positive one comes first,
  // which is what grounds the time the two others wait for an answer
  void aBufferIsReadByTheProofreaderTheEditorIsToldToUse();
  void theWordsOfACleanBufferAreLeftAlone();
  void stoppingTheProofreaderTakesTheUnderlinesAway();
  void aLanguageOfTheCheckerChangesWhatIsFound();
  void aLanguageToolServerUnderlinesTheTextOfABuffer();
  // the last case: it loads packaged modes, which stay registered for the rest
  // of the run
  void theModesOfTheTextBuffersFollowTheChoice();

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
  /*!
   * \brief the menu of the four choices, as the `Options` menu of the editor
   * builds it
   *
   * \note this is the one public door to the entries: they are built by \c
   * SpellingAndGrammar::addToMenu, which is what \c MainWindow::updateOptionsMenu
   * and \c GrammarCheckSession::addToMenu both call
   */
  static QMenu *choicesMenu(tfel::gui::EditorWidget &, const bool prose = true);
  //! the entry of a choice in a menu built by \c choicesMenu
  static QAction *choiceEntry(QMenu &menu, const Checker);
  /*!
   * \brief the submenu of languages in a menu built by \c choicesMenu
   *
   * \note the submenu is filled when it is opened, so a test which asks what it
   * offers emits \c aboutToShow first: that is the moment the menu itself reads
   * the settings, and the leaves it did not build yet exist only after it
   */
  static QMenu *languageSubmenu(QMenu &menu);
  /*!
   * \brief the number of suggestions the context menu offers for a word, 0 when
   * it offers none, or -1 when the word is missing from the buffer
   *
   * \note the suggestions of the context menu are the one public face of hunspell
   * a test can see: the underlines of the misspelled words are drawn by the
   * highlighter of the mode and held by the document
   *
   * \note one entry named `Suggestions` leads to a submenu of the suggestions
   * themselves: it is the entries of that submenu which are counted here, not
   * the entries of the menu
   */
  static int suggestionsFor(tfel::gui::MajorMode &mode,
                            tfel::gui::TextEditBase &frame,
                            const QString &word);
  //! spin the event loop until the predicate holds or the time is up
  static bool waitFor(const std::function<bool()> &predicate, const int ms);
  //! the directory the files of the cases are written in
  QTemporaryDir directory;
  //! whether a `harper-ls` is to be launched at all on this machine
  bool harper_available = false;
  //! whether a LanguageTool server is to be started at all on this machine
  bool tool_available = false;
};  // end of struct TestGrammarCheck

void TestGrammarCheck::isolateSettings() {
  // an own configuration file: the cases write the addresses the checkers read
  // and must leave the configuration of the user alone
  QApplication::setOrganizationName("tfel-editor-tests");
  QApplication::setApplicationName("test-grammar-check");
  auto settings = QSettings();
  settings.remove("spelling/checker");
  settings.remove("spelling/language");
  settings.remove("hunspell default dictionary");
  settings.remove("harper/executable");
  settings.remove("harper/dialect");
  settings.remove("language-tool/java-runtime");
  settings.remove("language-tool/installation");
  settings.remove("language-tool/language");
  settings.remove("language-tool/port");
  settings.remove("language-tool/java-options");
  settings.remove("language-tool/enabled-categories");
  settings.remove("language-tool/disabled-categories");
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

QMenu *TestGrammarCheck::choicesMenu(tfel::gui::EditorWidget &editor,
                                     const bool prose) {
  // the menu is built as the `Options` menu builds it, and it is rebuilt for
  // every buffer: the entries are the answer of the settings at that moment, so
  // a test which changed the choice builds a menu again rather than trusting the
  // entries it held before
  auto *const menu = new QMenu();
  tfel::gui::SpellingAndGrammar::addToMenu(*menu, editor, prose);
  return menu;
}  // end of TestGrammarCheck::choicesMenu

QAction *TestGrammarCheck::choiceEntry(QMenu &menu, const Checker c) {
  const auto label = tfel::gui::SpellingAndGrammar::labelOf(c);
  for (auto *const a : menu.actions()) {
    if (a->text().startsWith(label)) {
      return a;
    }
  }
  return nullptr;
}  // end of TestGrammarCheck::choiceEntry

QMenu *TestGrammarCheck::languageSubmenu(QMenu &menu) {
  return menu.findChild<QMenu *>(QStringLiteral("spelling-languages"));
}  // end of TestGrammarCheck::languageSubmenu

int TestGrammarCheck::suggestionsFor(tfel::gui::MajorMode &mode,
                                     tfel::gui::TextEditBase &frame,
                                     const QString &word) {
  auto cursor = frame.document()->find(word);
  if (cursor.isNull()) {
    return -1;
  }
  // the middle of the word: the suggestions are asked about the word the position
  // stands in, and a place between the first letter and the one which follows it
  // is in the word wherever the mouse was
  cursor.setPosition(cursor.selectionStart() + 3);
  auto menu = QMenu();
  mode.completeContextMenu(&menu, cursor);
  // what the dictionary answered is what the submenu of the `Suggestions` entry
  // holds; a menu which offers no such entry says the word is spelled well
  auto *const suggestions =
      menu.findChild<QMenu *>(QStringLiteral("spell-suggestions"));
  if (suggestions == nullptr) {
    return 0;
  }
  return static_cast<int>(suggestions->actions().size());
}  // end of TestGrammarCheck::suggestionsFor

bool TestGrammarCheck::waitFor(const std::function<bool()> &predicate,
                               const int ms) {
  auto timer = QElapsedTimer();
  timer.start();
  while (!predicate() && (timer.elapsed() < ms)) {
    QTest::qWait(50);
  }
  return predicate();
}  // end of TestGrammarCheck::waitFor

void TestGrammarCheck::initTestCase() {
  isolateSettings();
  QVERIFY(directory.isValid());
  this->harper_available =
      tfel::gui::SpellingAndGrammar::isAvailable(Checker::HunspellHarper);
  // the archive is a download: name the installation this build was configured
  // with, which is what the cases which ask the server anything need
  const auto installation = languageToolInstallation();
  if (!installation.isEmpty()) {
    QSettings().setValue("language-tool/installation", installation);
  }
  this->tool_available =
      tfel::gui::SpellingAndGrammar::isAvailable(Checker::LanguageTool);
}  // end of TestGrammarCheck::initTestCase

void TestGrammarCheck::cleanupTestCase() {
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  isolateSettings();
}  // end of TestGrammarCheck::cleanupTestCase

void TestGrammarCheck::theProofreaderIsStoredAndForgotten() {
  // what a freshly installed editor does: the words are looked up in a dictionary
  // and no sentence is read by anybody, which is what the editor did before a
  // checker of agreements existed
  QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(), Checker::Hunspell);
  QVERIFY2(!QSettings().contains("spelling/checker"),
           "the default of the proofreader is written to the configuration "
           "file");
  // the four choices are named, read back and shown by the same words: the menu,
  // the configuration dialog and the file agree because one list is what all of
  // them are built from
  QCOMPARE(tfel::gui::SpellingAndGrammar::names(),
           QStringList() << "none"
                        << "hunspell"
                        << "hunspell-harper"
                        << "language-tool");
  for (const auto &name : tfel::gui::SpellingAndGrammar::names()) {
    const auto c = tfel::gui::SpellingAndGrammar::checkerOf(name);
    QCOMPARE(tfel::gui::SpellingAndGrammar::nameOf(c), name);
    QVERIFY2(!tfel::gui::SpellingAndGrammar::labelOf(c).isEmpty(),
             qPrintable("the choice '" + name + "' is shown by nothing"));
  }
  // a name which is none of the choices is no choice at all, and the answer of
  // the setting is then what the editor does by itself: a hand-edited file can
  // not leave the editor reading the buffers with a program nobody named
  QCOMPARE(tfel::gui::SpellingAndGrammar::checkerOf(QStringLiteral("picky")),
           Checker::Hunspell);
  QSettings().setValue("spelling/checker", QStringLiteral("nonsense"));
  QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(), Checker::Hunspell);
  // a choice is stored, and the choice which is what the editor would have done
  // anyway is forgotten: the configuration file holds what was chosen, as every
  // other entry of the editor does
  tfel::gui::SpellingAndGrammar::setChecker(Checker::LanguageTool);
  QCOMPARE(QSettings().value("spelling/checker").toString(),
           QStringLiteral("language-tool"));
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  QVERIFY(!QSettings().contains("spelling/checker"));
  // and the three questions the choices answer are what they say they answer:
  // the words, the sentences, or nothing at all
  QVERIFY(!tfel::gui::SpellingAndGrammar::usesHunspell(Checker::None));
  QVERIFY(!tfel::gui::SpellingAndGrammar::usesHarper(Checker::None));
  QVERIFY(tfel::gui::SpellingAndGrammar::usesHunspell(Checker::Hunspell));
  QVERIFY(!tfel::gui::SpellingAndGrammar::usesHarper(Checker::Hunspell));
  QVERIFY(tfel::gui::SpellingAndGrammar::usesHunspell(Checker::HunspellHarper));
  QVERIFY(tfel::gui::SpellingAndGrammar::usesHarper(Checker::HunspellHarper));
  QVERIFY(!tfel::gui::SpellingAndGrammar::usesHunspell(Checker::LanguageTool));
  QVERIFY(tfel::gui::SpellingAndGrammar::usesLanguageTool(
      Checker::LanguageTool));
  // `None` is offered wherever the editor was built: it asks nothing of anybody.
  // The two programs are offered where they are, which the machine decides
  QVERIFY(tfel::gui::SpellingAndGrammar::isAvailable(Checker::None));
  QCOMPARE(tfel::gui::SpellingAndGrammar::isAvailable(Checker::HunspellHarper),
           this->harper_available);
  QCOMPARE(tfel::gui::SpellingAndGrammar::isAvailable(Checker::LanguageTool),
           this->tool_available);
}  // end of TestGrammarCheck::theProofreaderIsStoredAndForgotten

void TestGrammarCheck::theProofreaderDecidesWhatTheBuffersDo() {
  // the off switch of the spell checker is an empty language, which is what a
  // mode reads as 'no dictionary': the two choices which name no hunspell get
  // that empty language, and the two which do get the dictionary the setting
  // names
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  QSettings().setValue("hunspell default dictionary", QStringLiteral("en_GB"));
  QCOMPARE(tfel::gui::SpellingAndGrammar::spellCheckLanguage(Checker::Hunspell),
           QStringLiteral("en_GB"));
  QCOMPARE(tfel::gui::SpellingAndGrammar::spellCheckLanguage(
               Checker::HunspellHarper),
           QStringLiteral("en_GB"));
  QVERIFY(tfel::gui::SpellingAndGrammar::spellCheckLanguage(Checker::None)
              .isEmpty());
  QVERIFY(tfel::gui::SpellingAndGrammar::spellCheckLanguage(
              Checker::LanguageTool)
              .isEmpty());
  // and the language a buffer is actually given is that of the choice in use
  tfel::gui::SpellingAndGrammar::setChecker(Checker::LanguageTool);
  QVERIFY(tfel::gui::SpellingAndGrammar::spellCheckLanguage().isEmpty());
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  QCOMPARE(tfel::gui::SpellingAndGrammar::spellCheckLanguage(),
           QStringLiteral("en_GB"));
  QSettings().remove("hunspell default dictionary");
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  // what a source file is read with is not what a document of prose is read
  // with, and the difference is the one thing the choice can not say on its own:
  // LanguageTool is handed a text and answers what is wrong with it, and every
  // identifier of a source file is a typo to it. So the choice of LanguageTool
  // means the dictionary for a source file and the whole proofreader for a text
  const auto source = tfel::gui::SpellingAndGrammar::effectiveChecker(false);
  const auto prose = tfel::gui::SpellingAndGrammar::effectiveChecker(true);
  QCOMPARE(source, Checker::Hunspell);
  QCOMPARE(prose, Checker::Hunspell);
  tfel::gui::SpellingAndGrammar::setChecker(Checker::LanguageTool);
  // the words of a comment are words of a language, and a checker of sentences
  // which is not given the comments apart from the code has no way of telling
  // them apart: the dictionary is what a source file is read with
  QCOMPARE(tfel::gui::SpellingAndGrammar::effectiveChecker(false),
           Checker::Hunspell);
  // while the document of prose gets the proofreader that was asked for
  QCOMPARE(tfel::gui::SpellingAndGrammar::effectiveChecker(true),
           Checker::LanguageTool);
  // the two other choices read both kinds of buffer as they say they do: a
  // checker of sentences which reads the comments of a source file is a checker
  // which parses the file and finds the comments itself
  tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
  QCOMPARE(tfel::gui::SpellingAndGrammar::effectiveChecker(false),
           Checker::HunspellHarper);
  QCOMPARE(tfel::gui::SpellingAndGrammar::effectiveChecker(true),
           Checker::HunspellHarper);
  tfel::gui::SpellingAndGrammar::setChecker(Checker::None);
  QCOMPARE(tfel::gui::SpellingAndGrammar::effectiveChecker(true), Checker::None);
  QCOMPARE(tfel::gui::SpellingAndGrammar::effectiveChecker(false),
           Checker::None);
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
}  // end of TestGrammarCheck::theProofreaderDecidesWhatTheBuffersDo

void TestGrammarCheck::theMenuOffersTheFourChoicesAndTheirLanguages() {
  // the menu is the surface of the whole feature: the four choices, the language
  // of the choice in use, and the fact that clicking an entry picks that choice.
  // It is built as the `Options` menu of the editor builds it, on an editor which
  // holds a buffer to say what kind of buffer the entries stand for
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->createEmptyBuffer();
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  auto *const menu = choicesMenu(*editor, true);
  // the entries a machine can not run are not offered: `None` is, always
  auto *const none = choiceEntry(*menu, Checker::None);
  QVERIFY(none != nullptr);
  const auto offered = [menu](const Checker c) {
    return choiceEntry(*menu, c) != nullptr;
  };
  QCOMPARE(offered(Checker::Hunspell),
           tfel::gui::SpellingAndGrammar::isAvailable(Checker::Hunspell));
  QCOMPARE(offered(Checker::HunspellHarper), this->harper_available);
  QCOMPARE(offered(Checker::LanguageTool), this->tool_available);
  // no entry of a choice hangs a submenu off itself: an entry which carries a
  // submenu is *opened* by a click and never triggered, so a choice presented
  // that way can not be chosen at all — which is exactly what a person clicking
  // `LanguageTool` used to experience as a menu doing nothing
  for (const auto c : {Checker::None, Checker::Hunspell, Checker::HunspellHarper,
                       Checker::LanguageTool}) {
    auto *const e = choiceEntry(*menu, c);
    if (e != nullptr) {
      QVERIFY2(e->isCheckable(),
               qPrintable("the entry '" + e->text() + "' names no state"));
      QVERIFY2(e->menu() == nullptr,
               qPrintable("the entry '" + e->text() +
                          "' is a submenu header and can not be clicked"));
    }
  }
  // exactly one entry is ticked, and it is the one the setting names: a person
  // who looks at the menu has to see what the editor does now
  auto checked = QStringList();
  for (const auto c : {Checker::None, Checker::Hunspell, Checker::HunspellHarper,
                       Checker::LanguageTool}) {
    auto *const e = choiceEntry(*menu, c);
    if ((e != nullptr) && e->isChecked()) {
      checked << tfel::gui::SpellingAndGrammar::nameOf(c);
    }
  }
  QCOMPARE(checked,
           QStringList()
               << tfel::gui::SpellingAndGrammar::nameOf(
                      tfel::gui::SpellingAndGrammar::currentChecker()));
  // the languages stand in one submenu below the choices, which names those of
  // the choice in use, and is filled when it opens: a test asks it to open
  auto *const languages = languageSubmenu(*menu);
  QVERIFY(languages != nullptr);
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  // while `Hunspell` is the choice in use, the languages are the installed
  // dictionaries named as they name themselves
  Q_EMIT languages->aboutToShow();
  const auto dictionaries =
      tfel::gui::HunspellDictionariesManager::getHunspellDictionariesManager()
          .getAvailableDictionnaries();
  if (!dictionaries.isEmpty()) {
    auto names = QStringList();
    for (auto *const a : languages->actions()) {
      names << a->text();
    }
    QCOMPARE(names, dictionaries);
  }
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  // `None` asks for nothing at all — no dictionary and no agreement — and its
  // entry, which is now one that can be clicked, writes the choice and reports
  // it in the minibuffer, so that a person who clicked an entry of a menu knows
  // the editor did something
  none->trigger();
  QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(), Checker::None);
  QCOMPARE(QSettings().value("spelling/checker").toString(),
           QStringLiteral("none"));
  const auto report_of_none = QStringLiteral("checking with %1").arg(
      tfel::gui::SpellingAndGrammar::labelOf(Checker::None).toLower());
  QVERIFY2(editor->messages.contains(report_of_none),
           qPrintable("the choice of no check is reported as: " +
                      editor->messages.join(QStringLiteral(" | "))));
  // the languages of the choice which names no program: the submenu says why it
  // has none rather than opening on nothing
  Q_EMIT languages->aboutToShow();
  QVERIFY2(languages->actions().size() == 1,
           "the submenu of languages opens on more than a refusal");
  QVERIFY(!languages->actions().first()->isEnabled());
  // clicking the entry of a program is what picks it: the complaint this case
  // answers is a `LanguageTool` entry which left the choice untouched
  auto *const tool_entry = choiceEntry(*menu, Checker::LanguageTool);
  if (this->tool_available) {
    QVERIFY(tool_entry != nullptr);
    tool_entry->trigger();
    QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(),
             Checker::LanguageTool);
    // and the submenu follows the choice made after the menu was built: it
    // names the codes LanguageTool reads now, not the refusal it showed while
    // `None` was the choice
    Q_EMIT languages->aboutToShow();
    auto names = QStringList();
    for (auto *const a : languages->actions()) {
      QVERIFY2(a->isCheckable(),
               qPrintable("the language '" + a->text() + "' names no state"));
      names << a->text();
    }
    auto expected = QStringList();
    for (const auto &code : tfel::gui::LanguageToolSupport::languages()) {
      expected << tfel::gui::LanguageToolSupport::languageLabel(code);
    }
    QCOMPARE(names, expected);
  }
  if (this->harper_available) {
    auto *const harper = choiceEntry(*menu, Checker::HunspellHarper);
    QVERIFY(harper != nullptr);
    harper->trigger();
    QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(),
             Checker::HunspellHarper);
    Q_EMIT languages->aboutToShow();
    auto names = QStringList();
    for (auto *const a : languages->actions()) {
      names << a->text();
      QVERIFY2(a->isCheckable(),
               qPrintable("the language '" + a->text() + "' names no state"));
    }
    auto expected = QStringList();
    for (const auto &name : tfel::gui::LspHarperSupport::dialects()) {
      expected << tfel::gui::LspHarperSupport::dialectLabel(name);
    }
    QCOMPARE(names, expected);
    // the variety the checker reads a text by is the one ticked
    QVERIFY(
        languages
            ->actions()
            .at(tfel::gui::LspHarperSupport::dialects().indexOf(
                tfel::gui::LspHarperSupport::dialect()))
            ->isChecked());
    // British English, chosen here, is British English for every proofreader:
    // the three programs spell a variety three different ways and one buffer is
    // read by one of them, so the language of a choice is the language of the
    // editor. The leaf of the submenu writes the choice it names along with the
    // variety, which is what keeps a stored language from reaching a buffer of
    // another program
    auto *const british = languages->actions().at(3);
    QCOMPARE(british->text(), QStringLiteral("British English"));
    british->trigger();
    QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(),
             Checker::HunspellHarper);
    QCOMPARE(tfel::gui::LspHarperSupport::dialect(),
             QStringLiteral("British"));
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
    // and the dictionary of the choice which was not chosen follows the same
    // variety: the underlines of the words of a buffer read by another checker
    // are asked in the same English
    QCOMPARE(QSettings().value("hunspell default dictionary").toString(),
             QStringLiteral("en_GB"));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
    // a menu built afterwards ticks what the setting chose, which is what keeps
    // the menus of the buffers on screen from disagreeing with each other
    auto *const again = choicesMenu(*editor, true);
    auto *const again_harper = choiceEntry(*again, Checker::HunspellHarper);
    QVERIFY((again_harper != nullptr) && again_harper->isChecked());
    auto *const again_languages = languageSubmenu(*again);
    QVERIFY(again_languages != nullptr);
    Q_EMIT again_languages->aboutToShow();
    QVERIFY(again_languages->actions().at(3)->isChecked());
    QVERIFY(!again_languages->actions().at(0)->isChecked());
    delete again;
    tfel::gui::LspHarperSupport::setDialect(QStringLiteral("American"));
    QSettings().remove("hunspell default dictionary");
  }
  // the entry of LanguageTool of a source buffer says what it reads there, which
  // is not the whole of that buffer, and says the same again in the minibuffer
  // when clicked: the buffer of a program is read by the dictionary alone, and a
  // message which promised the sentences of the checker there would be a lie
  // about underlines which no buffer on the screen ever gets
  auto *const code_menu = choicesMenu(*editor, false);
  auto *const tool = choiceEntry(*code_menu, Checker::LanguageTool);
  if (this->tool_available) {
    QVERIFY(tool != nullptr);
    QVERIFY2(tool->text().contains(QStringLiteral("text files")),
             qPrintable("the entry of a source buffer does not say which "
                        "buffers it reads: '" +
                        tool->text() + "'"));
    tool->trigger();
    QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(),
             Checker::LanguageTool);
    QVERIFY2(editor->messages.contains(
                 QStringLiteral(
                     "LanguageTool reads no code: this buffer is checked "
                     "for spelling only")),
             qPrintable("LanguageTool chosen in a source buffer is reported "
                        "as: " +
                        editor->messages.join(QStringLiteral(" | "))));
  } else {
    QVERIFY2(tool == nullptr,
             "a menu offers LanguageTool where no server can be started");
  }
  delete code_menu;
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  QVERIFY(!QSettings().contains("spelling/checker"));
  delete menu;
  editor.reset();
}  // end of TestGrammarCheck::theMenuOffersTheFourChoicesAndTheirLanguages

void TestGrammarCheck::theHarperSettingsSayWhatIsStoredAndNothingElse() {
  // what a freshly installed editor answers: the program looked for in the
  // directories of the session, and the English of the checker itself
  QVERIFY2(!tfel::gui::LspHarperSupport::serverExecutable().isEmpty() ||
               !this->harper_available,
           "the program is missing and the checker says it is there");
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("American"));
  // a session speaks over the pipes the client writes to: without that argument
  // the server waits on a local socket and reads nothing of the buffers
  QCOMPARE(tfel::gui::LspHarperSupport::serverArguments(),
           QStringList() << QStringLiteral("--stdio"));
  // the settings a session is answered with live under the name of the server,
  // which is where it reads them and where it says it reads them; no other key is
  // sent, and the variety is the one the setting names
  tfel::gui::LspHarperSupport::setDialect(QStringLiteral("British"));
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("British"));
  const auto settings = tfel::gui::LspHarperSupport::configuration();
  QVERIFY2(settings.isObject(),
           "the settings a session answers with are not an object held under the "
           "name of the server");
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
}  // end of
   // TestGrammarCheck::theHarperSettingsSayWhatIsStoredAndNothingElse

void TestGrammarCheck::theVarietiesOfferedAreTheOnesTheCheckerReads() {
  const auto dialects = tfel::gui::LspHarperSupport::dialects();
  // the varieties of the server, the first of them its own default, which is
  // what the entry of the configuration and the submenu of a menu both offer
  QCOMPARE(dialects, QStringList() << "American" << "Canadian" << "Australian"
                                   << "British" << "Indian");
  QCOMPARE(tfel::gui::LspHarperSupport::dialectLabel(QStringLiteral("American")),
           QStringLiteral("American English"));
  QCOMPARE(tfel::gui::LspHarperSupport::dialectLabel(QStringLiteral("British")),
           QStringLiteral("British English"));
  // the varieties spell a word three different ways, and the submenu of the
  // hunspell choice is the dictionaries of the machine while the submenu of this
  // one is the five varieties above: the names of the languages of the checker
  // are of no use for the choice of hunspell, so the language of a checker is
  // kept with that checker and translated where the two have a word in common
  QCOMPARE(tfel::gui::SpellingAndGrammar::languageOf(Checker::HunspellHarper),
           tfel::gui::LspHarperSupport::dialect());
  tfel::gui::SpellingAndGrammar::setLanguage(Checker::HunspellHarper,
                                             QStringLiteral("Canadian"));
  QCOMPARE(tfel::gui::LspHarperSupport::dialect(), QStringLiteral("Canadian"));
  QCOMPARE(tfel::gui::SpellingAndGrammar::languageOf(Checker::HunspellHarper),
           QStringLiteral("Canadian"));
  tfel::gui::SpellingAndGrammar::setLanguage(Checker::HunspellHarper,
                                             QStringLiteral("American"));
  // and the choice itself is untouched by choosing a language: what the buffers are
  // read with is only ever written by the entries which choose it
  QVERIFY(!QSettings().contains("spelling/checker"));
  QSettings().remove("harper/dialect");
  QSettings().remove("hunspell default dictionary");
}  // end of
   // TestGrammarCheck::theVarietiesOfferedAreTheOnesTheCheckerReads

void TestGrammarCheck::theLanguageToolSettingsSayWhatIsStoredAndNothingElse() {
  if (languageToolInstallation().isEmpty()) {
    QSKIP(
        "no LanguageTool installation is named by this build or found on the "
        "machine: the entries of that checker have nothing to say");
  }
  // the server is a Java program: the runtime which starts it is looked for in
  // the directories of the session, then in JAVA_HOME, and the archive where the
  // program is installed is what a person has to name, since the program is a
  // download and nothing in a session holds it
  QVERIFY2(!tfel::gui::LanguageToolSupport::javaRuntime().isEmpty(),
           "no Java runtime is to be found: the cases which ask the server "
           "anything are about to be skipped anyway");
  QVERIFY(tfel::gui::LanguageToolSupport::serverJar().endsWith(
      QStringLiteral(".jar")));
  // the class path and the class of the server: started without `--public`, which
  // is what keeps the port to the machine the editor runs on
  const auto arguments = tfel::gui::LanguageToolSupport::serverArguments(
      QStringLiteral("/opt/languagetool/languagetool-server.jar"), 8081);
  QVERIFY2(arguments.contains(QStringLiteral("-cp")),
           "the class path of the server is not given to the runtime");
  QVERIFY2(arguments.contains(
               QStringLiteral("/opt/languagetool/languagetool-server.jar")),
           "the archive named is not the one on the class path");
  QVERIFY2(
      arguments.contains(QStringLiteral("org.languagetool.server.HTTPServer")),
      "the class of the server of the HTTP API is not the one started");
  QVERIFY2(arguments.contains(QStringLiteral("--port")),
           "the server is started with no port to listen on");
  QVERIFY2(!arguments.contains(QStringLiteral("--public")),
           "the server is started reachable from the whole network");
  // the answer of `port` is what the setting says, and nothing more: 0 is the
  // entry which says 'take a free one', and the free port is picked by the
  // client which starts the server, see `LanguageToolClient::startServer`. A
  // port of 0 is what lets two editors of this build sit beside each other on
  // one machine, each with a server of its own on a port nobody else holds
  QCOMPARE(QSettings().value("language-tool/port").toInt(), 0);
  QCOMPARE(tfel::gui::LanguageToolSupport::port(), 0);
  QSettings().setValue("language-tool/port", 8082);
  QCOMPARE(tfel::gui::LanguageToolSupport::port(), 8082);
  QSettings().remove("language-tool/port");
  // the language the text is read as: the code the server's own `language`
  // parameter reads, "en-US" when nothing was chosen
  QCOMPARE(tfel::gui::LanguageToolSupport::language(),
           QStringLiteral("en-US"));
  tfel::gui::LanguageToolSupport::setLanguage(QStringLiteral("fr"));
  QCOMPARE(tfel::gui::LanguageToolSupport::language(), QStringLiteral("fr"));
  QCOMPARE(QSettings().value("language-tool/language").toString(),
           QStringLiteral("fr"));
  tfel::gui::LanguageToolSupport::setLanguage(QStringLiteral("en-US"));
  QVERIFY2(!QSettings().contains("language-tool/language"),
           "the default language of the server is written to the file");
  // a code nobody knows is no choice at all: the entry of the server would be
  // answered with a plain-text error on every check
  tfel::gui::LanguageToolSupport::setLanguage(QStringLiteral("xx-XX"));
  QCOMPARE(tfel::gui::LanguageToolSupport::language(),
           QStringLiteral("en-US"));
  // the lists of categories are empty by default, which leaves the editor no
  // opinion of its own about what the program considers an error
  QVERIFY(tfel::gui::LanguageToolSupport::enabledCategories().isEmpty());
  QVERIFY(tfel::gui::LanguageToolSupport::disabledCategories().isEmpty());
  // the languages the menus offer: what the server detects on its own, the
  // varieties of English, then the other languages
  const auto codes = tfel::gui::LanguageToolSupport::languages();
  QVERIFY2(codes.contains(QStringLiteral("auto")),
           "the menus offer no automatic detection of the language");
  QVERIFY2(codes.contains(QStringLiteral("en-US")),
           "the menus offer no American English");
  QVERIFY2(codes.contains(QStringLiteral("fr")), "the menus offer no French");
  QCOMPARE(tfel::gui::LanguageToolSupport::languageLabel(
               QStringLiteral("en-US")),
           QStringLiteral("American English"));
  QCOMPARE(tfel::gui::LanguageToolSupport::languageLabel(QStringLiteral("fr")),
           QStringLiteral("French"));
  // the same variety of English, spelled the way the other two programs spell it:
  // choosing French here says French to the dictionaries of hunspell too
  tfel::gui::SpellingAndGrammar::setLanguage(Checker::LanguageTool,
                                             QStringLiteral("fr"));
  QCOMPARE(tfel::gui::LanguageToolSupport::language(), QStringLiteral("fr"));
  tfel::gui::LanguageToolSupport::setLanguage(QStringLiteral("en-US"));
}  // end of
   // TestGrammarCheck::theLanguageToolSettingsSayWhatIsStoredAndNothingElse

void TestGrammarCheck::aLanguageToolRequestAsksAboutTheTextItWasGiven() {
  // the request is form encoded, and the text is sent for what it is: the offsets
  // of the answer are places in the text which was sent, so no trimming and no
  // rewrapping is what keeps an answer standing where the buffer holds the words
  const auto text = QStringLiteral("This is an tests.\nSecond line.\n");
  const auto body = tfel::gui::LanguageToolSupport::requestBody(
      text, QStringLiteral("en-US"), QStringList(), QStringList());
  // the very bytes the server is handed: what `curl --data-urlencode` writes for
  // the same text, which is the reading the API documents and the one the offsets
  // of the answer are sure about. `+` for a space is the other reading of the
  // content type, and a text of newlines is not worth betting on it
  QCOMPARE(body,
           QByteArray("language=en-US&text=") +
               QUrl::toPercentEncoding(text, {}, "/"));
  // the body is read back fully decoded: `QUrlQuery` decodes an item with
  // `PrettyDecoded` by default, which leaves a `%0A` of the escaped text standing
  // for the newline it came from and answers the line break as its own name
  const auto parsed = QUrlQuery(QString::fromUtf8(body));
  QCOMPARE(parsed.queryItemValue("language", QUrl::FullyDecoded),
           QStringLiteral("en-US"));
  QCOMPARE(parsed.queryItemValue("text", QUrl::FullyDecoded), text);
  // the two lists of categories are named only when they name anything: an empty
  // one is a category the server is asked to enable none of, and the defaults of
  // the program are not the editor's business
  QVERIFY2(!parsed.hasQueryItem("enabledCategories"),
           "an empty list of enabled categories is sent to the server");
  QVERIFY2(!parsed.hasQueryItem("disabledCategories"),
           "an empty list of disabled categories is sent to the server");
  const auto with_lists = tfel::gui::LanguageToolSupport::requestBody(
      text, QStringLiteral("auto"), QStringList() << "TYPOGRAPHY",
      QStringList() << "TYPOS"
                   << "STYLE");
  const auto parsed_lists = QUrlQuery(QString::fromUtf8(with_lists));
  QCOMPARE(parsed_lists.queryItemValue("language"), QStringLiteral("auto"));
  QCOMPARE(parsed_lists.queryItemValue("enabledCategories"),
           QStringLiteral("TYPOGRAPHY"));
  QCOMPARE(parsed_lists.queryItemValue("disabledCategories"),
           QStringLiteral("TYPOS,STYLE"));
  // a text which is a whole paragraph of a document keeps its line breaks: the
  // checker reads a paragraph as one thought and a hard break is part of the text
  QVERIFY(body.contains("%0A"));
}  // end of
   // TestGrammarCheck::aLanguageToolRequestAsksAboutTheTextItWasGiven

void TestGrammarCheck::aLanguageToolAnswerIsDrawnWhereTheBufferHoldsTheWords() {
  // an answer of the server, cut from what the HTTP API answers: the offsets are
  // places in the text which was sent, in UTF-16 units, which is what the
  // coordinates of a `QString` are — so an offset of the answer is an offset of
  // the text with no conversion at all, and the case below is what says so
  const auto text =
      QStringLiteral("// This is an tests of the grammar.\n"
                     "// The variable is a plain line of the buffer.\n");
  const auto before = text.indexOf(QStringLiteral("an tests"));
  const auto matches = QJsonArray{
      QJsonObject{
          {"offset", before},
          {"length", 2},
          {"message", "Use \"an\" before a plural noun that starts with a "
                      "consonant sound."},
          {"shortMessage", QJsonValue::Undefined},
          {"replacements",
           QJsonArray{QJsonObject{{"value", "a"}, {"weight", 1}}}},
          {"context", QJsonObject{{"offset", before}, {"length", 8},
                                  {"text", "This is an tests of"}}},
          {"rule", QJsonObject{{"id", "EN_A_VS_AN"},
                               {"category",
                                QJsonObject{{"id", "COMMON_MISTAKES"}}}}},
      },
  };
  const auto diagnostics = tfel::gui::LanguageToolSupport::diagnosticsOf(
      matches, text, 0);
  QCOMPARE(static_cast<int>(diagnostics.size()), 1);
  // what the person is told, and what the tooltip of the underline repeats, is the
  // sentence of the server and the name of the rule it applied
  QVERIFY2(diagnostics[0].message.contains(QStringLiteral("an")),
           qPrintable("the message of the lint is: " +
                      diagnostics[0].message));
  QCOMPARE(diagnostics[0].code, QStringLiteral("EN_A_VS_AN"));
  // the range of a lint is in the coordinates the protocol counts: the line of
  // the text the words stand on, and the place on that line, which is where the
  // underline is drawn and where the mouse has to rest to read the sentence
  QCOMPARE(diagnostics[0].startLine, 0);
  QVERIFY(diagnostics[0].startCharacter > 0);
  QVERIFY(diagnostics[0].endCharacter > diagnostics[0].startCharacter);
  // and the second line of the buffer, which the answer says nothing about, is
  // answered no range at all: a lint which moved onto a clean line would underline
  // a word the server never named
  QVERIFY(diagnostics[0].endLine == 0);
  // a text which is a piece of the buffer, as a long buffer is cut into pieces,
  // has its answers put back where the whole text holds them
  const auto piece = text.mid(before);
  const auto pieces = tfel::gui::LanguageToolSupport::diagnosticsOf(
      QJsonArray{QJsonObject{{"offset", 0},
                             {"length", 2},
                             {"message", "Article"},
                             {"replacements", QJsonArray{}},
                             {"context", QJsonObject{{"offset", 0},
                                                     {"length", 8},
                                                     {"text", "an tests of"}}},
                             {"rule", QJsonObject{{"id", "EN_A_VS_AN"}}}}},
      piece, before);
  QCOMPARE(static_cast<int>(pieces.size()), 1);
  // the answer of the piece names the same place of the buffer as the answer of
  // the whole text: a long buffer cut into pieces is underlined where the words
  // it holds them, not where the piece holds them
  QCOMPARE(pieces[0].startLine, diagnostics[0].startLine);
  QCOMPARE(pieces[0].startCharacter, diagnostics[0].startCharacter);
  // what an answer which is no JSON says of itself: a wrong language code is
  // answered as plain text by the server, and that sentence is what the person
  // who chose the code needs to read in the minibuffer
  const auto error = tfel::gui::LanguageToolSupport::errorOf(
      "Error: 'xx-XX' is not a language code known to LanguageTool.\n");
  QVERIFY2(error.contains(QStringLiteral("language code")),
           qPrintable("the error of the server is reported as: " + error));
  // an answer of several lines is reported by its first one, which is what the
  // minibuffer holds: the rest of the sentence of the server is longer than the
  // line it is displayed on
  const auto long_error = tfel::gui::LanguageToolSupport::errorOf(
      "Error: bad request\nThe set of rules is named in the documentation\n");
  QVERIFY2(long_error == QStringLiteral("Error: bad request"),
           qPrintable("a several-line error is reported as: " + long_error));
}  // end of
   // TestGrammarCheck::aLanguageToolAnswerIsDrawnWhereTheBufferHoldsTheWords

void TestGrammarCheck::aReportAboutTheBlanksOfAModeIsNotADetailOfTheBuffer() {
  // what the checker is handed for a LaTeX buffer whose markup was blanked out:
  // the markup becomes spaces of the same length, so the answer stands where the
  // buffer holds the words, see LaTeXMajorMode::grammarCheckText. The two lines
  // which lead the text are the first line of that answer, verbatim
  const auto text = QStringLiteral(
      "        {A title}\n"
      "This is an tests of the grammar.\n"
      "        \n");
  const auto article = text.indexOf(QStringLiteral("an tests"));
  const auto trailing = text.lastIndexOf(QStringLiteral("        "));
  QVERIFY(article > 0);
  QVERIFY(trailing > article);
  const auto whitespace = [](const int offset, const int length) {
    return QJsonObject {
        {"offset", offset},
        {"length", length},
        {"message", "Possible typo: you repeated a whitespace"},
        {"replacements", QJsonArray {}},
        {"context", QJsonObject {{"offset", offset}, {"length", length}}},
        {"rule", QJsonObject {{"id", "WHITESPACE_RULE"},
                              {"category", QJsonObject {{"id", "TYPOGRAPHY"}}}}},
    };
  };
  const auto grammar = [&](const int offset) {
    return QJsonObject {
        {"offset", offset},
        {"length", 8},
        {"message", "The plural noun \xE2\x80\x9Ctests\xE2\x80\x9D cannot be "
                    "used with the article \xE2\x80\x9Can\xE2\x80\x9D."},
        {"replacements", QJsonArray {}},
        {"context", QJsonObject {{"offset", offset}, {"length", 8}}},
        {"rule", QJsonObject {{"id", "A_NNS"},
                              {"category", QJsonObject {{"id", "GRAMMAR"}}}}},
    };
  };
  // what the server says of that text, as the live server was verified to say
  // it: the article twice, and a repeated whitespace wherever markup was hidden
  const auto diagnostics = tfel::gui::LanguageToolSupport::diagnosticsOf(
      QJsonArray {whitespace(0, 8), grammar(article), whitespace(trailing, 8)},
      text, 0);
  // The runs of blanks are no reports: what they name is the text the editor
  // wrote to hide a `\section`, and a place of the buffer of which the checker
  // was handed nothing. The article survives, which is the whole of what a
  // person wants of a proofreader
  QCOMPARE(static_cast<int>(diagnostics.size()), 1);
  QCOMPARE(diagnostics[0].code, QStringLiteral("A_NNS"));
  // and the drop is not a cut into the middle of a *piece* of the buffer. A long
  // document is sent in pieces, and the answer of a piece names a place of that
  // piece while the ranges and the words are those of the whole text: the text
  // below is the whole text, whose first two lines are hidden markup, and the
  // answer is about its third line, handed with the place (18) the piece which
  // names it stands at. Read against the piece, that answer names the blanks of
  // the whole text and would be dropped; read against the whole text, it names
  // the article it is about
  const auto whole = QStringLiteral(
      "        \n"
      "        \n"
      "This is an tests of a piece.\n");
  const auto piece = whole.indexOf(QStringLiteral("This"));
  const auto in_a_piece = tfel::gui::LanguageToolSupport::diagnosticsOf(
      QJsonArray {
          grammar(whole.indexOf(QStringLiteral("an tests")) - piece)},
      whole, piece);
  QCOMPARE(static_cast<int>(in_a_piece.size()), 1);
  QCOMPARE(in_a_piece[0].code, QStringLiteral("A_NNS"));
  // and it stands on the line of the whole text the words are drawn on, not on
  // the first line of the piece
  QCOMPARE(in_a_piece[0].startLine, 2);
}  // end of
   // TestGrammarCheck::aReportAboutTheBlanksOfAModeIsNotADetailOfTheBuffer

void TestGrammarCheck::theWordsOfACleanBufferAreLeftAlone() {
  if (!this->harper_available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // a line the checker has no reason to touch, standing beside one it has: an
  // assertion on a buffer which holds nothing to find is an assertion on a session
  // which answered nothing at all, so the clean line is measured against the line
  // which proves the same session reads the same file
  tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
  const auto file = writeFile(
      QStringLiteral("clean.cxx"),
      QStringLiteral("// This is an tests of the grammar.\n"
                     "// The variable is a plain line of the buffer.\n"
                     "int main() { return 0; }\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 30000),
      "the session drew no underline at all: the clean line below is therefore "
      "no evidence of anything");
  const auto words = settledWords(frame);
  QVERIFY2(words.contains(QStringLiteral("an")),
           qPrintable("the dirty line is not underlined: " +
                      words.join(QStringLiteral(", "))));
  // the words of the clean line, and the name of the function of the line of code,
  // are left alone: the list above is what the checker said, and it said nothing of
  // these
  const auto untouched = QStringList() << "variable"
                                       << "plain"
                                       << "line"
                                       << "buffer"
                                       << "main"
                                       << "return";
  for (const auto &underlined : words) {
    QVERIFY2(!untouched.contains(underlined),
             qPrintable("a clean word is underlined: " + underlined));
  }
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  editor.reset();
}  // end of TestGrammarCheck::theWordsOfACleanBufferAreLeftAlone

void TestGrammarCheck::stoppingTheProofreaderTakesTheUnderlinesAway() {
  if (!this->harper_available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // the choice which names no check at all has to be seen on the screen: the
  // person who asks for no underline has to get no underline, which means the
  // lints of a session which stopped are taken off the text and not left there
  // until the buffer is opened again
  tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
  const auto file = writeFile(QStringLiteral("stopped.cxx"), checkedSource());
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 30000),
      "the session started by the choice drew no underline");
  // the session is running, which the state of the choice and the answer of the
  // session both say: a buffer whose checker is asked for is an active buffer
  QCOMPARE(tfel::gui::SpellingAndGrammar::currentChecker(),
           Checker::HunspellHarper);
  // and no choice of the editor is in force alone: the spell checker of the mode
  // follows it, which is what the frame reads as a language to look words up in
  tfel::gui::SpellingAndGrammar::setChecker(Checker::None);
  tfel::gui::SpellingAndGrammar::applyTo(*editor);
  QVERIFY2(
      waitFor([&frame] { return underlinedWords(frame).isEmpty(); }, 10000),
      "the underlines of a stopped session stayed on the buffer");
  QVERIFY2(frame.getSpellCheckLanguage().isEmpty(),
           "the choice of no check left a dictionary on the buffer");
  // and the choice which gives the dictionary back takes the language back with
  // it, on the buffer which is on the screen
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  QSettings().setValue("hunspell default dictionary", QStringLiteral("en_US"));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  tfel::gui::SpellingAndGrammar::applyTo(*editor);
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  QCOMPARE(frame.getSpellCheckLanguage(), QStringLiteral("en_US"));
  QSettings().remove("hunspell default dictionary");
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  editor.reset();
}  // end of TestGrammarCheck::stoppingTheProofreaderTakesTheUnderlinesAway

void TestGrammarCheck::aLanguageOfTheCheckerChangesWhatIsFound() {
  if (!this->harper_available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // the varieties of English disagree about a handful of words, and the checker is
  // asked in the one the person chose: `colour` is the spelling of another variety
  // than the one an editor reads a text by when nothing was chosen, which is what
  // both directions of the case are about
  //
  // a comment of a C++ buffer is chosen over a document of prose because that is
  // the half of the feature which needs no packaged mode loaded first: the checker
  // of sentences reads the comments of a source file by itself
  tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
  const auto file =
      writeFile(QStringLiteral("variety.cxx"),
                QStringLiteral("// The colour of the apple is red.\n"
                               "int main() { return 0; }\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(
      waitFor([&frame] {
        return underlinedWords(frame).contains(QStringLiteral("colour"));
      },
              30000),
      qPrintable("American English finds no fault in `colour`: " +
                 settledWords(frame).join(QStringLiteral(", "))));
  // British English, chosen in the submenu of languages of the choice, reads the
  // same text without saying anything about that word: what changed is an address
  // of the configuration and the texts read again, not a process restarted
  auto *const menu = choicesMenu(*editor, false);
  auto *const languages = languageSubmenu(*menu);
  QVERIFY(languages != nullptr);
  Q_EMIT languages->aboutToShow();
  languages->actions().at(3)->trigger();
  delete menu;
  QVERIFY2(
      waitFor([&frame] {
        return !underlinedWords(frame).contains(QStringLiteral("colour"));
      },
              20000),
      qPrintable("British English still finds `colour` wrong: " +
                 settledWords(frame).join(QStringLiteral(", "))));
  // back to the variety an editor reads a text by when nothing was chosen
  auto *const back = choicesMenu(*editor, false);
  auto *const back_languages = languageSubmenu(*back);
  QVERIFY(back_languages != nullptr);
  Q_EMIT back_languages->aboutToShow();
  back_languages->actions().at(0)->trigger();
  delete back;
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  editor.reset();
}  // end of TestGrammarCheck::aLanguageOfTheCheckerChangesWhatIsFound

void TestGrammarCheck::aBufferIsReadByTheProofreaderTheEditorIsToldToUse() {
  if (!this->harper_available) {
    QSKIP("harper-ls was not found: no grammar session can be started");
  }
  // the whole chain of the choice, with no menu touched: a buffer opened while
  // the setting names a proofreader is read by it, and the words it has something
  // to say about are underlined under the words they are made of
  tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
  const auto file = writeFile(QStringLiteral("by-choice.cxx"), checkedSource());
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 30000),
      "the choice started no session, or its answer reached no text");
  const auto words = settledWords(frame);
  // the article which does not fit the word it introduces, the verb which has to
  // be the base form after `to`, the form which has to agree with its pronoun:
  // the lints stand under the words they are about, and nowhere else
  QVERIFY2(words.contains(QStringLiteral("an")),
           qPrintable("the article of `an tests` is not underlined: " +
                      words.join(QStringLiteral(", "))));
  QVERIFY2(words.contains(QStringLiteral("don't")) ||
               words.contains(QStringLiteral("dont")),
           qPrintable("the contraction another variety spells otherwise is not "
                      "underlined: " +
                      words.join(QStringLiteral(", "))));
  // and the line of code the checker has no reason to touch is not underlined:
  // the checker of sentences reads the comments of the file and not its
  // statements, which is what the choice of a checker for a source file is about
  for (const auto &selection : frame.extraSelections()) {
    const auto underlined = selection.cursor.selectedText();
    QVERIFY2(!underlined.contains(QStringLiteral("main")),
             qPrintable("a line of code is underlined at: " + underlined));
  }
  // the mouse resting on an underlined run says what the checker said of it: the
  // sentence of a lint is longer than the minibuffer is wide, and the underline is
  // the one place where it is written in full
  const auto lint = [&frame] {
    for (const auto &selection : frame.extraSelections()) {
      if (selection.cursor.selectedText() == QStringLiteral("an")) {
        return selection.cursor;
      }
    }
    return QTextCursor();
  }();
  QVERIFY(!lint.isNull());
  auto *const view = frame.widget()->viewport();
  QVERIFY(view != nullptr);
  // the middle of the run, not the edge of the rectangle a cursor which holds a
  // selection answers with: that rectangle ends on the character the lint stops
  // before, which is a clean spot where the filter of the session answers nothing
  auto middle = QTextCursor(frame.document());
  middle.setPosition((lint.selectionStart() + lint.selectionEnd()) / 2);
  const auto rectangle = frame.cursorRect(middle);
  const auto over = QPoint(rectangle.x() + 2,
                           rectangle.y() + rectangle.height() / 2);
  QVERIFY(frame.cursorForPosition(over).position() == middle.position());
  QToolTip::hideText();
  // the virtual pointer's position is what the tooltip path reads, so the move
  // comes first, and the resting of the mouse — which is what a `ToolTip` event
  // says — comes as the event
  QTest::mouseMove(view, over);
  auto resting = QEvent(QEvent::ToolTip);
  QApplication::sendEvent(view, &resting);
  // the tip is a label of its own, wherever the platform put it
  auto tip = QString();
  QVERIFY(waitFor(
      [&] {
        for (auto *const w : QApplication::topLevelWidgets()) {
          auto *const label = qobject_cast<QLabel *>(w);
          if ((label != nullptr) && label->isVisible() &&
              (!label->text().isEmpty())) {
            tip = label->text();
            return true;
          }
        }
        return false;
      },
      5000));
  QToolTip::hideText();
  QVERIFY2(tip.contains('[') && tip.contains(']'),
           qPrintable("the tooltip of an underline names no rule: " + tip));
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  editor.reset();
}  // end of
   // TestGrammarCheck::aBufferIsReadByTheProofreaderTheEditorIsToldToUse

void TestGrammarCheck::aLanguageToolServerUnderlinesTheTextOfABuffer() {
  if (!this->tool_available) {
    QSKIP("no LanguageTool server can be started: the cases which ask it "
          "anything have nothing to run");
  }
  // the whole chain of the other proofreader: the Java process is started by the
  // first buffer which asks for it, the text of that buffer is handed to it as it
  // stands, and the answer is drawn under the words it names — with the
  // dictionary of hunspell taken away, which is what the choice of a checker
  // reading the whole text of a buffer means for the misspelled words
  tfel::gui::SpellingAndGrammar::setChecker(Checker::LanguageTool);
  // the mode of a text file is a plugin, and the editor is what loads it: the
  // library of the build tree carries the path of every library it needs in its own
  // run path, so loading it needs no help from the environment
  auto latex = QLibrary(QStringLiteral(TFEL_EDITOR_LATEX_MODE));
  QVERIFY2(latex.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      latex.errorString()));
  const auto file = writeFile(
      QStringLiteral("tool.tex"),
      QStringLiteral("\\section{A title}\n\n"
                     "This is an tests of the grammar.\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto *const mode = frame.setMajorMode(QStringLiteral("LaTeX"));
  QVERIFY(mode != nullptr);
  QVERIFY2(
      waitFor([&frame] { return !underlinedWords(frame).isEmpty(); }, 60000),
      qPrintable("the server started by the choice drew no underline: " +
                 editor->messages.join(QStringLiteral(" | "))));
  const auto words = settledWords(frame);
  QVERIFY2(words.contains(QStringLiteral("an tests")) ||
               words.contains(QStringLiteral("an")),
           qPrintable("the article of `an tests` is not underlined: " +
                      words.join(QStringLiteral(", "))));
  // the markup of the document is not text to be read: a checker handed the whole
  // of a LaTeX file sees every `\command` as a typo, and the mode of the buffer is
  // the only one who knows which characters are markup and which are the sentence
  for (const auto &selection : frame.extraSelections()) {
    const auto underlined = selection.cursor.selectedText();
    QVERIFY2(!underlined.contains(QStringLiteral("section")),
             qPrintable("the markup of the document is underlined at: " +
                        underlined));
  }
  // and the dictionary of hunspell is off that buffer: the one checker which says
  // both what is misspelled and what does not agree is not asked to share the
  // text with a machinery drawing a second red line under the same word
  // the mechanism of that off switch is the language of the frame: an empty one is
  // what the mode reads as 'no dictionary'. What it reads as on the screen — no
  // suggestions of a misspelled word — is the last case, on a buffer where the
  // dictionary was first asked for and answered
  tfel::gui::SpellingAndGrammar::applyTo(*editor);
  QVERIFY2(frame.getSpellCheckLanguage().isEmpty(),
           "the choice of LanguageTool left a dictionary on the buffer");
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  editor.reset();
}  // end of
   // TestGrammarCheck::aLanguageToolServerUnderlinesTheTextOfABuffer

void TestGrammarCheck::theModesOfTheTextBuffersFollowTheChoice() {
  // the two text modes of the editor were asked to follow the choice, and the
  // hunspell behaviour of a packaged mode is the one face of the choice a test
  // can see on such a mode: the suggestions of a misspelled word, which the
  // dictionary answers while the choice names it and which nothing answers once
  // the choice names a checker reading the whole text of the buffer
  //
  // the `LaTeX` mode is chosen over `Markdown` because that is the text mode whose
  // context menu asks the spell checker anything; both modes are loaded here, as
  // the editor loads its plugins, which leaves them registered for the rest of the
  // run: that is why this case is the last one
  auto latex = QLibrary(QStringLiteral(TFEL_EDITOR_LATEX_MODE));
  QVERIFY2(latex.load(), qPrintable(QStringLiteral(
                             "the packaged mode is not loaded: ") +
                         latex.errorString()));
  auto markdown = QLibrary(QStringLiteral(TFEL_EDITOR_MARKDOWN_MODE));
  QVERIFY2(markdown.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      markdown.errorString()));
  if (!tfel::gui::SpellingAndGrammar::isAvailable(Checker::Hunspell)) {
    QSKIP("this build has no hunspell: what the choice does to the dictionary of "
          "a text mode can not be observed");
  }
  const auto file = writeFile(
      QStringLiteral("modes.tex"),
      QStringLiteral("\\section{Title}\n\n"
                     "This is an tests with a mistaked word in it.\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<tfel::gui::EditorWidget>();
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the mode of the buffer, in hand and not as the `const MajorMode &` a frame
  // hands out: `setMajorMode` is the one public door to an instance of a mode a
  // plugin built
  auto *const mode = frame.setMajorMode(QStringLiteral("LaTeX"));
  QVERIFY(mode != nullptr);
  // the dictionary of this machine is asked for by name; a machine which has no
  // English dictionary has nothing to observe, which the precondition below is
  // what names
  frame.setSpellCheckLanguage(QStringLiteral("en_US"));
  const auto suggestions = [mode, &frame] {
    return suggestionsFor(*mode, frame, QStringLiteral("mistaked"));
  };
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  // the dictionary of this machine is asked for by name, and the same name is what
  // the choice of a checker is held to give back: the suggestions below are the
  // underlines of the misspelled words seen from the side of the context menu
  QSettings().setValue("hunspell default dictionary", QStringLiteral("en_US"));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  const auto count = suggestions();
  if (count == 0) {
    QSKIP(
        "no suggestions for `mistaked`: this build has no hunspell, or this "
        "machine no en_US dictionary");
  }
  // the choice of hunspell alone keeps the dictionary: the suggestions of the
  // misspelled word are asked about and answered
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  tfel::gui::SpellingAndGrammar::applyTo(*editor);
  QCOMPARE(frame.getSpellCheckLanguage(), QStringLiteral("en_US"));
  QCOMPARE(suggestions(), count);
  if (this->harper_available) {
    // the choice of the checker of sentences too keeps it: the two checkers were
    // asked for by two different needs, one says a word is not a word, the other
    // that the words do not agree, and a buffer which shows both says a great
    // deal
    tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
    tfel::gui::SpellingAndGrammar::applyTo(*editor);
    QCOMPARE(frame.getSpellCheckLanguage(), QStringLiteral("en_US"));
    QCOMPARE(suggestions(), count);
    QVERIFY2(waitFor([&frame] { return !underlinedWords(frame).isEmpty(); },
                     30000),
             "the checker of a LaTeX buffer drew no underline");
    // the two reports stand side by side on one buffer, drawn by two machineries:
    // the highlighter of the mode inks a word it does not know, the `grammar`
    // contribution to the extra selections inks what the agreement of the words
    // does not like
    auto grammar_drawn = false;
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.cursor.selectedText() == QStringLiteral("an")) &&
          (selection.format.underlineColor() == QColor(200, 0, 0))) {
        grammar_drawn = true;
      }
    }
    QVERIFY(grammar_drawn);
  }
  if (this->tool_available) {
    // and the choice of the checker which says both takes the dictionary away: a
    // checker which knows nothing of spelling has no business taking a dictionary
    // away, but a checker which knows both has no business sharing the text with
    // one which repeats it
    tfel::gui::SpellingAndGrammar::setChecker(Checker::LanguageTool);
    tfel::gui::SpellingAndGrammar::applyTo(*editor);
    QVERIFY2(frame.getSpellCheckLanguage().isEmpty(),
             "the choice of LanguageTool left a dictionary on the buffer");
    QVERIFY2(suggestions() == 0,
             "the spell checker was left on a buffer LanguageTool reads");
  }
  // the mode of a Markdown buffer is asked the same question: the choice is one
  // decision for the editor and not one per mode, and the modes of the two text
  // file names have nothing to say of their own about it
  const auto note = writeFile(QStringLiteral("modes.md"),
                              QStringLiteral("# Title\n\n"
                                             "This is an tests of the grammar.\n"));
  QVERIFY(!note.isEmpty());
  editor->openFile(note);
  auto &md_frame = editor->getCurrentBuffer().getMainFrame();
  auto *const md_mode = md_frame.setMajorMode(QStringLiteral("Markdown"));
  QVERIFY(md_mode != nullptr);
  if (this->tool_available) {
    tfel::gui::SpellingAndGrammar::setChecker(Checker::LanguageTool);
    tfel::gui::SpellingAndGrammar::applyTo(*editor);
    QVERIFY2(md_frame.getSpellCheckLanguage().isEmpty(),
             "the choice of LanguageTool left a dictionary on a Markdown buffer");
    auto *const session = md_mode->grammarCheckSession();
    QVERIFY2((session != nullptr) && session->readsWholeTextAsProse(),
             "the mode of a Markdown note says its text is not to be read");
  }
  if (this->harper_available) {
    // the checker of sentences reads the note the same way it reads a document:
    // the whole of it is prose, and the answer is drawn in the same red
    tfel::gui::SpellingAndGrammar::setChecker(Checker::HunspellHarper);
    tfel::gui::SpellingAndGrammar::applyTo(*editor);
    QVERIFY2(waitFor([&md_frame] {
               return underlinedWords(md_frame).contains(QStringLiteral("an"));
             },
                     30000),
             qPrintable("the checker of a Markdown note drew no underline: " +
                        settledWords(md_frame).join(QStringLiteral(", "))));
  }
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  QSettings().remove("hunspell default dictionary");
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  tfel::gui::SpellingAndGrammar::setChecker(Checker::Hunspell);
  editor.reset();
}  // end of TestGrammarCheck::theModesOfTheTextBuffersFollowTheChoice

QTEST_MAIN(TestGrammarCheck)

#include "TestGrammarCheck.moc"
