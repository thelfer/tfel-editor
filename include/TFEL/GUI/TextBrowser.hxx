/*!
 * \file  TextBrowser.hxx
 * \brief read-only text area used as a secondary task of a buffer.
 * \author Thomas Helfer
 * \date   19/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTBROWSEREDIT_HXX
#define LIB_TFEL_GUI_TEXTBROWSEREDIT_HXX

#include <QtWidgets/QTextBrowser>
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  /*!
   * \brief text area driven by a `QTextBrowser`, for the buffers
   * whose content is displayed rather than edited
   *
   * Like the other `TextEditBase` implementations, every operation is
   * a thin layer over the wrapped widget returned by `widget()`. That
   * widget is a `QTextBrowser`, that is a `QTextEdit` which handles the
   * links of the document it displays and keeps a navigation history,
   * which is what distinguishes this class from `TextEdit`.
   *
   * \note neither constructor changes the read only state or the text
   * interaction flags of the wrapped widget: the callers set them with
   * `setReadOnly` and `setTextInteractionFlags`
   *
   * \note no instance of this class is created in the current code
   * base: `print` and `centerCursor` are pure virtual in
   * `TextEditBase` and are not overridden here, so the class is still
   * abstract and only a derived class implementing them could be
   * instantiated
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextBrowser : public TextEditBase {
    /*!
     * \brief constructor, creating an empty text area
     * \param[in] g : editor widget the text area belongs to
     * \param[in] b : buffer displayed by this text area
     *
     * \note the vertical scroll bar of the wrapped widget is displayed
     * as a separate widget on its left, see
     * `setQAbstractScrollAreaInLayout`;
     * \note `TextEditBase::initialize` installs the event filter of
     * the key bindings on the wrapped widget and connects its cursor
     * movements to the current line highlighting
     */
    TextBrowser(EditorWidget&, Buffer&);

    /*!
     * \brief constructor, displaying the content of the given file
     * \param[in] f : name of the file to display
     * \param[in] g : editor widget the text area belongs to
     * \param[in] b : buffer displayed by this text area
     *
     * \note when `f` denotes an existing file, its whole content is
     * inserted as plain text, the modified flag of the document is
     * reset and the cursor is moved to the start of the document; if
     * the file can not be opened, a critical message box is displayed
     * and the file name is not set;
     * \note when no file has that name, the text area is simply left
     * empty;
     * \note unless the file could not be opened, the file name is set
     * to `f`, which selects the major mode, see
     * `TextEditBase::setFileName`
     */
    TextBrowser(const QString&, EditorWidget&, Buffer&);

    /*!
     * \brief destructor
     *
     * The wrapped widget is a child of this widget and is destroyed
     * with it.
     */
    ~TextBrowser() override;

   public slots:

    //! \brief remove the whole text
    void clear() override;

    /*!
     * \brief undo the last modification of the document
     *
     * \note nothing happens when undo/redo is disabled, see
     * `setUndoRedoEnabled`
     */
    void undo() override;

    /*!
     * \brief redo the last undone modification of the document
     *
     * \note nothing happens when undo/redo is disabled, see
     * `setUndoRedoEnabled`
     */
    void redo() override;

    //! \brief copy the selection to the clipboard and remove it
    void cut() override;

    //! \brief copy the selection to the clipboard
    void copy() override;

    //! \brief insert the clipboard contents at the cursor position
    void paste() override;

    //! \brief select the whole text
    void selectAll() override;

    //! \brief replace the whole text by the given plain text
    void setPlainText(const QString&) override;

    /*!
     * \brief insert plain text at the cursor position
     * \param[in] t : text to insert
     *
     * \note the cursor displayed in the text area is moved past the
     * inserted text
     */
    void insertPlainText(const QString&) override;

    /*!
     * \brief append plain text at the end of the document
     * \param[in] t : text to append
     *
     * \note the text is inserted through a copy of the text cursor, so
     * the cursor displayed in the text area is left untouched; no
     * paragraph separator is added, unlike `QTextEdit::append`
     */
    void appendPlainText(const QString&) override;

    /*!
     * \brief replace the whole text by the given html text
     * \param[in] t : html text
     *
     * \note this is implemented by clearing the text area and calling
     * `insertHtml`
     */
    void setHtml(const QString&) override;

    /*!
     * \brief insert html text at the cursor position
     * \param[in] t : html text
     *
     * \note the insertion is performed through a copy of the text
     * cursor and thus does not move the cursor displayed in the text
     * area
     */
    void insertHtml(const QString&) override;

    /*!
     * \brief append html text at the end of the document
     * \param[in] t : html text
     *
     * \note as in `appendPlainText`, the text is inserted through a
     * copy of the text cursor, at the end of the document and without
     * any paragraph separator
     */
    void appendHtml(const QString&) override;

    //! \brief set the text interaction flags of the wrapped widget
    void setTextInteractionFlags(Qt::TextInteractionFlags) override;

    /*!
     * \brief set the cursor displayed in the text area
     * \param[in] tc : new cursor, including its selection
     */
    void setTextCursor(const QTextCursor&) override;

    /*!
     * \brief make the text area read only or editable
     * \param[in] b : true for a read only text area
     */
    void setReadOnly(bool) override;

    /*!
     * \brief enable or disable the undo and redo commands
     * \param[in] b : true to enable undo/redo
     */
    void setUndoRedoEnabled(bool) override;

   public:
    /*!
     * \return the document edited by the wrapped widget
     *
     * \note the document is owned by the wrapped widget and is never
     * null
     */
    QTextDocument* document() const override;

    /*!
     * \brief move the cursor displayed in the text area
     * \param[in] mo : move operation to perform on the cursor
     * \param[in] mm : move mode, moving the anchor or extending the
     * selection
     */
    void moveCursor(QTextCursor::MoveOperation, QTextCursor::MoveMode) override;

    //! \return true if the text area is read only
    bool isReadOnly() override;

    /*!
     * \brief search the next occurrence of the given text
     * \param[in] t : text to search
     * \param[in] f : flags controlling the search, which is case
     * sensitive by default
     * \return true if a match was found
     *
     * \note on success, the cursor is moved to the match and the
     * matched text is selected; on failure, the cursor is left
     * untouched
     */
    bool find(
        const QString&,
        QTextDocument::FindFlags = QTextDocument::FindCaseSensitively) override;

    //! \return a copy of the cursor displayed in the text area
    QTextCursor textCursor() const override;

    /*!
     * \return the cursor located at the given position of the viewport
     * of the wrapped widget
     */
    QTextCursor cursorForPosition(const QPoint&) const override;

    /*!
     * \return the rectangle occupied by the given cursor in the
     * viewport of the wrapped widget
     */
    QRect cursorRect(const QTextCursor&) const override;

    /*!
     * \return the rectangle occupied by the displayed cursor in the
     * viewport of the wrapped widget
     */
    QRect cursorRect() const override;

    /*!
     * \brief replace the extra selections displayed on top of the text
     * \param[in] l : new list of extra selections
     *
     * \note in the current code base, only
     * `TextEditBase::highlightCurrentLine` uses this facility, to
     * display the background of the current line
     */
    void setExtraSelections(const QList<QTextEdit::ExtraSelection>&) override;

    /*!
     * \return the extra selections currently displayed, an empty list
     * when there are none
     */
    QList<QTextEdit::ExtraSelection> extraSelections() const override;

    //! \return true if the undo and redo commands are enabled
    bool isUndoRedoEnabled() const override;

    /*!
     * \return the wrapped text browser widget
     *
     * \note this is the widget on which `TextEditBase` installs its
     * event filter and to which focus and font changes are forwarded;
     * it is never null
     */
    QAbstractScrollArea* widget() override;

   protected:
    //! wrapped text browser widget, owned (parented) by this widget
    QTextBrowser* e;

   private:
    Q_OBJECT

  };  // end of struct TextBrowser

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTBROWSEREDIT_HXX */
