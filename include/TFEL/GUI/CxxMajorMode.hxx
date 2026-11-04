/*!
 * \file  CxxMajorMode.hxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_CXXMAJORMODE_HXX
#define LIB_TFEL_GUI_CXXMAJORMODE_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CMajorModeBase.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct LspClient;

  /*!
   * \brief major mode dedicated to the C++ language
   *
   * \note the mode is registered by a static \c StandardMajorModeProxy defined
   * at the bottom of its source file, which matches the C and C++ file
   * extensions. It is the only mode derived from \c CMajorModeBase in the
   * editor, and \c clang-format is always called with the C++ dialect
   *
   * \note when the setting `clangd/enable` allows it, the mode talks to a
   * language server, \c clangd, through the \c LspClient shared by the editor
   * widget: the diagnostics of the server are drawn under the text, and
   * \c Alt-Tab, \c Alt-. and \c Alt-? ask it for completions, definitions and
   * hover information (issue #125)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CxxMajorMode : public CMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     */
    CxxMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "C++", the name the mode is registered under
    QString getName() const override;

    //! \return "major mode dedicated to the C++ language"
    QString getDescription() const override;

    //! \return the C++ icon of the editor resources
    QIcon getIcon() const override;

    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     *
     * \c Alt-m runs a compilation command in the minibuffer.
     * \c Alt-Tab asks the language server for completions, \c Alt-. jumps to
     * the definition of the symbol under the cursor and \c Alt-? shows its
     * hover information.
     *
     * \return true if the mode handled the key, that is on those keys only;
     * any other key is left to \c CMajorModeBase, which handles none
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \return the completer of the language server while its popup is shown,
     * the one of the base class otherwise, that is none
     */
    QCompleter *getCompleter() override;

    /*!
     * \return 1 while the language server popup is shown, so that typing a
     * single character does not close it, 3 otherwise, as
     * \c MajorModeBase does
     */
    int getMinimalCompletionLength() override;

    /*!
     * \brief replace the word under the cursor by the completion the user
     * chose in the language server popup, without the blank the base class
     * appends: a C++ completion often ends in `(` or `::`
     */
    void completeCurrentWord(TextEditBase &, const QString &) override;

    /*!
     * \brief install the highlighter of the C++ language on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is owned by the document from this point on; it is
     * released, and the previous one removed, by the base class
     */
    void setSyntaxHighlighter(QTextDocument *const) override;
    //! destructor
    ~CxxMajorMode() override;

   protected:
    /*!
     * \return the name of the mode, "C++", which is also the language name:
     * the compilation command history is stored under the address
     * "C++/compilation/history"
     */
    QString getLanguageName() const override;

   private slots:
    /*!
     * \brief the server published diagnostics: draw them when they are the
     * ones of the file this buffer visits
     */
    void updateDiagnostics(const QString &);

   private:
    /*!
     * \brief open the document of the file the buffer visits, following the
     * renames of the buffer; the previous name is closed
     */
    void openOrRenameDocument();
    /*!
     * \brief underline the diagnostics the server last published, as the
     * `clangd` contribution of the extra selections of the text edit
     */
    void applyDiagnostics();
    //! ask the server for the completions at the cursor, synchronously
    void completeWithClangd();
    //! ask the server where the symbol under the cursor is defined
    void jumpToDefinition();
    //! ask the server what the symbol under the cursor is
    void showHover();
    //! the client of the editor widget, null when `clangd/enable` is false
    QPointer<LspClient> client;
    //! the file name the server knows this buffer by, empty if none
    QString documentFile;
    /*!\brief the document opened on the client under `documentFile`
     *
     * A \c QPointer, and not a call to `textEdit.document()`: the mode dies
     * as a child of the text edit widget, at a point where that widget's own
     * Qt widget is gone and the virtual `document()` would fall through a
     * dangling pointer, the very trap of issue #20 in a new place
     */
    QPointer<QTextDocument> document;
    //! the completion popup fed by the server, created on the first request
    QCompleter *lspCompleter = nullptr;
    //! true while the server popup is up
    bool lspCompletionActive = false;

    Q_OBJECT
  };  // end of struct CxxMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CXXMAJORMODE_HXX */
