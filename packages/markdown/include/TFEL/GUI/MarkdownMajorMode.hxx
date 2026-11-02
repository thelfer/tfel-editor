/*!
 * \file  MarkdownMajorMode.hxx
 * \brief MarkdownMajorMode: major mode for markdown, spell check, pandoc.
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTMAJORMODE_HXX
#define LIB_TFEL_GUI_MFRONTMAJORMODE_HXX

#include <QtGui/QSyntaxHighlighter>
#include <TFEL/GUI/SpellChecker.hxx>
#include "TFEL/GUI/CompiledLanguageMajorModeBase.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;

  //! \brief major mode for editing Markdown documents
  struct MarkdownMajorMode : public CompiledLanguageMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor the mode acts on, which is also its parent
     * widget
     */
    MarkdownMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "Markdown"
    QString getName() const override;

    //! \return "major mode dedicated to the Markdown language"
    QString getDescription() const override;

    /*!
     * \brief set the language the spell checker of the mode uses
     * \param[in] l : language name, as the installed dictionaries name it
     *
     * \note the document is rehighlighted so that the words are underlined
     * in the new language; an unknown language disables the spell checking
     */
    void setSpellCheckLanguage(const QString &) override;

    /*!
     * \brief install a `MarkdownSyntaxHighlighter` on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter refers to this mode, so the mode must outlive
     * it; the previous highlighter is released by \c setHighlighter
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \return a menu specific to this mode
     *
     * \note a null pointer: the mode adds no menu of its own to the menu
     * bar
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     *
     * \note nothing is added, not even the actions of \c MajorModeBase, so
     * the misspelled words of the document are underlined but never suggested
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \return the completer of the mode
     *
     * \note a null pointer: this mode proposes no completion
     */
    QCompleter *getCompleter() override;

    //! \return an empty string, as this mode proposes no completion
    QString getCompletionPrefix() override;

    /*!
     * \return "markdown", the address under which the history of the
     * compilation commands of this mode is stored in the settings
     *
     * \note this is what \c runCompilation builds its settings key from; this
     * mode never calls it and its "Alt-m" shortcut records the history of the
     * pandoc commands at another address, so this name is of no use here
     */
    QString getLanguageName() const override;

    /*!
     * \return "pandoc ", the command proposed to the user when no compilation
     * command has been recorded in the settings yet
     *
     * \note \c runCompilation is the only method which proposes it and this
     * mode never calls it: its "Alt-m" shortcut proposes another pandoc
     * command of its own, so this default never reaches the user
     */
    QString getDefaultCompilationCommand() const override;

    /*!
     * \brief no-op: this mode does not reformat the document as a whole;
     * \c keyPressEvent cuts the paragraph the cursor is in instead
     */
    void format() override;

    /*!
     * \brief handle the key events before the text edit does
     * \param[in] ev : key event
     * \return true when the event is handled here
     *
     * "Tab" and "Alt-q" cut the paragraph the cursor is in to 72 columns,
     * keeping the indentation of its first line, or of the list item it
     * belongs to, and keeping the cursor next to the same word. "Alt-m" asks
     * the pandoc command to run in the minibuffer.
     *
     * \note the paragraph stops at the first line which is not a plain text
     * line: a blank line, a header, a comment, a list item or a footnote
     * breaks it
     *
     * \note when the line of the cursor is one of those which cannot start a
     * paragraph, nothing is changed and the event is still handled
     *
     * \note any other key is handed over to
     * \c CompiledLanguageMajorModeBase::keyPressEvent
     *
     * \note the minibuffer input installed by "Alt-m" proposes the last pandoc
     * command of the history stored at "pandoc/compilation/history", or a
     * default command when none was recorded yet, and lets the output of the
     * command be displayed with the "markdown-output" major mode
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief treat the shortcuts which are specific to this mode
     * \param[in] k1 : first key, \c Qt::Key_X or \c Qt::Key_C
     * \param[in] m : modifier of the second key
     * \param[in] k2 : second key
     * \return true when the shortcut is handled here
     *
     * "Ctrl-C =" gathers the headers of the buffer, that is the lines which
     * start with a '#', in a read-only table of contents attached to the
     * buffer as a secondary task, highlighted in Markdown.
     *
     * \note the table of contents is a snapshot: it is not updated as the
     * buffer is modified, the user has to close it and ask for it again
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;

    //! \return the icon of the mode, the "x-office-document" theme icon
    QIcon getIcon() const override;

    /*!
     * \return the spell checker of this mode, which underlines the misspelled
     * words of the document
     *
     * \note the checker is a member of the mode, which owns it;
     * \c MarkdownSyntaxHighlighter asks it for the words to underline, the mode
     * itself never suggests a correction in the context menu
     */
    SpellChecker &getSpellChecker();

    //! destructor
    ~MarkdownMajorMode() override;

   protected slots:

    /*!
     * \brief slot called to prepare a Markdown analysis
     *
     * \note the buffer is saved first, through the minibuffer, when it holds
     * unsaved changes; the analysis is then started as soon as the save is
     * over, and never if the user does not save
     *
     * \note nothing triggers this slot anymore: the action which used to call
     * it is gone from the specific menu of the mode, which returns a null
     * pointer
     */
    virtual void runPandoc();
    /*!
     * \brief slot called to start a new Markdown analysis
     *
     * \note a message is displayed in the minibuffer when the buffer has no
     * name yet; no analysis is run in any case, as the frame which would
     * display its output is not created
     */
    virtual void startPandoc();

   protected:
    /*!
     * \brief the spell checker the syntax highlighter uses to underline the
     * misspelled words of the document; it is the checker \c getSpellChecker
     * returns and the dictionary of which \c setSpellCheckLanguage sets
     */
    SpellChecker spellChecker;

   private:
    Q_OBJECT
  };  // end of struct MarkdownMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTMAJORMODE_H */
