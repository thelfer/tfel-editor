/*!
 * \file   include/TFEL/GUI/TextEditQueryReplace.hxx
 * \brief the three user inputs of the query and replace command.
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX
#define LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "TFEL/GUI/CommandLine.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEdit;

  /*!
   * \brief input of the query replace, shown in the minibuffer
   *
   * The `Alt-%` and `Ctrl-Alt-%` key bindings of
   * `TextEditBase::handleKeyPressEventWithEmacsShortCuts` create this
   * input, which asks in turn for the string to be replaced (stage 0) and
   * for the replacement string (stage 1). Stage 2 installs a
   * `TextEditReplaceFilter` in the text area, turns the line edit into a
   * read-only label and lets the user answer 'y', 'n' or '!' for each
   * occurrence found.
   *
   * \note the three stages are three different inputs: each one removes
   * itself from the minibuffer and shows the next one, see
   * `treatUserInput`
   *
   * \note `isBlocking` is not overridden and returns false, so the
   * buffers stay enabled while the strings are typed; it is the filter
   * of stage 2 which grabs the key presses, and it does so from the
   * text area, not from the minibuffer
   *
   * \note the `Ctrl-Alt-%` binding asks the same three questions of a
   * regular expression, which the third argument of the constructors
   * says: the first answer is then an expression of `QRegularExpression`,
   * its invalidity is reported before the next question is asked, and the
   * replacement may read `\0` for the whole match and `\1` to `\9` for the
   * groups of the expression. The two commands keep two histories.
   */
  struct TextEditQueryReplace : public CommandLine {
    /*!
     * \brief constructor of the first stage, asking for the string to
     * be replaced
     * \param[in] t : text area the replacement is performed in
     * \param[in] p : editor widget the input is shown in
     * \param[in] r : the answer is a regular expression
     *
     * \note the label shows the last pair of the history as default
     * strings, when the history holds one;
     * \note the two strings are stored in the same list, alternating, so
     * the next to last entry is the string which was replaced and the
     * last one its replacement;
     * \note the entries are saved by `addToHistory`, not through the
     * settings address of the line edit, which stays empty here
     */
    TextEditQueryReplace(TextEditBase&, EditorWidget&, const bool r = false);
    //! destructor
    ~TextEditQueryReplace() override;

   protected:
    /*!
     * \return the strings previously used by the query replace, oldest
     * first; an empty list when the command was never used
     * \param[in] r : history of the regular expression command rather
     * than that of the plain text one
     *
     * \note the two commands keep two lists, at the addresses
     * "user input/query replace/history" and
     * "user input/query replace regexp/history": what one asks with a
     * regular expression is no answer to remember for a search of plain
     * text, whose answers are read as they stand
     */
    static QStringList getHistory(const bool r);

    /*!
     * \brief append a string to the history of the query replace
     * \param[in] i : string to append
     * \param[in] r : history of the regular expression command rather
     * than that of the plain text one
     *
     * \note the history is saved in the settings and only its hundred
     * last entries are kept
     */
    static void addToHistory(const QString&, const bool r);

    /*!
     * \brief constructor of the second stage, asking for the
     * replacement string
     * \param[in] t : text area the replacement is performed in
     * \param[in] p : editor widget the input is shown in
     * \param[in] s1_ : string to be replaced, shown in the label
     * \param[in] r : the string above is a regular expression
     *
     * \note no filter is installed yet: the focus stays in the line
     * edit while the user types the replacement string
     */
    TextEditQueryReplace(TextEditBase&,
                         EditorWidget&,
                         const QString&,
                         const bool r);

    /*!
     * \brief constructor of the last stage, performing the replacements
     * \param[in] t : text area the replacement is performed in
     * \param[in] p : editor widget the input is shown in
     * \param[in] s1_ : string to be replaced
     * \param[in] s2_ : replacement string
     * \param[in] r : the string to be replaced is a regular expression
     *
     * \note this constructor builds a `TextEditReplaceFilter`, which
     * searches the first occurrence, installs it and makes the line edit
     * read-only: from then on the answers are typed in the text area
     *
     * \note an expression which does not compile is refused before this
     * stage is ever reached, see `treatUserInput`; the filter refuses to
     * be installed when the buffer holds no match of the pattern, and the
     * input of this stage is then taken out of the minibuffer by the
     * destructor of the filter it never had
     */
    TextEditQueryReplace(TextEditBase&,
                         EditorWidget&,
                         const QString&,
                         const QString&,
                         const bool r);

    /*!
     * \brief ask the next stage: the replacement string at stage 0, the
     * filter at stage 1, nothing at stage 2
     *
     * At stage 0, an empty entry reuses the last pair of the history and
     * cancels with an "empty entry" message when the history holds no
     * such pair; a non empty entry is saved in the history. At stage 1,
     * the replacement string is saved in the history. In both cases the
     * input removes itself and shows the input of the next stage, so
     * this method always runs on a live input.
     *
     * \note at stage 0 of the regular expression command, an answer which
     * is no valid expression is reported and the same question is asked
     * again: an expression which does not compile is no question about a
     * buffer, and the next stage would answer it with a buffer holding no
     * match
     *
     * \note at stage 2 and beyond, nothing is done: the answers are
     * typed in the text area and handled by `TextEditReplaceFilter`
     *
     * \note the focus is given back to the text area when the last stage
     * is shown, as the answers must be typed there
     */
    void treatUserInput() override;

    //! text area the replacement is performed in
    TextEditBase& textEdit;
    //! string to be replaced, empty at stage 0, set from stage 1 on
    QString s1;
    //! true when the string to be replaced is a regular expression
    const bool regexp;
    //! stage of the command: 0, 1 or 2
    int stage;
  };  // end of struct TextEditQueryReplace

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX */
