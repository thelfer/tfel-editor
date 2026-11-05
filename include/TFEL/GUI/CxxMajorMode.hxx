/*!
 * \file  CxxMajorMode.hxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_CXXMAJORMODE_HXX
#define LIB_TFEL_GUI_CXXMAJORMODE_HXX

#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QVector>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CMajorModeBase.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/SettingsKey.hxx"

// forward declarations
class QKeyEvent;

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct LspClient;
  // forward declaration
  struct LspLocation;

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
   * widget: the diagnostics of the server are drawn under the text, the
   * \c Alt-Tab, \c Alt-. \c Alt-, \c Alt-R and \c Alt-? keys ask it for
   * completions, definitions, references, renames and hover information; the
   * parameter list of the call the cursor stands in is echoed by \c Alt-S and
   * by the editor itself when a character is typed where a call is being
   * typed, \c Alt-O asks which symbol of the file to go to, and the context
   * menu offers to jump to the definition of the symbol it was opened on and to
   * apply the fixes the server offers for the diagnostics of that line (issue
   * #125)
   *
   * \note what the identifiers of the file are is asked of the server and drawn
   * with the answer: a type, a function or a method, a macro, and a strike-out
   * over a name the server says is deprecated. The faces are an extra selection
   * of the text edit, so the keywords, strings, numbers and comments of the
   * highlighter are untouched, and the locals, parameters and members the
   * server names as well are left as they are; the \c Color identifiers entry
   * of the menu, and the setting `clangd/semantic-tokens` it writes, turn them
   * off
   *
   * \note the word the cursor sits on is underlined when the server knows
   * where it is defined or declared, and the mouse cursor becomes the icon of
   * that jump — \c go-jump-definition or \c go-jump-declaration of the
   * Freedesktop Icon Naming Specification, the pointing hand when the theme
   * has neither — while it rests on such a word. Neither asks the server on
   * every keystroke or every pixel: one settling timer of the mode is shared
   * by the cursor and the mouse, and the answers they get are remembered by
   * word until the text or the session changes
   *
   * \note the message of a diagnostic is shown when it is pointed at: the
   * minibuffer echoes it once the cursor has settled on the underlined range,
   * and resting the mouse on that range shows the same text as a tooltip
   *
   * \note the server is never started by the mode: a session is launched by
   * the user, from the "C/C++" menu of the mode or the \c launch-clangd
   * commands, with or without a compilation database, and a session which is
   * running is replaced by the one which is asked for. The session launched
   * without an explicit database uses the file given to \c tfel-editor as
   * \c --compile-commands-file, or the database \c searchCompileCommandsFile
   * finds in the project of the current buffer, or none. The documents of the
   * buffers are held by the client in the meantime and sent to the session
   * once it is up
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

    /*!
     * \return the entries of the configuration the mode uses: the four which
     * decide what is asked to \c clangd, in a theme of their own named after
     * the program
     *
     * \note this static method is what \c StandardMajorModeProxy reports as the
     * settings of the mode, see \c MajorModeProxy::getSettingsGroups. The
     * addresses are the ones \c LspCxxSupport reads
     */
    static QVector<SettingsGroup> declareSettings();

    //! \return the C++ icon of the editor resources
    QIcon getIcon() const override;

    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     *
     * \c Alt-m runs a compilation command in the minibuffer.
     * \c Alt-Tab asks the language server for completions, \c Alt-. jumps to
     * the definition of the symbol under the cursor, \c Alt-, lists the
     * references to it, \c Alt-R renames it through a question of the
     * minibuffer, \c Alt-S shows the parameter list of the call the cursor
     * stands in, and \c Alt-? shows its hover information. The context menu
     * asks the definition and the fixable diagnostics of the line it was
     * opened on together, and offers what came back as entries.
     *
     * \return true if the mode handled the key, that is on those keys only;
     * any other key is left to \c CMajorModeBase, which handles none
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief watch the mouse moves and the mouse leaving the text, to track
     * the word the pointer rests on
     *
     * \note the filter is installed on the viewport of the text edit by the
     * constructor, and only when the \c clangd support is enabled
     */
    bool eventFilter(QObject *const, QEvent *const) override;

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
     * \return the "C/C++" menu of the mode, with the two entries which launch
     * a \c clangd session and the one which ends it
     *
     * \note as any specific menu of a major mode, the caller owns the returned
     * menu, which is why its actions refer to the mode, not to the widget the
     * menu is displayed in
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief add a "Go to definition" entry to the context menu when the
     * language server has a definition for the symbol the menu was opened on
     * \param[in] m : menu to complete
     * \param[in] tc : text cursor at the position where the menu appears
     *
     * \note the entry is inserted at the top of the menu, before the entries
     * of the text edit and separated from them, and jumps where the server
     * says the symbol is defined, the file which holds it being opened if it
     * is not visited yet
     *
     * \note the definition is asked for here, as it is the only way to know
     * whether to offer the entry, and the answer is waited for a moment only:
     * a server which is not up, or which is busy parsing, leaves the menu
     * untouched rather than delaying it
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \brief install the highlighter of the C++ language on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is owned by the document from this point on; it is
     * released, and the previous one removed, by the base class
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!\brief ask the server to rename the symbol at \c position in this
     * buffer's document to \c name and apply the workspace edit it answers to
     * every buffer it visits; the report of what changed, or of the refusal,
     * goes to the minibuffer
     *
     * \note public because the answer of the minibuffer question comes from
     * an input of its own, see \c renameSymbol, and that input is asked on a
     * position which was the cursor's when the question was raised
     */
    void renameToAt(const QString &name, int position);
    /*!
     * \brief place the cursor on one of the symbols of the overview of the
     * file and say in the minibuffer which one it is
     * \param[in] symbol: an entry of the overview this mode asked for
     */
    void goToDocumentSymbol(const LspDocumentSymbol &symbol);

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
    /*!
     * \brief what the server answers about one of the tracked words: the word
     * the text cursor sits on, which gains an underline, and the word the
     * mouse rests on, which gains the icon of the jump as its mouse cursor
     *
     * \c capabilities is \c -1 while the question is on its way or was never
     * asked, \c 0 when the symbol answers to neither \c go-to-definition nor
     * \c go-to-declaration, and the or-ing of the \c HasDefinition and
     * \c HasDeclaration bits otherwise
     */
    struct TrackedWord {
      //! the identifier itself, empty when no word is tracked
      QString word;
      //! where it starts in the document, \c -1 with no word
      int start = -1;
      //! the revision of the document it was read at
      qint64 revision = -1;
      //! what the server says of it, see above
      int capabilities = -1;
    };
    //! the word under the text cursor, as far as it was resolved
    TrackedWord cursor_word;
    //! the word under the mouse, as far as it was resolved
    TrackedWord hovered_word;
    //! the viewport position the mouse last visited
    QPoint hovered_point;
    /*!\brief what was last echoed of a diagnostic, message and start of its
     * range glued; empty when nothing stands under the cursor to be told
     */
    QString echoed_diagnostic;
    //! the timer after which a settled position is asked about
    QTimer *navigation_timer = nullptr;
    /*!
     * \brief the timer after which a typed trigger character asks for the
     * signature of the call it belongs to
     *
     * Its own, and not the navigation one: a settling cursor asks about a
     * word, which is a different question from the one a typed `(` asks, and
     * the two answers belong in different places — an underline, and the
     * minibuffer
     */
    QTimer *signature_timer = nullptr;
    /*!\brief counts the signature questions, so that the answer of one which
     * is still on its way cannot overwrite the message of a later one
     */
    int signature_request = 0;
    //! true while the word under the cursor is to be asked for
    bool cursor_tracking = false;
    //! true while the word under the mouse is to be asked for
    bool hover_tracking = false;
    /*!
     * \brief the timer after which the identifiers of the file are asked what
     * they are, the semantic tokens of the protocol
     *
     * Its own, and much longer than the two others: the question is about the
     * whole file, not about a word, so it is worth a pause the size of a
     * keystroke, and the answer is the color of the file rather than anything
     * the person typing waits to be told
     */
    QTimer *tokens_timer = nullptr;
    /*!\brief counts the questions for the tokens of the file, so that the
     * answer of one which is still on its way cannot color the buffer with
     * what the server said of a text which is no longer on screen
     */
    int tokens_request = 0;
    /*!
     * \brief ask what the identifiers of the file are and color them with the
     * answer, the semantic tokens of the protocol
     *
     * \note silent on every path: this is the editor coloring a buffer on its
     * own, not answering a question somebody asked, and a minibuffer which
     * talks about the colors of the text would talk over everything else
     */
    void refreshSemanticTokens();
    /*!
     * \brief draw the given tokens on the buffer, under the `clangd-tokens`
     * contribution to the extra selections of the text edit
     * \param[in] tokens: what the server said the identifiers are
     * \param[in] revision: the revision the question was asked at, which the
     * document has to still be at for the answer to describe its text
     */
    void applySemanticTokens(const QVector<LspSemanticToken> &,
                             int sequence,
                             qint64 revision);
    /*!
     * \brief forget the colors the server gave: another file, a session which
     * is gone, or a person who asked not to have them
     */
    void clearSemanticTokens();
    /*!
     * \brief forget the tracked words, the underline and the mouse cursor:
     * another file, or another session, knows nothing of them
     */
    void resetNavigation();
    /*!
     * \brief ask the new name of the symbol under the cursor in the
     * minibuffer, and hand it to \c renameTo once it is answered
     */
    void renameSymbol();
    /*!
     * \brief apply the edits of a workspace edit to the documents they name,
     * opening the files which are not visited yet and coming back to the
     * buffer of this mode afterwards
     * \return the number of edits applied
     *
     * \note what a rename answers and what a code action carries are the same
     * `WorkspaceEdit` of the protocol, so the same code applies both, and a
     * quick fix which touches three files is applied to three files exactly
     * as a rename would be
     */
    int applyWorkspaceEdit(const QMap<QString, QVector<LspTextEdit>> &);
    //! ask about the tracked words, once the position has settled
    void refreshTrackedWords();
    /*!
     * \brief ask whether the word of one of the two tracked positions is
     * defined or declared somewhere
     * \param[in] track_cursor: ask for the word under the text cursor, else
     * for the one under the mouse
     */
    void askTrackedWord(bool track_cursor);
    //! the underline of the word under the cursor, once it is resolved
    void applyTrackedCursor();
    //! the mouse cursor over the word it rests on, once it is resolved
    void applyTrackedHover();
    /*!\brief echo the message of the diagnostic the cursor stands on, if any
     *
     * \c report is called only when the diagnostic under the cursor differs
     * from the one last echoed, so a settling timer restarted by editing in
     * the middle of a reported line does not repeat the message
     */
    void echoDiagnosticAtCursor();
    /*!\return the message of the diagnostic whose range visits the given
     * document position, empty when none does; the text the minibuffer and
     * the tooltip show, in the same words
     */
    QString diagnosticAt(const QTextDocument &, int position) const;
    //! ask the server for the completions at the cursor, synchronously
    void completeWithClangd();
    //! ask the server where the symbol under the cursor is defined
    void jumpToDefinition();
    /*!\brief ask the server for every use of the symbol under the cursor and
     * pop a menu of them to jump to, the references list of the protocol
     */
    void showReferences();
    /*!
     * \brief ask the overview of the file and ask in the minibuffer which of
     * its symbols to go to, the document symbols of the protocol
     *
     * \note the overview is asked when the key is pressed, not kept from a
     * previous press: the file is being edited, and a list of symbols which
     * lost a function to a cut is worse than a round trip
     */
    void showDocumentSymbols();
    /*!\brief ask the server what call the cursor is inside of and echo its
     * parameter list in the minibuffer, the active argument picked out of it
     * \param[in] asked_by_user: false when a typed character raised the
     * question, which stays silent when the server sees no call there rather
     * than reporting one on every keystroke
     */
    void showSignatureHelp(const bool asked_by_user);
    /*!
     * \brief start the signature question after a character the server says a
     * call is typed around has been typed
     * \param[in] e: the key event the base class is done with
     */
    void watchSignatureHelp(const QKeyEvent &e);
    /*!
     * \brief place the cursor on the given definition, opening the file which
     * holds it when that file is not the one of this buffer
     */
    void jumpTo(const LspLocation &);
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
