/*!
 * \file  MajorModeBase.hxx
 * \brief
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

    void update() override;

   public:
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
     */
    int getMinimalCompletionLength() override;

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
     */
    bool keyPressEvent(QKeyEvent* const) override;
    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default behaviour
     */
    bool mousePressEvent(QMouseEvent* const) override;
    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by
     * TextEditBase where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;
    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    void completeContextMenu(QMenu* const, const QTextCursor&) override;
    //! \brief indent the whole text
    void indentAll() override;
    /*!
     * indent the current line
     */
    void indentLine(const QTextCursor&) override;
    /*!
     * indent selected region
     */
    void indentRegion(const QTextCursor&) override;

    void setSpellCheckLanguage(const QString&) override;

    void comment() override;

    /*!
     * \brief report a message
     * \param[in] m: message
     */
    virtual void report(const QString&);

    virtual QString getCommentSyntax();
    //! destructor
    ~MajorModeBase() override;

   protected:
    int positionInCurrentBlock(const QTextCursor&) const;
    /*!
     * \return recursively search the given file in the given
     * directory
     * \param[in] d : directory
     * \param[in] f : file name
     * \param[in] m : max depth
     * \param[in] s : current depth
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
     * that b<=e. b and e are positioned at the beginning of their
     * blocks.
     * \param[out] b  : beginning of the selection
     * \param[out] e  : end of the selection
     * \param[in]  tc : text cursor
     */
    void beginAndEndOfSelection(QTextCursor&,
                                QTextCursor&,
                                const QTextCursor&) const;

    /*!
     * an helper function which returns the beginning and the end of
     * the current paragragraph
     */
    void findBeginningAndEndOfTheParagraph(QTextCursor&, QTextCursor&) const;

    QVector<QString> getSelectedLines(const QTextCursor&) const;

    EditorWidget& editor;

    Buffer& buffer;

    TextEditBase& textEdit;

   private:
    Q_OBJECT

  };  // end of struct CppMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAJORMODEBASE_HXX */
