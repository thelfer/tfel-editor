/*!
 * \file  MFrontMajorMode.hxx
 * \brief MFrontMajorMode: DSL-aware major mode for MFront files, runs mfront.
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTMAJORMODE_HXX
#define LIB_TFEL_GUI_MFRONTMAJORMODE_HXX

#include <QtCore/QTimer>
#include <QtCore/QStringListModel>
#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct MFrontSyntaxHighlighter;

  /*!
   * \brief a major mode for editing `MFront` input files
   *
   * The mode derives from \c CxxMajorMode, so indentation and formatting are
   * delegated to \c clang-format, which is called with the C++ dialect
   * (\c -assume-filename=cxx) and the style file of the directory of the
   * buffer (\c -style=file).
   *
   * \note the keywords, the syntax highlighting and the compilation of the
   * document depend on the DSL declared by the \c @DSL (or \c @Parser) token,
   * which the user may change at any time: a timer polls the buffer and
   * refreshes those when the DSL name changes
   */
  struct MFrontMajorMode : public CxxMajorMode {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     *
     * \note the MFront DSL and interface factories are initialised by
     * \c MFrontInitializer::init, which must have run before any DSL is
     * created
     *
     * \note a \c QCompleter is created on the keywords of the DSL in use; the
     * mode owns it and the text edit \c t is its widget, as
     * \c MajorModeBase::completeCurrentWord ignores a completer whose widget is
     * another text edit
     *
     * \note the timer \c rt is not started here: it is a single-shot debounce
     * restarted by \c QTextDocument::contentsChange, so the DSL of the buffer
     * is re-checked after typing pauses only, never by polling
     */
    MFrontMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "MFront", the name the mode is registered under
    QString getName() const override;

    //! \return "major mode dedicated to the MFront code generator"
    QString getDescription() const override;

    /*!
     * \brief install the syntax highlighter of the DSL currently used by the
     * buffer on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is installed through \c setHighlighter, so the
     * previous one is removed from the document and released
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \return the "MFront" menu, with the actions which run `mfront` and
     * `mfront-doc` on the buffer, the behaviour wizard, the code blocks of the
     * analysed file, the examples found in the directories of
     * `MFRONTGALLERYHOME` and `MFMHOME`, and links to the on-line
     * documentation
     *
     * \note a null pointer is returned when the parent of the mode is not a
     * widget, as the menu needs one to be a child of the user interface
     *
     * \note the caller takes ownership of the menu, so its actions refer to
     * the mode, not to the widget which displays the menu
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief add a "Help on ..." entry to the context menu when the line the
     * cursor stands on starts with a keyword of the DSL in use
     * \param[in] m : menu to complete
     * \param[in] tc : text cursor at the position where the menu appears
     *
     * \note the entry is inserted at the top of the menu, preceded by a
     * separator, and runs `mfront --help-keyword=<dsl>:<keyword>`; its output
     * is displayed in a secondary task of the buffer
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \return the completer built on the keywords of the DSL in use
     *
     * \note the completer is a child of the text edit, not of the mode, and so
     * outlives the mode when the buffer keeps being edited with another one;
     * its model is replaced whenever the DSL changes
     */
    QCompleter *getCompleter() override;

    /*!
     * \return the completion prefix
     *
     * The prefix is the current word preceded by \c '@' when that word is
     * introduced by an \c '@' token, as the keywords of the DSLs hold that
     * character, which the base implementation would drop
     */
    QString getCompletionPrefix() override;

    /*!
     * \return "mfront", the address under which the history of the compilation
     * commands of this mode is stored in the settings
     */
    QString getLanguageName() const override;

    //! \return "mfront --obuild ", the command proposed when the history of
    //! the mode is empty
    QString getDefaultCompilationCommand() const override;

    //! \return the MFront icon loaded from the resources of the editor
    QIcon getIcon() const override;
    /*!
     * \brief destructor
     *
     * The timer is a child of the mode and is released with it; the completer
     * is a child of the text edit and survives the mode; the syntax highlighter
     * is released by the base class as it belongs to the document
     */
    ~MFrontMajorMode() override;

   protected slots:
    /*!
     * \brief rebuild the syntax highlighter and the completer model
     *
     * It is triggered by \c rt, the debounce timer, only when the name of the
     * DSL in the buffer differs from \c current_dsl.
     *
     * \note nothing is refreshed while the completion popup is visible, to
     * avoid disturbing the user while he selects a completion; the highlighter
     * and the model of the completer are otherwise both rebuilt from the
     * keywords of the DSL in use
     */
    virtual void updateSyntaxHighlighterAndCompleter();
    /*!
     * \brief run the behaviour wizard, which asks the user for a behaviour
     * type, an integration scheme, a DSL, a brick, modelling hypotheses and
     * variables, and insert the corresponding template at the cursor
     *
     * \note the whole buffer is indented once the template has been inserted,
     * by \c clang-format; a cancelled wizard leaves the buffer untouched
     */
    void runMFrontBehaviourWizard();

   protected:
    /*!
     * \return the keywords of the DSL declared by the buffer, as reported by
     * the parser that \c mfront::DSLFactory creates for that DSL
     *
     * \note on failure (unknown DSL, tokenizer error), the reason is reported
     * in the minibuffer and a possibly empty list is returned
     */
    virtual QStringList getKeyWordsList();
    /*!
     * \brief run the code generator on the file of the buffer
     *
     * The buffer is saved first when it has been modified; the user then
     * chooses the options of the generation in a dialog box and the output of
     * `mfront` is displayed in a secondary task of the buffer.
     *
     * \note unlike the base implementation, no compilation command is asked
     * for in the minibuffer
     */
    void runCompilation() override;
    /*!
     * \brief single-shot debounce timer refreshing the syntax highlighting
     *
     * Restarted by \c QTextDocument::contentsChange and never started with a
     * fixed interval: the value after `@DSL` can change with the user inputs,
     * and this is checked once per typing pause instead of by polling.
     */
    QTimer *rt = nullptr;
    /*!
     * \brief completer of the keywords of the DSL in use
     *
     * Created by the constructor, with the text edit as widget; it completes
     * on \c keywordsModel, which \c updateSyntaxHighlighterAndCompleter
     * updates in place when the DSL changes
     */
    QCompleter *c = nullptr;
    /*!
     * \brief model of the keyword completions, owned by the mode
     *
     * \c QCompleter::setModel borrows the models it is given, so the mode
     * keeps the only model and refreshes it with \c setStringList instead of
     * replacing it (issue #66)
     */
    QStringListModel *keywordsModel = nullptr;
    /*!
     * \brief name of the DSL the syntax highlighter, the completer and the
     * menus were built for
     *
     * The timer compares the DSL of the buffer to this name and refreshes both
     * when they differ; the name of a buffer without any `@DSL` token is
     * "DefaultDSL".
     */
    QString current_dsl;

   private:
    Q_OBJECT
  };  // end of struct MFrontMajorMode

  /*!
   * \brief run the `mfront` code generator on the file of the current buffer
   * \param[in] editor : editor widget
   *
   * The buffer is saved first when it has been modified, then the user chooses
   * the options of the generation in a dialog box whose interface field keeps
   * the last value used for the corresponding kind of document.
   *
   * \note nothing is run when the buffer is not associated with an existing
   * file; the reason is displayed in the minibuffer
   *
   * \note this is the implementation of the "run-mfront" command
   */
  void runMFront(EditorWidget &);

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTMAJORMODE_H */
