/*!
 * \file  SpellingAndGrammar.hxx
 * \brief the proofreader a buffer is read with: the four choices, the language
 *        each of them reads, and the menu which offers them.
 * \author Thomas Helfer
 * \date   13 nov. 2026
 */

#ifndef LIB_TFEL_GUI_SPELLINGANDGRAMMAR_HXX
#define LIB_TFEL_GUI_SPELLINGANDGRAMMAR_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "TFEL/GUI/Config.hxx"

// forward declarations
class QMenu;

namespace tfel::gui {

  // forward declarations
  struct EditorWidget;

  /*!
   * \brief which proofreader the buffers of the editor read their text with
   *
   * \note one choice is made for the editor and every buffer follows it: the
   * underlines of a text are the surface where the person who wrote it looks at
   * what they wrote, and two buffers of one session disagreeing about what an
   * error is makes the choice unreadable. The buffers which are open when the
   * choice is made are told to follow it, see \c applyTo, and the ones opened
   * afterwards are built with it
   *
   * \note the languages of the three proofreaders are one choice and not three:
   * a person who reads a buffer of British English chooses British once, and the
   * dictionary, the dialect and the code the proofreader in use reads are all
   * that variety of English, see \c setLanguage
   *
   * \note the four choices are the surface of three programs, and what each of
   * them reads is what decides which of them reads a given buffer: LanguageTool
   * is handed a text and answers what is wrong with it, which is a sense it has
   * of the whole of a document of prose and of no source file at all — see
   * \c effectiveChecker
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SpellingAndGrammar {
    /*!
     * \brief what reads the words of a buffer
     */
    enum class Checker {
      //! nothing reads them: no underline of any kind
      None,
      //! `hunspell` only: the words nobody spells, and no agreement
      Hunspell,
      //! `hunspell` for the words and `harper-ls` for the sentences
      HunspellHarper,
      //! LanguageTool, which says both, and `hunspell` goes off it
      LanguageTool
    };
    //! \return the address of the setting the choice is kept in
    static QString checkerAddress();
    /*!
     * \return the choice the editor follows, from the setting
     * `spelling/checker`: `Hunspell` when nothing was stored, which is what the
     * editor did before the grammar checkers were added to it
     *
     * \note a choice whose program is not there is kept as it was written: the
     * entry of a menu says what was asked for and the minibuffer says why
     * nothing answers, which is clearer than a menu which quietly does something
     * else than what it shows
     */
    static Checker currentChecker();
    /*!
     * \brief store the choice in the setting `spelling/checker`
     *
     * \note `Hunspell` is the default and removes the address, as no entry of
     * the editor stores its own default
     */
    static void setChecker(const Checker);
    //! \return the name the choice is kept in the setting under
    static QString nameOf(const Checker);
    //! \return the choice the name of a setting says, `Hunspell` for an unknown
    //! one
    static Checker checkerOf(const QString &name);
    /*!
     * \return the name a choice is offered to a person under: "LanguageTool
     * (spelling and grammar)" for `LanguageTool`
     */
    static QString labelOf(const Checker);
    /*!
     * \return the names of the four choices, in the order they are offered:
     * none, hunspell, hunspell and harper, language tool
     */
    static QStringList names();
    //! \return true when the choice reads the spelling of the words with
    //! `hunspell`
    static bool usesHunspell(const Checker);
    //! \return true when the choice asks `harper-ls` about the sentences
    static bool usesHarper(const Checker);
    //! \return true when the choice asks LanguageTool about the whole text
    static bool usesLanguageTool(const Checker);
    /*!
     * \return what reads a buffer whose whole text is prose or is not, of the
     * choice the editor follows
     * \param[in] whole_text_is_prose : true for a buffer which is a text to be
     * read — a document of LaTeX, a note of Markdown — false for the source of a
     * program, whose words are its comments and whose comments are few
     *
     * \note the difference is the one thing the choice cannot say on its own:
     * LanguageTool is handed a text and answers what is wrong with it, and a
     * source file handed to it is a text full of names no language has — every
     * `\command` of a LaTeX file and every identifier of a C++ one is a typo to
     * it. What such a buffer is read with is therefore the choice which names
     * hunspell: the words its comments spell are underlined, and the sentences
     * are left to a checker which has no way of telling them from the code
     */
    static Checker effectiveChecker(const bool whole_text_is_prose);
    /*!
     * \return true when the program of the choice is to be found: the entries
     * which name it are worth showing only then
     *
     * \note `None` and `Hunspell` are offered wherever the editor was built: the
     * first asks nothing of anyone and the second is a library the editor links
     * to, whose absence is a fact of the build and not of the machine
     */
    static bool isAvailable(const Checker);
    /*!
     * \return the language `hunspell` is to be given for a buffer read by the
     * given choice: the dictionary the setting `hunspell default dictionary`
     * names, or an empty string when that choice takes the spelling away from
     * it, which is what turns the underlines of the misspelled words off — an
     * empty language is what `SpellChecker` reads as 'no dictionary'
     */
    static QString spellCheckLanguage(const Checker);
    //! \return \c spellCheckLanguage of the choice the editor follows
    static QString spellCheckLanguage();
    /*!
     * \return the language the given choice reads its text as, named the way
     * that choice names it: a dictionary, a dialect, or a code
     */
    static QString languageOf(const Checker);
    /*!
     * \brief the language of a choice, which is the language of the other two
     * wherever they name the same variety of English
     * \param[in] checker : the choice a language is chosen for
     * \param[in] language : a name of \c LspHarperSupport::dialects, a code of
     * \c LanguageToolSupport::languages, or the name of an installed dictionary,
     * whichever the choice reads
     *
     * \note the three proofreaders spell a variety of English three different
     * ways — `en-GB`, "British" and `en_GB` — and one buffer is read by one of
     * them. What this writes is the address of the choice named and the
     * addresses of the two others when they have an entry of their own for that
     * same variety, which is what keeps the `Language` submenu from ticking an
     * English of its own accord when the choice changes
     */
    static void setLanguage(const Checker, const QString &language);
    /*!
     * \brief add the four choices and their languages to a menu
     * \param[in] menu : menu to complete; the entries are children of it
     * \param[in] editor : editor widget whose buffers are to follow the choice
     * \param[in] whole_text_is_prose : true when the menu is built for a buffer
     * whose whole text is text to be read; false when it is built in the menu of
     * a mode whose text is the source of a program
     *
     * \note the four entries are exclusive, the one in use is ticked, and each
     * entry does its own choosing on the click: an entry above a submenu is
     * opened by Qt and never triggered, so the languages are not hung off the
     * entry which names their checker. They stand in one `Language` submenu
     * below the choices, which lists — as it is opened — the languages of the
     * choice in use, and whose leaves write the checker along with the
     * language. A choice whose program is not to be found is not offered at all
     *
     * \note the choice is one for the editor, so a menu built in a mode and a
     * menu built in `Options` write the same address and do the same to every
     * buffer. What differs is what the entry of LanguageTool has to say and
     * what the click reports: in a source file, where the program reads no
     * grammar at all, see \c effectiveChecker
     */
    static void addToMenu(QMenu &menu,
                          EditorWidget &editor,
                          const bool whole_text_is_prose = true);
    /*!
     * \brief make the buffers which are open follow the choice and the language
     * which are held in the settings now
     *
     * \note every text edit of the editor is asked, and not the current buffer
     * only: the checker of a buffer is what a person looks at while they read it,
     * wherever it is on the screen. A text edit whose mode holds no session has
     * no grammar checker to follow and gains only the language of the spell
     * checker it does have
     */
    static void applyTo(EditorWidget &editor);
  };  // end of struct SpellingAndGrammar

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SPELLINGANDGRAMMAR_HXX */
