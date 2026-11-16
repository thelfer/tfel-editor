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
#include <QtCore/QVector>
#include <TFEL/GUI/SpellChecker.hxx>
#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/SettingsKey.hxx"

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

    /*!
     * \return `Category::TEXT_EDITING`: a LaTeX buffer is read as prose, the
     * output of a compiler has nothing to share with it
     */
    Category getCategory() const override;

    //! \return "major mode dedicated to the LaTeX"
    QString getDescription() const override;

    /*!
     * \return the entries of the configuration the mode uses: the program a
     * document is compiled with, in a theme of its own named after that family
     * of programs
     *
     * \note this static method is what \c StandardMajorModeProxy reports as the
     * settings of the mode, see \c MajorModeProxy::getSettingsGroups. The
     * address is the one \c chosenInterpreter reads when a document is compiled
     * and the one the `Interpreter` submenu of the mode writes
     */
    static QVector<SettingsGroup> declareSettings();

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
     * When the position of the cursor stands in a misspelled word of the line,
     * the menu gains a `Suggestions` entry in front of the entries it already
     * holds, which leads to one entry per suggestion the dictionary answers, as
     * \c MajorModeBase::addSpellCheckSuggestions builds it.
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
     * \note the menu is named `LaTeX` and holds the two questions the mode asks
     * of the person using it: which program compiles the document, in the
     * `Interpreter` submenu, and which proofreader reads its text, in the four
     * choices of \c GrammarCheckSession::addToMenu. The first is always there;
     * the second is only when a checker can be launched on this machine, which
     * is the only case where the mode has nothing to add to the menu bar
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
     *
     * \note this is what the mode did before a grammar checker was ever
     * asked of it: a session of `harper-ls` says nothing about the spelling
     * of a word, so it takes nothing off the words hunspell does not know
     */
    void setSpellCheckLanguage(const QString &) override;

    /*!
     * \brief the text a proofreader reads of a buffer of this mode
     * \param[in] text : the whole text of the document
     * \return the text with every command of LaTeX blanked out
     *
     * \note LanguageTool reads a text and knows no markup: handed the source of
     * a `\section{Title}`, it says that `section` is misspelled and says nothing
     * of the title, which is the one thing of that line it has any business
     * reading. What is taken out is therefore blanked and not removed, since the
     * answer of the checker is a place in the text it was handed, see \c
     * MajorMode::grammarCheckText
     */
    QString grammarCheckText(const QString &text) const override;

    /*!
     * \brief the session of the proofreader of this buffer
     *
     * \note what \c SpellingAndGrammar walks the buffers with: the choice of the
     * proofreader is one for the editor and the buffers of this mode follow it
     */
    struct GrammarCheckSession *grammarCheckSession() const override;

    /*!
     * \brief destructor
     *
     * \note the highlighter is released by \c MajorMode, which detaches it
     * from the document the mode highlighted
     */
    ~LaTeXMajorMode() override;

   protected slots:

    /*!
     * \brief run the interpreter of the mode on the current buffer
     *
     * \note the file which is compiled is that of \c startLaTeX, and the
     * program which compiles it is the one \c chosenInterpreter answers with
     *
     * \note the buffer is saved first, through the minibuffer, when it holds
     * unsaved changes; the compilation is then started as soon as the save
     * is over, and never if the user does not save
     */
    virtual void runLaTeX();

    /*!
     * \brief start the process of the chosen interpreter on the file of the
     * buffer
     *
     * The master file announced by the buffer is used when it holds one, the
     * file of the buffer otherwise. The program is the one the address
     * `latex/interpreter` names — `latex`, `pdflatex` or `xelatex` — and is
     * read here, on every compile, so a choice made in the `Interpreter`
     * submenu or in the configuration dialog reaches the next compile with no
     * restart of the editor. The three take the same two options and the
     * output of whichever ran is displayed in a secondary task of the buffer
     * titled "*LaTeX* ouput", with the "latex-output" major mode: that mode
     * reads the report of a web2c engine, which all three are.
     *
     * \note nothing is run and a message is displayed in the minibuffer when
     * the file has no name or does not exist on the disk
     */
    virtual void startLaTeX();

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
     * plain text of the document and answers the suggestions the context menu
     * offers for them
     */
    SpellChecker spellChecker;

    /*!
     * \brief the frame the output of the process of the interpreter is
     * displayed in, a null pointer when the document was never compiled
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
