/*!
 * \file  TextEdit.hxx
 * \brief text editing area based on a `QTextEdit` widget.
 * \author Thomas Helfer
 * \date   19/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDIT_HXX
#define LIB_TFEL_GUI_TEXTEDIT_HXX

#include <QtWidgets/QTextEdit>
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  /*!
   * \brief text editing area based on a `QTextEdit` widget
   *
   * Every editing operation is forwarded to an owned `QTextEdit`.
   * Unlike `PlainTextEdit`, this widget handles rich text and is used
   * for read only displays, such as the completions pane of
   * `CommandLine::showCompletions`.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextEdit : public TextEditBase {
    /*!
     * \brief constructor, creating an empty text area
     * \param[in] g : editor widget the text area belongs to
     * \param[in] b : buffer edited by this text area
     *
     * \note the vertical scroll bar of the wrapped widget is displayed
     * as a separate widget on its left, see
     * `setQAbstractScrollAreaInLayout`, its context menu is disabled
     * and its cursor is as wide as an average character
     */
    TextEdit(EditorWidget&, Buffer&);

    /*!
     * \brief constructor, reading the given file
     * \param[in] f : name of the file to read
     * \param[in] g : editor widget the text area belongs to
     * \param[in] b : buffer edited by this text area
     *
     * \note when `f` exists, its contents replaces the text, the
     * modified flag of the document is reset and the cursor is placed
     * at the beginning of the text; if the file cannot be opened, a
     * critical message box is displayed;
     * \note when `f` does not exist, the text area is left empty; the
     * file name is set to `f` in all cases;
     * \note unlike the other constructor, the context menu of the
     * wrapped widget is left enabled
     */
    TextEdit(const QString&, EditorWidget&, Buffer&);
    //! destructor
    ~TextEdit() override;

   public slots:

    /*!
     * \brief print the text
     *
     * Opens a modal `QPrintDialog` and prints the document only if the
     * user accepts the dialog.
     */
    void print() override;

    //! \brief remove the whole text
    void clear() override;

    /*!
     * \brief undo the last modification
     *
     * \note this slot is triggered by the undo action created by
     * `TextEditBase::createContextMenuActions`; nothing happens when
     * undo/redo is disabled, see `setUndoRedoEnabled`
     */
    void undo() override;

    //! \brief redo the last undone modification
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
     * \note the cursor is left after the inserted text
     */
    void insertPlainText(const QString&) override;

    /*!
     * \brief append plain text at the end of the document
     * \param[in] t : text to append
     *
     * \note the insertion is performed through a copy of the text
     * cursor moved to the end of the document and thus does not move
     * the cursor displayed in the text area
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
     * \note the insertion is performed through a copy of the text
     * cursor moved to the end of the document and thus does not move
     * the cursor displayed in the text area
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
     * \brief scroll the text area so that the cursor is centered
     *
     * \note this implementation does nothing
     */
    void centerCursor() override;

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
     * \return the wrapped text edit widget
     *
     * \note this is the widget on which `TextEditBase` installs its
     * event filter and to which focus and font changes are forwarded;
     * it is never null
     */
    QAbstractScrollArea* widget() override;

   protected:
    //! wrapped text edit widget, owned (parented) by this widget
    QTextEdit* e;

   private:
    Q_OBJECT
  };  // end of struct TextEdit

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDIT_HXX */
