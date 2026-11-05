/*!
 * \file  CoreSettings.cxx
 * \brief the entries of the configuration the editor itself uses, by theme.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#include "TFEL/GUI/SettingsKey.hxx"
#include "TFEL/GUI/SettingsRegistry.hxx"

namespace tfel::gui {

  // the look of the editor and the key bindings it uses. The mode line and the
  // minibuffer take the same font as the text edits, which is why that address
  // bears no name which says what part of the editor it belongs to
  static const StandardSettingsGroupProxy general(
      "General",
      QObject::tr("The look of the editor and the key bindings it uses"),
      QVector<SettingsKey>()
          << SettingsKey::font(
                 "font/textedit",
                 QObject::tr("Font of the text edits"),
                 QObject::tr("Font of the main frames, of the mode line "
                             "and of the minibuffer. As long as no font "
                             "is chosen here, or in `Options > Main "
                             "Frames' Font`, the editor follows the font "
                             "of the application"))
          << SettingsKey::choice(
                 "shortcut style",
                 QObject::tr("Style of the key bindings"),
                 QObject::tr("`Emacs` takes the usual GNU Emacs bindings, `Qt` "
                             "lets the "
                             "line edits and the text editing areas handle the "
                             "usual Qt "
                             "shortcuts. Changing it here applies at once: the "
                             "menus are "
                             "rebuilt with the new bindings"),
                 QVector<SettingsKey::Choice>()
                     << SettingsKey::Choice(QStringLiteral("EMACS"),
                                            QObject::tr("Emacs"))
                     << SettingsKey::Choice(QStringLiteral("QT"),
                                            QObject::tr("Qt")),
                 QStringLiteral("EMACS"))
          << SettingsKey::choice(
                 "secondary tasks orientation",
                 QObject::tr("Orientation of the secondary tasks"),
                 QObject::tr(
                     "How the buffers a buffer shows next to its text, such as "
                     "the output of a compilation, are laid out. The buffers "
                     "which are already open keep the orientation they were "
                     "built with"),
                 QVector<SettingsKey::Choice>()
                     << SettingsKey::Choice(0, QObject::tr("Horizontally"))
                     << SettingsKey::Choice(1, QObject::tr("Vertically")),
                 QVariant::fromValue(0)));

  // what the editor remembers of its own account is not declared here, and has
  // hence no field in the dialog of the configuration: the histories of the
  // prompts, the list of recent files, the interface the dialog of `Run MFront`
  // proposes next. Those addresses are written by the very act of working, an
  // address \c CommandLine is given and written back when its answer is taken,
  // and a field of a dialog would show a stale list and be overwritten by the
  // next prompt: what is declared here, and only that, is what a person chooses
  // and what stays.

#ifdef TFEL_GUI_HUNSPELL_SUPPORT
  // the language the misspelled words are underlined with. The `Dictionaries`
  // menu of `Options` writes the same address, which is how that choice
  // survives the session, see issue #69
  static const StandardSettingsGroupProxy spell_checking(
      "Spell checking",
      QObject::tr("The language the misspelled words are underlined with"),
      QVector<SettingsKey>() << SettingsKey::text(
          "hunspell default dictionary",
          QObject::tr("Default dictionary"),
          QObject::tr(
              "The language the misspelled words are underlined with, named as "
              "the installed dictionaries name it, such as `en_US`. Leaving it "
              "empty takes the language of the session, and the `Dictionaries` "
              "menu of `Options` chooses the same language one buffer at a "
              "time")));
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */

}  // end of namespace tfel::gui
