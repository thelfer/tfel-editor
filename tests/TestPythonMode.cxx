/*!
 * \file   TestPythonMode.cxx
 * \brief  tests for the `Python` major mode: the mode itself, the faces of its
 *         highlighter, and the conversation its buffers hold with `pylsp`.
 * \author Thomas Helfer
 * \date   14 nov. 2026
 */

#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLibrary>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTextEdit>
#include <functional>
#include <memory>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/ProcessInteractionFrame.hxx"
#include "TFEL/GUI/SettingsKey.hxx"
#include "TFEL/GUI/TextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// The `Python` mode is a plugin, and its class is not exported: the library is
// loaded the way the editor loads it, and everything the cases ask goes through
// the keys and the public API, which is what a person's hands are made of.
//
// `pylsp` is the server the cases talk to. It is a program of its own, installed
// with `pip`, so every case which asks it something is skipped when it is not
// there; the cases about the mode, its faces, and its states need no server and
// run either way.
namespace {

  //! the menu the mode adds to the menu bar
  QMenu *pythonMenu(tfel::gui::EditorWidget &editor) {
    for (auto *const menu : editor.getCurrentBufferSpecificMenus()) {
      if (menu->title() == QStringLiteral("Python")) {
        return menu;
      }
    }
    return nullptr;
  }  // end of pythonMenu()

  /*!
   * \brief an editor widget which remembers what the minibuffer was told
   *
   * \note overriding this public virtual is the only door to the messages: which
   * project a session reads, and whether a server was found at all, is part of
   * what is under test, and none of it is written down anywhere else
   */
  struct RecordingEditor : tfel::gui::EditorWidget {
    QStringList messages;
    void displayInformativeMessage(const QString &m) override {
      messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
    //! true when one of the messages holds `what`
    bool told(const QString &what) const {
      for (const auto &m : this->messages) {
        if (m.contains(what)) {
          return true;
        }
      }
      return false;
    }
    //! the first message which starts with `prefix`, empty when none does
    QString messageStartingWith(const QString &prefix) const {
      for (const auto &m : this->messages) {
        if (m.startsWith(prefix)) {
          return m;
        }
      }
      return QString();
    }
  };  // end of struct RecordingEditor

  //! true when the selection is one of the faces the server draws a diagnostic with
  /*!
   * \note the underlines of the diagnostics are told apart from the one the word
   * under the cursor wears — the color of the links, see
   * `LspNavigationSession::applyTrackedCursor` — by the four colors the severity
   * of a report is drawn with, which is what
   * `LspNavigationSession::diagnosticColor` answers and is written here because
   * that function belongs to the anonymous namespace of a source file
   */
  bool isDiagnosticUnderline(const QTextEdit::ExtraSelection &s) {
    if (!s.format.fontUnderline()) {
      return false;
    }
    const auto c = s.format.underlineColor();
    const auto faces = QVector<QColor>() << QColor(170, 0, 0)
                                        << QColor(180, 100, 0)
                                        << QColor(0, 80, 160)
                                        << QColor(120, 120, 120);
    return faces.contains(c);
  }  // end of isDiagnosticUnderline()

  /*!
   * \brief the number of pixels of each family of face a widget shows
   *
   * \note the faces of a `QSyntaxHighlighter` are kept in the private extra
   * selections of the document, which `QTextCursor::charFormat` and
   * `TextEditBase::extraSelections` both ignore: all a test can read of them is
   * that they are inked, so the pixels of the text are counted and a face is
   * recognised by the shape of its colour — the channels which stand for it well
   * above the third — which tells the five faces apart and leaves black text, its
   * antialiasing, and the band of the current line alone
   */
  struct FaceCount {
    //! pixels of the colour of the keywords: dark magenta
    int keywords = 0;
    //! pixels of the colour of the literals: dark green
    int strings = 0;
    //! pixels of the colour of the comments and the numbers: dark red
    int comments = 0;
    //! pixels of the colour of the names and the builtins: dark cyan
    int names = 0;
    //! pixels of the colour of the types: dark blue
    int types = 0;
  };  // end of struct FaceCount

  FaceCount countFaces(QWidget &w) {
    auto counts = FaceCount();
    const auto image = w.grab().toImage().convertToFormat(QImage::Format_RGB32);
    for (auto y = 0; y != image.height(); ++y) {
      for (auto x = 0; x != image.width(); ++x) {
        const auto c = image.pixelColor(x, y);
        const auto r = c.red();
        const auto g = c.green();
        const auto b = c.blue();
        // a gray pixel, whatever its brightness, is text, its antialiasing, or
        // the band of the current line: only a pixel whose channels disagree is
        // the ink of a face
        const auto spread = qMax(qMax(r, g), b) - qMin(qMin(r, g), b);
        if (spread < 60) {
          continue;
        }
        if ((r > g + 30) && (b > g + 30)) {
          counts.keywords++;
        } else if ((g > r + 30) && (b > r + 30)) {
          counts.names++;
        } else if ((g > r + 30) && (b <= g)) {
          counts.strings++;
        } else if ((b > r + 30) && (b > g + 30)) {
          counts.types++;
        } else if ((r > g + 30) && (r > b + 30)) {
          counts.comments++;
        }
      }
    }
    return counts;
  }  // end of countFaces()

}  // end of anonymous namespace

struct TestPythonMode : QObject {
  Q_OBJECT

 public:
  TestPythonMode();

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void cleanup();
  void aPythonFileTakesThePythonMode();
  void theWordsOfTheLanguageAreInked();
  void theDocstringOfAFunctionIsReadAsADocstring();
  void runningTheFileIsWhatTheCompilationCommandAsks();
  void theServerStartsWithTheFileAndNamesTheProject();
  void startingTheServerByDefaultCanBeTurnedOff();
  void theDiagnosticsOfTheFileStandUnderTheWords();
  void theWordUnderTheCursorPromisesTheJumpAndKeepsIt();
  void theRenameReachesEveryUseOfTheName();
  void formatWritesTheBufferTheWayTheServerReadsIt();
  void theOverviewNamesTheSymbolsOfTheFile();
  void stoppingTheSessionTakesTheUnderlinesAway();
  void theMenuOffersTheToolsOfTheLanguage();
  void theTabKeyGoesDownTheLevelsOfTheLine();
  void shiftTabTakesTheLineOutOfItsBlock();
  void theTabKeyMovesARegionOneLevelRight();
  void anInterpreterWhichIsNotFoundIsNamedAndNoKeyIsLost();
  void theInterpreterRunsWhatTheBufferSends();
  void theTranscriptOfTheSessionReadsAsAConversation();
  void theToolsOfTheDialogChooseRuffAndNoHistory();
  void theCommandsOfTheToolsRefuseAnotherMode();
  void switchingALinterChangesWhatStandsUnderTheWords();
  void theFormatterChoiceDecidesWhoAnswers();
  void sortingTheImportsFollowsTheSorterChosen();

 private:
  /*!
   * \brief the entry of a menu which shows under the given text, null when the
   * menu holds no such entry
   */
  static QAction *entryOf(QMenu &menu, const QString &text);
  /*!
   * \brief the submenu of a menu which shows under the given text, null when the
   * menu holds no such submenu
   */
  static QMenu *subMenuOf(QMenu &menu, const QString &text);
  /*!
   * \brief the entry of the Python menu, or of one of its submenus, whichever
   * menu the text is found in first
   */
  static QAction *entryOf(tfel::gui::EditorWidget &, const QString &text);
  //! the settings of the tools, back to what the mode itself chooses
  void forgetToolChoices();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  //! write `content` in a file of the cases of this class
  QString writeSource(const QString &name, const QString &content);
  /*!
   * \brief a buffer of the `Python` mode visiting the given file, with the
   * minibuffer messages of its opening kept
   *
   * \note the returned editor is the caller's to destroy; a case which wants a
   * session asks for one after this, the way the menu does
   */
  std::unique_ptr<RecordingEditor> openInPythonMode(const QString &file);
  //! the client of `pylsp` of an editor widget, null while none was ever built
  static tfel::gui::LspClient *clientOf(tfel::gui::EditorWidget &);
  /*!
   * \brief start a session through the entry of the menu and wait for its
   * handshake, failing if either does not work out
   */
  void startSession(RecordingEditor &);
  //! the question the minibuffer holds under the given name, null if none
  static tfel::gui::CommandLine *question(tfel::gui::EditorWidget &,
                                          const char *);
  //! the question the minibuffer holds, whichever it is, null if none
  static tfel::gui::CommandLine *anyQuestion(tfel::gui::EditorWidget &);
  //! the line edit a question shows, that is, the one it still lays out
  static tfel::gui::LineEdit *lineEditOf(tfel::gui::CommandLine &);
  //! true when a widget is still laid out, that is, displayed
  static bool isLaidOut(QWidget &);
  /*!
   * \brief true when the run can ask the server anything
   * \return false, with the reason, when the mode or the server is missing; the
   * caller has to `QSKIP` itself, as the macro only leaves the function it is
   * written in
   */
  bool hasServer() const;
  /*!
   * \brief true when the run can ask an interpreter anything
   * \return false, with the reason, when the mode or the program is missing; the
   * caller has to `QSKIP` itself, as the macro only leaves the function it is
   * written in
   *
   * \note the program is the one the mode would run — what `python/executable`
   * names, `python3` when the address holds nothing — since a case which skips
   * because of a missing program must skip for the same reason the mode would
   * fail for
   */
  bool hasInterpreter() const;
  /*!
   * \brief the indentation each press of a key leaves a line of a buffer at
   * \param frame : the frame the presses are made in
   * \param line : the line of the buffer the key is pressed on
   * \param start : the number of blanks the line holds when the run starts
   * \param times : the number of presses
   * \param key : the key pressed
   * \param modifiers : the modifiers of that key
   * \return the columns the line was left at, one per press, in the order of the
   * presses
   *
   * The line keeps its text: only its leading blanks are replaced by the
   * indentation the run starts from, which is what the probe of `python.el' does
   * with `delete-horizontal-space' before each run, and the reason the answers of
   * the two are the same numbers.
   *
   * \note the presses are made in one run: the mode tells a press which continues
   * the cycle of the previous one from the line it finds and the place it stands
   * at, so a run which built a new buffer between two presses would ask the line
   * afresh every time.
   */
  static QString pressesOf(tfel::gui::TextEditBase &frame, const int line,
                           const int start, const int times,
                           const Qt::Key key,
                           const Qt::KeyboardModifiers modifiers);
  //! spin the event loop until the predicate holds or the time is up
  static bool waitFor(const std::function<bool()> &, int ms);

#ifdef TFEL_EDITOR_PYTHON_MODE
  //! the library of the packaged mode, named by the build
  QLibrary library;
#endif /* TFEL_EDITOR_PYTHON_MODE */
  //! whether the mode was loaded at all
  bool modeLoaded = false;
  //! whether a `pylsp` was found
  bool server = false;
  //! true when the run can ask an interpreter anything
  bool interpreter = false;
  //! the directory the files of the cases are written in
  QTemporaryDir directory;
};  // end of struct TestPythonMode

TestPythonMode::TestPythonMode() {
#ifdef TFEL_EDITOR_PYTHON_MODE
  this->library.setFileName(QStringLiteral(TFEL_EDITOR_PYTHON_MODE));
#endif /* TFEL_EDITOR_PYTHON_MODE */
}

void TestPythonMode::isolateSettings() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-python-mode");
  auto settings = QSettings();
  settings.remove("pylsp/executable");
  settings.remove("pylsp/arguments");
  // the interpreter of a session is the program the mode would run when the
  // address holds nothing; a case which names another one leaves it there for no
  // other case
  settings.remove("python/executable");
  // the command `Alt-m` proposes is the last answer of the same question, and the
  // file the cases name is not the one a previous run was about
  settings.remove("Python/compilation/history");
  // a session which starts with every file is the behaviour of one case and of
  // none of the others: the rest start their session the way the menu does, so
  // that what a session reads is decided by the case which asserts on it
  settings.setValue("pylsp/start-by-default", false);
  // the tools the server runs are choices of the menus, of the commands and of
  // the dialog, and a case which made one of them says so on the underlines of
  // every buffer: the cases which assert what the mode chooses by default have
  // to start from nothing written
  this->forgetToolChoices();
  settings.sync();
}  // end of TestPythonMode::isolateSettings

QString TestPythonMode::writeSource(const QString &name,
                                    const QString &content) {
  const auto file = directory.filePath(name);
  QDir().mkpath(QFileInfo(file).absolutePath());
  auto handle = QFile(file);
  if (!handle.open(QIODevice::WriteOnly)) {
    return QString();
  }
  handle.write(content.toUtf8());
  handle.close();
  return file;
}  // end of TestPythonMode::writeSource

std::unique_ptr<RecordingEditor> TestPythonMode::openInPythonMode(
    const QString &file) {
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  editor->getCurrentBuffer().getMainFrame().setMajorMode(
      QStringLiteral("Python"));
  QTest::qWait(200);
  return editor;
}  // end of TestPythonMode::openInPythonMode

tfel::gui::LspClient *TestPythonMode::clientOf(
    tfel::gui::EditorWidget &editor) {
  // by name: the editor widget holds the clients of the grammar checkers too, and
  // one of those is none to talk to about a name of a buffer
  return editor.findChild<tfel::gui::LspClient *>(QLatin1String("pylsp"));
}  // end of TestPythonMode::clientOf

void TestPythonMode::startSession(RecordingEditor &editor) {
  auto *const menu = pythonMenu(editor);
  QVERIFY2(menu != nullptr, "the mode adds no menu to the menu bar");
  menu->actions().at(0)->trigger();
  auto *const client = clientOf(editor);
  QVERIFY2(client != nullptr, "the entry which starts a session built none");
  QVERIFY2(client->waitUntilReady(40000),
           "the handshake with pylsp did not complete");
}  // end of TestPythonMode::startSession

bool TestPythonMode::isLaidOut(QWidget &w) {
  auto *const parent = w.parentWidget();
  auto *const layout = (parent != nullptr) ? parent->layout() : nullptr;
  return (layout != nullptr) && (layout->indexOf(&w) >= 0);
}  // end of TestPythonMode::isLaidOut

tfel::gui::CommandLine *TestPythonMode::question(
    tfel::gui::EditorWidget &editor, const char *const name) {
  // an input which was answered lingers as a child of the editor until the event
  // loop deletes it, so the question which is displayed is the one the minibuffer
  // still lays out
  for (auto *const candidate :
       editor.findChildren<tfel::gui::CommandLine *>()) {
    if ((candidate->objectName() == QLatin1String(name)) &&
        isLaidOut(*candidate)) {
      return candidate;
    }
  }
  return nullptr;
}  // end of TestPythonMode::question

tfel::gui::CommandLine *TestPythonMode::anyQuestion(
    tfel::gui::EditorWidget &editor) {
  for (auto *const candidate :
       editor.findChildren<tfel::gui::CommandLine *>()) {
    if (isLaidOut(*candidate)) {
      return candidate;
    }
  }
  return nullptr;
}  // end of TestPythonMode::anyQuestion

tfel::gui::LineEdit *TestPythonMode::lineEditOf(tfel::gui::CommandLine &q) {
  tfel::gui::LineEdit *found = nullptr;
  for (auto *const line : q.findChildren<tfel::gui::LineEdit *>()) {
    // of the line edits the input has, the one it lays out is the one its
    // constructor left in place
    if (isLaidOut(*line)) {
      return line;
    }
    found = line;
  }
  return found;
}  // end of TestPythonMode::lineEditOf

bool TestPythonMode::hasServer() const {
  return (this->modeLoaded) && (this->server);
}  // end of TestPythonMode::hasServer

bool TestPythonMode::hasInterpreter() const {
  return (this->modeLoaded) && (this->interpreter);
}  // end of TestPythonMode::hasInterpreter

namespace {

  //! the number of blanks a line holds in front of its text
  int leadingBlanks(const QString &line) {
    auto i = 0;
    while ((i < line.size()) && (line.at(i) == QLatin1Char(' '))) {
      ++i;
    }
    return i;
  }  // end of leadingBlanks()

}  // end of anonymous namespace

QString TestPythonMode::pressesOf(tfel::gui::TextEditBase &frame,
                                  const int line, const int start,
                                  const int times, const Qt::Key key,
                                  const Qt::KeyboardModifiers modifiers) {
  auto *const view = qobject_cast<QPlainTextEdit *>(frame.widget());
  if (view == nullptr) {
    return QStringLiteral("<the frame holds no plain text edit>");
  }
  auto c = QTextCursor(view->document());
  c.movePosition(QTextCursor::Start);
  for (auto i = 0; i != line; ++i) {
    c.movePosition(QTextCursor::NextBlock);
  }
  // the line is put at the indentation the run starts from, its text untouched
  const auto blanks = leadingBlanks(c.block().text());
  c.movePosition(QTextCursor::StartOfBlock);
  c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, blanks);
  c.insertText(QString(start, QLatin1Char(' ')));
  c.movePosition(QTextCursor::StartOfBlock);
  c.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, start);
  view->setTextCursor(c);
  auto sequence = QString();
  for (auto i = 0; i != times; ++i) {
    // the key is a `Qt::Key' and not an integer on purpose: `QTest::keyClick' is
    // overloaded on `char', an int argument selects that overload, and it maps its
    // argument through `QTest::asciiToKey' — which asserts on `Qt::Key_Tab' in a
    // build where `QT_NO_DEBUG' is not defined and, where it is, answers the key 0:
    // a press which does nothing at all, and a run whose assertions then say that
    // the indentation of the line never moved
    QTest::keyClick(frame.widget(), key, modifiers);
    QTest::qWait(30);
    if (i != 0) {
      sequence += QLatin1Char(' ');
    }
    sequence +=
        QString::number(leadingBlanks(view->document()
                                          ->findBlockByNumber(line)
                                          .text()));
  }
  return sequence;
}  // end of TestPythonMode::pressesOf

bool TestPythonMode::waitFor(const std::function<bool()> &predicate,
                             const int ms) {
  auto waited = 0;
  while (!predicate()) {
    if (waited > ms) {
      return false;
    }
    QTest::qWait(50);
    waited += 50;
  }
  return true;
}  // end of TestPythonMode::waitFor

QAction *TestPythonMode::entryOf(QMenu &menu, const QString &text) {
  // the entries of a menu, never its separators and never a submenu: what a
  // person reads is the text of the entry, and that is what is asserted on
  for (auto *const action : menu.actions()) {
    if ((action->menu() == nullptr) && (!action->isSeparator()) &&
        (action->text() == text)) {
      return action;
    }
  }
  return nullptr;
}  // end of TestPythonMode::entryOf(QMenu &)

QMenu *TestPythonMode::subMenuOf(QMenu &menu, const QString &text) {
  for (auto *const action : menu.actions()) {
    if ((action->menu() != nullptr) && (action->text() == text)) {
      return action->menu();
    }
  }
  return nullptr;
}  // end of TestPythonMode::subMenuOf

void TestPythonMode::forgetToolChoices() {
  // the linters and the two programs of a list are read from the addresses the
  // menu entries and the dialog write, so a case which clicked one of them has
  // to leave the address behind: what the mode chooses by default is what the
  // next case sees
  auto settings = QSettings();
  const QStringList plugins{QStringLiteral("ruff"), QStringLiteral("pyflakes"),
                            QStringLiteral("pycodestyle"),
                            QStringLiteral("flake8"), QStringLiteral("pylint"),
                            QStringLiteral("mccabe"),
                            QStringLiteral("pydocstyle"),
                            QStringLiteral("pylsp_mypy")};
  for (const auto &plugin : plugins) {
    settings.remove(QStringLiteral("pylsp/linters/") + plugin);
  }
  settings.remove(QStringLiteral("pylsp/formatter"));
  settings.remove(QStringLiteral("pylsp/import-sorter"));
  // and the two questions remember what was last answered, which is what the
  // completion of the next one is built from
  settings.remove(QStringLiteral("pylsp/formatter/history"));
  settings.remove(QStringLiteral("pylsp/import-sorter/history"));
  settings.sync();
}  // end of TestPythonMode::forgetToolChoices

QAction *TestPythonMode::entryOf(tfel::gui::EditorWidget &editor,
                                 const QString &text) {
  // the menu of the mode is rebuilt every time the buffers are asked for it, so
  // what this finds is a menu which ticks what the settings said when it was
  // built: a case which asserts that a click ticked an entry has to ask for the
  // menu again, exactly as the mouse does when the person opens it a second time
  auto *const menu = pythonMenu(editor);
  if (menu == nullptr) {
    return nullptr;
  }
  if (auto *const direct = entryOf(*menu, text)) {
    return direct;
  }
  for (auto *const action : menu->actions()) {
    if (auto *const sub = action->menu()) {
      if (auto *const entry = entryOf(*sub, text)) {
        return entry;
      }
    }
  }
  return nullptr;
}  // end of TestPythonMode::entryOf(EditorWidget &)

void TestPythonMode::initTestCase() {
  isolateSettings();
  QVERIFY(directory.isValid());
#ifdef TFEL_EDITOR_PYTHON_MODE
  QVERIFY2(this->library.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      this->library.errorString()));
  this->modeLoaded = true;
#else  /* TFEL_EDITOR_PYTHON_MODE */
  this->modeLoaded = false;
#endif /* TFEL_EDITOR_PYTHON_MODE */
  this->server =
      !QStandardPaths::findExecutable(QStringLiteral("pylsp")).isEmpty();
  // the interpreter of the mode is a program of the machine: `python3` by
  // default. A case which asks it what it answers is skipped when it is not
  // there, the way the cases which ask `pylsp` anything are
  this->interpreter =
      !QStandardPaths::findExecutable(QStringLiteral("python3")).isEmpty();
}  // end of TestPythonMode::initTestCase

void TestPythonMode::cleanupTestCase() {
  isolateSettings();
}  // end of TestPythonMode::cleanupTestCase

void TestPythonMode::cleanup() {
  // a case which turned the automatic session on — or off — leaves that choice to
  // its own case: what the settings hold is reset here, so that a case which
  // failed on its way out can not decide what the next one sees
  isolateSettings();
}  // end of TestPythonMode::cleanup

void TestPythonMode::aPythonFileTakesThePythonMode() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is not loaded: nothing can be asked of it");
  }
  const auto file =
      writeSource(QStringLiteral("mode/app.py"),
                  QStringLiteral("def greet(name):\n"
                                 "    return \"hello \" + name\n"));
  QVERIFY(!file.isEmpty());
  // a `.py` file is opened in the mode of the language without being asked for,
  // which is what the list of extensions of the proxy is for
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  auto &buffer = editor->getCurrentBuffer();
  auto *const mode =
      buffer.getMainFrame().setMajorMode(QStringLiteral("Python"));
  QVERIFY2(mode != nullptr, "no mode of the name Python was registered");
  QCOMPARE(mode->getName(), QStringLiteral("Python"));
  // a script of this language is source: the mode is one of the programming
  // modes, which is what gives it the compilation output machinery and the
  // sharing of an output with the buffers of the other languages
  QCOMPARE(mode->getCategory(), tfel::gui::MajorMode::Category::PROGRAMMING);
  // and `#` is what a line of this language starts a comment with, which is what
  // the comment command of any buffer asks the mode
  auto *const base = qobject_cast<tfel::gui::MajorModeBase *>(mode);
  QVERIFY2(base != nullptr, "the Python mode is not a MajorModeBase");
  QCOMPARE(base->getCommentSyntax(), QStringLiteral("#"));
  // the menu of the mode names the program it drives, and its first entry is the
  // one which starts a session
  auto *const menu = pythonMenu(*editor);
  QVERIFY2(menu != nullptr, "the mode adds no menu to the menu bar");
  QCOMPARE(menu->actions().at(0)->text(), QStringLiteral("Start pylsp"));
  QVERIFY2(!menu->actions().at(0)->isCheckable(),
           "the entry which starts a session is a switch, not a command");
  editor.reset();
}  // end of TestPythonMode::aPythonFileTakesThePythonMode

void TestPythonMode::theWordsOfTheLanguageAreInked() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is not loaded: nothing can be asked of it");
  }
  // a file which holds one of each of the faces the mode knows: a comment, the
  // keywords of an `import`, of a `class` and of a `def`, the name of the class,
  // the name the `def` introduces, a decorator, a builtin, a string
  const auto file = writeSource(
      QStringLiteral("faces/faces.py"),
      QStringLiteral("# a comment of its own\n"
                     "import os\n"
                     "\n"
                     "\n"
                     "class Widget:\n"
                     "    \"\"\"The doc of the class.\"\"\"\n"
                     "\n"
                     "    @staticmethod\n"
                     "    def build(name):\n"
                     "        count = len(name)\n"
                     "        return \"hi \" + name\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  QVERIFY2(waitFor(
      [&frame] {
        auto block = frame.document()->findBlockByNumber(1);
        return block.isValid() && (block.userState() != -1);
      },
      5000),
      "the highlighter of the mode drew nothing");
  const auto counts = countFaces(*frame.widget());
  QVERIFY2(counts.keywords > 0, "no keyword of the file is inked");
  QVERIFY2(counts.strings > 0, "no string of the file is inked");
  QVERIFY2(counts.comments > 0, "the comment of the file is not inked");
  QVERIFY2(counts.names > 0,
           "no name of a def, a builtin or a decorator is inked");
  QVERIFY2(counts.types > 0, "the name of the class is not inked");
  // and that ink is not what the plain words of the same alphabet are drawn
  // with: a buffer of the same mode, holding nothing the rules name, shows black
  // text only, which is what makes the five counts above the work of the
  // highlighter and not of the palette
  const auto control = writeSource(
      QStringLiteral("faces/control.py"),
      QStringLiteral("qqqq qqqq qqqq\nwwww wwww wwww\n"));
  QVERIFY(!control.isEmpty());
  auto plain = openInPythonMode(control);
  const auto quiet =
      countFaces(*plain->getCurrentBuffer().getMainFrame().widget());
  QCOMPARE(quiet.keywords, 0);
  QCOMPARE(quiet.strings, 0);
  QCOMPARE(quiet.comments, 0);
  QCOMPARE(quiet.names, 0);
  QCOMPARE(quiet.types, 0);
  editor.reset();
  plain.reset();
}  // end of TestPythonMode::theWordsOfTheLanguageAreInked

void TestPythonMode::theDocstringOfAFunctionIsReadAsADocstring() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is not loaded: nothing can be asked of it");
  }
  // What a block of a highlighted document is left with — its user state — is the
  // one decision of the highlighter a test can read directly, and it is the
  // decision this mode has to get right: whether the text behind three quotes is
  // the prose of a statement or a string of the program, which is drawn with two
  // different faces of the same green, so no count of pixels tells the two apart.
  //
  // The numbers are the `enum State` of the mode's own
  // `PythonSyntaxHighlighter.hxx`, which is a header of the plugin and is not on
  // the include path of the tests: `0` is no string open, `1` and `2` are a
  // string left open with `'''` and with `"""`, `3` and `4` are the same two
  // states of a docstring.
  auto statesOf = [this](const QString &name, const QString &source) {
    auto states = QList<int>();
    const auto file = writeSource(name, source);
    if (file.isEmpty()) {
      return states;
    }
    auto editor = openInPythonMode(file);
    auto &frame = editor->getCurrentBuffer().getMainFrame();
    const auto count = frame.document()->blockCount();
    const auto highlighted = waitFor(
        [&frame, &states, count] {
          states.clear();
          auto complete = true;
          for (auto i = 0; i != count; ++i) {
            const auto s =
                frame.document()->findBlockByNumber(i).userState();
            states << s;
            complete = complete && (s != -1);
          }
          return complete;
        },
        5000);
    editor.reset();
    if (!highlighted) {
      states.clear();
    }
    return states;
  };
  auto named = [](const QString &what, const QList<int> &states) {
    return QStringLiteral("the highlighter left a block of ") + what +
           QStringLiteral(" unhighlighted (") + QString::number(states.size()) +
           QStringLiteral(" states)");
  };
  // the docstring of a function, opened on the line below the `def` and closed
  // four lines down: every line it is written on is prose to its end, and the
  // state it leaves names the double delimiter and the docstring
  const auto docstring = statesOf(
      QStringLiteral("doc/docstring.py"),
      QStringLiteral("def f():\n"
                     "    \"\"\"Line one\n"
                     "    line two.\n"
                     "    \"\"\"\n"
                     "    return 1\n"));
  QVERIFY2(!docstring.isEmpty(), qPrintable(named("docstring", docstring)));
  QCOMPARE(docstring.value(0), 0);  // the `def` itself
  QCOMPARE(docstring.value(1), 4);  // the docstring is open
  QCOMPARE(docstring.value(2), 4);  // and still open
  QCOMPARE(docstring.value(3), 0);  // the delimiter closes it
  QCOMPARE(docstring.value(4), 0);  // the statement after it
  // the same three quotes standing in an assignment are a string of the program:
  // nothing above the line heads a body, so the state says `string` and not
  // `docstring`, which is the whole difference between the two cases
  const auto text = statesOf(
      QStringLiteral("text/string.py"),
      QStringLiteral("x = \"\"\"one\n"
                     "two\"\"\"\n"
                     "y = 1\n"));
  QCOMPARE(text.value(0), 2);  // a string is open, not a docstring
  QCOMPARE(text.value(1), 0);  // closed on the second line
  QCOMPARE(text.value(2), 0);
  // and a string of three single quotes, on the head line of a class, is a
  // docstring written with the other delimiter
  const auto single = statesOf(
      QStringLiteral("single/single.py"),
      QStringLiteral("class Widget:\n"
                     "    '''Doc of the class.\n"
                     "    More of it.'''\n"
                     "    VALUE = 1\n"));
  QCOMPARE(single.value(0), 0);
  QCOMPARE(single.value(1), 3);  // the other delimiter, still a docstring
  QCOMPARE(single.value(2), 0);
  QCOMPARE(single.value(3), 0);
}  // end of TestPythonMode::theDocstringOfAFunctionIsReadAsADocstring

void TestPythonMode::runningTheFileIsWhatTheCompilationCommandAsks() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is not loaded: nothing can be asked of it");
  }
  const auto file = writeSource(
      QStringLiteral("run/script.py"), QStringLiteral("print(\"hello\")\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // `Alt-m` of a programming buffer asks for the command which builds it; a
  // script has nothing to build, and the command the mode proposes is the
  // interpreter reading the file the buffer visits, in the directory of that file
  QTest::keyClick(frame.widget(), Qt::Key_M, Qt::AltModifier);
  auto *const q = anyQuestion(*editor);
  QVERIFY2(q != nullptr, "Alt-m asks nothing, so the buffer can not be run");
  auto *const line = lineEditOf(*q);
  QVERIFY(line != nullptr);
  QCOMPARE(line->text(), QStringLiteral("python3 script.py"));
  // an empty answer runs nothing: the minibuffer says so and no output frame is
  // built, which is what makes the question safe to leave
  line->setText(QString());
  q->userEditingFinished();
  QVERIFY(editor->told(QStringLiteral("empty command")));
  QVERIFY(editor->getCurrentBuffer().getCurrentSecondaryTask() == nullptr);
  editor.reset();
}  // end of TestPythonMode::runningTheFileIsWhatTheCompilationCommandAsks

void TestPythonMode::theServerStartsWithTheFileAndNamesTheProject() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  QSettings().setValue("pylsp/start-by-default", true);
  const auto file = writeSource(
      QStringLiteral("project/script.py"),
      QStringLiteral("def greet(name):\n"
                     "    return \"hello \" + name\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto *const client = clientOf(*editor);
  QVERIFY2(client != nullptr,
           "a file of this language started no session of pylsp");
  QVERIFY2(client->waitUntilReady(40000),
           "the session the file started never shook hands");
  // a session nobody asked for says what it reads: the project the answers about
  // a name are computed from is nothing a reader can guess, and the directory of
  // the file is what `pylsp` reads as the root of the program
  const auto message = editor->messageStartingWith(
      QStringLiteral("pylsp started with project root "));
  QVERIFY2(!message.isEmpty(),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  const auto root = message.mid(QStringLiteral(
      "pylsp started with project root ").size());
  QVERIFY2(QDir(root).exists(), qPrintable(root));
  QCOMPARE(QDir(root).canonicalPath(), QFileInfo(file).dir().canonicalPath());
  editor.reset();
}  // end of TestPythonMode::theServerStartsWithTheFileAndNamesTheProject

void TestPythonMode::startingTheServerByDefaultCanBeTurnedOff() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto file = writeSource(
      QStringLiteral("quiet/script.py"),
      QStringLiteral("def greet(name):\n"
                     "    return \"hello \" + name\n"));
  QVERIFY(!file.isEmpty());
  // the setting is off in the cases of this class, so the buffer opens with no
  // conversation at all: nothing is booted behind a person's back, and the
  // minibuffer stays silent about a server which was never asked for. The
  // *client* exists — the mode builds it with the buffer, since a session which
  // is asked for later needs it — but it is idle, which is the whole difference
  // between a program nobody started and one which reads the file
  auto editor = openInPythonMode(file);
  auto *const client = clientOf(*editor);
  QVERIFY2(client != nullptr, "the mode holds no client of pylsp at all");
  QVERIFY2(client->isIdle(),
           "a session was started although the setting refused it");
  QVERIFY2(!editor->told(QStringLiteral("pylsp")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // and the entry of the menu starts a session whatever the setting says: the
  // switch turns the automatic boot off, it does not take the feature away
  startSession(*editor);
  QVERIFY(clientOf(*editor)->isReady());
  // the same entry again replaces the session, and says so as plainly: the root a
  // session reads is chosen when it starts and can not be changed by asking
  startSession(*editor);
  QVERIFY2(editor->told(QStringLiteral("pylsp restarted")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  editor.reset();
}  // end of TestPythonMode::startingTheServerByDefaultCanBeTurnedOff

void TestPythonMode::theDiagnosticsOfTheFileStandUnderTheWords() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto file = writeSource(
      QStringLiteral("diagnostics/broken.py"),
      QStringLiteral("def greet(who):\n"
                     "    return \"hello \" + who\n"
                     "\n"
                     "\n"
                     "result = greet(nobody)\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  const auto word =
      frame.document()->toPlainText().indexOf(QStringLiteral("nobody"));
  QVERIFY(word > 0);
  auto underlined = [&frame, word] {
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.cursor.selectionStart() <= word) &&
          (selection.cursor.selectionEnd() >= word + 6) &&
          isDiagnosticUnderline(selection)) {
        return true;
      }
    }
    return false;
  };
  // the checker of the server reads the file a moment after it is opened, and what
  // it finds stands under the name it is about
  QVERIFY2(waitFor(underlined, 30000),
           "the undefined name of the file was underlined by nothing");
  // the text of the report, which the underline only locates, is echoed in the
  // minibuffer once the cursor rests on the underlined range
  auto c = QTextCursor(frame.document());
  c.setPosition(word + 2);
  frame.setTextCursor(c);
  QVERIFY2(waitFor(
               [&editor] {
                 for (const auto &m : editor->messages) {
                   if (m.contains(QStringLiteral("nobody"))) {
                     return true;
                   }
                 }
                 return false;
               },
               10000),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // and a name the file does define is left alone: the report of the server is
  // about the name which is nowhere defined, and the function beside it wears no
  // face of a diagnostic — whatever the underline which promises a jump says of it
  const auto greet =
      frame.document()->toPlainText().indexOf(QStringLiteral("greet(nobody)"));
  QVERIFY(greet > 0);
  QVERIFY2(!waitFor(
               [&frame, greet] {
                 for (const auto &selection : frame.extraSelections()) {
                   if ((selection.cursor.selectionStart() <= greet + 2) &&
                       (selection.cursor.selectionEnd() >= greet + 5) &&
                       isDiagnosticUnderline(selection)) {
                     return true;
                   }
                 }
                 return false;
               },
               2000),
           "the name of the function defined in the file is reported as an "
           "unknown one");
  editor.reset();
}  // end of TestPythonMode::theDiagnosticsOfTheFileStandUnderTheWords

void TestPythonMode::theWordUnderTheCursorPromisesTheJumpAndKeepsIt() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto source = QStringLiteral(
      "def greet(name):\n"
      "    return \"hello \" + name\n"
      "\n"
      "\n"
      "def shout(who):\n"
      "    msg = greet(who)\n"
      "    return msg.upper()\n");
  const auto file = writeSource(QStringLiteral("jump/jump.py"), source);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the call inside `shout`, which is the sixth line of the file, and the `def` of
  // the function it names, which is the first one: the assertion below holds
  // whatever the jump did if the cursor starts where the answer points
  const auto call = source.indexOf(QStringLiteral("greet(who)"));
  QVERIFY(call > 0);
  auto c = QTextCursor(frame.document());
  c.setPosition(call + 2);
  frame.setTextCursor(c);
  QVERIFY(frame.textCursor().blockNumber() == 5);
  auto underlined = [&frame, call] {
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.cursor.selectionStart() == call) &&
          (selection.cursor.selectionEnd() == call + 5) &&
          (selection.format.underlineStyle() != QTextCharFormat::NoUnderline)) {
        return true;
      }
    }
    return false;
  };
  // the word under the cursor is underlined once the server has answered about it,
  // which is the promise that the jump has somewhere to go
  QVERIFY2(waitFor(underlined, 30000),
           "the name under the cursor was never underlined");
  // `Alt-.` keeps that promise: the cursor stands on the definition, in the first
  // line of the file
  auto jumped = QElapsedTimer();
  jumped.start();
  while ((frame.textCursor().blockNumber() != 0) &&
         (jumped.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_Period, Qt::AltModifier);
    QTest::qWait(250);
  }
  QVERIFY2(frame.textCursor().blockNumber() == 0,
           qPrintable(QStringLiteral("Alt-. left the cursor on line ") +
                      QString::number(frame.textCursor().blockNumber())));
  editor.reset();
}  // end of TestPythonMode::theWordUnderTheCursorPromisesTheJumpAndKeepsIt

void TestPythonMode::theRenameReachesEveryUseOfTheName() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto source = QStringLiteral(
      "def greet(name):\n"
      "    return \"hello \" + name\n"
      "\n"
      "\n"
      "result = greet(\"world\")\n");
  const auto file = writeSource(QStringLiteral("rename/rename.py"), source);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  const auto call = source.indexOf(QStringLiteral("greet(\"world\")"));
  QVERIFY(call > 0);
  auto c = QTextCursor(frame.document());
  c.setPosition(call + 2);
  frame.setTextCursor(c);
  // `Alt-R` asks in the minibuffer what the symbol is to be called; the question is
  // empty on purpose, so that a bare `Return` can not rewrite a symbol with the
  // name it already has
  auto asked = [this, ed = editor.get()]() -> tfel::gui::CommandLine * {
    return question(*ed, "rename-symbol");
  };
  auto *q = asked();
  auto spent = QElapsedTimer();
  spent.start();
  while ((q == nullptr) && (spent.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_R, Qt::AltModifier);
    q = asked();
    if (q == nullptr) {
      QTest::qWait(200);
    }
  }
  QVERIFY2(q != nullptr, "the rename question never came up");
  auto *const line = lineEditOf(*q);
  QVERIFY(line != nullptr);
  QVERIFY2(line->text().isEmpty(),
           "the question of a rename is pre-filled with the name it is "
           "asked to give up");
  // the answer goes to every use of the name, the definition as well as the call.
  // The question is asked again as long as the buffer says nothing of it, which is
  // what a server still reading the file is waited for
  auto renamed = false;
  spent.start();
  while (!renamed && (spent.elapsed() < 60000)) {
    if (q == nullptr) {
      q = asked();
    }
    if (q != nullptr) {
      lineEditOf(*q)->setText(QStringLiteral("salute"));
      q->userEditingFinished();
      q = nullptr;
    }
    QTest::qWait(200);
    const auto text = frame.document()->toPlainText();
    renamed = text.contains(QStringLiteral("def salute(")) &&
              text.contains(QStringLiteral("result = salute("));
    if (!renamed) {
      QTest::keyClick(frame.widget(), Qt::Key_R, Qt::AltModifier);
    }
  }
  QVERIFY2(renamed, qPrintable(frame.document()->toPlainText()));
  // and the cursor is left where the rename was asked, on the line of the call:
  // the server of this language answers the whole file rewritten, since that is
  // what jedi gives back, and it is the edit narrowed to the lines it changed
  // which keeps the person on the line they were reading
  const auto block = frame.textCursor().blockNumber();
  QVERIFY2(
      frame.document()->findBlockByNumber(block)
          .text()
          .contains(QStringLiteral("result = salute(")),
      qPrintable(QStringLiteral("the rename left the cursor on line ") +
                 QString::number(block)));
  editor.reset();
}  // end of TestPythonMode::theRenameReachesEveryUseOfTheName

void TestPythonMode::formatWritesTheBufferTheWayTheServerReadsIt() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto source = QStringLiteral(
      "def total( a,b ):\n"
      "    return a + b\n"
      "\n"
      "\n"
      "value = total(1,2)\n");
  const auto file = writeSource(QStringLiteral("format/rough.py"), source);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // a key which formats never boots a server: that is a decision, and the menu and
  // the command are where it is taken, which is what the report says when there is
  // no session to ask. A buffer which is already formatted is the common answer of
  // this key and is left silent, and this one is not formatted, so the two cases
  // are told apart here by the buffer staying as it was
  QTest::keyClick(frame.widget(), Qt::Key_Q, Qt::AltModifier);
  QVERIFY2(editor->told(QStringLiteral("'start-pylsp' command")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QCOMPARE(frame.document()->toPlainText(), source);
  startSession(*editor);
  // the cursor is put on the line of the `return`, which is where a person reads
  // from and where the format has to leave them
  const auto returns = source.indexOf(QStringLiteral("return"));
  auto c = QTextCursor(frame.document());
  c.setPosition(returns + 3);
  frame.setTextCursor(c);
  auto spent = QElapsedTimer();
  spent.start();
  auto done = false;
  while (!done && (spent.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_Q, Qt::AltModifier);
    QTest::qWait(500);
    done = frame.document()->toPlainText() != source;
  }
  // what autopep8 makes of the file, which is the answer the mode is handed as
  // one edit spanning the whole text. The spaces around `+` are already in the
  // source on purpose: `E226`, the missing whitespace around an arithmetic
  // operator, is one of the rules autopep8 ignores by default, so a sample which
  // leans on it would be left as it is and read as the formatter doing nothing
  const auto formatted = QStringLiteral(
      "def total(a, b):\n"
      "    return a + b\n"
      "\n"
      "\n"
      "value = total(1, 2)\n");
  QVERIFY2(waitFor(
               [&frame, formatted] {
                 return frame.document()->toPlainText() == formatted;
               },
               30000),
           qPrintable(QStringLiteral("the server was asked to format the "
                                     "buffer and what is left is:\n") +
                      frame.document()->toPlainText()));
  // the narrowing of that whole-text edit to the lines it changed is what leaves
  // the cursor on the line of the `return` rather than at the end of the file
  const auto block = frame.textCursor().blockNumber();
  const auto column = frame.textCursor().positionInBlock();
  QVERIFY2(block == 1,
           qPrintable(QStringLiteral("the format left the cursor on line ") +
                      QString::number(block) + QStringLiteral(", column ") +
                      QString::number(column)));
  QVERIFY2(column > 0, "the format left the cursor at the start of its line");
  editor.reset();
}  // end of TestPythonMode::formatWritesTheBufferTheWayTheServerReadsIt

void TestPythonMode::theOverviewNamesTheSymbolsOfTheFile() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto source = QStringLiteral(
      "CONSTANT = 7\n"
      "\n"
      "\n"
      "class Widget:\n"
      "    \"\"\"A thing.\"\"\"\n"
      "\n"
      "    def paint(self, width):\n"
      "        return width * CONSTANT\n"
      "\n"
      "\n"
      "def twice(value):\n"
      "    return 2 * value\n");
  const auto file = writeSource(QStringLiteral("overview/overview.py"), source);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto asked = [this, ed = editor.get()]() -> tfel::gui::CommandLine * {
    return question(*ed, "go-to-symbol");
  };
  auto *q = asked();
  auto spent = QElapsedTimer();
  spent.start();
  while ((q == nullptr) && (spent.elapsed() < 60000)) {
    QTest::keyClick(frame.widget(), Qt::Key_O, Qt::AltModifier);
    q = asked();
    if (q == nullptr) {
      QTest::qWait(200);
    }
  }
  QVERIFY2(q != nullptr, "the symbol question never came up");
  // the overview is the list of completions of the question, under the title every
  // such list shares cut to the eight characters a tab shows: what is listed there
  // is what the answer is picked from
  auto &buffer = editor->getCurrentBuffer();
  auto *const overview = buffer.getCurrentSecondaryTask();
  QVERIFY2(overview != nullptr, "the overview of the file is shown nowhere");
  QVERIFY2(buffer.getSecondaryTaskTitle(overview).startsWith(
               QStringLiteral("*complet")),
           qPrintable(buffer.getSecondaryTaskTitle(overview)));
  auto *const listed = qobject_cast<tfel::gui::TextEdit *>(overview);
  auto *const view = (listed != nullptr)
                         ? listed
                         : overview->findChild<tfel::gui::TextEdit *>();
  QVERIFY2(view != nullptr, "the overview holds no text to read");
  const auto entries = view->document()->toPlainText();
  QVERIFY2(entries.contains(QStringLiteral("twice")), qPrintable(entries));
  // the method is named with the class it belongs to, which is what the tree of
  // symbols the handshake asks for is worth: a flat list of names says that a
  // `paint` stands somewhere in the file and nothing else
  QVERIFY2(entries.contains(QStringLiteral("Widget.paint")),
           qPrintable(entries));
  // the listing follows what is typed: the entries are picked out of the whole
  // overview, not out of what the completion of a prefix leaves
  lineEditOf(*q)->setText(QStringLiteral("paint"));
  QVERIFY2(waitFor(
               [v = view] {
                 const auto t = v->document()->toPlainText();
                 return t.contains(QStringLiteral("paint")) &&
                        (!t.contains(QStringLiteral("twice")));
               },
               10000),
           qPrintable(view->document()->toPlainText()));
  // answering a name goes to the definition of that symbol
  q->userEditingFinished();
  QVERIFY2(waitFor(
               [&frame] {
                 const auto line = frame.document()
                                       ->findBlockByNumber(
                                           frame.textCursor().blockNumber())
                                       .text();
                 return line.contains(QStringLiteral("def paint"));
               },
               10000),
           qPrintable(QStringLiteral("the answer named line ") +
                      QString::number(frame.textCursor().blockNumber())));
  editor.reset();
}  // end of TestPythonMode::theOverviewNamesTheSymbolsOfTheFile

void TestPythonMode::stoppingTheSessionTakesTheUnderlinesAway() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  const auto file = writeSource(
      QStringLiteral("stop/broken.py"),
      QStringLiteral("def greet(who):\n"
                     "    return \"hello \" + who\n"
                     "\n"
                     "\n"
                     "result = greet(nobody)\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto *const client = clientOf(*editor);
  const auto word =
      frame.document()->toPlainText().indexOf(QStringLiteral("nobody"));
  QVERIFY(word > 0);
  auto underlined = [&frame, word] {
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.cursor.selectionStart() <= word) &&
          (selection.cursor.selectionEnd() >= word + 6) &&
          (selection.format.fontUnderline())) {
        return true;
      }
    }
    return false;
  };
  QVERIFY2(waitFor(underlined, 30000),
           "the file was never read by the server of the session");
  // `stop-pylsp` ends the session, and the underlines of a program which is gone
  // say something which is no longer true: they go with it. The assertions which
  // follow run after the command, and a menu entry alone would answer them
  // whatever the command does, which is why the session is started again below
  editor->launchCommand(QStringLiteral("stop-pylsp"));
  QVERIFY2(client->isIdle(), "the command stopped no session");
  QVERIFY2(waitFor(
               [&frame] {
                 for (const auto &selection : frame.extraSelections()) {
                   if (isDiagnosticUnderline(selection)) {
                     return false;
                   }
                 }
                 return true;
               },
               5000),
           "a diagnostic of the stopped server is still drawn");
  QVERIFY2(editor->told(QStringLiteral("pylsp stopped")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // and the command which ends nothing says so, which is what makes a miss readable
  // instead of silent
  editor->launchCommand(QStringLiteral("stop-pylsp"));
  QVERIFY2(editor->told(QStringLiteral("pylsp is not started")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // the command which starts one again reaches the same client: what the stop took
  // away is the session the file would have started for itself
  editor->launchCommand(QStringLiteral("start-pylsp"));
  QVERIFY2(client->waitUntilReady(40000),
           "the command which starts a session started none");
  QVERIFY2(editor->told(QStringLiteral("pylsp started")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  editor.reset();
}  // end of TestPythonMode::stoppingTheSessionTakesTheUnderlinesAway

void TestPythonMode::theMenuOffersTheToolsOfTheLanguage() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: its menu is what is under "
          "test here");
  }
  const auto file = writeSource(QStringLiteral("menu/tools.py"),
                                QStringLiteral("value = 1\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto *const menu = pythonMenu(*editor);
  QVERIFY2(menu != nullptr, "the mode of the buffer built no menu of its own");
  // the entries of the menu, and the command name of each one of them in the
  // tooltip: a person who learns the entry by clicking it is told, by the same
  // tooltip, the name they would type in the minibuffer
  QVERIFY(entryOf(*menu, QStringLiteral("Start pylsp")) != nullptr);
  auto *const sort = entryOf(*menu, QStringLiteral("Sort imports"));
  QVERIFY2(sort != nullptr, "the menu holds no entry which sorts the imports");
  QVERIFY2(sort->toolTip().contains(QStringLiteral("python-sort-import")),
           qPrintable(sort->toolTip()));
  // the linters: the eight programs pylsp can be told to run, in the order of
  // the table of the mode, and each of them is checked against what it means —
  // an entry whose label is the program and whose address is the plugin
  auto *const linters = subMenuOf(*menu, QStringLiteral("Linters"));
  QVERIFY2(linters != nullptr, "the menu holds no submenu of linters");
  const QStringList linter_labels{QStringLiteral("Ruff"),
                                 QStringLiteral("Pyflakes"),
                                 QStringLiteral("Pycodestyle"),
                                 QStringLiteral("Flake8"),
                                 QStringLiteral("Pylint"),
                                 QStringLiteral("McCabe"),
                                 QStringLiteral("Pydocstyle"),
                                 QStringLiteral("Mypy")};
  auto labels = QStringList();
  for (auto *const action : linters->actions()) {
    labels << action->text();
  }
  QCOMPARE(labels, linter_labels);
  for (const auto &label : linter_labels) {
    auto *const action = entryOf(*linters, label);
    QVERIFY2(action != nullptr, qPrintable(label));
    QVERIFY2(action->isCheckable(), qPrintable(label));
    QVERIFY2(!action->toolTip().isEmpty(),
             qPrintable(QStringLiteral("%1 has no tooltip").arg(label)));
    // no exclusive group: the entries of this submenu are ticked together, and
    // the default of a `QActionGroup` is exclusiveness, which is the opposite
    // of the question (Qt 6.2)
    QVERIFY2(action->actionGroup() == nullptr,
             qPrintable(QStringLiteral("%1 stands in a group of its own and "
                                       "could not be ticked with the "
                                       "others").arg(label)));
  }
  // what is ticked is what the settings say reads the buffers, and the mode
  // says Ruff alone
  QVERIFY(entryOf(*linters, QStringLiteral("Ruff"))->isChecked());
  QVERIFY(!entryOf(*linters, QStringLiteral("Pycodestyle"))->isChecked());
  QVERIFY(!entryOf(*linters, QStringLiteral("Pylint"))->isChecked());
  // the two programs of a list, on the other hand, are one exclusive question
  // each: two formatters running at once is one answering and the other silent
  auto *const formatters = subMenuOf(*menu, QStringLiteral("Format with"));
  QVERIFY(formatters != nullptr);
  const QStringList formatter_labels{QStringLiteral("Ruff"),
                                     QStringLiteral("Black"),
                                     QStringLiteral("Autopep8"),
                                     QStringLiteral("Yapf"),
                                     QStringLiteral("None")};
  auto ticked = QStringList();
  for (auto *const action : formatters->actions()) {
    labels << action->text();
    if (action->isChecked()) {
      ticked << action->text();
    }
    QVERIFY2(action->isCheckable(), qPrintable(action->text()));
    QVERIFY2(action->actionGroup() != nullptr,
             qPrintable(QStringLiteral("%1 is not one of an exclusive "
                                       "question")
                            .arg(action->text())));
  }
  labels = QStringList();
  for (auto *const action : formatters->actions()) {
    labels << action->text();
  }
  QCOMPARE(labels, formatter_labels);
  QCOMPARE(ticked, QStringList() << QStringLiteral("Ruff"));
  auto *const sorters = subMenuOf(*menu, QStringLiteral("Sort imports with"));
  QVERIFY(sorters != nullptr);
  labels = QStringList();
  ticked = QStringList();
  for (auto *const action : sorters->actions()) {
    labels << action->text();
    if (action->isChecked()) {
      ticked << action->text();
    }
  }
  QCOMPARE(labels, QStringList() << QStringLiteral("Ruff")
                                << QStringLiteral("Isort")
                                << QStringLiteral("None"));
  QCOMPARE(ticked, QStringList() << QStringLiteral("Ruff"));
  // clicking a linter writes its own address, says what it did, and leaves the
  // linter which was already ticked ticked: several programs reading one text
  // is the normal case
  entryOf(*linters, QStringLiteral("Pycodestyle"))->trigger();
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/linters/pycodestyle")).toBool(),
           true);
  QVERIFY2(editor->told(QStringLiteral("Pycodestyle enabled")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // the menu is rebuilt for the buffer, and the entry of a submenu is not an
  // entry of the menu: `Linters` stands as the action which opens it, and what
  // was ticked is what the address now says
  auto *const second_menu = pythonMenu(*editor);
  QVERIFY(subMenuOf(*second_menu, QStringLiteral("Linters")) != nullptr);
  auto *const linters_again =
      subMenuOf(*second_menu, QStringLiteral("Linters"));
  QVERIFY(linters_again != nullptr);
  QVERIFY(entryOf(*linters_again, QStringLiteral("Ruff"))->isChecked());
  QVERIFY(entryOf(*linters_again, QStringLiteral("Pycodestyle"))->isChecked());
  // the last linter off is told, since a buffer with no underlines at all is
  // what the first switch of the first linter looks like to nobody
  entryOf(*linters_again, QStringLiteral("Ruff"))->trigger();
  QVERIFY2(editor->told(QStringLiteral("Ruff disabled")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QVERIFY(!editor->told(QStringLiteral("no linter is left enabled")));
  auto *const linters_third =
      subMenuOf(*pythonMenu(*editor), QStringLiteral("Linters"));
  entryOf(*linters_third, QStringLiteral("Pycodestyle"))->trigger();
  QVERIFY2(editor->told(QStringLiteral("no linter is left enabled")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // choosing the formatter and the sorter writes their address and ticks the
  // entry which names them, of a menu built after the choice
  entryOf(*subMenuOf(*pythonMenu(*editor), QStringLiteral("Format with")),
           QStringLiteral("Black"))
      ->trigger();
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/formatter")).toString(),
           QStringLiteral("black"));
  QVERIFY2(editor->told(QStringLiteral("formatting with Black")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  entryOf(*subMenuOf(*pythonMenu(*editor), QStringLiteral("Sort imports with")),
           QStringLiteral("Isort"))
      ->trigger();
  QCOMPARE(
      QSettings().value(QStringLiteral("pylsp/import-sorter")).toString(),
      QStringLiteral("isort"));
  // what the choice is worth to the person who made it: the plugin of `isort`
  // sorts as part of formatting the buffer, and the menu says so
  QVERIFY2(editor->told(QStringLiteral("Isort sorts as part of formatting")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // and choosing Ruff to format or to sort while the plugin of that name is off
  // says that the choice will answer nothing, since one flag of one plugin
  // decides all three of the things asked of it
  entryOf(*subMenuOf(*pythonMenu(*editor), QStringLiteral("Format with")),
           QStringLiteral("Ruff"))
      ->trigger();
  QVERIFY2(editor->told(QStringLiteral("Ruff plugin is switched off")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  auto *const sorter = entryOf(*subMenuOf(*pythonMenu(*editor),
                                         QStringLiteral("Sort imports with")),
                               QStringLiteral("Ruff"));
  QVERIFY(sorter != nullptr);
  sorter->trigger();
  QVERIFY2(editor->told(QStringLiteral("sorting the imports with Ruff")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  editor.reset();
}  // end of TestPythonMode::theMenuOffersTheToolsOfTheLanguage

  void TestPythonMode::theTabKeyGoesDownTheLevelsOfTheLine() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: the indentation key of the "
          "language is what is under test here");
  }
  // Every expectation below is what `python.el' answers on the same text, measured
  // press by press with `indent-for-tab-command' in a batch Emacs. A cycle is not
  // a thing a description pins down: the numbers are the specification, and the
  // shape which starts the list is the one whose cycle was complained about.
  struct Shape {
    const char *name;
    const char *source;
    int line;
    int start;
    const char *expected;
  };
  const Shape shapes[] = {
      // the statement under two `if' which hold no body yet: the first press puts
      // it in the inner one, the next two take it out of a block and then out of
      // the other, and the fourth goes back to the top of the ladder
      {"two blocks",
       "print('toto')\nif True:\n    if False:\nprint('toto')", 3, 0,
       "8 4 0 8"},
      // the same line started at the top of its own ladder: the presses walk the
      // same four places, because the cycle is decided by the blocks above the
      // line and not by how the line came to be where it is
      {"two blocks from the top",
       "print('toto')\nif True:\n    if False:\nprint('toto')", 3, 8,
       "8 4 0 8"},
      // the continuation line of a call is indented by the opening of that call,
      // which is no multiple of the level of the language: the ladder holds that
      // place, the level below it, and nothing
      {"arguments of a call", "def f(a,\nb)", 1, 0, "6 4 0 6"},
      {"three blocks",
       "def f():\n    if a:\n        if b:\npass", 3, 0, "12 8 4 0"},
      {"body of a function", "def f():\nq", 1, 0, "4 0 4 0"},
      // the line which follows the `return' of the block above it stands in no
      // block at all: the only level it has is the one it is already at, and the
      // key leaves it there instead of walking a ladder of a single place
      {"after a return", "def f():\n    return 1\nq", 2, 0, "0 0 0 0"},
      // the `else' of a block whose body ends with a `return' is read as standing
      // in the `if' above it, and no press moves a line which is already where the
      // language puts it
      {"else of a returned block",
       "def f():\n    if a:\n        return 1\n    else:", 3, 0, "4 4 4 4"},
      {"else three blocks deep",
       "if a:\n    if b:\n        if c:\n            pass\nelse:", 4, 0,
       "8 4 0 8"},
      {"closing parenthesis alone", "foo(\n    a,\nb)", 2, 0, "4 0 4 0"},
      {"body of a docstring", "def f():\n    \"\"\"doc\nmore\"\"\"\nq", 3, 0,
       "4 0 4 0"},
      // the statement after the body of an `except' is back at the left margin, and
      // the ladder of a line which opens no block of its own is one place deep
      {"after the body of an except",
       "try:\n    x = 1\nexcept E:\n    pass\nq", 4, 0, "0 0 0 0"},
      // a comment takes the indentation of the line it is read as standing over
      {"comment which copies a line", "if True:\n    # c\nq", 2, 0,
       "4 0 4 0"}};
  auto n = 0;
  for (const auto &s : shapes) {
    // a fresh buffer per shape and per starting place: the mode tells a press
    // which continues the cycle of the previous one from the line it finds and the
    // place it stands at, so a run which reused a buffer would inherit the cycle of
    // the run before it
    const auto file =
        writeSource(QStringLiteral("indent/shape-%1.py").arg(++n),
                    QString::fromUtf8(s.source));
    QVERIFY2(!file.isEmpty(), "the source of a shape could not be written");
    auto editor = openInPythonMode(file);
    auto &buffer = editor->getCurrentBuffer();
    const auto seen =
        pressesOf(buffer.getMainFrame(), s.line, s.start, 4, Qt::Key_Tab,
                  Qt::NoModifier);
    const auto expected = QString::fromUtf8(s.expected);
    QVERIFY2(seen == expected,
             qPrintable(QStringLiteral("%1: the presses left the line at [%2], "
                                       "not at [%3] as `python.el' leaves it")
                            .arg(QString::fromUtf8(s.name), seen, expected)));
    editor.reset();
  }
}  // end of TestPythonMode::theTabKeyGoesDownTheLevelsOfTheLine

void TestPythonMode::shiftTabTakesTheLineOutOfItsBlock() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: the key which takes a line out of "
          "its block is what is under test here");
  }
  // `Shift-Tab' walks the ladder `Tab' goes down, from the place the line stands
  // at, and stops at the leftmost place of it: a key which wrapped would put a line
  // back in the block it just left, which is what the other key is for. The
  // ladders are the measured ones, the same numbers as in the case above read from
  // their top.
  struct Shape {
    const char *name;
    const char *source;
    int line;
    int start;
    const char *expected;
  };
  const Shape shapes[] = {
      {"two blocks",
       "print('toto')\nif True:\n    if False:\nprint('toto')", 3, 8, "4 0 0 0"},
      {"three blocks",
       "def f():\n    if a:\n        if b:\npass", 3, 12, "8 4 0 0"},
      {"body of a function", "def f():\nq", 1, 4, "0 0 0 0"},
      {"arguments of a call", "def f(a,\nb)", 1, 6, "4 0 0 0"},
      {"closing parenthesis alone", "foo(\n    a,\nb)", 2, 4, "0 0 0 0"}};
  auto n = 0;
  for (const auto &s : shapes) {
    const auto file =
        writeSource(QStringLiteral("dedent/shape-%1.py").arg(++n),
                    QString::fromUtf8(s.source));
    QVERIFY2(!file.isEmpty(), "the source of a shape could not be written");
    auto editor = openInPythonMode(file);
    auto &buffer = editor->getCurrentBuffer();
    const auto seen =
        pressesOf(buffer.getMainFrame(), s.line, s.start, 4, Qt::Key_Backtab,
                  Qt::ShiftModifier);
    const auto expected = QString::fromUtf8(s.expected);
    QVERIFY2(seen == expected,
             qPrintable(QStringLiteral("%1: the presses left the line at [%2], "
                                       "not at [%3]")
                            .arg(QString::fromUtf8(s.name), seen, expected)));
    editor.reset();
  }
}  // end of TestPythonMode::shiftTabTakesTheLineOutOfItsBlock

void TestPythonMode::theTabKeyMovesARegionOneLevelRight() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: what the tab key does with a "
          "selection is what is under test here");
  }
  const auto source = QStringLiteral("def f():\n"
                                     "    if a:\n"
                                     "        return 1\n"
                                     "\n"
                                     "    return 0");
  const auto file = writeSource(QStringLiteral("indent/region.py"), source);
  QVERIFY2(!file.isEmpty(), "the source of the region could not be written");
  auto editor = openInPythonMode(file);
  auto &buffer = editor->getCurrentBuffer();
  auto *const w = buffer.getMainFrame().widget();
  auto *const view = qobject_cast<QPlainTextEdit *>(w);
  QVERIFY2(view != nullptr,
           "the main frame of a Python buffer is not a plain text edit");
  // A region is moved, not reindented: every non-blank line of it goes one level to
  // the right, and the inner levels of the block stay the difference between the
  // lines they are made of. The blank line of the region holds nothing to move and
  // is left as empty as it is, since an empty line is how this language separates
  // two functions from each other.
  //
  // The selection is made by the test and not by a mouse, and it is made again
  // before each press: the machinery which keeps the cursor next to the character it
  // was next to while the region moved leaves no selection behind.
  auto region = [view](const int lines) {
    auto c = QTextCursor(view->document());
    c.movePosition(QTextCursor::Start);
    c.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, lines);
    return c;
  };
  buffer.getMainFrame().setTextCursor(region(4));
  QTest::keyClick(w, Qt::Key_Tab);
  QTest::qWait(50);
  QCOMPARE(view->toPlainText(),
           QStringLiteral("    def f():\n"
                          "        if a:\n"
                          "            return 1\n"
                          "\n"
                          "        return 0"));
  // and the key which asks for nothing back takes the region to the place it stood
  // at, blank line included
  buffer.getMainFrame().setTextCursor(region(4));
  QTest::keyClick(w, Qt::Key_Backtab, Qt::ShiftModifier);
  QTest::qWait(50);
  QCOMPARE(view->toPlainText(), source);
  // A selection which stays inside one line is not a region: the key indents the
  // line its character stands on and leaves the lines beside it alone. The check
  // needs a line with somewhere to go, which none of the lines above is — each of
  // them stands at the top of its own ladder — so it is made in a buffer of its own
  // where the body of a `def' waits at the left margin. A region would have moved
  // the header of the function along with it, and that is what the answer of the
  // second line says.
  const auto alone =
      writeSource(QStringLiteral("indent/alone.py"),
                  QStringLiteral("def f():\nq"));
  QVERIFY2(!alone.isEmpty(), "the source of the line could not be written");
  auto second = openInPythonMode(alone);
  auto *const w2 = second->getCurrentBuffer().getMainFrame().widget();
  auto *const view2 = qobject_cast<QPlainTextEdit *>(w2);
  QVERIFY2(view2 != nullptr,
           "the main frame of a Python buffer is not a plain text edit");
  auto in_line = QTextCursor(view2->document());
  in_line.movePosition(QTextCursor::Start);
  in_line.movePosition(QTextCursor::Down);
  in_line.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
  second->getCurrentBuffer().getMainFrame().setTextCursor(in_line);
  QTest::keyClick(w2, Qt::Key_Tab);
  QTest::qWait(50);
  QCOMPARE(view2->toPlainText(), QStringLiteral("def f():\n    q"));
  editor.reset();
}  // end of TestPythonMode::theTabKeyMovesARegionOneLevelRight

void TestPythonMode::anInterpreterWhichIsNotFoundIsNamedAndNoKeyIsLost() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: the report of an interpreter "
          "which is not there is what is under test here");
  }
  const auto file =
      writeSource(QStringLiteral("interpreter/missing.py"),
                  QStringLiteral("print('nothing')"));
  QVERIFY2(!file.isEmpty(), "the source of the buffer could not be written");
  // The program of the configuration is a path which is not there. Two things are
  // worth asserting: that the person is told *which* program failed, since the path
  // is theirs by now and not one the mode invented, and that the keys of the session
  // are claimed whatever the outcome — an unclaimed `Ctrl-C Ctrl-L' reads as
  // `unknown shortcut', a report about the key bindings of the editor, for a
  // problem of a path.
  QSettings().setValue(QStringLiteral("python/executable"),
                       QStringLiteral("/no/such/python-not-here"));
  auto editor = openInPythonMode(file);
  auto &buffer = editor->getCurrentBuffer();
  auto *const w = buffer.getMainFrame().widget();
  QTest::keyClick(w, Qt::Key_C, Qt::ControlModifier);
  QTest::keyClick(w, Qt::Key_L, Qt::ControlModifier);
  // a program which is not there is not reported inside `start': Qt reports the
  // failure through `errorOccurred' a few milliseconds later, so the report is
  // waited for rather than read at once
  QVERIFY2(
      waitFor(
          [&]() {
            return editor->told(QStringLiteral("could not be started"));
          },
          5000),
      qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QVERIFY2(
      editor->told(QStringLiteral("/no/such/python-not-here")),
      qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QVERIFY2(!editor->told(QStringLiteral("unknown shortcut")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // and the frame is taken away, not left as a page which names no process: the
  // detach of a session whose program never came up is the one way the mode loses
  // sight of it, and a bare `close' would leave the registry of the secondary
  // tasks holding a dangling pointer
  auto *const frame = qobject_cast<tfel::gui::ProcessInteractionFrame *>(
      buffer.getCurrentSecondaryTask());
  QVERIFY2((frame == nullptr) || (frame->getProcess().state() == QProcess::NotRunning),
           "the frame of a program which did not start was left attached");
  editor.reset();
}  // end of TestPythonMode::anInterpreterWhichIsNotFoundIsNamedAndNoKeyIsLost

void TestPythonMode::theInterpreterRunsWhatTheBufferSends() {
  if (!this->hasInterpreter()) {
    QSKIP("the packaged Python mode or an interpreter is missing: what the "
          "session answers to the buffer is what is under test here");
  }
  // a buffer whose lines are a function whose body holds an empty line, and the
  // call of that function: the empty line of a body is what the interactive parser
  // of the interpreter reads as the end of the block, so the lines of a region can
  // not be written to it as they stand and the answer of the call is what says
  // they went whole
  const auto file = writeSource(
      QStringLiteral("interpreter/session.py"),
      QStringLiteral("def twice(value):\n"
                     "\n"
                     "    return 2 * value\n"
                     "\n"
                     "print(twice(21))\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto &buffer = editor->getCurrentBuffer();
  auto *const w = buffer.getMainFrame().widget();
  auto *const view = qobject_cast<QPlainTextEdit *>(w);
  QVERIFY(view != nullptr);
  // the selection of the function: its three lines, and not the empty line which
  // separates it from the call below it
  auto c = QTextCursor(view->document());
  c.movePosition(QTextCursor::Start);
  c.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 3);
  buffer.getMainFrame().setTextCursor(c);
  QTest::keyClick(w, Qt::Key_C, Qt::ControlModifier);
  QTest::keyClick(w, Qt::Key_R, Qt::ControlModifier);
  // the send is what started the session, and its frame is a page of the buffer
  auto *const out = qobject_cast<tfel::gui::ProcessInteractionFrame *>(
      buffer.getCurrentSecondaryTask());
  QVERIFY2(out != nullptr,
           "sending a region started no session of the interpreter");
  // what the session shows is the text edit of the frame, and the frame is a frame
  // of the editor, not a `QPlainTextEdit': the text of the session is read through
  // the widget it hosts
  auto *const shown = qobject_cast<QPlainTextEdit *>(out->widget());
  QVERIFY2(shown != nullptr,
           "the frame of the Python session holds no plain text edit");
  QVERIFY2(!editor->told(QStringLiteral("unknown shortcut")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // the call of the function is a line of the buffer: it goes to the session as it
  // stands, and the answer is what says the `def' above it reached the same
  // interpreter and stands there.
  //
  // The line is looked for by its number and the text found there is asserted, not
  // taken from `End' of the document: a file which ends with a newline has an empty
  // block as its last one, and sending the block the cursor is left on by `End' and
  // `StartOfBlock' sends an empty line, which the interpreter answers with a prompt
  // and nothing else — the symptom of a session which is up and answers nothing
  auto call = QTextCursor(view->document());
  call.movePosition(QTextCursor::Start);
  for (auto i = 0; i != 4; ++i) {
    call.movePosition(QTextCursor::NextBlock);
  }
  QVERIFY2(call.block().text().startsWith(QStringLiteral("print(")),
           qPrintable(call.block().text()));
  call.movePosition(QTextCursor::StartOfBlock);
  buffer.getMainFrame().setTextCursor(call);
  QTest::keyClick(w, Qt::Key_C, Qt::ControlModifier);
  QTest::keyClick(w, Qt::Key_L, Qt::ControlModifier);
  QVERIFY2(waitFor(
               [&]() {
                 return shown->toPlainText().contains(QStringLiteral("42"));
               },
               40000),
           qPrintable(shown->toPlainText()));
  // and the empty line of the body asked nothing about where the block ends: the
  // answer of the call is the only shape of a text which went whole
  QVERIFY2(
      !shown->toPlainText().contains(QStringLiteral("IndentationError")),
      qPrintable(shown->toPlainText()));
  // The whole buffer goes the same way the region did, and its answer is the second
  // one: the file the buffer was read from holds the `def' and the call, so running
  // the whole of it prints 42 again. A send which never reached the interpreter —
  // the trap of an unquoted file name, and the trap of a parenthesis left out, both
  // of which left the session waiting for the end of a line — is heard as nothing
  // happening here, since the answer of the call above is on the screen already
  const auto answers = shown->toPlainText().count(QStringLiteral("42"));
  QTest::keyClick(w, Qt::Key_C, Qt::ControlModifier);
  QTest::keyClick(w, Qt::Key_B, Qt::ControlModifier);
  QVERIFY2(waitFor(
               [&]() {
                 return shown->toPlainText().count(QStringLiteral("42")) > answers;
               },
               40000),
           qPrintable(shown->toPlainText()));
  QVERIFY2(
      !shown->toPlainText().contains(QStringLiteral("SyntaxError")),
      qPrintable(shown->toPlainText()));
  editor.reset();
}  // end of TestPythonMode::theInterpreterRunsWhatTheBufferSends

void TestPythonMode::theTranscriptOfTheSessionReadsAsAConversation() {
  if (!this->hasInterpreter()) {
    QSKIP("the packaged Python mode or an interpreter is missing: the shape "
          "of the session transcript is what is under test here");
  }
  // the line of the report: the same line sent twice to a session. A pipe echoes
  // nothing, so the transcript is what the mode writes of it: the echo of each
  // send at the prompt that line was written to, and the answer after it —
  // `>>> print('toto')' over `toto', twice. A line written to the interpreter as
  // it is asked, instead of at a prompt, answers before any echo can be placed
  // (the banner and the answer of the first line can share one piece of the
  // stream), and the transcript then reads `>>> toto': the answer standing
  // behind the prompt of the line it answers, with the line itself nowhere
  const auto file = writeSource(
      QStringLiteral("interpreter/transcript.py"),
      QStringLiteral("print('toto')\n"));
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  auto &buffer = editor->getCurrentBuffer();
  auto *const w = buffer.getMainFrame().widget();
  auto *const view = qobject_cast<QPlainTextEdit *>(w);
  QVERIFY(view != nullptr);
  // the block the cursor stands in is the line sent: block 0 here, found from
  // `Start' and asserted before it is sent, the trap of the empty last block of
  // a file which ends with a newline
  auto c = QTextCursor(view->document());
  c.movePosition(QTextCursor::Start);
  QVERIFY2(c.block().text() == QStringLiteral("print('toto')"),
           qPrintable(c.block().text()));
  buffer.getMainFrame().setTextCursor(c);
  // the first press starts the session and sends the line
  QTest::keyClick(w, Qt::Key_C, Qt::ControlModifier);
  QTest::keyClick(w, Qt::Key_L, Qt::ControlModifier);
  auto *const out = qobject_cast<tfel::gui::ProcessInteractionFrame *>(
      buffer.getCurrentSecondaryTask());
  QVERIFY2(out != nullptr,
           "sending a line started no session of the interpreter");
  auto *const shown = qobject_cast<QPlainTextEdit *>(out->widget());
  QVERIFY2(shown != nullptr,
           "the frame of the Python session holds no plain text edit");
  // the second press sends the same line again, which is the run of the report
  QTest::keyClick(w, Qt::Key_C, Qt::ControlModifier);
  QTest::keyClick(w, Qt::Key_L, Qt::ControlModifier);
  // the transcript waits for both echoes and both answers; the empty parts are
  // dropped, since the echo of a line opens the block its answer is written on
  // and the last prompt of a session is followed by nothing
  auto lines = QStringList();
  const auto transcript = [shown]() {
    auto text = shown->toPlainText();
    text.replace(QChar(0x2029), QChar('\n'));
    auto parts = text.split(QLatin1Char('\n'));
    for (auto i = parts.size() - 1; i >= 0; --i) {
      if (parts.at(i).isEmpty()) {
        parts.removeAt(i);
      }
    }
    return parts;
  };  // end of transcript()
  QVERIFY2(waitFor(
               [&]() {
                 lines = transcript();
                 return (lines.count(QStringLiteral("toto")) == 2) &&
                        (lines.count(
                             QStringLiteral(">>> print('toto')")) == 2);
               },
               40000),
           qPrintable(lines.join(QStringLiteral(" | "))));
  // The raw blocks, not the filtered lines: the answer stands **directly under**
  // its echo. A frame which cuts a paragraph at every read of the pipe leaves an
  // empty line between the two — the echo opens the block its answer is written
  // on, and a forced new paragraph strands that block empty
  auto *const doc = shown->document();
  auto echoes = QVector<int>();
  for (auto b = doc->begin(); b.isValid(); b = b.next()) {
    if (b.text() == QStringLiteral(">>> print('toto')")) {
      echoes << b.blockNumber();
    }
  }
  QCOMPARE(echoes.size(), 2);
  // the first echo stands below the banner, which is two lines deep
  QVERIFY2(echoes.at(0) >= 2,
           "the echo of the first line stands at or above the banner");
  for (auto i = 0; i != echoes.size(); ++i) {
    const auto answer = doc->findBlockByNumber(echoes.at(i) + 1).text();
    QVERIFY2(answer == QStringLiteral("toto"),
             qPrintable(QStringLiteral("the block under echo %1 is [%2]")
                            .arg(i)
                            .arg(answer)));
  }
  // and the two exchanges stand directly in one another's track: an echo per
  // prompt, in the order of the presses, with nothing inserted between
  QCOMPARE(echoes.at(1), echoes.at(0) + 2);
  // the shape of the defect is nowhere: a line whose answer stands behind the
  // prompt is the report itself
  QVERIFY2(!lines.contains(QStringLiteral(">>> toto")),
           qPrintable(lines.join(QStringLiteral(" | "))));
  editor.reset();
}  // end of TestPythonMode::theTranscriptOfTheSessionReadsAsAConversation

void TestPythonMode::theToolsOfTheDialogChooseRuffAndNoHistory() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: its entries of the "
          "configuration are what is under test here");
  }
  const auto groups = tfel::gui::MajorModeFactory::getMajorModeFactory()
                          .getMajorModeSettingsGroups(QStringLiteral("Python"));
  // the mode drives two programs: the server which reads the file and the
  // interpreter which runs it, and each of them is a theme named after itself
  QVERIFY2(groups.size() == 2,
           "the mode declares a theme per program it drives, not one per mode");
  auto theme = tfel::gui::SettingsGroup();
  auto interpreter = tfel::gui::SettingsGroup();
  for (const auto &group : groups) {
    if (group.name == QLatin1String("pylsp")) {
      theme = group;
    } else if (group.name == QLatin1String("python")) {
      interpreter = group;
    }
  }
  QVERIFY2(!theme.keys.isEmpty(), qPrintable(QStringLiteral(
             "no theme named `pylsp` among the %1 groups of the mode")
             .arg(groups.size())));
  QVERIFY2(!interpreter.keys.isEmpty(),
           qPrintable(QStringLiteral("no theme named `python` among the %1 "
                                     "groups of the mode")
                        .arg(groups.size())));
  QVERIFY2(theme.mode == QLatin1String("Python") &&
               interpreter.mode == QLatin1String("Python"),
           "a theme of a mode says which mode exported it");
  // the addresses the mode reads, gathered by address: an assertion on a list
  // of entries says nothing about which one of them is wrong
  auto kinds = QMap<QString, tfel::gui::SettingsKey::Type>();
  auto booleans = QMap<QString, bool>();
  auto texts = QMap<QString, QString>();
  auto choices = QMap<QString, QStringList>();
  for (const auto &key : theme.keys) {
    kinds.insert(key.key, key.type);
    if (key.type == tfel::gui::SettingsKey::BOOLEAN) {
      // the default of a switch is read as a bool: `QVariant::toString()` of a
      // `false` is the empty string in Qt 6, there is no conversion from a bool
      // to a text registered
      booleans.insert(key.key, key.default_value.toBool());
    } else {
      texts.insert(key.key, key.default_value.toString());
    }
    auto values = QStringList();
    for (const auto &choice : key.choices) {
      values << choice.value.toString();
    }
    choices.insert(key.key, values);
  }
  // the three addresses of the session itself are still there
  QVERIFY(kinds.contains(QStringLiteral("pylsp/executable")));
  QVERIFY(kinds.contains(QStringLiteral("pylsp/arguments")));
  // the eight linters, each an entry of its own, with Ruff alone on: what reads
  // the buffers is not one choice among eight but eight switches, since each
  // program sees a mistake the others do not
  const auto linters = QMap<QString, bool>{
      {QStringLiteral("ruff"), true},      {QStringLiteral("pyflakes"), false},
      {QStringLiteral("pycodestyle"), false}, {QStringLiteral("flake8"), false},
      {QStringLiteral("pylint"), false},   {QStringLiteral("mccabe"), false},
      {QStringLiteral("pydocstyle"), false}, {QStringLiteral("pylsp_mypy"), false}};
  for (auto it = linters.constBegin(); it != linters.constEnd(); ++it) {
    // the address of a linter is named after the plugin it switches, which is
    // also the key the configuration of the server is built with: the menu case
    // `theMenuOffersTheToolsOfTheLanguage` clicks one of these entries and reads
    // the same address back from the file, which is what makes the two surfaces
    // of the same question unable to disagree with each other
    const auto address = QStringLiteral("pylsp/linters/") + it.key();
    QVERIFY2(kinds.contains(address), qPrintable(address));
    QCOMPARE(kinds.value(address), tfel::gui::SettingsKey::BOOLEAN);
    QVERIFY(booleans.contains(address));
    QCOMPARE(booleans.value(address), it.value());
  }
  // the formatter and the sorter are the choices they are offered, with Ruff
  // first and chosen: the program which does the work of two others is what a
  // person who installs nothing gets
  QCOMPARE(kinds.value(QStringLiteral("pylsp/formatter")),
           tfel::gui::SettingsKey::CHOICE);
  QCOMPARE(choices.value(QStringLiteral("pylsp/formatter")),
           QStringList() << QStringLiteral("ruff") << QStringLiteral("black")
                        << QStringLiteral("autopep8") << QStringLiteral("yapf")
                        << QStringLiteral("none"));
  QCOMPARE(texts.value(QStringLiteral("pylsp/formatter")),
           QStringLiteral("ruff"));
  QCOMPARE(kinds.value(QStringLiteral("pylsp/import-sorter")),
           tfel::gui::SettingsKey::CHOICE);
  QCOMPARE(choices.value(QStringLiteral("pylsp/import-sorter")),
           QStringList() << QStringLiteral("ruff") << QStringLiteral("isort")
                        << QStringLiteral("none"));
  QCOMPARE(texts.value(QStringLiteral("pylsp/import-sorter")),
           QStringLiteral("ruff"));
  // no history, and nothing the editor remembers by itself: a `CommandLine` is
  // given an address and writes its answer there, so such a field would show a
  // stale list and be overwritten by the next prompt on its own account
  for (const auto &key : theme.keys) {
    QVERIFY2(!key.key.contains(QStringLiteral("history")),
             qPrintable(key.key));
    QVERIFY2(!key.key.contains(QStringLiteral("recent")), qPrintable(key.key));
    // and every entry of the theme is about the server of the language
    QVERIFY2(key.key.startsWith(QStringLiteral("pylsp/")), qPrintable(key.key));
    // an entry of its own theme needs no group of its own
    QVERIFY2(key.group.isEmpty(), qPrintable(key.key));
    QVERIFY2(!key.label.isEmpty(), qPrintable(key.key));
    QVERIFY2(key.description.size() > 40, qPrintable(key.key));
  }
  // three addresses of the session, eight linters, two programs of a list
  QCOMPARE(theme.keys.size(), 13);
  // The theme of the interpreter holds the one thing a person has to name: the
  // program a session runs. Its default is the program the mode ran before the
  // entry existed, so the field shows what would be started by a key which asks
  // for one, and an answer equal to that name is removed from the file rather
  // than stored. What the mode remembers of the commands it was asked to run is
  // a history, and a history is no entry of the configuration.
  QCOMPARE(interpreter.keys.size(), 1);
  const auto &program = interpreter.keys.at(0);
  QCOMPARE(program.key, QStringLiteral("python/executable"));
  QCOMPARE(program.type, tfel::gui::SettingsKey::STRING);
  QCOMPARE(program.default_value.toString(), QStringLiteral("python3"));
  QVERIFY(!program.label.isEmpty());
  QVERIFY(program.description.size() > 40);
  QVERIFY(program.group.isEmpty());
}  // end of
   // TestPythonMode::theToolsOfTheDialogChooseRuffAndNoHistory

void TestPythonMode::theCommandsOfTheToolsRefuseAnotherMode() {
  if (!this->modeLoaded) {
    QSKIP("the packaged Python mode is missing: its commands are not "
          "registered at all");
  }
  const auto file = writeSource(QStringLiteral("refusals/other.cc"),
                                QStringLiteral("int main() { return 0; }\n"));
  QVERIFY(!file.isEmpty());
  auto editor = std::make_unique<RecordingEditor>();
  editor->show();
  editor->openFile(file);
  editor->getCurrentBuffer().getMainFrame().setMajorMode(
      QStringLiteral("C++"));
  QTest::qWait(200);
  // the commands are about a session of the language and the buffers that
  // session reads: run from a buffer of another language, each of them writes
  // an address that mode never reads or asks a question no server of it has an
  // answer to, so they say what they refuse instead
  const QStringList commands{QStringLiteral("python-sort-import"),
                             QStringLiteral("python-set-formatter"),
                             QStringLiteral("python-set-import-sorter"),
                             QStringLiteral("python-enable-pyflakes"),
                             QStringLiteral("python-disable-ruff"),
                             QStringLiteral("python-enable-mypy")};
  for (const auto &command : commands) {
    editor->launchCommand(command);
    QVERIFY2(editor->told(QStringLiteral("is a Python command: the major mode "
                                         "of the current buffer is C++")),
             qPrintable(QStringLiteral("%1 said: %2")
                            .arg(command,
                                 editor->messages.join(
                                   QStringLiteral(" | ")))));
  }
  // the refusal of a question installs no question: the minibuffer is left to
  // the mode which is on screen
  QVERIFY2(!editor->hasUserInput(),
           "a command of the language installed a question in a buffer of "
           "another one");
  // and a refusal writes nothing
  const QStringList addresses{QStringLiteral("pylsp/linters/pyflakes"),
                              QStringLiteral("pylsp/linters/ruff"),
                              QStringLiteral("pylsp/linters/pylsp_mypy"),
                              QStringLiteral("pylsp/formatter"),
                              QStringLiteral("pylsp/import-sorter")};
  for (const auto &address : addresses) {
    QVERIFY2(QSettings().value(address).isNull(), qPrintable(address));
  }
  // the same commands, in a buffer of the language, do the thing they name:
  // what refuses is the mode of the buffer and nothing else
  editor->getCurrentBuffer().getMainFrame().setMajorMode(
      QStringLiteral("Python"));
  QTest::qWait(200);
  editor->launchCommand(QStringLiteral("python-enable-pycodestyle"));
  QVERIFY2(editor->told(QStringLiteral("Pycodestyle enabled")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/linters/pycodestyle")).toBool(),
           true);
  editor.reset();
}  // end of TestPythonMode::theCommandsOfTheToolsRefuseAnotherMode

void TestPythonMode::switchingALinterChangesWhatStandsUnderTheWords() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  // one file with one mistake which `ruff` alone sees — a name which is nowhere
  // defined, of the `F` rules it runs by default — and one which only
  // `pycodestyle` sees: the missing space after a colon is `E231`, of the `E2`
  // family, which the default rules of `ruff` leave to the style checker
  const auto source = QStringLiteral("import sys\n"
                                     "\n"
                                     "mapping = {\"a\":1}\n"
                                     "result = nobody(sys)\n");
  const auto file = writeSource(QStringLiteral("linters/two_checkers.py"), source);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  const auto undefined = source.indexOf(QStringLiteral("nobody"));
  const auto colon = source.indexOf(QStringLiteral("\":"));
  QVERIFY((undefined > 0) && (colon > 0));
  // what the buffer shows under one of its own positions, among the faces the
  // server draws a report with
  auto underlined = [&frame](const int position, const int length) {
    for (const auto &selection : frame.extraSelections()) {
      if ((selection.cursor.selectionStart() <= position) &&
          (selection.cursor.selectionEnd() >= position + length) &&
          isDiagnosticUnderline(selection)) {
        return true;
      }
    }
    return false;
  };
  // with the choice the mode makes — Ruff alone — the undefined name stands
  // under an underline and the style mistake of the line above stands under
  // none: the first is the report of the program which is on, the second is
  // the report of the one which is off
  QVERIFY2(waitFor([&] { return underlined(undefined, 6); }, 30000),
           "the undefined name of the file was underlined by nothing");
  QTest::qWait(1500);
  QVERIFY2(!underlined(colon + 1, 1),
           "a style mistake of `pycodestyle` stands under an underline while "
           "that program is off");
  // turning the second program on hands the choice to the session which is
  // running, and the underline of the mistake it sees arrives without a key
  // being typed in the buffer
  editor->launchCommand(QStringLiteral("python-enable-pycodestyle"));
  QVERIFY2(editor->told(QStringLiteral("Pycodestyle enabled")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QVERIFY2(
      waitFor([&] { return underlined(colon + 1, 1); }, 30000),
      "the line too tight of the file was left ununderlined once pycodestyle "
      "was turned on");
  // turning Ruff off takes its own underline away and leaves the other one: the
  // server republishes what it finds under the settings it was handed, and the
  // buffer draws what the last answer says
  editor->launchCommand(QStringLiteral("python-disable-ruff"));
  QVERIFY2(editor->told(QStringLiteral("Ruff disabled")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QVERIFY2(waitFor([&] { return !underlined(undefined, 6); }, 30000),
           "the report of a program which was turned off is still drawn under "
           "the name it is about");
  QVERIFY(underlined(colon + 1, 1));
  editor.reset();
}  // end of
   // TestPythonMode::switchingALinterChangesWhatStandsUnderTheWords

void TestPythonMode::theFormatterChoiceDecidesWhoAnswers() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  if (QStandardPaths::findExecutable(QStringLiteral("ruff")).isEmpty()) {
    QSKIP("the test formats with ruff first, and the program is not there");
  }
  if (QStandardPaths::findExecutable(QStringLiteral("autopep8")).isEmpty()) {
    QSKIP("the test formats the same buffer with autopep8 after it, and the "
          "program is not there");
  }
  // the one difference between the two programs which is not a matter of taste:
  // `ruff` and `black` write the string with double quotes, `autopep8` is the
  // smallest change which makes the style right and leaves the quotes of the
  // file where they are. Both fix the spaces of the call, so an answer which
  // kept the quotes *and* the spaces would read as nothing answering
  const auto rough = QStringLiteral("def total( a,b ):\n"
                                    "    return 'abc' + str(a)\n"
                                    "\n"
                                    "\n"
                                    "value = total(1,2)\n");
  const auto file = writeSource(QStringLiteral("formatter/two_programs.py"), rough);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  auto format = [&frame, &rough]() {
    auto spent = QElapsedTimer();
    spent.start();
    while (spent.elapsed() < 60000) {
      QTest::keyClick(frame.widget(), Qt::Key_Q, Qt::AltModifier);
      QTest::qWait(400);
      if (frame.document()->toPlainText() != rough) {
        return true;
      }
    }
    return false;
  };
  // the choice the mode makes by default: Ruff, which writes what Black writes.
  // The question of the formatter is asked *before* the key which needs an
  // answer is pressed: `EditorWidget::launchCommand` reports a key whose command
  // asks the minibuffer as "command attempted to use the minibuffer while in
  // the minibuffer", and a test which presses the key first is left with a
  // refusal on the lips of the editor and no question to answer
  editor->launchCommand(QStringLiteral("python-set-formatter"));
  auto *const first_question = question(*editor, "pylsp-formatter");
  QVERIFY2(first_question != nullptr,
           qPrintable(QStringLiteral("the command which chooses the formatter "
                                     "asked nothing: %1")
                          .arg(editor->messages.join(QStringLiteral(" | ")))));
  lineEditOf(*first_question)->setText(QStringLiteral("ruff"));
  first_question->userEditingFinished();
  QVERIFY2(format(), qPrintable(frame.document()->toPlainText()));
  QVERIFY2(frame.document()->toPlainText().contains(QStringLiteral("\"abc\"")),
           qPrintable(frame.document()->toPlainText()));
  // answering the question of the formatter with a prefix of a name is enough:
  // `auto` is `autopep8` among the five of this list, and the answer reaches the
  // program which is chosen, not the one which was typed
  editor->launchCommand(QStringLiteral("python-set-formatter"));
  auto *const question_of_formatter = question(*editor, "pylsp-formatter");
  QVERIFY2(question_of_formatter != nullptr,
           "the command which chooses the formatter asked nothing in the "
           "minibuffer");
  auto *const input = lineEditOf(*question_of_formatter);
  QVERIFY(input != nullptr);
  input->setText(QStringLiteral("auto"));
  question_of_formatter->userEditingFinished();
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/formatter")).toString(),
           QStringLiteral("autopep8"));
  QVERIFY2(editor->told(QStringLiteral("formatting with Autopep8")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // the same question, on the same text, under the choice which was just made:
  // the answer is the one of the other program, and the difference between them
  // is the quote of the string. What is asked is the key of the mode; what
  // answers is the program the setting names
  frame.document()->setPlainText(rough);
  QTest::qWait(300);
  QVERIFY2(format(), qPrintable(frame.document()->toPlainText()));
  const auto written = frame.document()->toPlainText();
  QVERIFY2(written.contains(QStringLiteral("'abc'")), qPrintable(written));
  QVERIFY2(written.contains(QStringLiteral("def total(a, b):")),
           qPrintable(written));
  // and the choice of `none` says what it means in the minibuffer, which is the
  // only place a person can learn that the key of the mode will do nothing
  editor->launchCommand(QStringLiteral("python-set-formatter"));
  auto *const none_question = question(*editor, "pylsp-formatter");
  QVERIFY(none_question != nullptr);
  lineEditOf(*none_question)->setText(QStringLiteral("none"));
  none_question->userEditingFinished();
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/formatter")).toString(),
           QStringLiteral("none"));
  QVERIFY2(editor->told(QStringLiteral("no formatter")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // a name of no program among the five is answered with what the five are
  editor->launchCommand(QStringLiteral("python-set-formatter"));
  auto *const bad_question = question(*editor, "pylsp-formatter");
  QVERIFY(bad_question != nullptr);
  lineEditOf(*bad_question)->setText(QStringLiteral("prettier"));
  bad_question->userEditingFinished();
  QVERIFY2(editor->told(QStringLiteral("no program named 'prettier'")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/formatter")).toString(),
           QStringLiteral("none"));
  // the answer which named nothing spent itself: the question is over, the
  // choice is the one which stood before it, and asking again is what starts a
  // second try. Cancelling that one with the key which always ends a question
  // of the minibuffer changes nothing either
  QVERIFY(question(*editor, "pylsp-formatter") == nullptr);
  editor->launchCommand(QStringLiteral("python-set-formatter"));
  auto *const again = question(*editor, "pylsp-formatter");
  QVERIFY(again != nullptr);
  QTest::keyClick(lineEditOf(*again), Qt::Key_G, Qt::ControlModifier);
  QVERIFY2(!editor->hasUserInput(),
           "the key which ends a question left it standing");
  QCOMPARE(QSettings().value(QStringLiteral("pylsp/formatter")).toString(),
           QStringLiteral("none"));
  editor.reset();
}  // end of TestPythonMode::theFormatterChoiceDecidesWhoAnswers

void TestPythonMode::sortingTheImportsFollowsTheSorterChosen() {
  if (!this->hasServer()) {
    QSKIP("the packaged Python mode or a pylsp is missing: the cases "
          "which ask the server anything are skipped");
  }
  if (QStandardPaths::findExecutable(QStringLiteral("ruff")).isEmpty()) {
    QSKIP("the test sorts with ruff first, and the program is not there");
  }
  if (QStandardPaths::findExecutable(QStringLiteral("isort")).isEmpty()) {
    QSKIP("the test sorts the same buffer with isort after it, and the program "
          "is not there");
  }
  const auto rough = QStringLiteral("import sys\n"
                                    "import os\n"
                                    "\n"
                                    "print(os.name, sys.version)\n");
  const auto sorted = QStringLiteral("import os\n"
                                     "import sys\n"
                                     "\n"
                                     "print(os.name, sys.version)\n");
  const auto file = writeSource(QStringLiteral("sorters/two_programs.py"), rough);
  QVERIFY(!file.isEmpty());
  auto editor = openInPythonMode(file);
  startSession(*editor);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // sorting the imports is the one entry which rewrites the buffer and is asked
  // for by name, so a retry is what a server which is still reading the file is
  // waited for with
  auto sort = [this, ed = editor.get(), &frame, &sorted]() {
    auto spent = QElapsedTimer();
    spent.start();
    while (spent.elapsed() < 60000) {
      ed->launchCommand(QStringLiteral("python-sort-import"));
      QTest::qWait(400);
      if (frame.document()->toPlainText() == sorted) {
        return true;
      }
    }
    return false;
  };
  // the choice the mode makes by default: Ruff, which offers a change of the
  // `source.organizeImports` kind and is given its `I` rules by the editor for
  // exactly that
  QVERIFY2(sort(), qPrintable(frame.document()->toPlainText()));
  // the sorter is chosen through the question of the command, and the answer is
  // the name of the settings, not the label of the menu
  editor->launchCommand(QStringLiteral("python-set-import-sorter"));
  auto *const sorter_question = question(*editor, "pylsp-import-sorter");
  QVERIFY2(sorter_question != nullptr,
           "the command which chooses the sorter asked nothing in the "
           "minibuffer");
  lineEditOf(*sorter_question)->setText(QStringLiteral("isort"));
  sorter_question->userEditingFinished();
  QCOMPARE(
      QSettings().value(QStringLiteral("pylsp/import-sorter")).toString(),
      QStringLiteral("isort"));
  QVERIFY2(editor->told(QStringLiteral("sorting the imports with Isort")),
           qPrintable(editor->messages.join(QStringLiteral(" | "))));
  // `isort` answers as part of formatting the buffer, so taking the formatter
  // away with the same hand says that the sort is done by the program which is
  // chosen and not by the formatter which happens to be on
  editor->launchCommand(QStringLiteral("python-set-formatter"));
  auto *const formatter_question = question(*editor, "pylsp-formatter");
  QVERIFY(formatter_question != nullptr);
  lineEditOf(*formatter_question)->setText(QStringLiteral("none"));
  formatter_question->userEditingFinished();
  frame.document()->setPlainText(rough);
  QTest::qWait(300);
  QVERIFY2(sort(),
           qPrintable(QStringLiteral("isort, alone, left the imports of the "
                                     "buffer as they were: ") +
                      frame.document()->toPlainText()));
  // no sorter at all is the one answer of the entry which is not about the
  // server: it says what it is, and leaves the text where it is
  editor->launchCommand(QStringLiteral("python-set-import-sorter"));
  auto *const none_question = question(*editor, "pylsp-import-sorter");
  QVERIFY(none_question != nullptr);
  lineEditOf(*none_question)->setText(QStringLiteral("none"));
  none_question->userEditingFinished();
  frame.document()->setPlainText(rough);
  QTest::qWait(300);
  editor->launchCommand(QStringLiteral("python-sort-import"));
  QVERIFY2(
      editor->told(QStringLiteral("no import sorter is chosen")),
      qPrintable(editor->messages.join(QStringLiteral(" | "))));
  QTest::qWait(1500);
  QCOMPARE(frame.document()->toPlainText(), rough);
  editor.reset();
}  // end of TestPythonMode::sortingTheImportsFollowsTheSorterChosen

QTEST_MAIN(TestPythonMode)

#include "TestPythonMode.moc"
