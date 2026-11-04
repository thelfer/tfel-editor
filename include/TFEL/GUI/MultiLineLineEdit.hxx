/*!
 * \file   MultiLineLineEdit.hxx
 * \brief  text edit used by the minibuffer and the dialogs in place of
 * `QLineEdit`: one line tall by default, growing and shrinking with its
 * content.
 * \author Thomas Helfer
 * \date   03/11/2026
 */

#ifndef LIB_TFEL_GUI_MULTILINEEDIT_HXX
#define LIB_TFEL_GUI_MULTILINEEDIT_HXX

#include <QtCore/QPointer>
#include <QtWidgets/QTextEdit>
#include <TFEL/GUI/Config.hxx>

class QValidator;

namespace tfel::gui {

  /*!
   * \brief a `QTextEdit` shaped like a `QLineEdit`: one line tall by
   * default, which grows when the text it holds requires more lines and
   * shrinks back when it does not
   *
   * The class provides the subset of the `QLineEdit` interface the editor
   * uses (`text`, `setText`, `insert`, the cursor and selection primitives
   * of `LineEdit`, the `validator`, and the `returnPressed` and
   * `textChanged` signals), so that `LineEdit` may inherit from it instead
   * of `QLineEdit` without rewriting any of the Emacs short cuts.
   *
   * Differences with `QLineEdit` worth knowing:
   *
   * - `Return` emits `returnPressed` without inserting anything, as in a
   *   `QLineEdit`, while `Shift+Return` inserts a new line (`QLineEdit`
   *   treated both keys alike);
   * - pasted newlines are kept: the widget grows to display them;
   * - the height grows up to ten lines, beyond which a vertical scroll bar
   *   takes over;
   * - rich text is never accepted: pasted content is inserted as plain text.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT MultiLineLineEdit : QTextEdit {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    explicit MultiLineLineEdit(QWidget *const p = nullptr);
    //! destructor
    ~MultiLineLineEdit() override;
    /*!
     * \return the text of the widget, the lines being separated by `'\n'`
     *
     * \note the paragraph separators `QTextDocument::toPlainText` uses are
     * translated, so that the value round-trips through `setText`
     */
    QString text() const;
    /*!
     * \brief replace the whole text
     * \param[in] t: new text
     *
     * \note as in `QLineEdit`, the validator is not consulted and the cursor
     * is moved at the end of the new text
     */
    void setText(const QString &);
    /*!
     * \brief insert text at the cursor position, replacing the selection if
     * there is one
     * \param[in] t: text to insert
     *
     * \note the text is not inserted if the validator rejects it
     */
    void insert(const QString &);

    //! \return position of the cursor in the text
    int cursorPosition() const;
    /*!
     * \return index of the first selected character, or `-1` if there is no
     * selection, as `QLineEdit::selectionStart` does
     */
    int selectionStart() const;
    /*!
     * \brief select the given range
     * \param[in] pos: position of the first selected character
     * \param[in] len: number of characters to select; a null length clears
     * the selection
     */
    void setSelection(const int pos, const int len);
    /*!
     * \brief move the cursor at the beginning of the line
     * \param[in] withSelection: extend the selection if true
     */
    void home(const bool withSelection = false);
    /*!
     * \brief move the cursor at the end of the line
     * \param[in] withSelection: extend the selection if true
     */
    void end(const bool withSelection = false);
    /*!
     * \brief move the cursor to the left
     * \param[in] withSelection: extend the selection if true
     * \param[in] n: number of characters to move
     */
    void cursorBackward(const bool withSelection = false, const int n = 1);
    /*!
     * \brief move the cursor to the right
     * \param[in] withSelection: extend the selection if true
     * \param[in] n: number of characters to move
     */
    void cursorForward(const bool withSelection = false, const int n = 1);
    /*!
     * \brief move the cursor to the beginning of the previous word
     * \param[in] withSelection: extend the selection if true
     */
    void cursorWordBackward(const bool withSelection = false);
    /*!
     * \brief move the cursor to the beginning of the next word
     * \param[in] withSelection: extend the selection if true
     */
    void cursorWordForward(const bool withSelection = false);
    /*!
     * \brief remove the selection, or the character before the cursor if
     * there is none, as `QLineEdit::backspace` does
     */
    void backspace();
    /*!
     * \brief remove the selection, or the character after the cursor if
     * there is none, as `QLineEdit::del` does
     */
    void del();
    /*!
     * \brief set the validator constraining the text
     * \param[in] v: validator; the caller keeps the ownership, which is
     * `QLineEdit`'s convention too
     *
     * \note as in `QLineEdit`, the validator is consulted when text is typed
     * or inserted, not when it is set as a whole with `setText`
     */
    void setValidator(const QValidator *);
    //! \return the validator constraining the text
    const QValidator *validator() const;
    /*!
     * \return the size hint of the widget: a `QLineEdit`'s width, the current
     * fixed height of its content
     *
     * \note `QAbstractScrollArea::sizeHint` would ask for a wide and tall
     * scroll area, which would make the minibuffer and the dialogs enormous
     */
    QSize sizeHint() const override;

   signals:
    /*!
     * \brief emitted when the `Return` key is pressed without the `Shift`
     * modifier, which does not insert a new line then
     */
    void returnPressed();
    /*!
     * \brief emitted whenever the text changes, whatever its origin
     * \param[in] t: new text
     */
    void textChanged(const QString &);

   protected:
    /*!
     * \brief handle a key press
     * \param[inout] ev: key event
     *
     * `Return` emits `returnPressed` and is consumed; a character rejected by
     * the validator is ignored; any other event is passed to `QTextEdit`
     */
    void keyPressEvent(QKeyEvent *) override;
    /*!
     * \brief react to a change of the font or of the palette
     * \param[inout] ev: event
     *
     * \note the height of a line, hence the height of the widget, depends on
     * the font: the geometry is updated when it changes
     */
    void changeEvent(QEvent *) override;
    /*!
     * \brief recompute the height of the widget, which depends on the width
     * the text is laid out in
     * \param[inout] ev: resize event
     */
    void resizeEvent(QResizeEvent *) override;
    /*!
     * \brief watch the size of the window, which bounds the height of the
     * widget
     *
     * \param[in] o: the object the event is sent to
     * \param[inout] ev: event
     *
     * \note the window is watched because the height the content is allowed to
     * take depends on it: the scroll bar has to take over when the window
     * shrinks and the content has to grow back when it grows again
     */
    bool eventFilter(QObject *, QEvent *) override;
    /*!
     * \brief handle the request to propagate the height of the content to the
     * enclosing layouts
     * \param[inout] ev: event
     *
     * \note the request is a posted event of a high priority, which the event
     * loop deals with before the layout requests the change of the constraints
     * generates, and as many times as it is posted again during the same round
     */
    bool event(QEvent *) override;

   private:
    /*!
     * \brief adjust the fixed height of the widget to the height of its
     * document, which requires a layout of the document to be up to date
     */
    void updateHeight();
    /*!
     * \brief activate the layouts the widget takes part in, and those above
     * them, so that they adopt the new height of the content
     *
     * \c QWidget::setFixedHeight updates the geometry of the widget but leaves
     * the size hint the enclosing layouts computed before, and hence the height
     * of the widgets they are set on: those layouts shall be activated again
     */
    void propagateHeightToLayout();
    /*!
     * \brief emit `textChanged` and update the height after a modification
     * of the document
     */
    void onContentsChange();
    /*!
     * \return true if the validator does not reject the text obtained by
     * replacing the selection with the given text
     * \param[in] t: text about to be inserted
     */
    bool acceptsInsert(const QString &) const;
    /*!
     * \return the height of the widget when its content is a single line, which
     * is the smallest height it can take
     */
    int singleLineHeight() const;
    /*!
     * \return the window whose size bounds the height of this widget, null when
     * there is none
     *
     * A widget without a parent, and a widget which is itself the window, have
     * no window to measure themselves against. A dialog is left out too: it is
     * sized by its content, so a fraction of it is a fraction of what the
     * content asks for, which bounds nothing.
     */
    QWidget *boundedWindow() const;
    /*!
     * \return the greatest height the content of the widget may take: never
     * more than \c max_lines lines, nor more than a quarter of the window the
     * widget belongs to, never less than a single line
     */
    int maximumContentHeight() const;
    /*!
     * \brief watch the size of the window this widget bounds itself to, instead
     * of the one watched until now
     */
    void watchWindow();

    //! validator constraining the text, not owned
    const QValidator *v_ = nullptr;
    //! height reported by `sizeHint`, the fixed height of the content
    int contentHeight_ = -1;
    //! window whose size bounds the height of the widget, see \c boundedWindow
    QPointer<QWidget> window_ = nullptr;
    /*!
     * \brief ask the event loop to propagate the height of the content to the
     * enclosing layouts, unless it was already asked for
     *
     * \note the request is an event posted to this widget, which the event loop
     * drops when the widget is destroyed, as the editor does for a user input
     * as soon as it got an answer
     */
    void schedulePropagation();
    //! true while a deferred propagation of the height of the content to the
    //! enclosing layouts is pending: the propagation can not run while the
    //! widget is being resized and is deferred to the event loop, where it is
    //! retried until the chain of layouts has settled on the new height
    bool pendingPropagation_ = false;
    //! height a dialog hosting this widget was last given by the propagation,
    //! which tells a dialog that shrank back from one somebody resized
    //! meanwhile
    int propagatedDialogHeight_ = -1;
    //! smallest height seen for the dialog hosting this widget while this
    //! widget was a single line tall, which is the height the dialog was opened
    //! with, or a smaller one its user gave it. -1 while nothing was seen yet
    int restingDialogHeight_ = -1;
    //! number of times the propagation is still allowed to be asked for
    //! without the height of the widget changing, which stops a propagation
    //! that achieves nothing from being posted over and over
    int propagationTries_ = 0;
    //! maximum number of lines displayed before a scroll bar takes over
    static constexpr int max_lines = 10;

    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MULTILINEEDIT_HXX */
