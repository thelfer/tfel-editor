/*!
  * \file   SpellingAndGrammar.cxx
  * \brief  the choice of the proofreader a buffer is read with, the language it
  *         reads, and the menu which offers them.
  * \author Thomas Helfer
  * \date   13 nov. 2026
 */

#include <QtCore/QSettings>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtWidgets/QMenu>

#include "TFEL/GUI/SpellingAndGrammar.hxx"

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
#include "TFEL/GUI/HunspellDictionariesManager.hxx"
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
#include "TFEL/GUI/LanguageToolClient.hxx"
#include "TFEL/GUI/LanguageToolSupport.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/SettingsKey.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {

    // the three ways one English is named by the three proofreaders: the
    // dictionary calls it `en_GB`, `harper-ls` reads it as "British" and the
    // code LanguageTool wants of it is `en-GB`. A person chooses an English and
    // not three addresses, which is what this table is for
    struct VarietyOfEnglish {
      const char *dialect;     // what `harper-ls` reads
      const char *dictionary;  // what `hunspell` calls it
      const char *code;        // what LanguageTool reads
    };

    const VarietyOfEnglish english_varieties[] = {
        {"American", "en_US", "en-US"},
        {"Canadian", "en_CA", "en-CA"},
        {"Australian", "en_AU", "en-AU"},
        {"British", "en_GB", "en-GB"},
        {"Indian", "en_IN", "en-IN"},
    };

    // the language `hunspell` is given by the choice which is held: the stored
    // dictionary, and nothing at all when the choice took the spelling away from
    // it. An empty language is what `SpellChecker` reads as 'no dictionary',
    // which is the one way of turning the underlines of the misspelled words off
    // a mode has
    QString hunspellLanguage() {
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
      return HunspellDictionariesManager::getHunspellDictionariesManager()
          .getDefaultSpellCheckLanguage();
#else  /* TFEL_GUI_HUNSPELL_SUPPORT */
      return {};
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
    }  // end of hunspellLanguage()

    // the variety of English a language names, whichever of the three names it
    // is spelled with; null when the language names none — a language which is
    // no English, or a dictionary of an English no checker here knows
    const VarietyOfEnglish *varietyOf(const QString &language) {
      for (const auto &v : english_varieties) {
        if ((language == QLatin1String(v.dialect)) ||
            (language == QLatin1String(v.dictionary)) ||
            (language == QLatin1String(v.code))) {
          return &v;
        }
      }
      return nullptr;
    }  // end of varietyOf()

    // the languages a choice reads: what the installed dictionaries name, the
    // five varieties `harper-ls` reads, or the codes of LanguageTool. The three
    // programs know different sets, and a choice whose languages nobody
    // installed has none
    void gatherLanguages(const SpellingAndGrammar::Checker c,
                         QStringList &names,
                         QStringList &labels) {
      if (SpellingAndGrammar::usesHarper(c)) {
        for (const auto &name : LspHarperSupport::dialects()) {
          names << name;
          labels << LspHarperSupport::dialectLabel(name);
        }
      } else if (SpellingAndGrammar::usesLanguageTool(c)) {
        for (const auto &code : LanguageToolSupport::languages()) {
          names << code;
          labels << LanguageToolSupport::languageLabel(code);
        }
      } else if (SpellingAndGrammar::usesHunspell(c)) {
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
        // the dictionaries the machine installed are the languages hunspell can
        // be asked in; a machine which installed none has nothing to choose
        for (const auto &dictionary :
             HunspellDictionariesManager::getHunspellDictionariesManager()
                 .getAvailableDictionnaries()) {
          names << dictionary;
          labels << dictionary;
        }
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
      }
    }  // end of gatherLanguages()

    // what the minibuffer says when an entry of the menu is clicked: the choice
    // which was made and, for the source buffer where LanguageTool reads no
    // grammar, what that choice does not do *here* — which a bare "checking
    // with languagetool" would leave a person to discover by getting no
    // underlines at all. The sentence of the source buffer is the one the
    // session of such a buffer reports with, so that the menu and the buffer it
    // steers speak the same words
    QString choiceReport(const SpellingAndGrammar::Checker c,
                         const bool whole_text_is_prose) {
      if ((c == SpellingAndGrammar::Checker::LanguageTool) &&
          (!whole_text_is_prose)) {
        return QObject::tr(
            "LanguageTool reads no code: this buffer is checked for "
            "spelling only");
      }
      return QObject::tr("checking with %1")
          .arg(SpellingAndGrammar::labelOf(c).toLower());
    }  // end of choiceReport()

  }  // end of anonymous namespace

  QString SpellingAndGrammar::checkerAddress() {
    return QStringLiteral("spelling/checker");
  }  // end of SpellingAndGrammar::checkerAddress

  QString SpellingAndGrammar::nameOf(const Checker c) {
    switch (c) {
      case Checker::None:
        return QStringLiteral("none");
      case Checker::Hunspell:
        return QStringLiteral("hunspell");
      case Checker::HunspellHarper:
        return QStringLiteral("hunspell-harper");
      case Checker::LanguageTool:
        return QStringLiteral("language-tool");
    }
    return QStringLiteral("hunspell");
  }  // end of SpellingAndGrammar::nameOf

  SpellingAndGrammar::Checker SpellingAndGrammar::checkerOf(const QString &n) {
    if (n == QLatin1String("none")) {
      return Checker::None;
    }
    if (n == QLatin1String("hunspell-harper")) {
      return Checker::HunspellHarper;
    }
    if (n == QLatin1String("language-tool")) {
      return Checker::LanguageTool;
    }
    // "hunspell" is what the editor did before the grammar checkers came, and
    // what a value nobody wrote — a misspelled one included — reads as
    return Checker::Hunspell;
  }  // end of SpellingAndGrammar::checkerOf

  QString SpellingAndGrammar::labelOf(const Checker c) {
    switch (c) {
      case Checker::None:
        return QObject::tr("No spell or grammar checking");
      case Checker::Hunspell:
        return QObject::tr("Hunspell (spelling only)");
      case Checker::HunspellHarper:
        return QObject::tr("Hunspell and Harper (spelling and grammar)");
      case Checker::LanguageTool:
        return QObject::tr("LanguageTool (spelling and grammar)");
    }
    return {};
  }  // end of SpellingAndGrammar::labelOf

  QStringList SpellingAndGrammar::names() {
    auto result = QStringList();
    for (const auto c : {Checker::None, Checker::Hunspell,
                         Checker::HunspellHarper, Checker::LanguageTool}) {
      result << nameOf(c);
    }
    return result;
  }  // end of SpellingAndGrammar::names

  SpellingAndGrammar::Checker SpellingAndGrammar::currentChecker() {
    return checkerOf(QSettings()
                         .value(checkerAddress(), nameOf(Checker::Hunspell))
                         .toString());
  }  // end of SpellingAndGrammar::currentChecker

  void SpellingAndGrammar::setChecker(const Checker c) {
    auto settings = QSettings();
    if (c == Checker::Hunspell) {
      // the default of the editor is not stored, as no entry stores its own
      // default: the file holds what was chosen and nothing else
      settings.remove(checkerAddress());
      return;
    }
    settings.setValue(checkerAddress(), nameOf(c));
  }  // end of SpellingAndGrammar::setChecker

  bool SpellingAndGrammar::usesHunspell(const Checker c) {
    return (c == Checker::Hunspell) || (c == Checker::HunspellHarper);
  }  // end of SpellingAndGrammar::usesHunspell

  bool SpellingAndGrammar::usesHarper(const Checker c) {
    return c == Checker::HunspellHarper;
  }  // end of SpellingAndGrammar::usesHarper

  bool SpellingAndGrammar::usesLanguageTool(const Checker c) {
    return c == Checker::LanguageTool;
  }  // end of SpellingAndGrammar::usesLanguageTool

  bool SpellingAndGrammar::isAvailable(const Checker c) {
    switch (c) {
      case Checker::None:
        return true;
      case Checker::Hunspell:
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
        // a dictionary is none of the entries of the editor: without one,
        // hunspell says nothing about any word, which is the answer of a
        // machine where nothing was installed and not a reason to hide the
        // choice the person who installed the dictionaries wants
        return true;
#else  /* TFEL_GUI_HUNSPELL_SUPPORT */
        // the spell checker is compiled out: the entry would underline
        // nothing at all and is none of the four choices of such a build
        return false;
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
      case Checker::HunspellHarper:
        return !LspHarperSupport::serverExecutable().isEmpty();
      case Checker::LanguageTool:
        return LanguageToolClient::isAvailable();
    }
    return false;
  }  // end of SpellingAndGrammar::isAvailable

  SpellingAndGrammar::Checker SpellingAndGrammar::effectiveChecker(
      const bool whole_text_is_prose) {
    const auto checker = currentChecker();
    if ((checker == Checker::LanguageTool) && (!whole_text_is_prose)) {
      // LanguageTool is handed a text and answers what is wrong with it: a
      // source file is no text to it, it is a thousand words of no language, and
      // every identifier in it a typo. What such a buffer is read with is
      // therefore the choice which names hunspell alone: the words its comments
      // spell are underlined and the sentences are left alone, which is what a
      // person who chose a proofreader for their prose wants of a source file
      // they are not reading as prose
      return Checker::Hunspell;
    }
    return checker;
  }  // end of SpellingAndGrammar::effectiveChecker

  QString SpellingAndGrammar::spellCheckLanguage(const Checker c) {
    // a dictionary for the choices which name hunspell, and an empty language for
    // the two which do not: an empty language is what `SpellChecker` reads as 'no
    // dictionary', which is the off switch of the underlines a mode has
    return usesHunspell(c) ? hunspellLanguage() : QString();
  }  // end of SpellingAndGrammar::spellCheckLanguage

  QString SpellingAndGrammar::spellCheckLanguage() {
    // what the choice in use says of the spelling of the words: a language for
    // hunspell, or nothing at all for the buffer LanguageTool reads on its own
    return spellCheckLanguage(currentChecker());
  }  // end of SpellingAndGrammar::spellCheckLanguage

  QString SpellingAndGrammar::languageOf(const Checker c) {
    if (usesHarper(c)) {
      return LspHarperSupport::dialect();
    }
    if (usesLanguageTool(c)) {
      return LanguageToolSupport::language();
    }
    return hunspellLanguage();
  }  // end of SpellingAndGrammar::languageOf

  void SpellingAndGrammar::setLanguage(const Checker c,
                                       const QString &language) {
    // the address of the choice which was chosen is written, and the two others
    // too when the language names a variety they have an entry for: the menus of
    // the choices which are not in use tick what the person who is reading the
    // buffer chose, and not an English of their own which is what was chosen
    // months ago for another checker
    if (usesHarper(c)) {
      LspHarperSupport::setDialect(language);
    } else if (usesLanguageTool(c)) {
      LanguageToolSupport::setLanguage(language);
    }
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
    const auto *const variety = varietyOf(language);
    auto settings = QSettings();
    if (usesHunspell(c)) {
      settings.setValue(QStringLiteral("hunspell default dictionary"),
                        (variety != nullptr) ? QLatin1String(variety->dictionary)
                                             : language);
    } else if (variety != nullptr) {
      settings.setValue(QStringLiteral("hunspell default dictionary"),
                        QLatin1String(variety->dictionary));
    }
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
  }  // end of SpellingAndGrammar::setLanguage

  void SpellingAndGrammar::applyTo(EditorWidget &editor) {
    // every text edit of the editor is asked, and not the current buffer only:
    // a person who reads two buffers of one project wants the two of them under-
    // lined alike. A text edit whose mode holds no session has no grammar
    // checker to start and gains only the language of the spell checker it does
    // have — which is what a buffer whose choice names hunspell and nothing else
    // is to every text edit of the editor
    const auto language = spellCheckLanguage();
    const auto edits = editor.findChildren<TextEditBase *>();
    for (auto *const e : edits) {
      if (e == nullptr) {
        continue;
      }
      if (e->hasMajorMode()) {
        auto &mode = e->getMajorMode();
        if (auto *const session = mode.grammarCheckSession();
            session != nullptr) {
          // the session decides for its own buffer: LanguageTool reads the whole
          // text of a buffer of prose and nothing of a source file, so which
          // hunspell language a text edit ends up with is its answer and not the
          // one this loop could guess
          session->followChecker(false);
          continue;
        }
      }
      // A text edit with no major mode has nothing to underline and keeps the
      // language all the same: a mode installed afterwards is handed the
      // language of its text edit, and the buffer of a `*scratch*` is no reason
      // for a dictionary to reach the editor in parts
      e->setSpellCheckLanguage(language);
    }
  }  // end of SpellingAndGrammar::applyTo

  void SpellingAndGrammar::addToMenu(QMenu &menu,
                                     EditorWidget &editor,
                                     const bool whole_text_is_prose) {
    // the four choices are one exclusive question: what reads the words of the
    // buffers. Every entry does its own choosing on the click. An entry which
    // carries a submenu is only ever *opened* by Qt, never triggered, so
    // hanging the languages off the entry which names their checker made that
    // entry unclickable — picking `LanguageTool` looked like a choice which did
    // nothing at all, and the buffers kept the checker they had. The languages
    // stand therefore in one submenu below the choices, which names the
    // languages of the choice in use
    auto *const group = new QActionGroup(&menu);
    group->setExclusive(true);
    const auto current = currentChecker();
    for (const auto c : {Checker::None, Checker::Hunspell,
                         Checker::HunspellHarper, Checker::LanguageTool}) {
      if (!isAvailable(c)) {
        continue;
      }
      auto label = labelOf(c);
      if ((c == Checker::LanguageTool) && (!whole_text_is_prose)) {
        // what the choice reads is not the same in every buffer: LanguageTool is
        // handed a text and no source file, so the entry of a source buffer says
        // that the words of *this* buffer are left to the dictionary while the
        // choice reads the documents of prose the editor holds
        label += QObject::tr(" (text files)");
      }
      auto *const entry = menu.addAction(label);
      entry->setCheckable(true);
      entry->setChecked(c == current);
      group->addAction(entry);
      QObject::connect(
          entry, &QAction::triggered,
          [&editor, c, whole_text_is_prose]() {
            SpellingAndGrammar::setChecker(c);
            SpellingAndGrammar::applyTo(editor);
            editor.displayInformativeMessage(
                choiceReport(c, whole_text_is_prose));
          });
    }
    // the languages of the choice in use, gathered when the submenu is opened
    // and not when the menu was built: the choice can have changed since — by
    // an entry of this very menu, or by the menu of another buffer — and a list
    // of languages disagreeing with the choice above it is worse than none. The
    // leaves write the checker along with the language: a stored language of
    // its own must never reach a buffer read by another program, and it is what
    // keeps the ticked language of each choice honest
    menu.addSeparator();
    auto *const languages = menu.addMenu(QObject::tr("Language"));
    languages->setObjectName(QStringLiteral("spelling-languages"));
    QObject::connect(
        languages, &QMenu::aboutToShow,
        [&editor, languages, whole_text_is_prose]() {
          languages->clear();
          const auto c = SpellingAndGrammar::currentChecker();
          auto names = QStringList();
          auto labels = QStringList();
          gatherLanguages(c, names, labels);
          if (names.isEmpty()) {
            // the choice which names no program, or the dictionary nobody
            // installed: there is nothing to name and the submenu says why
            auto *const nothing =
                languages->addAction((c == Checker::None)
                                         ? QObject::tr("no proofreader is chosen")
                                         : QObject::tr("no language is installed"));
            nothing->setEnabled(false);
            return;
          }
          auto *const language_group = new QActionGroup(languages);
          language_group->setExclusive(true);
          const auto chosen = SpellingAndGrammar::languageOf(c);
          for (auto i = 0; i != names.size(); ++i) {
            auto *const a = languages->addAction(labels.at(i));
            a->setCheckable(true);
            a->setChecked(names.at(i) == chosen);
            language_group->addAction(a);
            const auto name = names.at(i);
            QObject::connect(
                a, &QAction::triggered,
                [&editor, c, name, whole_text_is_prose]() {
                  SpellingAndGrammar::setChecker(c);
                  SpellingAndGrammar::setLanguage(c, name);
                  // the buffers follow the language they are read with: a text
                  // which changed by nothing is never read again by itself, and
                  // the underlines of the English which is gone would stay on
                  // the screen
                  SpellingAndGrammar::applyTo(editor);
                  editor.displayInformativeMessage(
                      choiceReport(c, whole_text_is_prose));
                });
          }
        });
  }  // end of SpellingAndGrammar::addToMenu

  namespace {
    /*!
     * \return the entries of the `Spelling and grammar` theme
     *
     * \note the language of the dictionary is an entry of this theme and not a
     * theme of its own: which program reads the words and which words it reads
     * are one decision, and a person who looks for the spellings of a buffer
     * looks in the corner of the dialog where the proofreaders are
     */
    QVector<SettingsKey> spelling_entries() {
      auto entries = QVector<SettingsKey>{};
      entries << SettingsKey::choice(
          "spelling/checker",
          QObject::tr("Proofreader of the buffers"),
          QObject::tr("What the buffers are read with: `hunspell` underlines "
                      "the words it does not know, `hunspell` and `harper-ls` "
                      "add the wrong agreements and the articles which do not "
                      "fit, and LanguageTool says both, in every language it "
                      "knows, and takes the dictionary off the buffers whose "
                      "whole text is prose. `none` underlines nothing. The "
                      "`Orthograph/Grammar` menu of `Options` writes this same "
                      "address and the buffers which are open follow it at once"),
          QVector<SettingsKey::Choice>()
              << SettingsKey::Choice(SpellingAndGrammar::nameOf(
                                         SpellingAndGrammar::Checker::None),
                                     QObject::tr("None"))
              << SettingsKey::Choice(
                     SpellingAndGrammar::nameOf(
                         SpellingAndGrammar::Checker::Hunspell),
                     QObject::tr("Hunspell (spelling only)"))
              << SettingsKey::Choice(
                     SpellingAndGrammar::nameOf(
                         SpellingAndGrammar::Checker::HunspellHarper),
                     QObject::tr("Hunspell and Harper"))
              << SettingsKey::Choice(
                     SpellingAndGrammar::nameOf(
                         SpellingAndGrammar::Checker::LanguageTool),
                     QObject::tr("LanguageTool")),
          SpellingAndGrammar::nameOf(SpellingAndGrammar::Checker::Hunspell));
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
      // the language of the words the dictionary is asked about. What reads the
      // words at all is the entry above; what is said here is only which words
      // that reading holds to be spelled right, see issue #69
      entries << SettingsKey::text(
          "hunspell default dictionary",
          QObject::tr("Default dictionary"),
          QObject::tr("The language the misspelled words are underlined with, "
                      "named as the installed dictionaries name it, such as "
                      "`en_US`. Leaving it empty takes the language of the "
                      "session. The language submenu of the `Hunspell` entries "
                      "of `Options > Orthograph/Grammar` writes this same "
                      "address, and a buffer whose whole text LanguageTool "
                      "reads is underlined by that program alone: the "
                      "dictionary then says nothing of that buffer"));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
      return entries;
    }  // end of spelling_entries
  }    // end of anonymous namespace

  // the choice itself, in a theme of its own: a person who wants the misspelled
  // words underlined and nothing else, or a whole proofreader, looks for one
  // entry. The entries of the programs that choice can pick are submenus of
  // this theme, named after those programs, since which program runs and what
  // it is asked with are one decision with the choice itself
  static const StandardSettingsGroupProxy spelling_and_grammar(
      "Spelling and grammar",
      QObject::tr("What reads the words of the buffers, the language it reads "
                  "them as, and what it underlines"),
      spelling_entries());

}  // end of namespace tfel::gui
