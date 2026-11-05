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
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
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
#include "TFEL/GUI/ShortCutStyle.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  using tfel::gui::CommandFactory;
  using tfel::gui::ConfigurationDialog;
  using tfel::gui::EditorWidget;
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
  // hunspell is found by this build, so the theme which names the language of
  // the underlines is part of the configuration
  QVERIFY2(themes.contains("Spell checking"),
           "the spell checking theme is not declared");
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
  // what the editor remembers by itself is not part of the configuration: the
  // histories of the prompts, the compilation commands and the recent files are
  // written by the very act of working, and an address nothing declares has no
  // field in the dialog, see `CoreSettings.cxx`
  const auto forgotten = QStringList() << "recent files"
                                       << "command/history"
                                       << "command/shell/history"
                                       << "command/grep/history"
                                       << "user input/query replace/history"
                                       << "C++/compilation/history"
                                       << "make/compilation/history";
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
  // after the program it drives is there, with the four entries the clangd
  // support reads
  QVERIFY2(
      themes.contains(QStringLiteral("clangd")),
      qPrintable("the clangd theme is missing. Themes: " + themes.join(", ")));
  const auto clangd = r.getSettingsKeysOfGroup(QStringLiteral("clangd"));
  QCOMPARE(addressesOf(clangd), QStringList() << "clangd/enable"
                                              << "clangd/executable"
                                              << "clangd/arguments"
                                              << "clangd/semantic-tokens");
  QCOMPARE(clangd[0].type, SettingsKey::BOOLEAN);
  QVERIFY2(clangd[0].default_value.toBool(),
           "the clangd support is not asked for by default");
  QCOMPARE(clangd[2].type, SettingsKey::STRING_LIST);
  // a mode which uses no entry of the configuration has no theme of its own:
  // the mode `C++` asks `clangd` the four entries above and nothing else, the
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
  // and the theme says which mode exported it: a theme named after a program
  // would otherwise look like a theme of the editor to the dialog
  for (const auto& g : r.getGroups()) {
    if (g.name == QStringLiteral("clangd")) {
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
  // The list holds more rows than it has pages: the titles which keep the
  // themes of the modes apart from those of the editor name no page and cannot
  // be chosen, so the themes are the rows which can be chosen
  const auto& r = SettingsRegistry::getSettingsRegistry();
  auto* const themes = dialog.findChild<QListWidget*>("configuration-themes");
  auto* const pages = dialog.findChild<QStackedWidget*>("configuration-pages");
  QVERIFY(themes != nullptr);
  QVERIFY(pages != nullptr);
  auto names = QStringList{};
  for (auto row = 0; row != themes->count(); ++row) {
    auto* const item = themes->item(row);
    if (item->flags().testFlag(Qt::ItemIsSelectable)) {
      names << item->text();
    }
  }
  QCOMPARE(names, r.getGroupsNames());
  QCOMPARE(pages->count(), names.size());
  // picking a theme shows the page of that theme, which is not the row the
  // theme sits on: the titles in between shift the rows of the modes
  for (const auto& name : names) {
    themes->setCurrentItem(themes->findItems(name, Qt::MatchExactly).at(0));
    QVERIFY(themes->currentItem() != nullptr);
    QCOMPARE(pages->currentIndex(),
             themes->currentItem()->data(Qt::UserRole).toInt());
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
  auto* const themes = dialog.findChild<QListWidget*>("configuration-themes");
  auto* const pages = dialog.findChild<QStackedWidget*>("configuration-pages");
  QVERIFY((themes != nullptr) && (pages != nullptr));
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
  // the titles are the rows which can not be chosen, and a title shows no page
  auto editor_title = -1;
  auto modes_title = -1;
  for (auto row = 0; row != themes->count(); ++row) {
    auto* const item = themes->item(row);
    if (item->flags().testFlag(Qt::ItemIsSelectable)) {
      continue;
    }
    QVERIFY2(item->data(Qt::UserRole).toInt() < 0,
             qPrintable("the row '" + item->text() +
                        "' is a title and shows a "
                        "page"));
    if (item->text().isEmpty()) {
      continue;  // the empty row which keeps the two blocks apart
    }
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
  for (auto row = 0; row != themes->count(); ++row) {
    auto* const item = themes->item(row);
    if (!item->flags().testFlag(Qt::ItemIsSelectable)) {
      continue;
    }
    const auto message = "the theme '" + item->text() + "' of the mode '" +
                         mode_of(item->text()) + "' is shown at row " +
                         QString::number(row) + ", titles at " +
                         QString::number(editor_title) + " and " +
                         QString::number(modes_title);
    if (mode_of(item->text()).isEmpty()) {
      QVERIFY2((row > editor_title) && (row < modes_title),
               qPrintable(message));
    } else {
      QVERIFY2(row > modes_title, qPrintable(message));
    }
  }
  // choosing a title changes nothing: no page is reached by a row which is not
  // a theme, and the page which was shown stays shown
  const auto before = pages->currentIndex();
  QVERIFY(before >= 0);
  themes->setCurrentRow(modes_title);
  QCOMPARE(pages->currentIndex(), before);
  // the page of a theme a mode exported says whose it is, and only those pages
  // do
  for (auto row = 0; row != themes->count(); ++row) {
    auto* const item = themes->item(row);
    if (!item->flags().testFlag(Qt::ItemIsSelectable)) {
      continue;
    }
    const auto mode = mode_of(item->text());
    auto* const area = qobject_cast<QScrollArea*>(
        pages->widget(item->data(Qt::UserRole).toInt()));
    QVERIFY(area != nullptr);
    QVERIFY(area->widget() != nullptr);
    auto* const caption = area->widget()->findChild<QLabel*>(
        QStringLiteral("configuration-page-mode"));
    if (mode.isEmpty()) {
      QVERIFY2(caption == nullptr,
               qPrintable("the page of the theme '" + item->text() +
                          "' of the editor claims a major mode"));
      continue;
    }
    QVERIFY2(caption != nullptr,
             qPrintable("the page of the theme '" + item->text() +
                        "' does not say the mode it belongs to"));
    QVERIFY2(caption->text().contains(mode),
             qPrintable("the page of the theme '" + item->text() +
                        "' names another mode than '" + mode + "'"));
    QVERIFY2(caption->font().bold(),
             "the name of the mode on the page is not set apart from the "
             "entries");
  }
}  // end of
   // TestSettings::theThemesOfTheMajorModesAreSeparatedFromThoseOfTheEditor

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
