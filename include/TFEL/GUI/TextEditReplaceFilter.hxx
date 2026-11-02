/*!
 * \file  TextEditReplaceFilter.hxx
 * \brief key press event filter of the query and replace.
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX
#define LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX

#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEdit;
  // forward declaration
  struct TextEditQueryReplace;

  /*!
   * \brief a key press event filter which forward the user
   * answers of the query replace: 'y', 'n' or '!'
   *
   * The filter installed by the last stage of `TextEditQueryReplace`:
   * the focus is given to the text area, the occurrence to replace is
   * selected there and each key press moves that selection to the next
   * occurrence, replaces it, or replaces all the remaining ones.
   *
   * \note when the text area has a selection, the replacements are
   * restricted to that region, whose bounds are saved in `bc` and `ec`
   */
  struct TextEditReplaceFilter : public TextEditKeyPressEventFilter {
    /*!
     * \brief constructor, which selects the first occurrence to replace
     * \param[in] w : editor widget the minibuffer messages are displayed in
     * \param[in] t : text area the replacements are performed in
     * \param[in] qr_ : query replace input shown in the minibuffer
     * \param[in] s1_ : string to be replaced
     * \param[in] s2_ : replacement string
     *
     * \note when the text area holds a selection, the cursor is moved to
     * the beginning of that selection, whatever the direction it was
     * made in, and only the region it covers is searched afterwards
     *
     * \note the constructor searches for the first occurrence of `s1_`;
     * when there is none, it displays, in the minibuffer, that the string
     * was not found and `isOk` returns false, so
     * `TextEditBase::setKeyPressEventFilter` refuses the filter and the
     * caller destroys it
     */
    TextEditReplaceFilter(EditorWidget&,
                          TextEditBase&,
                          TextEditQueryReplace&,
                          const QString&,
                          const QString&);

    /*!
     * \return true if the string to be replaced has been found by the
     * constructor, false otherwise, in which case the filter shall not
     * be installed
     */
    bool isOk() const override;

    /*!
     * \brief handle a key press of the text area during a query replace
     * \param[in] ev : event
     *
     * \return always true: the query replace consumes every key press,
     * so none of the key bindings of the text area triggers while it runs
     *
     * \note 'y' replaces the selected occurrence with `s2` and searches
     * for the next one; 'n' only searches for the next one. When no
     * further occurrence is found, the selection is cleared and the
     * filter removes itself, which ends the query replace
     *
     * \note '!' replaces the selected occurrence and all the remaining
     * ones, in a single edit block, then removes the filter
     *
     * \note `Ctrl-G` clears the selection and removes the filter without
     * replacing anything; it still displays the message described below
     * as it is neither 'y', 'n' nor '!'. A mouse press in the text area
     * removes the filter too, see
     * `TextEditBase::handleMousePressEvent`
     *
     * \note any other key press leaves the text untouched and displays
     * "type either 'y', 'n' or '!'" in the minibuffer
     */
    bool filterKeyPressEvent(QKeyEvent* const) override;
    /*!
     * \brief destructor
     *
     * Removes the query replace input from the minibuffer and displays,
     * in the minibuffer, the number of occurrences which have been
     * replaced, in singular or plural form.
     *
     * \note it runs whatever ended the query replace; the filter is only
     * deleted later, once it has been removed from the text area by
     * `TextEditBase::removeKeyPressEventFilter`, so no further key press
     * reaches it
     */
    ~TextEditReplaceFilter() override;

   protected:
    /*!
     * \return true if the given cursor selects an occurrence to replace
     * \param[in] nc : cursor returned by `QTextDocument::find`, which selects
     * the occurrence found and stands at its end
     *
     * \note a null cursor, that is a search which found nothing, is rejected.
     * \c QTextDocument::find does not wrap around, so no occurrence which was
     * already behind the cursor comes back
     *
     * \note when the replacements are restricted to a selection, the position
     * of the cursor, that is the end of the occurrence, must not be beyond the
     * end of that selection, which an occurrence ending exactly at the end of
     * the selection satisfies as well
     */
    bool isInsideWorkingRegion(const QTextCursor& nc) const;

    /*!
     * \brief find the next occurrence of s1
     * \return a boolean stating if a occurrence has been found
     *
     * \note the search starts from the cursor shown by the text area and never
     * wraps around; on success, that cursor is moved to the occurrence found,
     * which is selected
     *
     * \note when the replacements are restricted to a selection, an occurrence
     * is kept when it ends at or before the end of that selection; an
     * occurrence ending at the very end of the document is kept too, as the
     * cursor returned by \c QTextDocument::find stands at the end of the
     * occurrence it found
     */
    bool findNext();

    //! editor widget the query replace input and the messages are shown in
    EditorWidget& editor;

    //! text area the occurrences are searched for and replaced in
    TextEditBase& textEdit;

    /*!
     * \brief query replace input this filter answers for, which the
     * destructor removes from the minibuffer
     */
    TextEditQueryReplace& qr;
    //! beginning of the working region
    QTextCursor bc;
    /*!
     * \brief end of the working region, that is of the selection the
     * replacements are restricted to when `hasSelection` is true
     */
    QTextCursor ec;
    //! string to be replaced
    const QString s1;
    //! replacement string
    const QString s2;
    //! number of replacements done
    int nb = 0;
    //! options used for searching the replaced string; nothing changes
    //! them here, so the search of the query replace is case sensitive
    QTextDocument::FindFlag opts = QTextDocument::FindCaseSensitively;
    //! states if the filter is called on a portion of the file
    bool hasSelection = false;
    /*!
     * \brief current status: false when the string to be replaced has not
     * been found, which `isOk` reports so that the filter is not installed
     */
    bool bOK = true;

  };  // end of TextEditReplaceFilter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX */
