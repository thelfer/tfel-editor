/*!
 * \file  PlainTextEdit.hxx
 * \brief main text area of a buffer, based on `QPlainTextEdit`.
 * \author Thomas Helfer
 * \date   19/08/2012
 */

#ifndef LIB_TFEL_GUI_PLAINTEXTEDIT_HXX
#define LIB_TFEL_GUI_PLAINTEXTEDIT_HXX

#include <QtWidgets/QPlainTextEdit>
#include <QtPrintSupport/QPrinter>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  /*!
   * \brief main widget used to edit text
   *
   * This is the concrete `TextEditBase` implementation used for the
   * main frame of a buffer, see `Buffer::Buffer`: every editing
   * operation is forwarded to an owned `QPlainTextEdit` widget.
   * `ProcessOutputFrame` derives from this widget to display the
   * output of a `QProcess`.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT PlainTextEdit : public TextEditBase {
    /*!
     * \brief constructor, creating an empty text area
     * \param[in] g : editor widget the text area belongs to
     * \param[in] b : buffer edited by this text area
     *
     * \note the wrapped widget has word wrap enabled
     * (`QTextOption::WrapAnywhere`) and its vertical scroll bar is
     * displayed as a separate widget on its left, see
     * `setQAbstractScrollAreaInLayout`;
     * \note changes of the modification state of the document are
     * relayed by the `modificationChanged` signal
     */
    PlainTextEdit(EditorWidget&, Buffer&);

    /*!
     * \brief constructor, treating the given name
     * \param[in] f : name of a directory, of an existing file or of a
     * file to create
     * \param[in] g : editor widget the text area belongs to
     * \param[in] b : buffer edited by this text area
     *
     * \note when `f` denotes an existing directory, a
     * `DirectoryMajorMode` is installed and the major mode may not be
     * changed afterwards;
     * \note when `f` denotes an existing file, it is read as UTF-8 and
     * the cursor line saved when this file was last closed is
     * restored, see the destructor; if the file cannot be opened, a
     * critical message box is displayed and the text area is left
     * empty;
     * \note otherwise, the text of the template file associated with
     * the extension of `f` is inserted, see `readTemplateFile`; an
     * empty `f` leaves the text area empty;
     * \note in all cases, the file name is set to `f` and the context
     * menu of the wrapped widget is disabled
     */
    PlainTextEdit(const QString&, EditorWidget&, Buffer&);

    /*!
     * \brief destructor
     *
     * Saves the line of the current cursor position in the "positions
     * in files" setting, indexed by the complete file name, so that
     * the constructor taking a file name restores this position when
     * the file is opened again.
     */
    ~PlainTextEdit() override;

   public slots:

    /*!
     * \brief print the text
     *
     * Opens a modal `QPrintDialog` and prints the document only if the
     * user accepts the dialog. This slot is triggered by the print
     * action of the main window, through `EditorWidget::print`, and by
     * the key handling of `TextEditBase`.
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

   signals:

    /*!
     * \brief emitted when the modification state of the document
     * changes
     * \param[in] c : true if the document is modified now
     *
     * \note this signal is relayed from the document by the
     * constructor without a file name and is not connected anywhere in
     * the current code base
     */
    void modificationChanged(const bool);

   protected:
    /*!
     * \brief insert the text of a template file in the text area
     * \param[in] f : file name whose extension selects the template
     *
     * \note the template file `$HOME/.templates/TEMPLATE.<ext>.tpl`,
     * where `<ext>` is the complete suffix of `f`, is used; nothing
     * happens when there is no such file;
     * \note keywords of the form `(>>>KEY<<<)` are substituted before
     * insertion, for example `(>>>FILE<<<)` or `(>>>DATE<<<)`;
     * `(>>>AUTHOR<<<)`, `(>>>USER_NAME<<<)` and `(>>>LOGIN_NAME<<<)`
     * are handled on unix systems only;
     * \note the `(>>>POINT<<<)` markers are all removed and the cursor
     * is left at the position of the first one; without such a marker,
     * the cursor is placed at the beginning of the document;
     * \note the modified flag of the document is reset at the end;
     * \note only called by the constructor taking a file name, when
     * that file does not exist yet
     */
    void readTemplateFile(const QString&);

    //! wrapped text edit widget, owned (parented) by this widget
    QPlainTextEdit* e;

   private:
    Q_OBJECT
  };  // end of struct PlainTextEdit

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PLAINTEXTEDIT_HXX */
