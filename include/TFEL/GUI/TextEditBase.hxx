/*!
 * \file  TextEditBase.hxx
 * \brief base class of the editable text areas of the interface.
 * \author Thomas Helfer
 * \date   19/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITBASE_HXX
#define LIB_TFEL_GUI_TEXTEDITBASE_HXX

#include <functional>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QTextCursor>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/TextEditKeyPressEventFilter.hxx"

namespace tfel::gui {

  //! forward declaration
  struct EditorWidget;
  //! forward declaration
  struct Buffer;
  //! forward declaration
  struct MajorMode;

  /*!
   * \brief base class of the editable text areas of the interface
   *
   * `PlainTextEdit`, `TextEdit` and `TextBrowser` derive from this
   * class. It implements everything those widgets have in common: the
   * key bindings (Emacs-like or Qt-like, see `ShortCutStyle`), the
   * major mode handling, the file saving, the search, the completion
   * and the context menu. The editing primitives themselves are pure
   * virtual: the derived classes implement them as thin layers over
   * the Qt widget returned by `widget()`.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextEditBase : public QWidget {
   signals:

    /*!
     * \brief emitted when the buffer starts visiting a file
     * \param[in] f : absolute path of the visited file
     *
     * \note emitted by `setFileName` only; connected by `Buffer` to
     * refresh the buffer name
     */
    void fileNameChanged(const QString &);

    //! \brief emitted when a new major mode has been set
    void majorModeChanged();

    /*!
     * \brief emitted when the specific menu of the major mode changes
     *
     * \note this signal is currently never emitted nor connected
     */
    void specificMenuChanged();

    /*!
     * \brief emitted when the text cursor moved or its selection
     * changed
     *
     * \note emitted by `emitCursorPositionChanged`, which
     * `initialize` connects to the cursor movement signal of the
     * underlying Qt widget
     */
    void cursorPositionChanged();

   public slots:

    /*!
     * \brief format the region containing the cursor
     *
     * \note delegates to `MajorMode::format`; does nothing when no
     * major mode is set. The Alt-Q binding performs the same call on
     * the mode directly and clears the selection afterwards
     */
    virtual void format();

    /*!
     * \brief pop up a print dialog and print the document if the user
     * accepts
     *
     * \note triggered by Ctrl-X P and by the main window menu
     */
    virtual void print() = 0;

    //! \brief delete the whole content of the buffer
    virtual void clear() = 0;

    /*!
     * \brief undo the last change of the document, if the undo/redo
     * mechanism is enabled and the undo stack is not empty
     */
    virtual void undo() = 0;

    /*!
     * \brief redo the last change undone in the document, if the
     * undo/redo mechanism is enabled and the redo stack is not empty
     */
    virtual void redo() = 0;

    //! \brief move the selected text to the clipboard
    virtual void cut() = 0;

    //! \brief copy the selected text to the clipboard
    virtual void copy() = 0;

    /*!
     * \brief insert the clipboard content at the cursor, replacing the
     * selection if any
     */
    virtual void paste() = 0;

    //! \brief select the whole content of the buffer
    virtual void selectAll() = 0;

    /*!
     * \brief replace the whole content of the buffer by the given
     * rich text
     *
     * \note the overrides implemented in this tree clear the buffer
     * and call `insertHtml`
     */
    virtual void setHtml(const QString &) = 0;

    /*!
     * \brief insert the given rich text at the cursor, replacing the
     * selection if any
     *
     * \note the overrides insert the text through a copy of the text
     * cursor, so the cursor shown by the widget is left untouched
     */
    virtual void insertHtml(const QString &) = 0;

    /*!
     * \brief append the given rich text to the buffer
     *
     * \note this does not move the cursor shown by the widget
     */
    virtual void appendHtml(const QString &) = 0;

    /*!
     * \brief replace the whole content of the buffer by the given text
     *
     * \note the constructors which load a file call it and then
     * explicitly move the cursor to the start of the document
     */
    virtual void setPlainText(const QString &) = 0;

    /*!
     * \brief insert the given text at the cursor, replacing the
     * selection if any
     *
     * \note this moves the cursor shown by the widget past the
     * inserted text
     */
    virtual void insertPlainText(const QString &) = 0;

    /*!
     * \brief append the given text to the buffer
     *
     * \note `PlainTextEdit` appends it as a new paragraph; the other
     * overrides append at the end of the document through a copy of
     * the text cursor, leaving the cursor shown by the widget
     * untouched
     */
    virtual void appendPlainText(const QString &) = 0;

    /*!
     * \brief make the buffer read-only (true) or editable (false)
     *
     * \note many key bindings display "read only buffer" in the
     * minibuffer instead of modifying a read-only buffer
     */
    virtual void setReadOnly(const bool) = 0;

    /*!
     * \brief forward the given text interaction flags to the
     * underlying Qt widget
     */
    virtual void setTextInteractionFlags(Qt::TextInteractionFlags) = 0;

    /*!
     * \brief move the text cursor shown by the editor to the position
     * and selection held by the argument
     *
     * \note this is how the methods which operate on a copy returned
     * by `textCursor` make their changes visible
     */
    virtual void setTextCursor(const QTextCursor &) = 0;

    /*!
     * \brief scroll the view so that the cursor is at its vertical
     * center
     *
     * \note the override implemented by `TextEdit` does nothing
     */
    virtual void centerCursor() = 0;

    //! \brief enable or disable the undo/redo mechanism of the buffer
    virtual void setUndoRedoEnabled(const bool) = 0;

    /*!
     * \brief set the font of the underlying Qt widget
     *
     * \note `initialize` calls it with the font saved in the
     * "font/textedit" setting, when present
     */
    virtual void setFont(const QFont &);
    /*!
     * \brief create and install the major mode whose name is `n`
     * \param[in] n : mode name
     *
     * \return the installed mode
     *
     * \note the mode is created by `MajorModeFactory` and installed
     * by `setMajorMode(MajorMode *const)`
     */
    virtual MajorMode *setMajorMode(const QString &);

    /*!
     * \brief create and install the major mode which the
     * `MajorModeFactory` associates with the file visited by the
     * buffer
     *
     * \return the installed mode
     *
     * \note the mode is guessed from the file name returned by
     * `getFileName` and installed by
     * `setMajorMode(MajorMode *const)`
     */
    virtual MajorMode *setMajorMode();

    /*!
     * \brief install the given major mode
     * \param[in] m : mode to install; this object takes ownership of
     * it and deletes the previous mode with `deleteLater`
     *
     * \note does nothing if `m` is null; if the change is not allowed
     * (`allowMajorModeChange` is false), a message is displayed in
     * the minibuffer and the mode is neither installed nor deleted
     *
     * \note when the change is accepted, a message is displayed in
     * the minibuffer, the spell-check language and the syntax
     * highlighter are propagated to the mode and `majorModeChanged`
     * is emitted
     */
    virtual void setMajorMode(MajorMode *const);

    /*!
     * \brief set the file visited by the buffer
     * \param[in] f : file name; it is stored as an absolute path
     *
     * \note if no major mode is set yet, one is guessed from the file
     * name and `fileNameChanged` is emitted; this method neither
     * reads nor writes the file and does not move the cursor
     */
    virtual void setFileName(const QString &);

    /*!
     * \brief ask the user for a file name in the minibuffer and write
     * the buffer to that file
     *
     * \note a `WriteFile` user input, pre-filled with the directory of
     * the visited file or the current directory, is installed with
     * `EditorWidget::setUserInput`
     */
    virtual void writeFile();

    /*!
     * \brief write the buffer to the given file
     * \param[in] f : file name
     *
     * \note the document is marked as modified beforehand so that
     * `save` does not skip the write; if the file exists and is not
     * writable, a message is displayed in the minibuffer but the save
     * is attempted anyway
     */
    virtual void writeFile(const QString &);

    /*!
     * \brief save the buffer in the file it visits, as plain text
     *
     * \note does nothing, apart from a message in the minibuffer, if
     * the document has not been modified, and falls back to
     * `writeFile` if the buffer visits no file; the modified flag is
     * reset on success and the outcome is displayed in the minibuffer
     */
    virtual void save();

    /*!
     * \brief move the cursor to the given line and center the view on
     * it
     * \param[in] l : line number; the first line is 1 (0 is handled
     * like 1) and negative values count back from the last line
     *
     * \note the motion uses `moveMode`, so it can extend the
     * selection
     */
    virtual void gotoLine(const int);
    /*!
     * \brief move the cursor to the given position and center the
     * view on it
     * \param[in] l: line number
     * \param[in] c: column number; the first column is 1, values
     * larger than the line length are clamped and non-positive values
     * count back from the end of the line
     *
     * \note the line is a text block of the document, not a displayed
     * line: the column is not affected by the word wrapping policy
     *
     * \note this moves the text cursor shown by the editor
     */
    virtual void gotoPosition(const int, const int);

    /*!
     * \brief insert the selected completion at the cursor
     * \param[in] completion : the text to insert
     *
     * \note delegates to `MajorMode::completeCurrentWord` and does
     * nothing when no major mode is set; triggered by Tab or Backtab
     * while the completion popup is visible
     */
    virtual void insertCompletion(const QString &);

    /*!
     * \brief set the move mode used by the cursor motions driven by
     * the key bindings
     * \param[in] m : move mode
     *
     * \note Ctrl-Space (set mark) switches it to
     * `QTextCursor::KeepAnchor` and most editing commands restore
     * `QTextCursor::MoveAnchor`
     */
    virtual void setMoveMode(QTextCursor::MoveMode);

   public:
    //! return the underlying editor widget
    EditorWidget &getEditorWidget();
    //! return the underlying editor widget
    const EditorWidget &getEditorWidget() const;

    /*!
     * \brief a `CommandLine` asking the user whether to save the
     * current buffer
     */
    struct SaveInput : public YesOrNoUserInput {
      //! \brief the input is blocking: the editor waits for the answer
      bool isBlocking() const override;

      /*!
       * \brief save the buffer if the user answered 'y', do nothing
       * otherwise
       */
      void treatUserInput() override;

     protected:
      friend struct TextEditBase;

      /*!
       * \brief build the "save current buffer" question
       * \param[in] w : editor widget the input is displayed in
       * \param[in] p : text edit which would be saved
       *
       * \note created through `TextEditBase::getSaveInput` only
       */
      SaveInput(EditorWidget &, TextEditBase &);

      //! text edit asked about
      TextEditBase &textEdit;
    };  // end of struct SaveInput

    /*!
     * "kill other buffer and write file" user input
     */
    struct KillOtherBufferAndWriteFile;

    /*!
     * "overwrite file" user input
     */
    struct OverWriteFile;

    /*!
     * "write file" user input
     */
    struct WriteFile;

    /*!
     * \brief build the base class: the widget becomes a child of the
     * editor and the spell-check language is initialized with the
     * default language of the `HunspellDictionariesManager`
     * \param[in] g : editor widget this text area belongs to
     * \param[in] b : buffer this text area edits
     */
    TextEditBase(EditorWidget &, Buffer &);

    /*!
     * \brief delete the key press event filter and the context menu
     * actions and schedule the major mode for deletion
     */
    ~TextEditBase() override;

    /*!
     * \param[in] e : event
     * \brief convert a mouse move event to the mouseMoved signal.
     * \note this function is only called if mouse tracking is
     * enabled; the implementation is currently empty, so mouse move
     * events are simply consumed
     */
    void mouseMoveEvent(QMouseEvent *) override;

    /*!
     * \return the underlying Qt scroll area, that is the
     * `QPlainTextEdit`, `QTextEdit` or `QTextBrowser` this class
     * drives
     *
     * \note it is a child of this widget, created and owned by the
     * derived class
     */
    virtual QAbstractScrollArea *widget() = 0;

    /*!
     * \return true if the buffer is a main frame
     */
    virtual bool isMainFrame() const;

    /*!
     * \brief set the language used for spell checking
     * \param[in] l : language name
     *
     * \note the language is also propagated to the major mode, if
     * any; the LaTeX and Markdown modes use it to underline the
     * misspelled words
     */
    virtual void setSpellCheckLanguage(const QString &);

    /*!
     * \return the language used for spell checking of this frame
     */
    QString getSpellCheckLanguage() const;

    /*!
     * \return true if the buffer is not a main frame
     */
    virtual bool isSecondaryTask() const;

    //! \brief give the focus to the underlying Qt widget
    virtual void setFocus();

    /*!
     * \return the name of the file visited by the buffer or an empty
     * string if the buffer visits no file; the base class returns the
     * same value as `getCompleteFileName`
     */
    virtual QString getFileName() const;

    /*!
     * \return the absolute path of the file visited by the buffer or
     * an empty string if the buffer visits no file
     */
    virtual QString getCompleteFileName() const;

    /*!
     * \return the absolute path to the directory containing the
     * document being treated.
     */
    virtual QString getDirectory() const;

    /*!
     * \return a newly allocated input asking the user whether the
     * buffer shall be saved
     *
     * \note the caller is in charge of handing it to
     * `EditorWidget::setUserInput`, which takes ownership, typically
     * after connecting to its `finished` signal
     */
    virtual SaveInput *getSaveInput();

    /*!
     * \return the word under cursor
     */
    virtual QString getCurrentWord() const;

    //! \return true if the document holds unsaved modifications
    virtual bool isModified() const;

    //! \return true if a major mode is set
    virtual bool hasMajorMode() const;

    /*!
     * \return the major mode
     *
     * \note throws a `std::runtime_error` if no major mode is set;
     * use `hasMajorMode` to check first
     */
    virtual const MajorMode &getMajorMode() const;

    /*!
     * \return the specific menu of the major mode or an empty vector
     * if there is none
     *
     * \note the menu is owned by the major mode, not by the caller
     */
    virtual std::vector<QMenu *> getSpecificMenus();

    /*!
     * \return the icon of the major mode or a null icon if no major
     * mode is set
     */
    virtual QIcon getIcon() const;

    /*!
     * \brief install a filter which sees the key press events before
     * the Emacs-like key bindings do
     * \param[in] f : filter; this object takes ownership of it on
     * success, the caller must destroy it otherwise
     *
     * \return true if the filter was installed
     *
     * \note returns false, without installing anything, if `f` is
     * null or if the filter reports that it is not ok; returns also
     * false, with a message in the minibuffer, if a filter is already
     * set
     *
     * \note the incremental search and replace inputs install such a
     * filter
     */
    virtual bool setKeyPressEventFilter(TextEditKeyPressEventFilter *const);

    /*!
     * \brief delete the key press event filter, if any
     *
     * \note a mouse press in the text area also removes the filter,
     * which is how an ongoing incremental search or replace ends
     */
    virtual void removeKeyPressEventFilter();

    /*!
     * \return the document edited by the underlying Qt widget; it is
     * owned by that widget
     */
    virtual QTextDocument *document() const = 0;

    /*!
     * \brief move the text cursor shown by the editor
     *
     * \note the default move mode is `QTextCursor::MoveAnchor`; the
     * key bindings pass their own `moveMode` instead to extend the
     * selection while moving
     */
    virtual void moveCursor(
        QTextCursor::MoveOperation,
        QTextCursor::MoveMode = QTextCursor::MoveAnchor) = 0;

    /*!
     * \return true if the underlying Qt widget is read-only
     */
    virtual bool isReadOnly() = 0;

    /*!
     * \brief search the next occurrence of the given string, starting
     * from the current cursor position
     *
     * \return true if a match was found, in which case the match is
     * selected and the text cursor shown by the editor is moved
     *
     * \note the default flags are case sensitive;
     * `QTextDocument::FindBackward` makes the search go backward;
     * used by the incremental search
     */
    virtual bool find(
        const QString &,
        QTextDocument::FindFlags = QTextDocument::FindCaseSensitively) = 0;

    /*!
     * \return a copy of the text cursor shown by the editor
     *
     * \note operations performed on the returned cursor modify the
     * document but not the cursor shown by the editor, until it is
     * handed back to `setTextCursor`
     */
    virtual QTextCursor textCursor() const = 0;

    /*!
     * \brief return the cursor at the given viewport position
     * \param[in] pos : position, relative to the viewport of the
     * underlying Qt widget
     *
     * \return the closest text cursor to that position; used to
     * locate the word under the mouse in the context menu
     */
    virtual QTextCursor cursorForPosition(const QPoint &pos) const = 0;

    /*!
     * \brief return the rectangle of the viewport where the given
     * cursor is drawn
     * \param[in] cursor : cursor to locate
     *
     * \return its bounding rectangle, used to place the completion
     * popup
     */
    virtual QRect cursorRect(const QTextCursor &cursor) const = 0;

    /*!
     * \return the rectangle of the viewport where the text cursor
     * shown by the editor is drawn, used to place the completion
     * popup
     */
    virtual QRect cursorRect() const = 0;

    /*!
     * \brief replace the whole list of extra selections of the
     * underlying Qt widget
     *
     * \note the base class uses the extra selections to highlight the
     * current line (see `highlightCurrentLine`)
     */
    virtual void setExtraSelections(
        const QList<QTextEdit::ExtraSelection> &) = 0;

    /*!
     * \return the current list of extra selections of the underlying
     * Qt widget
     */
    virtual QList<QTextEdit::ExtraSelection> extraSelections() const = 0;

    //! \return true if the undo/redo mechanism is enabled
    virtual bool isUndoRedoEnabled() const = 0;

   protected slots:

    /*!
     * \brief highlight the current line with a translucent yellow,
     * full-width background
     *
     * \note connected to the cursor movement signal of the underlying
     * Qt widget by `initialize`; it replaces the whole list of extra
     * selections
     */
    virtual void highlightCurrentLine();

    /*!
     * \brief reset the `filter` pointer to null
     *
     * \note connected to the `destroyed` signal of the key press
     * event filter, so that a filter destroyed by its own owner is
     * not used any more
     */
    virtual void keyPressEventFilterDestroyed();

    /*!
     * \brief re-emit the cursor movement signal of the underlying Qt
     * widget as `cursorPositionChanged`
     *
     * \note connected to the cursor movement signal by `initialize`
     */
    virtual void emitCursorPositionChanged();

   protected:
    /*!
     * a editor user input which asks
     * the user where he wants to go
     */
    struct GotoLine : public CommandLine {
      /*!
       * \brief build the "go to line :" minibuffer input, which only
       * accepts integers
       * \param[in] t : text edit whose cursor will be moved
       * \param[in] p : editor widget the input is displayed in
       *
       * \note Alt-G installs such an input
       */
      GotoLine(TextEditBase &, EditorWidget &);

      //! default destructor
      ~GotoLine() override;

     protected:
      /*!
       * \brief go to the line entered by the user; the input is
       * ignored if it is not a valid integer
       */
      void treatUserInput() override;

      //! text edit whose cursor is moved
      TextEditBase &textEdit;

    };  // end of struct GotoLine

    /*!
     * \brief describe the modifier keys of a key event
     * \param[in] e : key event
     *
     * \return a string such as "Ctrl", "Alt-Shift" or
     * "Ctrl-Alt-Shift-Meta", empty if no modifier is pressed
     *
     * \note used to describe unknown Ctrl-X and Ctrl-C shortcuts in
     * the minibuffer
     */
    static QString getModifier(const QKeyEvent &);

    /*!
     * \brief handle the events sent to this widget itself
     *
     * \note Tab key press events are routed to `handleKeyPressEvent`
     * and accepted, all the other events are handled by
     * `QWidget::event`
     *
     * \return true if the event was consumed
     */
    bool event(QEvent *) override;

    /*!
     * \brief filter which `initialize` installs on the underlying Qt
     * widget and on its viewport: key press events on the widget are
     * routed to `handleKeyPressEvent` and mouse presses on the
     * viewport to `handleMousePressEvent`
     *
     * \return true if the event was consumed; the other events are
     * passed to `QObject::eventFilter`
     */
    bool eventFilter(QObject *, QEvent *) override;

    /*!
     * \brief handle a key press event which reached this widget
     * instead of the underlying Qt widget
     *
     * \note this happens for the events redirected by the completion
     * popup; the `keyProcessing` flag guards the call to
     * `handleKeyPressEvent` so that an event ignored by the
     * underlying widget, which propagates back to its parent, cannot
     * loop infinitely
     */
    void keyPressEvent(QKeyEvent *) override;

    /*!
     * \brief dispatch a key press event to
     * `handleKeyPressEventWithQtShortCuts` or
     * `handleKeyPressEventWithEmacsShortCuts`, according to the style
     * selected in the `ShortCutStyle` settings
     *
     * \return true if the event was handled, which both handlers
     * essentially always do
     */
    virtual bool handleKeyPressEvent(QKeyEvent *);

    /*!
     * \brief handle a key press event using the Emacs-like key
     * bindings
     *
     * \note the key press event filter and `MajorMode::keyPressEvent`
     * see the event first; Ctrl-X and Ctrl-C are prefix keys which
     * arm the `ctrlx` and `ctrlc` states; the remaining bindings
     * implement the cursor motions, the kill/yank commands, the
     * indentation (Tab), the formatting (Alt-Q), the commenting
     * (Alt-;), the incremental search (Ctrl-S/Ctrl-R), the completion
     * and, as a fall-back, the transmission of the event to the
     * underlying Qt widget for self-insertion
     *
     * \return true: the event is always consumed
     */
    virtual bool handleKeyPressEventWithEmacsShortCuts(QKeyEvent *);

    /*!
     * \brief handle a key press event using the Qt-like key bindings
     *
     * \note Tab and Backtab validate the completion popup when it is
     * visible; otherwise the event is sent to the underlying Qt
     * widget, with the event filter temporarily disabled, and the
     * completion popup is updated or hidden
     *
     * \return true: the event is always consumed
     */
    virtual bool handleKeyPressEventWithQtShortCuts(QKeyEvent *);

    /*!
     * \brief handle a mouse press on the viewport of the underlying
     * Qt widget
     *
     * \note any key press event filter is removed first, which stops
     * an ongoing incremental search or replace; the major mode is
     * then given the chance to handle the event and, for the right
     * button, the context menu is built from the context menu actions
     * and `MajorMode::completeContextMenu` and executed
     *
     * \return true if the event was consumed (a right button press or
     * a mode which handled the event), false otherwise
     */
    virtual bool handleMousePressEvent(QMouseEvent *);

    /*!
     * \brief initialize this base class, to be called by the
     * constructor of the derived class once the Qt widget is created
     * \param[in] e : the Qt widget driven by this class
     *
     * \note the context menu action pointers are reset, the minimum
     * height of the widget is set, the saved font is applied, the
     * cursor movement signal is connected to `highlightCurrentLine`
     * and `emitCursorPositionChanged`, and this object is installed as
     * an event filter on the widget and on its viewport
     */
    virtual void initialize(QAbstractScrollArea *const);

    /*!
     * \brief push a killed text on the kill ring of the editor
     * \param[in] t : killed text
     *
     * \note forwards to `EditorWidget::addToKillRing`; used by the
     * kill commands (Ctrl-W, Ctrl-K, Alt-D, Alt-W)
     */
    virtual void addToKillRing(const QString &);

    /*!
     * \brief (re)create the undo, redo, cut, copy, paste and select
     * all actions used by the context menu
     *
     * \note the previous actions are deleted first; called on each
     * right click by `handleMousePressEvent`
     */
    virtual void createContextMenuActions();

    /*!
     * \brief delete the context menu actions and reset the
     * corresponding members to null
     */
    virtual void deleteContextMenuActions();

    //! reference to the widget
    EditorWidget &editor;

    //! buffer edited by this text area
    Buffer &buffer;

    /*!
     * \brief major mode
     *
     * \note null when no major mode is set; this object owns it and
     * deletes it with `deleteLater` on replacement or destruction
     */
    MajorMode *mode = nullptr;

    /*!
     * \brief key press filter
     *
     * \note null when no filter is set; this object owns the filter
     * installed by `setKeyPressEventFilter`
     */
    TextEditKeyPressEventFilter *filter = nullptr;

    /*!
     * \brief move mode used by the cursor motions driven by the key
     * bindings: the motions extend the selection when it is
     * `QTextCursor::KeepAnchor`
     */
    QTextCursor::MoveMode moveMode = QTextCursor::MoveAnchor;

    /*!
     * \brief absolute path of the file visited by the buffer; empty
     * when the buffer visits no file
     */
    QString fileName;

    /*!
     * \brief language used by the spell checker; initialized with the
     * default language of the `HunspellDictionariesManager`
     */
    QString spellCheckLanguage;

    //! undo action of the context menu
    QAction *ua = nullptr;
    //! redo action of the context menu
    QAction *ra = nullptr;
    //! cut action of the context menu
    QAction *ca = nullptr;
    //! copy action of the context menu
    QAction *coa = nullptr;
    //! paste action of the context menu
    QAction *pa = nullptr;
    //! select all action of the context menu
    QAction *sa = nullptr;

    /*!
     * \brief position in the kill ring of the text inserted by the
     * last yank
     *
     * \note set by Ctrl-Y and cycled backward by Alt-Y (yank-pop);
     * undefined before the first yank
     */
    int pring;

    /*!
     * \brief state flag: true when the last command was a yank
     * (Ctrl-Y), which is the precondition of yank-pop (Alt-Y)
     */
    bool yank = false;

    /*!
     * \brief state flag: true while the Ctrl-X prefix key has been
     * typed and the second key of the shortcut is awaited
     */
    bool ctrlx = false;

    /*!
     * \brief state flag: true while the Ctrl-C prefix key has been
     * typed and the second key of the shortcut is awaited
     */
    bool ctrlc = false;

    /*!
     * \brief when false, `setMajorMode` refuses to change the major
     * mode; set to false by `PlainTextEdit` when the buffer visits a
     * directory, whose `DirectoryMajorMode` shall not be replaced
     */
    bool allowMajorModeChange = true;

   private:
    //! `Buffer` is a friend: it alone calls `setMainFrame`
    friend struct Buffer;

    /*!
     * \brief mark this text area as the main frame of the buffer
     *
     * \note called by `Buffer` only; the secondary tasks of a buffer
     * are the text areas which are not the main frame
     */
    virtual void setMainFrame(const bool);

    //! true if this text area is the main frame of its buffer
    bool mainFrame = false;

    /*!
     * \brief re-entrance guard used by `keyPressEvent` to prevent an
     * infinite loop when the underlying widget ignores an event and
     * lets it propagate back to its parent
     */
    bool keyProcessing = false;

    Q_OBJECT

  };  // end of struct TextEditBase

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITBASE_HXX */
