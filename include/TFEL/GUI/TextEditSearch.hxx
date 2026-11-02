/*!
 * \file  TextEditSearch.hxx
 * \brief incremental search: a key press filter and a user input.
 * \author Thomas Helfer
 * \date   02/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITSEARCH_HXX
#define LIB_TFEL_GUI_TEXTEDITSEARCH_HXX

#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEdit;
  // forward declaration
  struct TextEditSearch;

  /*!
   * \brief a key press event filter which forward the user
   * key presses typed in the text area to the incremental search
   *
   * During an incremental search, the focus stays in the text area, so
   * the characters the user types arrive here instead of reaching the
   * line edit of the search. This filter sends them there, which
   * triggers a new search, and handles itself the keys which drive or
   * end the search.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextEditSearchFilter
      : public TextEditKeyPressEventFilter {
    /*!
     * \brief constructor
     * \param[in] t : text area whose key press events are filtered
     * \param[in] s : incremental search those events are forwarded to
     *
     * \note the filter is not installed on the text area here,
     * `TextEditSearch` does that in its own constructor; this filter
     * only arranges for deleting itself once the search input is
     * destroyed
     */
    TextEditSearchFilter(TextEditBase&, TextEditSearch&);

    //! \return always true: the constructor of this filter can not fail
    bool isOk() const override;

    /*!
     * \brief handle a key press of the text area during a search
     * \param[in] e : event
     *
     * \return true if the search has consumed the event, in which case
     * the key bindings of `TextEditBase` do not see it
     *
     * \note `Escape` removes the filter, which ends the search, and is
     * consumed; the arrow keys and `Return`, with no modifier, end the
     * search the same way but are not consumed, so the text area handles
     * them as if no search had been started
     *
     * \note `Ctrl-S` searches for the next occurrence, dropping the
     * backward option first if `Ctrl-R` had set it; `Ctrl-R` searches
     * for the previous one, setting that option the first time it is
     * used. Both consume the event, as does `Ctrl` alone, which is
     * ignored
     *
     * \note any other event is sent to the line edit of the search as
     * if it had been typed there, so the text of the search changes and
     * the document is searched again
     */
    bool filterKeyPressEvent(QKeyEvent* const) override;
    //! destructor
    ~TextEditSearchFilter() override;

   public slots:

    /*!
     * \brief delete this filter when the search input is destroyed, so
     * that the text area does not keep a filter on a dead input
     *
     * \note connected to the `destroyed` signal of the search input by
     * the constructor
     */
    void lineEditDestroyed();

   protected:
    //! text area whose key press events are filtered
    TextEditBase& textEdit;

    /*!
     * \brief incremental search this filter feeds
     *
     * \note this filter deletes itself when that input is destroyed,
     * see `lineEditDestroyed`; the input, in turn, forgets about this
     * filter when the filter goes away, see
     * `TextEditSearch::searchFilterDestroyed`
     */
    TextEditSearch& s;

   private:
    Q_OBJECT

  };  // end of TextEditSearchFilter

  /*!
   * \brief input of the incremental search, shown in the minibuffer
   *
   * `Ctrl-S` and `Ctrl-R`, handled by
   * `TextEditBase::handleKeyPressEventWithEmacsShortCuts`, create this
   * input and give the focus back to the text area. The constructor
   * installs a `TextEditSearchFilter` on that area to relay the keys
   * typed there to this line edit and, since `textChanged` is connected
   * to `search`, the document is searched again as soon as the text of
   * the search changes.
   *
   * \note the search is not a modal input: `isBlocking` is not
   * overridden and returns false, so the buffers stay enabled and
   * `Ctrl-X o` gives the focus back to the main frame while the search
   * is running
   *
   * \note the search is finalised as soon as the filter is removed,
   * which `Escape`, `Return`, the arrow keys and a mouse press in the text
   * area all do; the input is then removed from the minibuffer, see
   * `searchFilterDestroyed`, and the text selected by the last search is
   * left selected, unless the key which ended the search moves the cursor
   *
   * \note `Ctrl-G` works the other way round: the filter relays it to the
   * line edit, which cancels the input, and the filter deletes itself once
   * that input is gone
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextEditSearch : public CommandLine {
    /*!
     * \brief constructor, which starts the incremental search
     * \param[in] t : text area to search in
     * \param[in] p : editor widget the search is shown in
     * \param[in] f : options of the search, case sensitive by default,
     * `QTextDocument::FindBackward` makes it backward
     *
     * \note the label of the input is set by `setSearchOptions`, to
     * "i-search :" or "i-search backward :"
     *
     * \note the position of the cursor of the text area is saved as the
     * anchor of the search, see the `cursor` member; the constructor
     * also installs the filter and connects the signals which keep the
     * filter and the input alive together
     */
    TextEditSearch(
        TextEditBase&,
        EditorWidget&,
        const QTextDocument::FindFlags = QTextDocument::FindCaseSensitively);

    /*!
     * \brief search for the next occurrence of the text typed so far
     *
     * The search starts from the end of the current selection, which is
     * kept when there is one so that repeated calls walk through the
     * matches instead of finding the same one again.
     *
     * \note on success, the match is selected in the text area and
     * becomes the new anchor of the search; on failure, the cursor is
     * left untouched and "failed search" is displayed in the minibuffer
     */
    virtual void findNext();

    /*!
     * \brief set the options used for searching and update the label of
     * the input accordingly
     * \param[in] o : search options, `QTextDocument::FindBackward`
     * makes the search backward
     */
    virtual void setSearchOptions(const QTextDocument::FindFlags);

    /*!
     * \return the options used for searching, case sensitive by default
     * and backward when `QTextDocument::FindBackward` is set
     */
    virtual QTextDocument::FindFlags getSearchOptions() const;
    //! destructor
    ~TextEditSearch() override;

   protected slots:

    /*!
     * \brief search the document for the given text
     * \param[in] s : text to search, the whole text of the line edit
     *
     * \note the search starts from the anchor of the search, not from
     * the cursor shown by the text area, so that the search restarts
     * from the same place every time the typed text changes; connected
     * to `textChanged`, so it runs on every keystroke
     *
     * \note on failure, the cursor of the text area is left at that
     * anchor and "failing search" is displayed in the minibuffer
     */
    virtual void search(const QString&);

   public slots:

    /*!
     * \brief forget the filter when it has been destroyed and remove
     * this input from the minibuffer, which ends the search
     *
     * \note connected to the `destroyed` signal of the filter by the
     * constructor; deleting the filter is how the search is ended from
     * the text area, see `TextEditSearchFilter::filterKeyPressEvent`
     */
    virtual void searchFilterDestroyed();

   protected:
    /*!
     * \brief do nothing: the document is searched as the text is typed, so
     * there is nothing to do once the input is validated
     *
     * \note should the input be validated anyway, by `Return` typed in the
     * line edit of the minibuffer, the search is over all the same: the
     * input is destroyed and the filter goes away with it
     */
    void treatUserInput() override;

    //! text area the search is performed in
    TextEditBase& textEdit;

    /*!
     * \brief filter relaying the key presses of the text area to this
     * input, null once the filter has been removed
     *
     * \note the text area owns the filter as a Qt child of that area;
     * this pointer is set to null by `searchFilterDestroyed`
     */
    TextEditSearchFilter* sf;

    /*!
     * \brief position of the cursor at the beginning of the search
     *
     * \note `search` restores that cursor before searching, so a new
     * keystroke restarts the search from where it started; `findNext`
     * moves it to the end of the match it has found
     */
    QTextCursor cursor;
    /*!
     * \brief options used for searching
     *
     * \note case sensitive by default; `Ctrl-R` and `Ctrl-S` toggle
     * `QTextDocument::FindBackward`, see `setSearchOptions`
     */
    QTextDocument::FindFlags opts = QTextDocument::FindFlags();

   private:
    Q_OBJECT

  };  // end of struct TextEditSearch

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITSEARCH_HXX */
