/*!
 * \file  LaTeXMajorMode.hxx
 * \brief LaTeXMajorMode: major mode for latex files, runs latex, spell check.
 * \author Thomas Helfer
 * \date   18/08/2012
 */

#ifndef LIB_TFEL_GUI_LATEXMAJORMODE_HXX
#define LIB_TFEL_GUI_LATEXMAJORMODE_HXX

#include <QtCore/QPointer>
#include <QtCore/QRegularExpression>
#include <TFEL/GUI/SpellChecker.hxx>
#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  //! forward declaration
  struct TextEditBase;
  //! forward declaration
  struct GrammarCheckSession;
  //! forward declaration
  struct ProcessOutputFrame;
  //! forward declaration
  struct LaTeXSyntaxHighlighter;

  //! \brief major mode for editing LaTeX documents
  struct LaTeXMajorMode : public MajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor the mode acts on, which is also its parent
     * widget
     *
     * \note the regular expressions the indentation and the reformatting
     * need are built here
     *
     * \note the TeX master file announced by the first lines of the buffer
     * is looked up once, at construction, and printed with \c qDebug
     */
    LaTeXMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "LaTeX"
    QString getName() const override;

    //! \return "major mode dedicated to the LaTeX"
    QString getDescription() const override;

    /*!
     * \brief install a `LaTeXSyntaxHighlighter` on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter refers to this mode, so the mode must outlive
     * it; the previous highlighter is released by \c setHighlighter
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \brief complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     *
     * The suggestions of the spell checker are inserted in front of the
     * menu when the cursor stands on a misspelled word.
     *
     * \note only the words of a portion of the line which is neither a
     * comment nor a LaTeX command may be misspelled: the very same split of
     * the line is used here and in the highlighter, by
     * \c LaTeXSyntaxHighlighter::parseLine, so that the menu and the
     * underlining always agree
     *
     * \note nothing is added when the mode has no syntax highlighter yet
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \brief insert the completion the user chose in the word under the
     * cursor of the given text edit
     * \param[in] t : text edit the word is completed in
     * \param[in] w : completion chosen by the user
     *
     * \note this override only calls the default implementation, which
     * leaves the text untouched as long as the mode has no completer, which
     * is the case of this mode
     */
    void completeCurrentWord(TextEditBase &, const QString &) override;

    /*!
     * \return a menu specific to this mode
     *
     * \note a null pointer: the mode adds no menu of its own to the menu
     * bar
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief handle the key events before the text edit does
     * \param[in] e : key event
     * \return true when "Alt-m" is pressed, in which case a `LaTeX` run is
     * started
     *
     * \note any other key is left to \c TextEditBase
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief handle the mouse button presses before the text edit does
     * \param[in] e : mouse event, which this implementation ignores
     * \return always false: this mode handles no mouse event
     */
    bool mousePressEvent(QMouseEvent *const) override;

    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by TextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     *
     * "Ctrl-C Ctrl-E" is the only shortcut this mode handles: it asks the
     * name of an environment in the minibuffer, through a
     * \c LaTeXInsertBlock, and surrounds the selection or an empty block with
     * the `\begin` and `\end` commands of that environment.
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;
    /*!
     * indent the current line
     *
     * The line is indented by two blanks per indentation level, the level
     * being that of the `\begin` and `\end` commands which enclose it; see
     * \c getLineIndentation.
     *
     * \note a line which is empty once its comment is removed is left
     * untouched: reindenting shall never write trailing blanks on a blank
     * line
     */
    void indentLine(const QTextCursor &) override;
    /*!
     * indent selected region
     *
     * Every line of the region is indented as \c indentLine does, the
     * indentation level being carried from one line to the next one, and the
     * blank lines are left untouched.
     *
     * \note the region is left untouched when the cursor holds no selection
     *
     * \note the editor's cursor is restored next to the character it stood
     * next to, thanks to \c getCursorMark and \c restoreCursorMark
     */
    void indentRegion(const QTextCursor &) override;

    //! \return the icon of the `LaTeX` language, held by the resources
    QIcon getIcon() const override;

    /*!
     * format the selected paragraph
     *
     * The paragraph the editor's cursor belongs to, that is the set of
     * consecutive non blank lines around it, is rewritten word by word, cut
     * so that no line is longer than 72 columns. The blank lines it holds are
     * restored as such, a trailing comment is kept on a line of its own, the
     * separators such as `\begin{...}`, `\end{...}`, `\item` or `\\` always
     * open a line and the lines are indented as \c indentLine does.
     *
     * \note a paragraph which reaches the end of the document is reformatted
     * too; no blank line is ever introduced, at the beginning nor at the end
     * of the paragraph
     *
     * \note the editor's cursor is kept next to the same character, thanks to
     * \c getAnchorMark and \c restoreAnchorMark: reflowing the paragraph
     * moves its words from one line to another and may change the number of
     * lines, so neither a position nor a line and a column could be reused
     */
    void format() override;

    //! \return "%", which opens a comment in `LaTeX`
    QString getCommentSyntax() override;

    /*!
     * \return the spell checker of this mode, which underlines the misspelled
     * words of the plain text of the document
     *
     * \note the checker is a member of the mode, which owns it; it is used by
     * the syntax highlighter and by the context menu
     */
    SpellChecker &getSpellChecker();

    /*!
     * \brief set the language the spell checker of the mode uses
     * \param[in] l : language name, as the installed dictionaries name it
     *
     * \note the document is rehighlighted so that the words are underlined
     * in the new language; an unknown language disables the spell checking
     */
    void setSpellCheckLanguage(const QString &) override;

    /*!
     * \brief destructor
     *
     * \note the highlighter is released by \c MajorMode, which detaches it
     * from the document the mode highlighted
     */
    ~LaTeXMajorMode() override;

   protected slots:

    /*!
     * \brief run `latex` on the current buffer
     *
     * \note the file which is compiled is that of \c startLaTeX
     *
     * \note the buffer is saved first, through the minibuffer, when it holds
     * unsaved changes; the compilation is then started as soon as the save
     * is over, and never if the user does not save
     */
    virtual void runLaTeX();

    /*!
     * \brief start the `latex` process on the file of the buffer
     *
     * The master file announced by the buffer is used when it holds one, the
     * file of the buffer otherwise. The output of the process is displayed
     * in a secondary task of the buffer titled "*LaTeX* ouput", with the
     * "latex-output" major mode.
     *
     * \note nothing is run and a message is displayed in the minibuffer when
     * the file has no name or does not exist on the disk
     */
    virtual void startLaTeX();

    /*!
     * \brief replace the misspelled word the context menu was completed for
     * by the suggestion the user chose
     * \param[in] a : action of the menu the user triggered
     *
     * The word which lies between \c bmwp and \c emwp is replaced by the data
     * the action holds.
     *
     * \note the slot is triggered by \c QMenu::triggered for any action of
     * that menu: it does nothing when the action is not one of the
     * \c suggestions it offered
     */
    virtual void replaceMispelledWordBySuggestion(QAction *);

   protected:
    /*!
     * \brief the minibuffer input which asks the name of the environment to
     * insert
     *
     * It proposes "figure", "minipage" and "equation" as inline completions.
     * Once the name is entered, the `\begin` and `\end` commands of that
     * environment are written around the selection of the text edit, or
     * around an empty block when it holds no selection; an empty name inserts
     * nothing.
     *
     * \note defined in the implementation file; the editor owns the input
     * once it has been given it as the user input
     */
    struct LaTeXInsertBlock;

    /*!
     * \brief add the suggestions of the spell checker for the misspelled
     * word the cursor is on in front of the given menu
     * \param[out] m   : context menu
     * \param[in]  t   : portion of text
     * \param[in]  pos : position of the cursor relative to the
     * beginning of the portion of text in the line
     * \param[in] apos : abosolution position of the beginning of the
     * portion of text in the line
     *
     * The portion of text is scanned word after word: the word which holds
     * `pos` is looked up in the dictionary and, when it is misspelled, an
     * action per suggestion is inserted in front of the menu, followed by a
     * separator when the menu is not empty.
     *
     * \note the positions of the word are memorised in \c bmwp and \c emwp
     * and the previous suggestions are destroyed; the menu is connected to
     * \c replaceMispelledWordBySuggestion
     *
     * \note nothing is added when the word is spelled well, when it holds no
     * letter or when the dictionary proposes no suggestion
     */
    virtual void completeContextMenuForMispelledWord(QMenu *,
                                                     const QString &,
                                                     const int,
                                                     const int);
    /*!
     * compute indentation of the current line
     * \param[in]  tc : cursor of the line to compute the indentation of
     * \param[out] i  : indentation level of that line, that is the sum of
     * the increments of all the preceding lines (0 if first line)
     * \param[out] di : indentation increment of the given line
     * \param[out] b  : true if the line does not close an environment, so
     * that its own increment shall not be used to indent it
     *
     * \c indentLine and \c indentRegion indent the line by `2 * i` when \c b
     * is true and by `2 * (i + di)` otherwise, so that the `\end` of an
     * environment stands at the level of its `\begin`.
     *
     * \note the comments of the lines are ignored, as well as
     * `\begin{document}` and `\end{document}`, which do not open nor close an
     * indentation level
     */
    virtual void getLineIndentation(const QTextCursor &,
                                    int &,
                                    int &,
                                    bool &) const;

    /*!
     * \return the indentation increment the given line brings to the lines
     * which follow it
     * \param[in] line : line
     *
     * Every `\begin{...}` which is not `\begin{document}` increases it by
     * one, every `\end{...}` which is not `\end{document}` decreases it by
     * one and each pair of braces which is not escaped by a backslash counts
     * the same way.
     *
     * \note the comment of the line is removed before counting: a brace
     * which is commented out does not change the indentation
     */
    virtual int getIndentationIncrement(const QString &) const;

    /*!
     * \return the master file the current buffer belongs to, as announced by a
     * line of the form `%%% TeX-master: "file"`, an empty string when the
     * buffer announces none
     *
     * \note the buffer is scanned from its first line and the first line
     * which announces a master file wins; the announced name is returned as
     * it is written in the buffer
     */
    virtual QString getTeXMasterFile();

    //! matches a `\begin{...}` command, wherever it stands in the line
    QRegularExpression rb;
    //! matches an `\end{...}` command, wherever it stands in the line
    QRegularExpression re;
    /*!
     * matches a line which opens an environment; kept for symmetry with
     * \c re2, no method of this mode uses it
     */
    QRegularExpression rb2;
    //! matches a line which closes an environment, whatever its indentation
    QRegularExpression re2;

    /*!\brief the grammar checker of this buffer, and the entries of the menu
     * which steer it
     *
     * \note the whole text of a LaTeX buffer, but the commands, is English, and
     * the checker reads that; while it runs, \c spellChecker is given no
     * language at all: the two say what is wrong with a word in the same place,
     * in different colors, and the checker is the one a person asked for
     */
    GrammarCheckSession *grammar = nullptr;
    /*!
     * \brief the spell checker which underlines the misspelled words of the
     * plain text of the document and proposes the suggestions of the context
     * menu
     */
    SpellChecker spellChecker;

    /*!
     * \brief the suggestions the last context menu offered, kept so that
     * \c replaceMispelledWordBySuggestion can tell its own actions from the
     * other actions of that menu
     *
     * \note the mode owns them: they are destroyed as soon as a new set of
     * suggestions is built
     */
    QVector<QAction *> suggestions;
    //! absolute postion of the beginning of the mispelled word
    int bmwp = 0;
    //! absolute postion of the end of the mispelled word
    int emwp = 0;

    /*!
     * \brief the frame the output of the `latex` process is displayed in, a
     * null pointer when the document was never compiled
     *
     * \note a \c QPointer: the frame is a secondary task of the buffer, which
     * the user may close at any time, and \c startLaTeX creates a new one
     * then
     */
    QPointer<ProcessOutputFrame> po;

   private:
    Q_OBJECT

  };  // end of struct LaTeXMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LATEXMAJORMODE_HXX */
