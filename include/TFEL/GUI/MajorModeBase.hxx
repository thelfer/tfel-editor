/*!
 * \file  MajorModeBase.hxx
 * \brief helper class to build major modes.
 * \author Thomas Helfer
 * \date   04/07/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODEBASE_HXX
#define LIB_TFEL_GUI_MAJORMODEBASE_HXX

#include <QtCore/QString>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/MajorMode.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  /*!
   * \brief a helper class to build major modes
   */
  struct TFEL_GUI_VISIBILITY_EXPORT MajorModeBase : public MajorMode {
   public slots:

    /*!
     * \brief update the mode, which is expected to emit \c updatedMenu once its
     * own state is up to date
     *
     * \note the default implementation does nothing else than emitting that
     * signal
     *
     * \note neither the editor nor the modes call this slot, and no receiver
     * is connected to the signal of a mode: the menus of the menu bar are
     * refreshed through \c Buffer::updateMenu instead
     */
    void update() override;

   public:
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on
     * \param[in] p: parent widget. \c MajorModeBase derived classes pass the
     * text editor widget they are built for, as a mode lives as long as the
     * text edit it formats
     *
     * \note the editor, the buffer and the text edit are kept as references:
     * all three outlive the mode, which does not own them
     */
    MajorModeBase(EditorWidget&, Buffer&, TextEditBase&, QWidget* const);

    /*!
     * default implementation of the getCompleter method
     * \return a NULL pointer
     */
    QCompleter* getCompleter() override;
    /*!
     * \return this->textEdit.getCurrentWord()
     */
    QString getCompletionPrefix() override;
    /*!
     * \return the minimal number of characters for a completion to be
     * considered
     *
     * \note the default implementation returns 3
     */
    int getMinimalCompletionLength() override;

    /*!
     * \brief override which inserts the completion \c w in the word under the
     * cursor of \c t
     * \param[in] t : text edit to complete the word in
     * \param[in] w : completion chosen by the user
     *
     * \note nothing is inserted if the mode has no completer or if that
     * completer was built for another text edit
     *
     * \note the missing part of the completion is inserted at the end of the
     * word, followed by a blank, and the cursor of \c t is moved there
     */
    void completeCurrentWord(TextEditBase&, const QString&) override;
    /*!
     * \brief default implementation of the getSpecificMenu method
     * \return a NULL pointer
     */
    QMenu* getSpecificMenu() override;
    /*!
     * default implementation of the getIcon method
     * \return a default constructed QICon
     */
    QIcon getIcon() const override;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     * \return always false here: this default implementation handles no key
     * and leaves the event to \c TextEditBase
     */
    bool keyPressEvent(QKeyEvent* const) override;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default behaviour
     * \return always false here: this default implementation handles no mouse
     * event
     */
    bool mousePressEvent(QMouseEvent* const) override;
    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
     * TextEditBase where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode. The default
     * implementation handles none and always returns false
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;
    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     *
     * \note the default implementation adds no action to the menu
     */
    void completeContextMenu(QMenu* const, const QTextCursor&) override;
    /*!
     * \brief indent the whole text
     *
     * \note the whole document is selected and handed over to
     * \c indentRegion, which this default implementation leaves empty
     */
    void indentAll() override;
    /*!
     * indent the current line
     *
     * \note a tabulation is inserted at the beginning of the line, whatever
     * its current indentation. The caller's cursor is left untouched: it is
     * the text edit's own cursor which would follow the inserted text
     */
    void indentLine(const QTextCursor&) override;
    /*!
     * indent selected region
     *
     * \note the default implementation does nothing: modes which know how to
     * indent their text, such as \c CMajorModeBase, override this method
     */
    void indentRegion(const QTextCursor&) override;

    /*!
     * \brief set the language the spell checker of the mode uses
     * \param[in] l : language name
     *
     * \note the default implementation ignores the language and does nothing
     */
    void setSpellCheckLanguage(const QString&) override;

    /*!
     * \brief comment or uncomment the selection with the syntax returned by
     * \c getCommentSyntax
     *
     * With a selection, the lines it covers are commented unless all of them
     * already start with the comment syntax, in which case they are
     * uncommented. Without a selection, the comment syntax is inserted at the
     * cursor and the line is indented.
     *
     * \note the editor's cursor is moved to the end of the commented region
     *
     * \note the selection is handled on a line basis and ignores anything the
     * comment syntax may mean inside a string or a block comment; modes whose
     * language requires more care, such as \c CMajorModeBase, override this
     * method
     */
    void comment() override;

    /*!
     * \brief report a message in the minibuffer
     * \param[in] m: message
     *
     * \note the message is displayed through
     * \c EditorWidget::displayInformativeMessage and disappears by itself
     */
    virtual void report(const QString&);

    /*!
     * \return the string which starts a comment line in the language of the
     * mode, an empty string when the mode has no comment syntax
     *
     * \note an empty string makes \c comment do nothing but report that no
     * comment syntax is defined
     */
    virtual QString getCommentSyntax();
    //! destructor
    ~MajorModeBase() override;

   protected:
    /*!
     * \return the position of the given cursor in its own block, that is its
     * column in the line it stands on
     * \param[in] tc : text cursor
     */
    int positionInCurrentBlock(const QTextCursor&) const;
    /*!
     * \return recursively search the given file in the given
     * directory
     * \param[in] d : directory
     * \param[in] f : file name
     * \param[in] m : max depth
     * \param[in] s : current depth
     *
     * \note the returned paths are the full paths of the files found, relative
     * to \c d. The list is empty when \c d does not exist, when nothing
     * matches or when the depth bounds are not valid
     */
    QStringList findFiles(const QString&,
                          const QString&,
                          const int = 10,
                          const int = 0);
    /*!
     * indent the line under the given text cursor by i spaces. This
     * ensure the cursor remains correctly positionned.
     * \param[in] tc : text cursor
     * \param[in] i  : indentation
     * \param[in] updateEditorCursor : move the editor's cursor to the
     * indented line. Shall be false when indenting multiple lines: calling
     * `TextEditBase::setTextCursor` for each line forces the editor to
     * relayout the viewport in the middle of a large pending edit, which
     * can crash Qt's text layout (QTextLine::naturalTextRect). The caller
     * is then responsible for setting the editor's cursor once the whole
     * region has been indented.
     */
    void doIndentLine(const QTextCursor&,
                      const int,
                      const bool updateEditorCursor = true) const;
    /*!
     * a description of the cursor position which is not affected by
     * the indentation of the lines: the cursor is described by its
     * offset to the first non blank character of its line, which is
     * preserved by indentation. If the cursor is in the blanks at the
     * beginning of the line (negative offset), it is restored on the
     * first non blank character of the line (as `doIndentLine` does).
     */
    struct CursorMark {
      //! number of the block containing the cursor
      int blockNumber;
      //! column of the cursor in its block
      int column;
      //! column of the first non blank character of the block
      int firstNonBlankColumn;
    };
    /*!
     * memorize the position of the given cursor, so that it can be
     * restored once the region has been indented
     * \param[in] tc : text cursor
     */
    CursorMark getCursorMark(const QTextCursor&) const;
    /*!
     * restore the editor's cursor from a cursor mark. The block
     * containing the mark is expected to be at the same position of
     * the document than when the mark was taken: indenting a region
     * shall not add nor remove lines.
     * \param[in] m : cursor mark
     */
    void restoreCursorMark(const CursorMark&) const;
    /*!
     * a description of the cursor position which survives a reflow of the
     * region containing the cursor. Reformatting a region (see
     * `MajorMode::format`) moves its words from one line to another, adds
     * and removes blanks and may change the number of lines: neither a plain
     * offset nor a line and a column can be reused. The words themselves are
     * left untouched, though, so the cursor is described by the number of non
     * blank characters which precede it in the region and by the side of the
     * adjacent character it stands on.
     */
    struct AnchorMark {
      //! number of non blank characters which precede the cursor
      int nonBlanks;
      //! the cursor stands on the left of a non blank character, either because
      //! it is in the middle of a word or because it was in the blanks at the
      //! beginning of its line
      bool leftOfWord;
    };
    /*!
     * memorize the position of the given cursor in the region which starts at
     * `b` and ends at `c`, so that it can be restored once this region has
     * been reformatted
     * \param[in] b : beginning of the region. Shall not be after `c`.
     * \param[in] c : cursor, and end of the region
     */
    AnchorMark getAnchorMark(const QTextCursor& b, const QTextCursor& c) const;
    /*!
     * restore the editor's cursor from an anchor mark. `[first,last]` shall be
     * the region the mark was taken in, once reformatted: the cursor is placed
     * on the left of the (`nonBlanks`+1)-th non blank character of this region,
     * or right after its `nonBlanks`-th non blank character when the cursor did
     * not stand on the left of a non blank character. A mark which does not fit
     * in the region anymore, because words have been dropped, is restored on
     * the right of the last non blank character of the region. As in
     * `doIndentLine`, a cursor which was in the blanks at the beginning of the
     * region is attached to its first non blank character. \param[in] m :
     * anchor mark \param[in] first : position of the beginning of the
     * reformatted region \param[in] last : position of the end of the
     * reformatted region
     *
     * \note The bounds are positions and not text cursors: a cursor standing at
     * the beginning of a region which is rewritten in place is moved to the end
     * of the inserted text, so it cannot delimit the reformatted region
     * anymore.
     */
    void restoreAnchorMark(const AnchorMark& m,
                           const int first,
                           const int last) const;
    /*!
     * an helper function which returns the beginning b and the end e
     * of the selection of the cursor tc. This function guarantees
     * that b<=e. b and e are cursors without any selection, placed
     * where the given cursor selects, that is not at the beginning
     * of their blocks: it is up to the caller to extend them if it
     * needs whole lines.
     * \param[out] b  : beginning of the selection
     * \param[out] e  : end of the selection
     * \param[in]  tc : text cursor
     *
     * \note when \c tc holds no selection, both b and e are placed at the
     * position of that cursor
     */
    void beginAndEndOfSelection(QTextCursor&,
                                QTextCursor&,
                                const QTextCursor&) const;

    /*!
     * an helper function which returns the beginning and the end of
     * the current paragragraph, that is the set of consecutive non
     * blank lines which the editor's cursor belongs to. Both cursors
     * are taken from the editor's cursor: b stands at the beginning
     * of the first line of the paragraph, e at the end of its last
     * line.
     * \param[out] b : beginning of the paragraph
     * \param[out] e : end of the paragraph
     *
     * \note the editor's cursor is left untouched
     *
     * \note a cursor which stands on a blank line gives a paragraph made of
     * that blank line only
     *
     * \note when the paragraph reaches the end of the document, e stops on the
     * last non blank line: stopping on the blank line a file ending with a
     * line break has would make the callers consume that last line break
     */
    void findBeginningAndEndOfTheParagraph(QTextCursor&, QTextCursor&) const;

    /*!
     * \return the lines covered by the selection of the given cursor, each
     * one of them being truncated to the part which is selected
     * \param[in] tc : text cursor
     *
     * \note the returned vector is empty when the cursor holds no selection
     */
    QVector<QString> getSelectedLines(const QTextCursor&) const;

    //! editor widget the mode belongs to
    EditorWidget& editor;

    //! buffer the mode is attached to
    Buffer& buffer;

    //! text editor widget the mode acts on
    TextEditBase& textEdit;

   private:
    Q_OBJECT

  };  // end of struct CppMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAJORMODEBASE_HXX */
