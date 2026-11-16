/*!
 * \file   TestLaTeXMode.cxx
 * \brief  tests for the program a document of LaTeX is compiled with: the entry
 *         of the configuration which names it, the `Interpreter` submenu which
 *         writes the same address, and the process a compile starts.
 * \author Thomas Helfer
 * \date   16 nov. 2026
 */

#include <QtTest/QtTest>
#include <QtCore/QLibrary>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <memory>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/ConfigurationDialog.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// The `LaTeX` mode is a plugin, and its class is not exported: the library is
// loaded the way the editor loads it, which registers both the mode and the
// entries it declares, and everything the cases ask goes through the public API
// — the menu of the mode, the settings registry, the dialog those entries are
// shown in, the slots of the mode reached by name through the meta-object, and the
// frame of the process a compile starts.
//
// A document is compiled here by the real slots of the mode, on a real file of a
// temporary directory, and what the cases assert on is the program and the
// arguments the process was started with: whether the machine holds a TeX which
// turns that file into a page is another question, and one the answers of these
// cases do not depend on — a program which is not there is a program the process
// was still asked to run, and says so.
namespace {

  using tfel::gui::ConfigurationDialog;
  using tfel::gui::SettingsKey;
  using tfel::gui::SettingsRegistry;

  //! the address the choice of the engine is kept in
  auto address() { return QStringLiteral("latex/interpreter"); }

  //! \return the addresses of the given entries, in the order they stand in
  QStringList addressesOf(const QVector<tfel::gui::SettingsKey> &keys) {
    auto s = QStringList{};
    for (const auto &k : keys) {
      s << k.key;
    }
    return s;
  }  // end of addressesOf()

  /*!
   * \brief the source of the document the cases compile: a file which is a valid
   * document, since one of the engines which may be started here is a real one
   */
  auto documentSource() {
    return QStringLiteral("\\documentclass{article}\n"
                          "\\begin{document}\n"
                          "hello\n"
                          "\\end{document}\n");
  }  // end of documentSource()

  /*!
   * \brief an editor widget which remembers what the minibuffer was told: which
   * engine a click ended up choosing is part of the behaviour under test, and
   * overriding this public virtual is the only door to the messages
   */
  struct RecordingEditor : tfel::gui::EditorWidget {
    QStringList messages;
    void displayInformativeMessage(const QString &m) override {
      messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
    //! \return true if the minibuffer was told exactly \c m at some point
    bool told(const QString &m) const { return messages.contains(m); }
  };  // end of struct RecordingEditor

}  // end of anonymous namespace

struct TestLaTeXMode : QObject {
  Q_OBJECT

 public:
  TestLaTeXMode();

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void theModeDeclaresTheProgramWhichCompilesADocument();
  void theConfigurationDialogOffersTheSameQuestion();
  void theMenuOffersTheThreeEnginesAsOneQuestion();
  void choosingAnEngineWritesTheAddressAndMovesTheTick();
  void theSubmenuNamesTheEngineTheSettingsHold();
  void aCompileRunsTheEngineChosenAndNoOther();

 private:
  //! the settings of the editor are none of this test's business
  void isolateSettings();
  //! write `content` in a temporary file of the given name and return its path
  QString writeFile(const QString &name, const QString &content);
  /*!
   * \return the `Interpreter` submenu of the menu of a mode, a null pointer when
   * the mode offers none
   *
   * \note \c getSpecificMenu is the one public door to the entries: they are
   * built there, and the menu bar of the editor is only a consumer of what that
   * method hands out
   */
  static QMenu *interpreterSubmenu(tfel::gui::MajorMode &mode);
  //! \return the entry of the submenu which names \c engine, null when none does
  static QAction *engineEntry(QMenu &submenu, const QString &engine);
  /*!
   * \brief a buffer of the `LaTeX` mode, visiting a file the cases compile
   * \param[in] name : name of the file to write in the temporary directory
   * \param[out] editor : the editor widget the buffer belongs to, which the
   * caller has to keep alive, and which is the one to ask what was reported
   */
  tfel::gui::MajorMode *makeBuffer(const QString &name,
                                   std::unique_ptr<RecordingEditor> &editor);
  /*!
   * \brief the frame of a buffer running \c engine
   * \param[in] editor : editor widget the buffer belongs to
   * \param[in] buffer : buffer a compile was asked of
   * \param[in] engine : name of the program the process was started with
   *
   * \note the frame is waited for: a process which has not been started yet names
   * no program, and the buffer of the cases is shown, which is what lets a frame
   * attached to it be registered as a secondary task at all
   */
  static tfel::gui::ProcessOutputFrame *
      frameRunning(tfel::gui::EditorWidget &editor,
                   tfel::gui::Buffer &buffer,
                   const QString &engine);
  //! end the process a compile of the cases started, whatever it reached
  static void stopProcess(tfel::gui::ProcessOutputFrame &frame);
  //! the directory the files of the cases are written in
  QTemporaryDir directory;
#ifdef TFEL_EDITOR_LATEX_MODE
  //! the library of the packaged mode, named by the build
  QLibrary library;
#endif /* TFEL_EDITOR_LATEX_MODE */
  //! whether the mode the cases drive can be built at all on this machine
  bool available = false;
};  // end of struct TestLaTeXMode

TestLaTeXMode::TestLaTeXMode() {
#ifdef TFEL_EDITOR_LATEX_MODE
  // the build names the library of the packaged mode, which is what the cases
  // load in `initTestCase`
  this->library.setFileName(QStringLiteral(TFEL_EDITOR_LATEX_MODE));
#endif /* TFEL_EDITOR_LATEX_MODE */
}

void TestLaTeXMode::initTestCase() {
  // an own configuration file: the cases write the address the mode reads and
  // must leave the configuration of the user alone
  QApplication::setOrganizationName("tfel-editor-tests");
  QApplication::setApplicationName("test-latex-mode");
  this->isolateSettings();
#ifdef TFEL_EDITOR_LATEX_MODE
  QVERIFY2(this->library.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      this->library.errorString()));
  // loading the library registered the mode and what it declares, which is what
  // the cases below ask about
  this->available = true;
#else  /* TFEL_EDITOR_LATEX_MODE */
  this->available = false;
#endif /* TFEL_EDITOR_LATEX_MODE */
}

void TestLaTeXMode::cleanupTestCase() {
  this->isolateSettings();
}

void TestLaTeXMode::isolateSettings() {
  auto settings = QSettings();
  settings.remove(address());
  settings.sync();
}  // end of TestLaTeXMode::isolateSettings

QString TestLaTeXMode::writeFile(const QString &name, const QString &content) {
  const auto file = directory.filePath(name);
  auto handle = QFile(file);
  if (!handle.open(QIODevice::WriteOnly)) {
    return QString();
  }
  handle.write(content.toUtf8());
  handle.close();
  return file;
}  // end of TestLaTeXMode::writeFile

QMenu *TestLaTeXMode::interpreterSubmenu(tfel::gui::MajorMode &mode) {
  auto *const menu = mode.getSpecificMenu();
  if (menu == nullptr) {
    return nullptr;
  }
  // the submenu is singled out by its object name, and not by its position: the
  // choices of the proofreader stand below it, and how many of them a machine
  // offers is the business of that machine
  return menu->findChild<QMenu *>(QStringLiteral("latex-interpreter"));
}  // end of TestLaTeXMode::interpreterSubmenu

QAction *TestLaTeXMode::engineEntry(QMenu &submenu, const QString &engine) {
  for (auto *const a : submenu.actions()) {
    if (a->text() == engine) {
      return a;
    }
  }
  return nullptr;
}  // end of TestLaTeXMode::engineEntry

tfel::gui::MajorMode *
TestLaTeXMode::makeBuffer(const QString &name,
                          std::unique_ptr<RecordingEditor> &editor) {
  // `QVERIFY` can not be used in a function which answers something, so what the
  // caller is asked to check is reported back as a null pointer
  const auto file = writeFile(name, documentSource());
  if (file.isEmpty()) {
    return nullptr;
  }
  editor = std::make_unique<RecordingEditor>();
  // a buffer which is not shown registers no secondary task, and the frame of a
  // compile is one
  editor->show();
  editor->openFile(file);
  auto &frame = editor->getCurrentBuffer().getMainFrame();
  // the mode of the buffer, in hand and not as the `const MajorMode &` a frame
  // hands out: `setMajorMode` is the one public door to an instance of a mode a
  // plugin built
  return frame.setMajorMode(QStringLiteral("LaTeX"));
}  // end of TestLaTeXMode::makeBuffer

tfel::gui::ProcessOutputFrame *TestLaTeXMode::frameRunning(
    tfel::gui::EditorWidget &editor,
    tfel::gui::Buffer &buffer,
    const QString &engine) {
  // What is looked for is a frame of the buffer whose process was started with the
  // engine named, and the loop waits for the machine: `startLaTeX` only attaches
  // the task, so the frame is found among the tasks of the buffer and not as its
  // current secondary task, and a process which is not installed yet is a process
  // whose program is not known yet
  for (auto i = 0; i != 60; ++i) {
    for (const auto &t : editor.getSecondaryTasks(&buffer)) {
      auto *const f = qobject_cast<tfel::gui::ProcessOutputFrame *>(t.w.data());
      if ((f != nullptr) && (f->getProcess().program() == engine)) {
        return f;
      }
    }
    QTest::qWait(100);
  }
  return nullptr;
}  // end of TestLaTeXMode::frameRunning

void TestLaTeXMode::stopProcess(tfel::gui::ProcessOutputFrame &frame) {
  auto &p = frame.getProcess();
  if (p.state() == QProcess::NotRunning) {
    return;
  }
  // What ends the process of a case is the compile itself: the document is four
  // lines and `-interaction=nonstopmode' keeps an engine from waiting for an
  // answer. `terminate' is reached only by a process which is `Running', because a
  // child which never left `Starting' has not executed its program yet: it still
  // runs the code of the test and its signal handlers, so the signal is caught by
  // Qt Test inside that child, which reports the case as `Received signal 15' and
  // `Received a fatal error' while the test process itself goes on and passes it.
  // `waitForFinished' both ends the wait for a compile which answers and lets the
  // child be one.
  if (p.waitForFinished(30000)) {
    return;
  }
  if (p.state() == QProcess::Running) {
    p.terminate();
    p.waitForFinished(5000);
  }
}  // end of TestLaTeXMode::stopProcess

void TestLaTeXMode::theModeDeclaresTheProgramWhichCompilesADocument() {
  if (!this->available) {
    QSKIP("the packaged `LaTeX` mode is not loaded: nothing declares its entry");
  }
  auto &r = SettingsRegistry::getSettingsRegistry();
  // the mode names its one entry after the family of programs it launches, as the
  // C++ mode does for `clangd` and the `Cast3M` mode for `castem`
  const auto themes = r.getGroupsNames();
  QVERIFY2(themes.contains(QStringLiteral("latex")),
           qPrintable("the latex theme is missing. Themes: " +
                      themes.join(", ")));
  const auto keys = r.getSettingsKeysOfGroup(QStringLiteral("latex"));
  QCOMPARE(addressesOf(keys), QStringList() << address());
  // and the theme says whose it is: `latex` is the name of a program, which is
  // what a theme of the editor may also be, and only the mode field tells the two
  // apart
  for (const auto &g : r.getGroups()) {
    if (g.name == QStringLiteral("latex")) {
      QCOMPARE(g.mode, QStringLiteral("LaTeX"));
      QVERIFY2(g.parent.isEmpty(),
               qPrintable("the theme of the engine of a document is a submenu of "
                          "'" + g.parent + "'"));
    }
  }
  const auto k = r.getSettingsKey(address());
  QVERIFY2(k.has_value(),
           "the entry naming the program which compiles a document is not "
           "declared");
  QCOMPARE(k->type, SettingsKey::CHOICE);
  QVERIFY2(!k->label.isEmpty(), "the entry of the engine has no label");
  QVERIFY2(!k->description.isEmpty(),
           "the entry of the engine says nothing about itself");
  // the three engines, in the order they are offered, and the traditional one as
  // the default: it is the program the mode ran before there was a choice to
  // make, so a configuration file which says nothing keeps compiling as it did
  auto engines = QStringList();
  for (const auto &c : k->choices) {
    engines << c.value.toString();
    QVERIFY2(c.label == c.value.toString(),
             qPrintable("the engine '" + c.value.toString() +
                        "' is offered under a name which does not name it"));
  }
  QCOMPARE(engines, QStringList() << QStringLiteral("latex")
                                  << QStringLiteral("pdflatex")
                                  << QStringLiteral("xelatex"));
  QCOMPARE(k->default_value.toString(), QStringLiteral("latex"));
}  // end of
   // TestLaTeXMode::theModeDeclaresTheProgramWhichCompilesADocument

void TestLaTeXMode::theConfigurationDialogOffersTheSameQuestion() {
  if (!this->available) {
    QSKIP("the packaged `LaTeX` mode is not loaded: nothing declares its entry");
  }
  this->isolateSettings();
  std::unique_ptr<RecordingEditor> editor;
  auto *const mode = makeBuffer(QStringLiteral("dialog.tex"), editor);
  QVERIFY(mode != nullptr);
  auto &buffer = editor->getCurrentBuffer();
  auto dialog = ConfigurationDialog(*editor);
  // the entry the mode declared is a page of the dialog: an address which is
  // declared but not shown, or shown and not declared, is the two surfaces this
  // feature is made of disagreeing
  QVERIFY2(dialog.hasSetting(address()),
           "the program which compiles a document is not an entry of the dialog");
  // and what the field answers when nothing was ever stored is the default of the
  // entry, not an empty field: the person sees the program the mode would run
  QCOMPARE(dialog.value(address()).toString(), QStringLiteral("latex"));
  // the dialog and the submenu of the mode answer one question: they write the one
  // address the mode reads when it starts a process
  dialog.setValue(address(), QStringLiteral("xelatex"));
  QVERIFY2(dialog.apply().contains(address()),
           "the engine chosen in the dialog was not written");
  QCOMPARE(QSettings().value(address()).toString(), QStringLiteral("xelatex"));
  // and the compile is what proves the two are the same question: the mode of the
  // buffer was built while `latex` was the answer and is rebuilt by nothing
  QMetaObject::invokeMethod(mode, "startLaTeX");
  auto *const frame = frameRunning(*editor, buffer, QStringLiteral("xelatex"));
  QVERIFY2(frame != nullptr,
           qPrintable("the engine chosen in the dialog did not reach the "
                      "compile: " +
                      editor->messages.join(QStringLiteral(" | "))));
  stopProcess(*frame);
  this->isolateSettings();
  editor.reset();
}  // end of TestLaTeXMode::theConfigurationDialogOffersTheSameQuestion

void TestLaTeXMode::theMenuOffersTheThreeEnginesAsOneQuestion() {
  if (!this->available) {
    QSKIP("the packaged `LaTeX` mode is not loaded: it has no menu to offer");
  }
  this->isolateSettings();
  std::unique_ptr<RecordingEditor> editor;
  auto *const mode = makeBuffer(QStringLiteral("menu.tex"), editor);
  QVERIFY(mode != nullptr);
  auto *const submenu = interpreterSubmenu(*mode);
  QVERIFY2(submenu != nullptr, "the mode offers no `Interpreter` submenu");
  QCOMPARE(submenu->title(), QObject::tr("Interpreter"));
  auto names = QStringList();
  for (const auto *const a : submenu->actions()) {
    names << a->text();
  }
  QCOMPARE(names, QStringList() << QStringLiteral("latex")
                                << QStringLiteral("pdflatex")
                                << QStringLiteral("xelatex"));
  auto checked = 0;
  for (auto *const a : submenu->actions()) {
    QVERIFY2(a->isCheckable(),
             qPrintable("the entry '" + a->text() + "' of the engine can not "
                        "be ticked"));
    // an entry which carries a submenu is opened by Qt and never triggered, so an
    // engine hung above anything else would be an engine which can not be chosen
    QVERIFY2(a->menu() == nullptr,
             qPrintable("the entry '" + a->text() + "' carries a submenu, which "
                        "is what made the choices of the proofreader "
                        "unclickable"));
    if (a->isChecked()) {
      ++checked;
    }
  }
  // one exclusive question: exactly one of the three stands ticked, and it is the
  // program the address names, which nothing stored says is `latex`
  QCOMPARE(checked, 1);
  QVERIFY(engineEntry(*submenu, QStringLiteral("latex"))->isChecked());
  QVERIFY(!engineEntry(*submenu, QStringLiteral("pdflatex"))->isChecked());
  editor.reset();
}  // end of TestLaTeXMode::theMenuOffersTheThreeEnginesAsOneQuestion

void TestLaTeXMode::choosingAnEngineWritesTheAddressAndMovesTheTick() {
  if (!this->available) {
    QSKIP("the packaged `LaTeX` mode is not loaded: nothing writes the address");
  }
  this->isolateSettings();
  std::unique_ptr<RecordingEditor> editor;
  auto *const mode = makeBuffer(QStringLiteral("choice.tex"), editor);
  QVERIFY(mode != nullptr);
  auto *const submenu = interpreterSubmenu(*mode);
  QVERIFY(submenu != nullptr);
  auto *const entry = engineEntry(*submenu, QStringLiteral("pdflatex"));
  QVERIFY(entry != nullptr);
  QVERIFY2(!entry->isCheckable() || !entry->isChecked(),
           "the engine to choose is already the one in use: the case would "
           "assert nothing about the click");
  // the entry is a leaf of the submenu, which is what makes `trigger()` the same
  // thing as the mouse on it: a click on a leaf ends with that very signal
  entry->trigger();
  QCOMPARE(QSettings().value(address()).toString(),
           QStringLiteral("pdflatex"));
  // and the menu says what it just chose: an exclusive question whose tick stayed
  // on the answer which is no more is a menu which contradicts its own click
  QVERIFY(entry->isChecked());
  QVERIFY(!engineEntry(*submenu, QStringLiteral("latex"))->isChecked());
  // the minibuffer names the program the next compile will run
  QVERIFY2(editor->told(QObject::tr("compiling with %1").arg(
                             QStringLiteral("pdflatex"))),
           qPrintable("the choice of an engine reported: " +
                      editor->messages.join(QStringLiteral(" | "))));
  // and the address is the one the entry of the configuration writes: the click
  // above is a hand on the same field as the dialog
  const auto k = SettingsRegistry::getSettingsRegistry().getSettingsKey(
      address());
  QVERIFY(k.has_value());
  QCOMPARE(k->read().toString(), QStringLiteral("pdflatex"));
  this->isolateSettings();
  editor.reset();
}  // end of
   // TestLaTeXMode::choosingAnEngineWritesTheAddressAndMovesTheTick

void TestLaTeXMode::theSubmenuNamesTheEngineTheSettingsHold() {
  if (!this->available) {
    QSKIP("the packaged `LaTeX` mode is not loaded: it has no menu to refresh");
  }
  this->isolateSettings();
  std::unique_ptr<RecordingEditor> editor;
  auto *const mode = makeBuffer(QStringLiteral("stale.tex"), editor);
  auto *const submenu = interpreterSubmenu(*mode);
  QVERIFY(submenu != nullptr);
  // the menu was built while nothing was stored, that is while `latex` was the
  // engine; the configuration dialog, another buffer's menu, or a hand in the
  // configuration file can change that without the menu being rebuilt, and a menu
  // ticking the program which is not the one a compile would run is a menu which
  // lies about what `Alt+M` does
  QVERIFY(engineEntry(*submenu, QStringLiteral("latex"))->isChecked());
  QSettings().setValue(address(), QStringLiteral("xelatex"));
  Q_EMIT submenu->aboutToShow();
  QVERIFY(engineEntry(*submenu, QStringLiteral("xelatex"))->isChecked());
  QVERIFY(!engineEntry(*submenu, QStringLiteral("latex"))->isChecked());
  QVERIFY(!engineEntry(*submenu, QStringLiteral("pdflatex"))->isChecked());
  this->isolateSettings();
  editor.reset();
}  // end of TestLaTeXMode::theSubmenuNamesTheEngineTheSettingsHold

void TestLaTeXMode::aCompileRunsTheEngineChosenAndNoOther() {
  if (!this->available) {
    QSKIP("the packaged `LaTeX` mode is not loaded: nothing compiles");
  }
  this->isolateSettings();
  std::unique_ptr<RecordingEditor> editor;
  const auto name = QStringLiteral("compile.tex");
  auto *const mode = makeBuffer(name, editor);
  auto &buffer = editor->getCurrentBuffer();
  auto *const submenu = interpreterSubmenu(*mode);
  QVERIFY(submenu != nullptr);
  // the file of the buffer is what is compiled: the buffer announces no master
  // file, and it holds no unsaved change, so `startLaTeX` — the slot `Alt+M`
  // reaches once the save question is answered — goes straight to the process
  auto file = directory.filePath(name);
  QMetaObject::invokeMethod(mode, "startLaTeX");
  auto *const frame = frameRunning(*editor, buffer, QStringLiteral("latex"));
  QVERIFY2(frame != nullptr,
           qPrintable("no process of the traditional engine was started for the "
                      "buffer: " +
                      editor->messages.join(QStringLiteral(" | "))));
  // the engine nothing names is the traditional one, which is the program the mode
  // ran before the entry existed
  QCOMPARE(frame->getProcess().program(), QStringLiteral("latex"));
  // and the file is named as it stands: a quoted `\input' command used to be put
  // in front of it, which is a trick of how `pdftex' reads its command line and
  // not how `xelatex' does — handed that argument, XeTeX looks for a file named `"'
  // and stops, which is measured. The two options and the name are what the three
  // engines of the choice are all started with
  QCOMPARE(frame->getProcess().arguments(),
           QStringList() << QStringLiteral("-file-line-error")
                        << QStringLiteral("-interaction=nonstopmode") << file);
  stopProcess(*frame);
  // the same mode, with no rebuild of anything, runs the engine the submenu was
  // clicked on: the address is read when a process is started, so a choice reaches
  // the next compile and no restart of the editor is needed
  engineEntry(*submenu, QStringLiteral("xelatex"))->trigger();
  QMetaObject::invokeMethod(mode, "startLaTeX");
  auto *const second =
      frameRunning(*editor, buffer, QStringLiteral("xelatex"));
  QVERIFY2(second != nullptr,
           qPrintable("no process of the engine chosen by the submenu was "
                      "started: " +
                      editor->messages.join(QStringLiteral(" | "))));
  QCOMPARE(second->getProcess().arguments(),
           QStringList() << QStringLiteral("-file-line-error")
                        << QStringLiteral("-interaction=nonstopmode") << file);
  stopProcess(*second);
  this->isolateSettings();
  editor.reset();
}  // end of TestLaTeXMode::aCompileRunsTheEngineChosenAndNoOther

QTEST_MAIN(TestLaTeXMode)
#include "TestLaTeXMode.moc"
