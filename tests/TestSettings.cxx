/*!
 * \file   tests/TestSettings.cxx
 * \brief  tests of the entries of the configuration: what is declared by the
 *         editor and its major modes, what the dialog of the configuration
 *         shows, and what writing it does.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QLibrary>
#include <QtCore/QMap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStackedWidget>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandFactory.hxx"
#include "TFEL/GUI/ConfigurationDialog.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/ShortCutStyle.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::CommandFactory;
  using tfel::gui::ConfigurationDialog;
  using tfel::gui::EditorWidget;
  using tfel::gui::LspHarperSupport;
  using tfel::gui::SettingsKey;
  using tfel::gui::SettingsRegistry;
  using tfel::gui::ShortCutStyle;
  using tfel::gui::StandardSettingsGroupProxy;

  // the shape of the application: an editor with a menu bar above it, since the
  // dialog reports what it wrote in the minibuffer and hands the font to every
  // frame of the editor
  struct Shell : QMainWindow {
    EditorWidget* const ed;
    explicit Shell() : ed(new EditorWidget()) {
      menuBar()->addMenu("File")->addAction("Open");
      setCentralWidget(ed);
      resize(800, 600);
      ed->createEmptyBuffer();
    }
    ~Shell() override {
      // no buffer is modified: no buffer is asked what to do with it when the
      // window, hence the editor and its buffers, goes away
      pump(2);
    }
    void pump(int n = 4) {
      for (int k = 0; k != n; ++k) {
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
      }
    }
  };  // end of struct Shell

  // the dialog of the configuration with `accept` reachable: `apply` only
  // writes, while `accept` is what applies live the entries whose effect the
  // user has to see under their hands
  struct ExposedDialog : ConfigurationDialog {
    using ConfigurationDialog::accept;
    explicit ExposedDialog(EditorWidget& w) : ConfigurationDialog(w) {}
  };

  // the addresses an entry of the configuration holds are forgotten, so that a
  // case starts from what a freshly installed editor answers and leaves nothing
  // behind for the next one
  void forgetEverything() {
    auto settings = QSettings();
    const auto& r = SettingsRegistry::getSettingsRegistry();
    for (const auto& k : r.getSettingsKeys()) {
      settings.remove(k.key);
    }
    settings.sync();
  }  // end of forgetEverything()

  QStringList addressesOf(const QVector<SettingsKey>& keys) {
    auto s = QStringList{};
    for (const auto& k : keys) {
      s << k.key;
    }
    return s;
  }  // end of addressesOf()

  QStringList declaredAddresses() {
    return addressesOf(
        SettingsRegistry::getSettingsRegistry().getSettingsKeys());
  }  // end of declaredAddresses()

}  // end of namespace

namespace {

  // two themes which declare the same addresses with different promises, which
  // is what two modes agreeing on one entry of the configuration look like to
  // the registry: a mode which derives from another one inherits its
  // declarations, and two plugins can name the same theme
  const StandardSettingsGroupProxy probe_a(
      "probe-theme-a",
      QObject::tr("First declaration of the probe entries"),
      QVector<SettingsKey>()
          << SettingsKey::text("probe/one",
                               QObject::tr("First label"),
                               QObject::tr("What the first declaration says"))
          // an integer, which no entry of the editor is: the spin box a field
          // of that kind is has no other way of being seen, and the bounds it
          // offers are what a value edited by hand in the configuration file
          // is held to
          << SettingsKey::integer("probe/two",
                                  QObject::tr("Second label"),
                                  QObject::tr("What only this theme declares"),
                                  3,
                                  1,
                                  5));

  const StandardSettingsGroupProxy probe_b(
      "probe-theme-b",
      QObject::tr("Second declaration of the probe entries"),
      QVector<SettingsKey>() << SettingsKey::text(
          "probe/one",
          QObject::tr("Another label"),
          QObject::tr("What another declaration says about the same address")));

  // the items of a tree, in the order a reader of the list meets them: an item,
  // then what hangs under it. The list of themes is a tree since a theme can be
  // a submenu of another one, and an assertion which walks the rows of the
  // screen has to walk them the way they are drawn
  void appendItems(QTreeWidgetItem *const item,
                   QVector<QTreeWidgetItem *> &items) {
    for (auto k = 0; k != item->childCount(); ++k) {
      auto *const c = item->child(k);
      items << c;
      appendItems(c, items);
    }
  }  // end of appendItems

  QVector<QTreeWidgetItem *> readList(QTreeWidget *const w) {
    auto items = QVector<QTreeWidgetItem *>{};
    appendItems(w->invisibleRootItem(), items);
    return items;
  }  // end of readList

}  // end of namespace

struct TestSettings : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void theRegistryDeclaresTheEntriesOfTheEditor();
  void theMajorModesExportTheEntriesTheyUse();
  void theDialogShowsWhatIsDeclared();
  void theThemesOfTheMajorModesAreSeparatedFromThoseOfTheEditor();
  void theCheckBoxesOfAThemeStandInOneColumnOnTheirOwnLine();
  void writingAnEntryStoresItAndReadingItAgainAnswersTheSame();
  void anEntrySetBackToItsDefaultValueIsForgotten();
  void anUnknownAddressAndAValueOfAnotherKindAreLeftAlone();
  void restoreDefaultsSetsTheWidgetsBackToTheDefaultValues();
  void acceptingTheDialogAppliesTheLiveEntries();
  void anAddressDeclaredTwiceIsShownOnce();
  void openingTheDialogIsACommand();
  void aMajorModeLoadedAsAPluginExportsTheEntriesItUses();

 private:
  void pump(int n = 4);
};

void TestSettings::initTestCase() {
  // an own configuration file: the cases write addresses the editor reads, and
  // must leave the configuration of the user alone
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-settings");
  forgetEverything();
}  // end of TestSettings::initTestCase

void TestSettings::cleanupTestCase() {
  forgetEverything();
}  // end of TestSettings::cleanupTestCase

void TestSettings::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestSettings::pump

void TestSettings::theRegistryDeclaresTheEntriesOfTheEditor() {
  auto& r = SettingsRegistry::getSettingsRegistry();
  const auto themes = r.getGroupsNames();
  QVERIFY2(themes.contains(QStringLiteral("General")),
           qPrintable("the theme of the editor is not declared. Themes: " +
                      themes.join(", ")));
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  // hunspell is found by this build, so the language of the underlines is an
  // entry of the configuration — and an entry of the theme of the proofreaders,
  // not a theme of its own: which program reads the words and which words it
  // holds to be spelled right is one decision, and the dialog shows one theme
  // for it
  QVERIFY2(!themes.contains(QStringLiteral("Spell checking")),
           "the language of the dictionary is a theme beside the proofreaders");
  {
    const auto k = r.getSettingsKey("hunspell default dictionary");
    QVERIFY2(k.has_value(), "the language of the dictionary is not declared");
    QCOMPARE(k->group, QStringLiteral("Spelling and grammar"));
  }
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  // an entry the editor reads has a label and a sentence of its own
  auto addresses = QStringList() << "font/textedit"
                                 << "shortcut style"
                                 << "secondary tasks orientation";
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  addresses << "hunspell default dictionary";
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  for (const auto& key : addresses) {
    const auto k = r.getSettingsKey(key);
    QVERIFY2(k.has_value(),
             qPrintable("the entry '" + key + "' is not declared"));
    QVERIFY2(!k->label.isEmpty(),
             qPrintable("the entry '" + key + "' has no label"));
    QVERIFY2(!k->description.isEmpty(),
             qPrintable("the entry '" + key + "' says nothing about itself"));
  }
  // the key bindings style is a choice whose default is the first value it
  // offers: what the editor does when the configuration file holds nothing
  const auto style = r.getSettingsKey("shortcut style");
  QVERIFY(style.has_value());
  QCOMPARE(style->type, SettingsKey::CHOICE);
  QCOMPARE(style->choices.size(), 2);
  QCOMPARE(style->choices[0].value.toString(), QStringLiteral("EMACS"));
  QCOMPARE(style->default_value.toString(), QStringLiteral("EMACS"));
  // the font of the text edits follows the application font until one is
  // stored, which is why it declares no default value
  const auto font = r.getSettingsKey("font/textedit");
  QVERIFY(font.has_value());
  QCOMPARE(font->type, SettingsKey::FONT);
  QVERIFY2(!font->default_value.isValid(),
           "the font entry pretends a default font");
  // which way of checking the language of a buffer the editor uses is an entry
  // of the editor: it is one decision for every buffer, whatever mode it wears,
  // and the submenu which writes it sits in the `Options` menu of the editor
  {
    const auto k = r.getSettingsKey("spelling/checker");
    QVERIFY2(k.has_value(),
             "the entry of the way of checking the language is not declared");
    QCOMPARE(k->type, SettingsKey::CHOICE);
    auto choices = QStringList{};
    for (const auto& c : k->choices) {
      choices << c.value.toString();
    }
    QCOMPARE(choices, QStringList() << "none" << "hunspell" << "hunspell-harper"
                                    << "language-tool");
    // what the editor does when the configuration file holds nothing is what it
    // did before a checker of sentences existed: the words are looked up
    QCOMPARE(k->default_value.toString(), QStringLiteral("hunspell"));
  }
  // the programs the two ways of checking a text run are entries of the editor
  // as well: a person chooses them from the `Options` menu, in whatever buffer,
  // and the C++ mode declaring the grammar entries used to make them look like
  // entries of that mode
  for (const auto& theme :
       {QStringLiteral("Spelling and grammar"), QStringLiteral("Harper"),
        QStringLiteral("LanguageTool")}) {
    QVERIFY2(themes.contains(theme),
             qPrintable("the theme '" + theme +
                        "' of the language of a buffer is not declared. "
                        "Themes: " + themes.join(", ")));
    for (const auto& g : r.getGroups()) {
      if (g.name == theme) {
        QVERIFY2(g.mode.isEmpty(),
                 qPrintable("the theme '" + theme + "' of the editor claims "
                            "the mode '" + g.mode + "'"));
      }
    }
  }
  // which program checks a text, and what that program is asked with, is one
  // decision: the theme of a proofreader is a submenu of the theme of the
  // choice, and the list answers a submenu directly under its parent
  {
    auto parent_of = QMap<QString, QString>{};
    for (const auto& g : r.getGroups()) {
      parent_of.insert(g.name, g.parent);
    }
    QCOMPARE(parent_of.value(QStringLiteral("Spelling and grammar")), QString());
    QCOMPARE(parent_of.value(QStringLiteral("Harper")),
             QStringLiteral("Spelling and grammar"));
    QCOMPARE(parent_of.value(QStringLiteral("LanguageTool")),
             QStringLiteral("Spelling and grammar"));
    // and the order of the list is that nesting, not the order two static
    // initialisers happened to run in
    const auto chosen = themes.indexOf(QStringLiteral("Spelling and grammar"));
    QVERIFY2(chosen >= 0, "the theme of the proofreaders is not in the list");
    QVERIFY2(chosen + 1 < themes.size(),
             "the theme of the proofreaders holds no submenu");
    QCOMPARE(themes[chosen + 1], QStringLiteral("Harper"));
    QCOMPARE(themes[chosen + 2], QStringLiteral("LanguageTool"));
    // the themes a major mode exports are on their own: what a mode drives is
    // not a corner of the choice of a proofreader
    for (const auto& g : r.getGroups()) {
      if (!g.mode.isEmpty()) {
        QVERIFY2(g.parent.isEmpty(),
                 qPrintable("the theme '" + g.name + "' of the mode '" + g.mode +
                            "' is a submenu of '" + g.parent + "'"));
      }
    }
  }
  // the choice of the way of checking decides what the buffers do, so the
  // program a session runs and the Java runtime which starts it have to be
  // chosen before one asks for them
  auto needs = QStringList() << "harper/executable"
                             << "language-tool/java-runtime"
                             << "language-tool/installation"
                             << "language-tool/port";
  for (const auto& key : needs) {
    QVERIFY2(r.hasSettingsKey(key),
             qPrintable("the entry '" + key + "', which names a program the "
                        "language check of a buffer runs, is not declared"));
  }
  // what the editor remembers by itself is not part of the configuration: the
  // histories of the prompts, the compilation commands and the recent files are
  // written by the very act of working, and an address nothing declares has no
  // field in the dialog, see `CoreSettings.cxx`
  const auto forgotten = QStringList()
                         << "recent files"
                         << "command/history"
                         << "command/shell/history"
                         << "command/grep/history"
                         << "user input/query replace/history"
                         << "C++/compilation/history"
                         << "make/compilation/history"
                         << "explorer/panel-shown"
                         << "explorer/panel-width";
  for (const auto& key : forgotten) {
    QVERIFY2(!r.hasSettingsKey(key),
             qPrintable("the entry '" + key +
                        "' is an entry of the configuration, it is not"));
    QVERIFY2(!declaredAddresses().contains(key),
             qPrintable("the dialog shows '" + key + "', which remembers"));
  }
}  // end of
   // TestSettings::theRegistryDeclaresTheEntriesOfTheEditor

void TestSettings::theMajorModesExportTheEntriesTheyUse() {
  auto& r = SettingsRegistry::getSettingsRegistry();
  const auto themes = r.getGroupsNames();
  // the C++ mode is part of the library the test links, so the theme it names
  // after the program it drives is there, with the five entries the clangd
  // support reads
  QVERIFY2(
      themes.contains(QStringLiteral("clangd")),
      qPrintable("the clangd theme is missing. Themes: " + themes.join(", ")));
  const auto clangd = r.getSettingsKeysOfGroup(QStringLiteral("clangd"));
  QCOMPARE(addressesOf(clangd), QStringList() << "clangd/enable"
                                              << "clangd/start-by-default"
                                              << "clangd/executable"
                                              << "clangd/arguments"
                                              << "clangd/semantic-tokens");
  // what the mode uses the language server for is one entry: the checker of
  // sentences a C++ buffer runs is about the language of a comment, not about
  // the code, so its entries belong to the editor and are shown under `Harper`
  // and `LanguageTool`, both themes of the editor, see the first test
  QCOMPARE(clangd[0].type, SettingsKey::BOOLEAN);
  QVERIFY2(clangd[0].default_value.toBool(),
           "the clangd support is not asked for by default");
  // a file of a project which says how it is compiled is a file which was
  // opened to be worked on with those options: the session which reads them is
  // started without being asked for, and that choice is the second entry
  QCOMPARE(clangd[1].type, SettingsKey::BOOLEAN);
  QVERIFY2(clangd[1].default_value.toBool(),
           "a project which has a compilation database starts no session by "
           "default");
  QCOMPARE(clangd[3].type, SettingsKey::STRING_LIST);
  // the checker of sentences is a program of its own, and what its sessions
  // read is declared by the core library: which program such a session runs, and
  // which English it reads a text as. The `LaTeX`, `Markdown` and C++ modes all
  // offer the choices of the language under the `Options` menu and declare
  // nothing of their own, which is why these addresses are shown once. The theme
  // is spelled as the program is spelled, `Harper`, whatever the addresses of its
  // entries are
  const auto harper = r.getSettingsKeysOfGroup(QStringLiteral("Harper"));
  QCOMPARE(addressesOf(harper),
           QStringList() << "harper/executable" << "harper/dialect");
  QCOMPARE(harper[0].type, SettingsKey::STRING);
  // the varieties the checker reads a text as are the names the server reads in
  // its settings, in the order they are offered, and the first of them is the
  // default of the server itself. The entry of the dialog and the submenu of a
  // menu both come from \c LspHarperSupport, which is what a session is answered
  // with, so the three can not disagree about which varieties there are
  QCOMPARE(harper[1].type, SettingsKey::CHOICE);
  QCOMPARE(harper[1].default_value.toString(), QStringLiteral("American"));
  auto varieties = QStringList{};
  for (const auto& c : harper[1].choices) {
    varieties << c.value.toString();
    QVERIFY2(c.label.startsWith(c.value.toString()),
             qPrintable("the variety '" + c.value.toString() +
                        "' is offered under a name which does not name it"));
  }
  QCOMPARE(varieties, LspHarperSupport::dialects());
  QCOMPARE(r.getSettingsKey("harper/dialect")->group,
           QStringLiteral("Harper"));
  // the server of the other way of checking a text is a Java program, which is
  // why the runtime which starts it is an entry beside the archive it runs. The
  // two lists of categories are the two filters on what of the rules of the
  // program reach a text, and no category is named by the editor itself
  const auto tool = r.getSettingsKeysOfGroup(QStringLiteral("LanguageTool"));
  QCOMPARE(addressesOf(tool),
           QStringList() << "language-tool/installation"
                        << "language-tool/java-runtime"
                        << "language-tool/port"
                        << "language-tool/java-options"
                        << "language-tool/language"
                        << "language-tool/enabled-categories"
                        << "language-tool/disabled-categories");
  for (const auto& key : {"language-tool/installation",
                          "language-tool/java-runtime"}) {
    const auto k = r.getSettingsKey(QString::fromLatin1(key));
    QVERIFY2(k.has_value(), qPrintable(QString("'%1' is not declared").arg(key)));
    QCOMPARE(k->type, SettingsKey::STRING);
    QCOMPARE(k->group, QStringLiteral("LanguageTool"));
  }
  // a port of 0 is what lets a session pick a free one for itself, which is what
  // makes two editors of this build sit beside each other. What is stored is the
  // port the person asked to start the server on, never the one a session picked
  QCOMPARE(r.getSettingsKey("language-tool/port")->default_value.toInt(), 0);
  QCOMPARE(r.getSettingsKey("language-tool/port")->minimum, 0);
  QCOMPARE(r.getSettingsKey("language-tool/port")->maximum, 65535);
  // the language the text is read as is an entry of its own, and the submenu of
  // the menu writes that very address: the dialog and the menu can not disagree
  // about which language a buffer is read as. Its default is empty and not
  // `en-US`: what an unset address reads as is the answer of
  // `LanguageToolSupport::language`, and the default of a program is none of the
  // editor's business to write in every configuration file
  QCOMPARE(r.getSettingsKey("language-tool/language")->type,
           SettingsKey::STRING);
  QCOMPARE(r.getSettingsKey("language-tool/language")->default_value.toString(),
           QString());
  // a mode which uses no entry of the configuration has no theme of its own:
  // the mode `C++` asks `clangd` the five entries above and nothing else, the
  // grep output is registered in this library and exports nothing
  const auto silent = QStringList() << "C++"
                                    << "grep-output"
                                    << "compilation-output";
  for (const auto& mode : silent) {
    QVERIFY2(!themes.contains(mode),
             qPrintable("the mode '" + mode +
                        "' exports no entry and has a theme of its own"));
  }
  // an entry which belongs to a theme of its own is shown there and nowhere
  // else
  QCOMPARE(r.getSettingsKey("clangd/enable")->group, QStringLiteral("clangd"));
  // and a theme says which mode exported it: a theme named after a program
  // would otherwise look like a theme of the editor to the dialog
  for (const auto& g : r.getGroups()) {
    if (g.name == QStringLiteral("clangd")) {
      // the theme the C++ mode declares is named after the program it drives, and
      // it is read from that mode: what clangd says of a buffer says nothing of
      // the language of a comment, and the entries of the menu which turn the
      // session on sit in that mode's menu
      QCOMPARE(g.mode, QStringLiteral("C++"));
    } else {
      QVERIFY2(g.mode.isEmpty(), qPrintable("the theme '" + g.name +
                                            "' of the editor claims the "
                                            "mode '" +
                                            g.mode + "'"));
    }
  }
  auto declared = QStringList{};
  for (const auto& k : r.getSettingsKeys()) {
    QVERIFY2(!declared.contains(k.key),
             qPrintable("the entry '" + k.key + "' is declared twice"));
    declared << k.key;
  }
}  // end of
   // TestSettings::theMajorModesExportTheEntriesTheyUse

void TestSettings::theDialogShowsWhatIsDeclared() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  // every entry the registry declares has a field, and nothing else has one
  QCOMPARE(dialog.keys(), declaredAddresses());
  QVERIFY(dialog.hasSetting("font/textedit"));
  QVERIFY(dialog.hasSetting("clangd/semantic-tokens"));
  QVERIFY2(!dialog.hasSetting("no/such/address"),
           "the dialog shows an entry nobody declared");
  // one page per theme, and the list on the left names them in the same order.
  // The list holds more items than it has pages: the titles which keep the
  // themes of the modes apart from those of the editor name no page and cannot
  // be chosen, and a sub-theme is an item hung under the theme it is a corner
  // of. What the walk answers is therefore the order of the registry, parents
  // and children included, which is the order of the names of the themes
  const auto& r = SettingsRegistry::getSettingsRegistry();
  auto* const themes = dialog.findChild<QTreeWidget*>("configuration-themes");
  auto* const pages = dialog.findChild<QStackedWidget*>("configuration-pages");
  QVERIFY(themes != nullptr);
  QVERIFY(pages != nullptr);
  auto shown = QVector<QTreeWidgetItem*>{};
  appendItems(themes->invisibleRootItem(), shown);
  auto names = QStringList{};
  for (auto* const item : shown) {
    if (item->flags().testFlag(Qt::ItemIsSelectable)) {
      names << item->text(0);
    }
  }
  QCOMPARE(names, r.getGroupsNames());
  QCOMPARE(pages->count(), names.size());
  // picking a theme shows the page of that theme: the item of a theme names the
  // page of the stack it was given, and the items in between name none
  for (auto* const item : shown) {
    const auto page = item->data(0, Qt::UserRole).toInt();
    if (page < 0) {
      continue;
    }
    themes->setCurrentItem(item);
    QVERIFY(pages->currentIndex() == page);
  }
  // an address which was never written shows what the entry answers then: the
  // font of the application for the font, the default of the choice for the
  // key bindings style
  const auto font = dialog.value("font/textedit");
  QVERIFY2(font.isValid(), "the font of the text edits shows nothing");
  QCOMPARE(font.value<QFont>().family(), QApplication::font().family());
  QCOMPARE(dialog.value("shortcut style").toString(), QStringLiteral("EMACS"));
}  // end of
   // TestSettings::theDialogShowsWhatIsDeclared

void TestSettings::theThemesOfTheMajorModesAreSeparatedFromThoseOfTheEditor() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  auto* const themes = dialog.findChild<QTreeWidget*>("configuration-themes");
  auto* const pages = dialog.findChild<QStackedWidget*>("configuration-pages");
  QVERIFY((themes != nullptr) && (pages != nullptr));
  // the walk is the order of the screen: an item, then what hangs under it. The
  // rows below are places in that order, which is how an item of one block is
  // compared to an item of another one
  const auto shown = readList(themes);
  QVERIFY(!shown.isEmpty());
  // the themes of the editor and those a major mode exports are two kinds of
  // things: the first are always there, the second come with the plugins which
  // were loaded and are about a program the mode drives. What tells them apart
  // is what a dialog has to show: a title over each block, and the name of the
  // mode on the pages of the second one
  const auto& r = SettingsRegistry::getSettingsRegistry();
  auto mode_of = [&r](const QString& name) -> QString {
    for (const auto& g : r.getGroups()) {
      if (g.name == name) {
        return g.mode;
      }
    }
    return QStringLiteral("<no theme of that name>");
  };
  auto exported = 0;
  for (const auto& g : r.getGroups()) {
    if (!g.mode.isEmpty()) {
      ++exported;
    }
  }
  QVERIFY2(exported != 0,
           "no major mode exports an entry, the separation has nothing to "
           "separate");
  // the titles are the items which can not be chosen, and a title shows no page
  auto editor_title = -1;
  auto modes_title = -1;
  for (auto row = 0; row != shown.size(); ++row) {
    auto* const item = shown[row];
    if (item->flags().testFlag(Qt::ItemIsSelectable)) {
      continue;
    }
    QVERIFY2(item->data(0, Qt::UserRole).toInt() < 0,
             qPrintable("the item '" + item->text(0) +
                        "' is a title and shows a page"));
    if (editor_title < 0) {
      editor_title = row;
    } else {
      modes_title = row;
    }
  }
  QVERIFY2(editor_title >= 0, "the themes of the editor carry no title");
  QVERIFY2(modes_title > editor_title,
           "the themes the major modes export carry no title");
  // each block is on its own side of its title
  for (auto row = 0; row != shown.size(); ++row) {
    auto* const item = shown[row];
    if (!item->flags().testFlag(Qt::ItemIsSelectable)) {
      continue;
    }
    const auto message = "the theme '" + item->text(0) + "' of the mode '" +
                         mode_of(item->text(0)) + "' is shown at row " +
                         QString::number(row) + ", titles at " +
                         QString::number(editor_title) + " and " +
                         QString::number(modes_title);
    if (mode_of(item->text(0)).isEmpty()) {
      QVERIFY2((row > editor_title) && (row < modes_title),
               qPrintable(message));
    } else {
      QVERIFY2(row > modes_title, qPrintable(message));
    }
  }
  // a sub-theme is a child of the item of its parent, and of nothing else: what
  // a proofreader is asked with belongs under the choice of that proofreader,
  // which is the whole point of the nesting
  {
    auto parent_of = QMap<QString, QString>{};
    for (const auto& g : r.getGroups()) {
      parent_of.insert(g.name, g.parent);
    }
    for (auto* const item : shown) {
      if (!item->flags().testFlag(Qt::ItemIsSelectable)) {
        continue;
      }
      const auto parent = parent_of.value(item->text(0));
      if (parent.isEmpty()) {
        // a top-level item of a tree has no parent item at all: the invisible
        // root which holds it is not an item. `qPrintable` of the message is
        // evaluated whether the assertion holds or not, so the name of the
        // parent is taken in a null-safe way before the test, not in it
        auto *const par = item->parent();
        const auto held = (par == nullptr) ? QStringLiteral("<none>")
                                           : par->text(0);
        QVERIFY2(par == nullptr,
                 qPrintable("the theme '" + item->text(0) +
                            ", which is one of its own, hangs under '" + held +
                            "'"));
        continue;
      }
      auto* const held = item->parent();
      QVERIFY2((held != nullptr) && (held->text(0) == parent),
               qPrintable("the sub-theme '" + item->text(0) + "' hangs under '" +
                          ((held == nullptr) ? QStringLiteral("<none>")
                                             : held->text(0)) +
                          "' and not under '" + parent + "'"));
      // and the submenu is opened: an entry behind a click nobody makes is an
      // entry which is not found
      QVERIFY2(held->isExpanded(),
               qPrintable("the submenu of '" + parent + "' is folded"));
    }
  }
  // choosing a title changes nothing: no page is reached by an item which is not
  // a theme, and the page which was shown stays shown
  auto* const a_theme = [shown]() {
    for (auto* const item : shown) {
      if (item->flags().testFlag(Qt::ItemIsSelectable)) {
        return item;
      }
    }
    return static_cast<QTreeWidgetItem *>(nullptr);
  }();
  QVERIFY(a_theme != nullptr);
  themes->setCurrentItem(a_theme);
  const auto before = pages->currentIndex();
  QVERIFY(before >= 0);
  themes->setCurrentItem(shown[modes_title]);
  QCOMPARE(pages->currentIndex(), before);
  // the page of a theme a mode exported says whose it is, and only those pages
  // do
  for (auto* const item : shown) {
    if (!item->flags().testFlag(Qt::ItemIsSelectable)) {
      continue;
    }
    const auto mode = mode_of(item->text(0));
    auto* const area = qobject_cast<QScrollArea*>(
        pages->widget(item->data(0, Qt::UserRole).toInt()));
    QVERIFY(area != nullptr);
    QVERIFY(area->widget() != nullptr);
    auto* const caption = area->widget()->findChild<QLabel*>(
        QStringLiteral("configuration-page-mode"));
    if (mode.isEmpty()) {
      QVERIFY2(caption == nullptr,
               qPrintable("the page of the theme '" + item->text(0) +
                          "' of the editor claims a major mode"));
      continue;
    }
    QVERIFY2(caption != nullptr,
             qPrintable("the page of the theme '" + item->text(0) +
                        "' does not say the mode it belongs to"));
    QVERIFY2(caption->text().contains(mode),
             qPrintable("the page of the theme '" + item->text(0) +
                        "' names another mode than '" + mode + "'"));
    QVERIFY2(caption->font().bold(),
             "the name of the mode on the page is not set apart from the "
             "entries");
  }
}  // end of
   // TestSettings::theThemesOfTheMajorModesAreSeparatedFromThoseOfTheEditor

// A check box answers the name which stands on its line, and the boxes of a theme
// are a list of questions with a yes or no answer which is read column by column:
// a box under its name, or a box placed at the end of each name of its own, makes
// the page a list of orphans. What is asserted here is therefore the three
// shape facts of the row of a boolean — the name and the box on one line, all
// of a theme at one place, that place the right edge of the page — on the page
// which is shown, since only that page has a geometry: the hidden pages of the
// stack are what a hidden window leaves its widgets with, 100 x 30 pixels.
void TestSettings::theCheckBoxesOfAThemeStandInOneColumnOnTheirOwnLine() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  auto* const pages = dialog.findChild<QStackedWidget*>("configuration-pages");
  QVERIFY(pages != nullptr);
  dialog.show();
  dialog.resize(760, 520);
  pump(20);
  auto seen = 0;
  for (auto i = 0; i != pages->count(); ++i) {
    pages->setCurrentIndex(i);
    pump(6);
    auto* const area = qobject_cast<QScrollArea*>(pages->widget(i));
    auto* const page = (area != nullptr) ? area->widget() : nullptr;
    QVERIFY2(page != nullptr, "a page of the dialog is not a scroll area");
    auto column = -1;
    for (auto* const box : page->findChildren<QCheckBox*>()) {
      ++seen;
      const auto x = box->mapTo(page, QPoint(0, 0)).x();
      if (column < 0) {
        column = x;
      }
      QVERIFY2(x == column,
               qPrintable(QStringLiteral("a check box of the page %1 stands at "
                                         "x=%2, another one at x=%3")
                              .arg(i)
                              .arg(x)
                              .arg(column)));
      // the name of the entry the box answers is held by the line of the box
      // itself, and stands to its left on that same line
      auto* const row = box->parentWidget();
      QVERIFY2(row != nullptr, "a check box of the dialog has no line of its own");
      auto* const name = row->findChild<QLabel*>();
      QVERIFY2(name != nullptr,
               "the line of a check box holds no name for it");
      QVERIFY2((name->geometry().bottom() >= box->geometry().top()) &&
                   (name->geometry().top() <= box->geometry().bottom()),
               qPrintable(QStringLiteral("the name '%1' and its check box are "
                                         "not on one line")
                              .arg(name->text())));
      QVERIFY2(box->geometry().left() > name->geometry().right() - 1,
               qPrintable(QStringLiteral("the check box of '%1' stands on the "
                                         "name it answers")
                              .arg(name->text())));
      // the column of a theme is the right edge of the page: the box is flush
      // with the end of its line, whatever the width of the name it answers.
      // The three pixels of slack are what a style may leave between the last
      // item of a line and its edge; a column set behind the longest name of
      // the theme, the shape this replaces, stood 23 pixels away on the
      // `clangd' page and 182 on the `pylsp' one, and the ragged edge of no
      // stretch at all ends 316 pixels from it.
      QVERIFY2((box->geometry().right() >= row->width() - 3) &&
                   (box->geometry().right() <= row->width() - 1),
               qPrintable(QStringLiteral("the check box of '%1' ends at x=%2 "
                                         "while its line is %3 pixels wide: it "
                                         "does not stand at the right edge")
                              .arg(name->text())
                              .arg(box->geometry().right())
                              .arg(row->width() - 1)));
      // the name says what it belongs to, and gives it the click it receives
      QVERIFY2(name->buddy() == box,
               qPrintable(QStringLiteral("the name '%1' is not the buddy of its "
                                         "check box")
                              .arg(name->text())));
    }
  }
  // the assertion above is about the boxes which exist: the `clangd' theme holds
  // three of them in this process, where only the C++ mode is registered
  QVERIFY2(seen >= 3,
           qPrintable(QStringLiteral("the dialog of the configuration answers "
                                     "%1 questions with a check box")
                          .arg(seen)));
}  // end of
   // TestSettings::theCheckBoxesOfAThemeStandInOneColumnOnTheirOwnLine

void TestSettings::writingAnEntryStoresItAndReadingItAgainAnswersTheSame() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  const auto arguments = QStringList()
                         << QStringLiteral("--background-index=false")
                         << QStringLiteral("--header-insertion=never");
  dialog.setValue("clangd/enable", false);
  dialog.setValue("clangd/executable", QStringLiteral("/usr/bin/clangd-15"));
  dialog.setValue("clangd/arguments", arguments);
  dialog.setValue("shortcut style", QStringLiteral("QT"));
  dialog.setValue("probe/two", 4);
  auto font = QFont(QStringLiteral("Monospace"));
  font.setPointSize(13);
  dialog.setValue("font/textedit", font);

  const auto written = dialog.apply();
  const auto expected = QStringList() << "clangd/enable"
                                      << "clangd/executable"
                                      << "clangd/arguments"
                                      << "shortcut style"
                                      << "probe/two"
                                      << "font/textedit";
  for (const auto& key : expected) {
    QVERIFY2(written.contains(key),
             qPrintable(QStringLiteral("'%1' was not written: %2")
                            .arg(key, written.join(", "))));
  }
  const auto settings = QSettings();
  QCOMPARE(settings.value("clangd/enable").toBool(), false);
  QCOMPARE(settings.value("clangd/executable").toString(),
           QStringLiteral("/usr/bin/clangd-15"));
  QCOMPARE(settings.value("clangd/arguments").toStringList(), arguments);
  QCOMPARE(settings.value("font/textedit").value<QFont>().pointSize(), 13);
  QCOMPARE(settings.value("probe/two").toInt(), 4);
  // a second dialog reads what the first one wrote, which is the round trip
  // through the format of the configuration file of this platform
  auto second = ConfigurationDialog(*shell.ed);
  QCOMPARE(second.value("clangd/enable").toBool(), false);
  QCOMPARE(second.value("clangd/executable").toString(),
           QStringLiteral("/usr/bin/clangd-15"));
  QCOMPARE(second.value("clangd/arguments").toStringList(), arguments);
  QCOMPARE(second.value("shortcut style").toString(), QStringLiteral("QT"));
  QCOMPARE(second.value("font/textedit").value<QFont>().pointSize(), 13);
  QCOMPARE(second.value("probe/two").toInt(), 4);
  // a list of one entry is what the format of the configuration file stores as
  // a plain text, and a text which holds a comma reads back as a list: the
  // entry answers the list it holds either way
  dialog.setValue("clangd/arguments",
                  QStringList{QStringLiteral("--header-insertion=never")});
  dialog.apply();
  const auto one = QStringList{QStringLiteral("--header-insertion=never")};
  QCOMPARE(QSettings().value("clangd/arguments").toStringList(), one);
  auto third = ConfigurationDialog(*shell.ed);
  QCOMPARE(third.value("clangd/arguments").toStringList(), one);
}  // end of
   // TestSettings::writingAnEntryStoresItAndReadingItAgainAnswersTheSame

void TestSettings::anEntrySetBackToItsDefaultValueIsForgotten() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  dialog.setValue("clangd/semantic-tokens", false);
  QVERIFY(dialog.apply().contains("clangd/semantic-tokens"));
  QVERIFY(QSettings().contains("clangd/semantic-tokens"));
  // what the user asks for is what would happen without that entry: the address
  // has nothing to say in the configuration file
  dialog.setValue("clangd/semantic-tokens", true);
  QVERIFY(dialog.apply().contains("clangd/semantic-tokens"));
  QVERIFY2(!QSettings().contains("clangd/semantic-tokens"),
           "an entry whose value is the default one is kept in the file");
  // and an entry which was never written and is not changed is left out of it
  QVERIFY2(!QSettings().contains("clangd/executable"),
           "an entry nobody changed is written to the file");
}  // end of
   // TestSettings::anEntrySetBackToItsDefaultValueIsForgotten

void TestSettings::anUnknownAddressAndAValueOfAnotherKindAreLeftAlone() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  const auto enabled = dialog.value("clangd/enable");
  QVERIFY(enabled.isValid());
  // a value of another kind than the widget edits: the dialog does not convert
  // what a person could not have typed into that field
  dialog.setValue("clangd/enable", QStringLiteral("not a boolean"));
  QCOMPARE(dialog.value("clangd/enable").toBool(), enabled.toBool());
  dialog.setValue("clangd/enable", 12);
  QCOMPARE(dialog.value("clangd/enable").toBool(), enabled.toBool());
  // a value out of the bounds the entry declares is clamped to what the spin
  // box answers, which is what a configuration file edited by hand is held to
  dialog.setValue("probe/two", 42);
  QCOMPARE(dialog.value("probe/two").toInt(), 5);
  // an address the dialog does not show invents nothing
  dialog.setValue("no/such/address", true);
  QVERIFY2(!dialog.value("no/such/address").isValid(),
           "the dialog answered an entry it does not show");
  QVERIFY2(!QSettings().contains("no/such/address"),
           "the dialog wrote an address nobody declared");
}  // end of
   // TestSettings::anUnknownAddressAndAValueOfAnotherKindAreLeftAlone

void TestSettings::restoreDefaultsSetsTheWidgetsBackToTheDefaultValues() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  dialog.setValue("clangd/enable", false);
  dialog.setValue("clangd/semantic-tokens", false);
  dialog.setValue("clangd/arguments",
                  QStringList() << QStringLiteral("--background-index=false"));
  dialog.setValue("shortcut style", QStringLiteral("QT"));
  QVERIFY(!dialog.apply().isEmpty());
  dialog.restoreDefaults();
  // a boolean is the one its entry declares, a list is empty, a choice is what
  // the editor does when the file holds nothing
  QCOMPARE(dialog.value("clangd/enable").toBool(), true);
  QCOMPARE(dialog.value("clangd/semantic-tokens").toBool(), true);
  QVERIFY(dialog.value("clangd/arguments").toStringList().isEmpty());
  QCOMPARE(dialog.value("shortcut style").toString(), QStringLiteral("EMACS"));
  // the addresses which were stored are the ones the following write forgets,
  // and restoring writes nothing by itself
  const auto written = dialog.apply();
  QVERIFY2(
      written.contains("clangd/enable") &&
          written.contains("clangd/semantic-tokens") &&
          written.contains("clangd/arguments") &&
          written.contains("shortcut style"),
      qPrintable(QStringLiteral("restoring the defaults writes nothing: %1")
                     .arg(written.join(", "))));
  QVERIFY2(!QSettings().contains("clangd/enable"),
           "the entry of a restored default is still stored");
  QVERIFY2(!QSettings().contains("clangd/arguments"),
           "the list of a restored default is still stored");
}  // end of
   // TestSettings::restoreDefaultsSetsTheWidgetsBackToTheDefaultValues

void TestSettings::acceptingTheDialogAppliesTheLiveEntries() {
  forgetEverything();
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ExposedDialog(*shell.ed);
  auto font = QFont(QStringLiteral("Monospace"));
  font.setPointSize(14);
  dialog.setValue("font/textedit", font);
  dialog.setValue("shortcut style", QStringLiteral("QT"));
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  dialog.setValue("hunspell default dictionary", QStringLiteral("en_GB"));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  dialog.accept();
  // the font of the configuration reaches the frames of the editor, as it does
  // through `Options > Main Frames' Font`
  const auto& edited = *shell.ed->getCurrentBuffer().getMainFrame().widget();
  QVERIFY2(edited.font().pointSize() == 14,
           qPrintable(QStringLiteral("the main frame kept the font of size %1")
                          .arg(edited.font().pointSize())));
  // the key bindings style is held by a singleton which reports the change to
  // whoever rebuilds the menus
  QCOMPARE(ShortCutStyle::getShortCutStyle().getStyle(), ShortCutStyle::QT);
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  // the language of the underlines reaches every buffer, not the only one which
  // happens to be on screen
  QCOMPARE(shell.ed->getCurrentBuffer().getMainFrame().getSpellCheckLanguage(),
           QStringLiteral("en_GB"));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  // and the editor is left with the style it started with
  ShortCutStyle::getShortCutStyle().setStyle(ShortCutStyle::EMACS);
  forgetEverything();
}  // end of
   // TestSettings::acceptingTheDialogAppliesTheLiveEntries

void TestSettings::anAddressDeclaredTwiceIsShownOnce() {
  auto& r = SettingsRegistry::getSettingsRegistry();
  // the address is shown once, by the first declaration it got
  auto seen = QStringList{};
  for (const auto& k : r.getSettingsKeys()) {
    if (k.key.startsWith(QStringLiteral("probe/"))) {
      seen << k.key;
    }
  }
  QCOMPARE(seen, (QStringList() << "probe/one"
                                << "probe/two"));
  QCOMPARE(r.getSettingsKey("probe/one")->label, QStringLiteral("First label"));
  QCOMPARE(r.getSettingsKey("probe/two")->type, SettingsKey::INTEGER);
  // the theme which brought nothing new is not a page of the dialog
  QVERIFY(!r.getGroupsNames().contains(QStringLiteral("probe-theme-b")));
  QVERIFY(r.getGroupsNames().contains(QStringLiteral("probe-theme-a")));
}  // end of TestSettings::anAddressDeclaredTwiceIsShownOnce

void TestSettings::openingTheDialogIsACommand() {
  QVERIFY(CommandFactory::getCommandFactory().hasCommand(
      QStringLiteral("configure-tfel-editor")));
}  // end of TestSettings::openingTheDialogIsACommand

void TestSettings::aMajorModeLoadedAsAPluginExportsTheEntriesItUses() {
#ifdef TFEL_EDITOR_CASTEM_MODE
  // the packaged modes are not linked by this test: the editor loads them as
  // plugins, and a plugin is what hands the registry a theme of its own. The
  // build tree holds the absolute path of every library a plugin needs in its
  // own run path, so the library built for the `Cast3M` mode is loaded as the
  // editor loads it
  auto library = QLibrary{QStringLiteral(TFEL_EDITOR_CASTEM_MODE)};
  QVERIFY2(library.load(),
           qPrintable(QStringLiteral("the packaged mode is not loaded: ") +
                      library.errorString()));
  auto& r = SettingsRegistry::getSettingsRegistry();
  // the theme is the one the mode names itself, since its entry is about the
  // program it launches: no page is added under the name of the mode
  const auto entry = r.getSettingsKey(QStringLiteral("castem/executable"));
  QVERIFY2(entry.has_value(),
           "the Cast3M mode exports no entry of the configuration");
  QCOMPARE(entry->group, QStringLiteral("castem"));
  QCOMPARE(entry->type, SettingsKey::STRING);
  // the theme of a program says which mode exports it, which is what lets the
  // dialog keep it in the block of the major modes
  for (const auto& g : r.getGroups()) {
    if (g.name == entry->group) {
      QCOMPARE(g.mode, QStringLiteral("Cast3M"));
    }
  }
  QVERIFY2(!entry->label.isEmpty() && !entry->description.isEmpty(),
           "the entry which names the program has no label or no sentence");
  // what the mode launches when nothing was chosen is a program, not nothing:
  // the field of the dialog shows that program, and a value written back equal
  // to it is forgotten rather than stored
  QVERIFY2(!entry->default_value.toString().trimmed().isEmpty(),
           "the entry of the program answers nothing when nothing was chosen");
  QVERIFY2(!r.getGroupsNames().contains(QStringLiteral("Cast3M")),
           "a theme bears the name of a mode whose entries are about a program "
           "it drives");
  // the name of a program is stored as it is written, since it is what decides
  // which session the mode starts
  Shell shell;
  shell.show();
  pump(8);
  auto dialog = ConfigurationDialog(*shell.ed);
  const auto program = QStringLiteral("/opt/castem/17/bin/castem");
  QVERIFY(dialog.hasSetting(QStringLiteral("castem/executable")));
  dialog.setValue(QStringLiteral("castem/executable"), program);
  QVERIFY(dialog.apply().contains(QStringLiteral("castem/executable")));
  QCOMPARE(QSettings().value("castem/executable").toString(), program);
  // and a dialog built afterwards answers the same name, which is the address
  // the mode reads every time it starts a session
  QCOMPARE(ConfigurationDialog(*shell.ed)
               .value(QStringLiteral("castem/executable"))
               .toString(),
           program);
  // nothing is left behind for the cases which would read the address
  forgetEverything();
#else  /* TFEL_EDITOR_CASTEM_MODE */
  QSKIP("this build does not name the libraries of the packaged modes");
#endif /* TFEL_EDITOR_CASTEM_MODE */
}  // end of
   // TestSettings::aMajorModeLoadedAsAPluginExportsTheEntriesItUses

QTEST_MAIN(TestSettings)

#include "TestSettings.moc"
