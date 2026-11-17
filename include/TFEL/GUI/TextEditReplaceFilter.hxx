/*!
 * \file   include/TFEL/GUI/TextEditReplaceFilter.hxx
 * \brief filter used to interactively replace a string or a regular expression.
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX
#define LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX

#include <QtCore/QString>

#include <QtGui/QTextCursor>

#include "TFEL/GUI/SearchPattern.hxx"
#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel::gui {
  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct TextEditQueryReplace;

  /*!
   * \brief A filter used to interactively replace a text or a regular
   * expression with another.
   *
   * The filter is installed by the third stage of the query replace when the
   * two strings have been given. It selects the first match of the pattern,
   * which the text area shows as its own selection, and takes the keys typed in
   * the text area from there on: 'y' replaces the match on the screen and
   * searches the next one, 'n' leaves that match alone and searches the next
   * one, '!' replaces that match and every match which follows it in one go and
   * ends the walk.
   *
   * \note The answers are typed in the text area, not in the minibuffer: that
   * is why the third stage of the query replace turns its own input into a
   * read-only label and gives the focus back to the text area, where this filter
   * collects the keys. The filter's constructor searches the first occurrence
   * and reports false from `isOk` when the buffer holds none: the caller lets go
   * of a filter which refuses, which is also what ends the query replace.
   *
   * \note Whatever ends the walk — '!' at the last match, 'y' or 'n' on a
   * buffer which holds no further match, or Ctrl-G — the filter is removed from
   * the text area, and the destructor is what reports how many replacements
   * were made and takes the query replace input out of the minibuffer.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextEditReplaceFilter
      : public TextEditKeyPressEventFilter {
    /*!
     * \brief constructor: search for the first match
     * \param[in] w : editor widget, used to display messages and to remove the
     * user input of the query replace when the walk ends
     * \param[in] t : text edit the replacement is performed in
     * \param[in] qr_ : user input of the query replace, taken out of the
     * minibuffer by the destructor
     * \param[in] p : pattern looked for, with the text a match is rewritten to
     *
     * \note When the text edit shows a selection, the selection defines a
     * working region: the matches are searched from its beginning on and no
     * match standing after its end is touched, which is what makes the query
     * replace of a region a replacement of that region. The selection itself is
     * cleared, since the first match found becomes the selection of the text
     * edit.
     *
     * \note `isOk` answers false when the buffer holds no match, which is the
     * `'%1' not found` message of the minibuffer, and the caller is then left
     * destroying the filter.
     */
    TextEditReplaceFilter(EditorWidget&,
                          TextEditBase&,
                          TextEditQueryReplace&,
                          const SearchPattern&);

    /*!
     * \brief handle a key press event of the text area: 'y', 'n', '!' and
     * Ctrl-G drive the walk, every other key only says what may be typed
     * \param[in] ev : key press event
     */
    virtual bool filterKeyPressEvent(QKeyEvent* const ev) override;

    //! \return true when the buffer holds a match to drive the walk to
    virtual bool isOk() const override;

    //! destructor: says how many times the pattern was replaced
    ~TextEditReplaceFilter() override;

   protected:
    /*!
     * \return true when a match is inside the working region
     * \param[in] m : match to check
     *
     * Without a selection every match of the buffer is inside. With one, a
     * match whose first character stands at or before the end of the region is,
     * which is the rule of a replacement asked for a region: the place an
     * occurrence starts is what makes it one of the lines the selection covers,
     * whether its last character reaches past the end or not.
     */
    bool isInsideWorkingRegion(const SearchMatch& m) const;

    /*!
     * \return the place the next search starts from, the far end of the
     * selection the text edit shows when it holds one, its cursor otherwise
     *
     * The match on the screen is the selection, so searching from the far end
     * of that selection is what keeps the walk from answering the same match
     * again, and after a replacement the cursor stands behind the text written,
     * which is the place to look on from.
     */
    int searchFrom() const;

    /*!
     * \brief search for the next match inside the working region and select it
     * \return false if the buffer holds no such match
     */
    bool findNext();

    /*!
     * \brief rewrite the match the walk stands on with what the pattern says
     * \return the text which was written there
     *
     * The places of the match are what is rewritten — never a length counted
     * back from the cursor, which is right only when the match is as long as
     * the text it was searched with, and a regular expression is not.
     *
     * \note The cursor of the text edit ends behind the text written, with no
     * selection: that is the place the next search of the walk starts from.
     */
    QString replaceCurrentMatch();

    //! associated editor widget
    EditorWidget& editor;
    //! text edit the replacement is performed in
    TextEditBase& textEdit;
    //! user input of the query replace, removed when the walk ends
    TextEditQueryReplace& qr;

    /*!
     * \brief pattern looked for and text a match is rewritten to
     *
     * \note the search is always case sensitive, as it was before this class
     * knew regular expressions, and asks for nothing of the neighbours of a
     * match.
     */
    const SearchPattern pattern;

    //! match the walk stands on, the selection of the text edit
    SearchMatch current;

    /*!
     * \brief cursor pointing to the beginning of the working area (valid
     * only if hasSelection is true)
     */
    QTextCursor bc;

    /*!
     * \brief cursor pointing to the end of the working area (valid only if
     * hasSelection is true)
     */
    QTextCursor ec;

    //! true if the initial state contains a selection
    bool hasSelection;

    /*!
     * \brief true if a match was found at construction time, hence if this
     * filter can be used
     */
    bool bOK;

    //! number of times the pattern has been replaced
    int nb = 0;
  };  // end of struct TextEditReplaceFilter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITREPLACEFILTER_HXX */
