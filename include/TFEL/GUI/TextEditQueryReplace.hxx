/*!
 * \file   include/TFEL/GUI/TextEditQueryReplace.hxx
 * \brief the three user inputs of the query and replace command.
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX
#define LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX

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
   */
  struct TextEditQueryReplace : public CommandLine {
    /*!
     * \brief constructor of the first stage, asking for the string to
     * be replaced
     * \param[in] t : text area the replacement is performed in
     * \param[in] p : editor widget the input is shown in
     *
     * \note the label shows the last pair of the history as default
     * strings, when the history holds one;
     * \note the two strings are stored in the same list, alternating, so
     * the next to last entry is the string which was replaced and the
     * last one its replacement;
     * \note the entries are saved by `addToHistory`, not through the
     * settings address of the line edit, which stays empty here
     */
    TextEditQueryReplace(TextEditBase&, EditorWidget&);
    //! destructor
    ~TextEditQueryReplace() override;

   protected:
    /*!
     * \return the strings previously used by the query replace, oldest
     * first, read from the "user input/query replace/history" setting;
     * an empty list when the command was never used
     */
    static QStringList getHistory();

    /*!
     * \brief append a string to the history of the query replace
     * \param[in] i : string to append
     *
     * \note the history is saved in the settings and only its hundred
     * last entries are kept
     */
    static void addToHistory(const QString&);

    /*!
     * \brief constructor of the second stage, asking for the
     * replacement string
     * \param[in] t : text area the replacement is performed in
     * \param[in] p : editor widget the input is shown in
     * \param[in] s1_ : string to be replaced, shown in the label
     *
     * \note no filter is installed yet: the focus stays in the line
     * edit while the user types the replacement string
     */
    TextEditQueryReplace(TextEditBase&, EditorWidget&, const QString&);

    /*!
     * \brief constructor of the last stage, performing the replacements
     * \param[in] t : text area the replacement is performed in
     * \param[in] p : editor widget the input is shown in
     * \param[in] s1_ : string to be replaced
     * \param[in] s2_ : replacement string
     *
     * \note the label asks the user for 'y', 'n' or '!' and the filter
     * installed on the text area reads those answers there, so the line
     * edit is made read-only, given no focus and the text area gets the
     * focus;
     * \note the filter looks for the first occurrence as it is built and
     * displays "'s1' not found", where s1 is the string to be replaced, in
     * the minibuffer when there is none, in which case
     * `TextEditBase::setKeyPressEventFilter` refuses it and this
     * constructor destroys it;
     * \note when the filter is destroyed, which is how the replacements
     * end, this input is validated and removed from the minibuffer
     */
    TextEditQueryReplace(TextEditBase&,
                         EditorWidget&,
                         const QString&,
                         const QString&);

    /*!
     * \brief move to the next stage of the query replace
     *
     * At stage 0, an empty entry reuses the last pair of the history and
     * cancels with an "empty entry" message when the history holds no
     * such pair; a non empty entry is saved in the history. At stage 1,
     * the replacement string is saved in the history. In both cases the
     * input removes itself and shows the input of the next stage, so
     * this method always runs on a live input.
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
    /*!
     * \brief stage of the query replace: 0 when asking for the string to
     * be replaced, 1 when asking for the replacement string, 2 while
     * replacing the occurrences
     *
     * \note `treatUserInput` and the label of the input depend on it
     */
    int stage;

  };  // end of struct TextEditQueryReplace

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITQUERYREPLACE_HXX */
