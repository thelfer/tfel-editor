/*!
 * \file  LspNavigationSession.hxx
 * \brief the conversation of a buffer with the language server of its language:
 *        the diagnostics it draws, the completion, navigation, rename, symbol
 *        overview, signature help and quick fixes it answers.
 * \author Thomas Helfer
 * \date   14/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPNAVIGATIONSESSION_HXX
#define LIB_TFEL_GUI_LSPNAVIGATIONSESSION_HXX

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/LspProtocol.hxx"

// forward declarations
class QCompleter;
class QKeyEvent;
class QMenu;
class QMouseEvent;
class QTimer;
class QTextCharFormat;
class QTextCursor;
class QTextDocument;

namespace tfel::gui {

  // forward declarations
  struct Buffer;
  struct EditorWidget;
  struct LspClient;
  struct TextEditBase;

  /*!
   * \brief a language server and the buffer it is asked about
   *
   * \note one session belongs to one text edit, the way a highlighter and a
   * \c GrammarCheckSession do: the questions are asked of the text of a frame
   * and the answers — underlines, places to jump to, colors — are drawn on that
   * same frame. The server itself is shared by every buffer of the editor which
   * speaks its language, see \c LspClient::get; what this class holds is the
   * document, the tracked words, the three settling timers and the faces
   *
   * \note the half of a mode which talks to a server is here rather than in the
   * mode because the questions and the answers are the protocol's, not the
   * language's: \c textDocument/definition asks the same question of \c clangd
   * and of \c pylsp, and a mode adds to this the few answers which are not —
   * what its menu holds, whether the server knows declarations at all, what it
   * colors from the semantic tokens, and whether a file of a project starts a
   * session of its own accord
   *
   * \note what a session draws is keyed on the name of the server it speaks
   * with — `clangd`, `clangd-navigation`, `clangd-tokens` —, which is what keeps
   * the diagnostics of one server and the lints of a proofreader, or the faces
   * of two servers, from drawing over one another, see
   * \c TextEditBase::setExtraSelections
   *
   * \note the session never starts a server: which session runs, and what it
   * was started with, is a decision of the user, and the entries of the menu of
   * the mode are where it is taken. \c startSessionIfNeeded is the one place a
   * mode may ask for a session of its own, and the C++ mode is the only one
   * which does, for the project of a file which says how it is built
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspNavigationSession : public QObject {
    Q_OBJECT
   public:
    /*!
     * \brief constructor
     * \param[in] w: editor widget the session belongs to, which holds the
     * client of the server this session talks with
     * \param[in] b: buffer whose text is asked about, which is what a rename
     * comes back to once every other buffer it touched is edited
     * \param[in] t: text edit the text of which is asked about and drawn on
     * \param[in] name: the server this session speaks with, which is the name
     * the minibuffer messages call it, the object name the client of the editor
     * is found by, and the key of the faces this session draws
     * \param[in] language: what the files handed to that server are, in the sense
     * of the `languageId` of the protocol, which is what decides how it parses
     * them; the client of an editor widget is built by the first session of that
     * widget and keeps the answer, as it keeps everything else the handshake reads
     * \param[in] p: parent object, which is the mode that owns the session
     *
     * \note the constructor asks nothing: \c openOrRenameDocument is what hands
     * the document of the buffer to the server and is called by the mode at the
     * end of its own constructor, not from here, because the answer of a derived
     * session — whether a session of its own is to be started for this file —
     * is not knowable before the derived part of the object exists
     */
    LspNavigationSession(EditorWidget &w,
                         Buffer &b,
                         TextEditBase &t,
                         const QString &name,
                         const QString &language,
                         QObject *const p = nullptr);
    /*!
     * \return the client of the server this session talks with, so that the mode
     * which owns it can start and end a session, and tell the client what the
     * files it hands over are
     *
     * \note one client per editor widget and per server, shared by every buffer of
     * that language: a session holds no server of its own
     */
    LspClient &client();
    /*!
     * \return the file the document of this buffer is known as to the server,
     * empty when the buffer visits no file yet
     */
    QString visitedFile() const;
    /*!
     * \brief destructor: releases the document on the server, which tells the
     * server the file is closed when this buffer was its last user
     *
     * \note `textEdit.widget()` and `textEdit.document()` are not called from
     * here: a mode is destroyed as a child of the text edit widget, whose own Qt
     * widget is gone by then, and a virtual call would read it back through a
     * dangling pointer. The `QPointer` says what is alive
     */
    ~LspNavigationSession() override;

    /*!
     * \brief hand the document of the buffer to the server, following the
     * renames of the buffer; the previous name is closed
     *
     * \note the words tracked on the previous file, and what the server said of
     * them, are none of the renamed buffer's business: the tracking starts over
     */
    void openOrRenameDocument();

    /*!
     * \brief the keys of the server: \c Alt-Tab asks the completions of the
     * word at the cursor, \c Alt-. jumps to the definition of the symbol under
     * it, \c Alt-, lists the references to it, \c Alt-R renames it through a
     * question of the minibuffer, \c Alt-S shows the parameter list of the call
     * the cursor stands in, \c Alt-? shows its hover information, and \c Alt-O
     * asks which symbol of the file to go to
     * \return true when one of those keys was handled, false when the key is
     * the text edit's and the mode's own keys are worth consulting
     */
    bool keyPressEvent(QKeyEvent *const);
    /*!
     * \brief start the signature question after a character the server says a
     * call is typed around has been typed
     * \param[in] e : the key event the mode has just handed back
     *
     * \note to be called once the mode and the base class have both said they
     * handle nothing: the character typed reaches the document only after that,
     * so the question is asked by a settling timer, by which time the call on
     * screen is the call the answer is about
     */
    void watchSignatureHelp(const QKeyEvent &);
    /*!
     * \brief jump to the definition, or to the declaration, of the symbol the
     * \c Ctrl-Left click was pressed on
     * \param[in] e : the mouse press the text edit is about to handle itself
     *
     * \note the click is the mouse's \c Alt-.: the icon of the jump which the
     * session put under the pointer promised a place to go, and a pointer which
     * promises and does nothing is worse than one which says nothing. Where the
     * symbol is nothing to jump to, the click is handed back to the text edit,
     * which places the cursor as it always does
     *
     * \note the answers the tracking of the mouse already got for that word are
     * used as they are, so the click which follows the icon of the jump asks
     * nothing and lands at once; a word which was never rested on is asked
     * about, and waited for a moment only, as the context menu is
     *
     * \return true when the click jumped, that is when the text edit shall not
     * move the cursor to the place it was clicked on
     */
    bool mousePressEvent(QMouseEvent *const);
    /*!
     * \return the completer of the server while its popup is shown, null
     * otherwise, which is what leaves the completion of the mode itself standing
     */
    QCompleter *getCompleter();
    /*!
     * \return 1 while the popup of the server is shown, so that typing a single
     * character does not close it, 3 otherwise, as \c MajorModeBase does
     */
    int getMinimalCompletionLength();
    /*!
     * \brief replace the word under the cursor by the completion the user chose
     * in the popup of the server, without the blank the base class appends: a
     * completion of a callable often ends in `(`
     * \return true when the popup of the server was up and the word was
     * replaced, false when it was not, which leaves the mode to complete the
     * word its own way
     */
    bool completeCurrentWord(TextEditBase &, const QString &);
    /*!
     * \brief add to the context menu the entry which jumps to the definition of
     * the symbol the menu was opened on and the entries which apply the fixes the
     * server offers for the diagnostics of that line
     * \param[in] m : menu to complete
     * \param[in] tc : text cursor at the position where the menu appears
     *
     * \note the entries are inserted at the top of the menu, before the entries
     * of the text edit and separated from them
     *
     * \note the two questions are asked here, as that is the only way to know
     * whether to offer anything, and their answer is waited for a moment only: a
     * server which is not up, or which is busy parsing, leaves the menu untouched
     * rather than delaying it
     */
    void completeContextMenu(QMenu *const, const QTextCursor &);
    /*!
     * \brief ask the overview of the file and ask in the minibuffer which of its
     * symbols to go to, the document symbols of the protocol
     *
     * \note public because the menu of a mode holds an entry of its own for it: a
     * key one has never heard of is a key never pressed
     */
    void showDocumentSymbols();
    /*!
     * \brief ask the server to reformat this buffer's document, the
     * \c textDocument/formatting question of the protocol, and apply the answer
     * \return true when the question was asked and its answer applied; false
     * when there was no session to ask, or nothing to change, which is what
     * leaves a mode to fall back on the formatter of its own language
     */
    bool formatDocument();
    /*!
     * \brief ask the server for the change which sorts the imports of this
     * buffer's file and apply it: the `source.organizeImports` kind of the
     * question about the changes a server offers on a range
     * \param[out] why : what the minibuffer should say when nothing was applied
     * — that no such change was offered, or that the one offered wrote what the
     * text already said
     * \return true when an answer was applied to the buffers of the editor
     *
     * \note the kind is asked for in the question and picked out of the answer:
     * a server is not bound by the list of kinds it is handed, and `ruff`, of
     * `pylsp`, answers every change it has whatever the question named
     * (measured), so a caller which wants one kind of change has to choose it
     * itself once it is on this side of the wire
     *
     * \note a server whose sorter offers no change of this kind — the `isort`
     * plugin of `pylsp` sorts as part of the format request and answers no
     * action at all — is answered nothing here: \c formatDocument is the way to
     * ask such a server
     */
    bool sortImports(QString &why);
    /*!
     * \brief the same question, with the reason nothing was applied
     * \param[out] why : what the minibuffer has to say when false comes back —
     * that no server was up to ask, that it kept the question, or that its
     * answer wrote what the text already says
     * \return true when an answer changed the text
     *
     * \note the version above is what a key of the mode calls, which is silent on
     * a buffer which is already formatted; this is what an entry which was asked
     * for by name reports, since a person who asked for a reformat wants to know
     * when none happened
     */
    bool formatDocument(QString &why);
    /*!
     * \brief tell the server that the file of this buffer now says what the
     * buffer says, which is what a linter reading the file itself waits for
     *
     * \note asked by itself when the document stops being modified, the mark a
     * save leaves behind: `pylint`, of `pylsp`, runs its program over the file
     * on disk, keeps the results it had while the buffer was dirty and refreshes
     * them on this cue alone (measured), so a session which is never told would
     * go on underlining the file as the last save left it
     */
    void noteFileSaved();
    /*!
     * \brief ask the server to rename the symbol at \c position of this buffer's
     * document to \c name and apply the workspace edit it answers to every buffer
     * it visits; the report of what changed, or of the refusal, goes to the
     * minibuffer
     *
     * \note public because the answer of the minibuffer question comes from an
     * input of its own, and that input is asked on a position which was the
     * cursor's when the question was raised
     */
    void renameToAt(const QString &name, int position);
    /*!
     * \brief place the cursor on one of the symbols of the overview of the file
     * and say in the minibuffer which one it is
     * \param[in] symbol: an entry of the overview this session asked for
     */
    void goToDocumentSymbol(const LspDocumentSymbol &symbol);
    /*!
     * \brief restart the question about the identifiers of the file, which is
     * what the entry turning the colors on does
     */
    void askSemanticTokens();
    /*!
     * \brief forget the colors the server gave: another file, a session which is
     * gone, or a person who asked not to have them
     */
    void clearSemanticTokens();
    /*!
     * \brief say what a session of this server is called in the minibuffer
     * \param[in] m : the message to say
     *
     * \note the mode's own report, which is the same function of the same editor
     * widget; a derived session overrides this to give a message of its own a
     * shape the generic wording can not give
     */
    virtual void report(const QString &m);

   protected:
    /*!
     * \return the name of the menu of the mode which holds the entries starting
     * and ending a session, which is what the message of a session which was
     * never started points at; empty when the mode has no such menu
     */
    virtual QString menuName() const;
    /*!
     * \return the name of the command which starts a session, for the same
     * message; empty when there is none
     */
    virtual QString startCommandName() const;
    /*!
     * \return true when the server answers `textDocument/declaration`. A server
     * which has no such question answers an error to it, and a word which is
     * defined has its answer twice over: an underline and a jump are decided by
     * the definition question alone in that case
     */
    virtual bool declarationSupported() const;
    /*!
     * \brief a file has just been handed to the server: start a session for it
     * when the language has a session which is worth starting without being
     * asked for
     *
     * \note the C++ mode is the only one which overrides this, and only when the
     * setting allows it: a session which reads the compilation options of a
     * project is not a guess, see the notes of \c CxxMajorMode
     */
    virtual void startSessionIfNeeded();
    /*!
     * \return true when the identifiers of the file are to be asked what they
     * are. A server which announces no legend to read an answer by is never asked
     * anything, whether this answers true or false; a mode answers false to keep
     * the colors of its highlighter alone, which is what a server which knows the
     * type of every name would do over them
     */
    virtual bool semanticTokensWanted() const;
    /*!
     * \brief the face a run of text the server says what it is is drawn with
     * \param[in] token: the identifier and what the server called its kind
     * \param[out] format: the face to draw it with
     * \return true when the token is to be drawn with that face
     *
     * \note the names of the kinds are those of the legend the server announced,
     * not of the protocol, so whoever colors the identifiers knows that list
     */
    virtual bool semanticFaceOf(const LspSemanticToken &token,
                                QTextCharFormat &format) const;
    /*!
     * \brief whether a change the server offers is worth an entry of the
     * context menu of the buffer
     * \param[in] action : what the server offered, which always carries the
     * text it would write — an action carrying none is never offered, whoever
     * answers
     * \return true, the default: the answers of a C++ server are the quick fix
     * of a diagnostic and the refactor of the range the menu was opened on,
     * which is what a context menu is for
     *
     * \note a mode answers false to the kinds which rewrite a whole buffer and
     * are asked for on a key of their own: a menu opened on a word is not where
     * a person asks for the file to be reformatted, and a server which offers
     * them on every range — `ruff` answers its `source` actions wherever the
     * menu was opened (measured) — would fill every context menu of the
     * language with entries which say nothing about the word under the pointer
     */
    virtual bool codeActionOffered(const LspCodeAction &action) const;
    /*!
     * \return the editor widget, for a derived session which reports or asks what
     * the generic code knows nothing of
     */
    EditorWidget &editorWidget();
    /*!
     * \return the document of the buffer this session asks about, null when the
     * buffer which holds it is gone
     */
    QTextDocument *documentOfBuffer() const;

   private slots:
    /*!
     * \brief the server published diagnostics: draw them when they are the ones
     * of the file this buffer visits
     */
    void updateDiagnostics(const QString &);

   private:
    //! the editor widget, which holds the client of the server
    EditorWidget &editor;
    //! the buffer whose text is asked about, which a rename comes back to
    Buffer &buffer;
    //! the text edit whose text is asked about and drawn on
    TextEditBase &textEdit;
    //! the server this session speaks with, shared by every buffer of the editor
    LspClient *lsp_client;
    /*!
     * \brief the name of the server: the name the minibuffer messages give it,
     * and the key of the diagnostics this session draws
     */
    QString server_name;
    //! the key of the underline of the tracked word under the cursor
    QString navigation_key;
    //! the key of the faces of the identifiers of the file
    QString tokens_key;
    //! the object name of the menu of the references of a symbol
    QString references_name;
    //! the file the document of this buffer is known as, empty with no file
    QString documentFile;
    /*!
     * \brief the document handed to the server, null once the buffer which holds
     * it is gone
     *
     * \note a `QPointer`, not a reference: the destructor of the session runs
     * after the Qt widget of the text edit is gone, see the note there
     */
    QPointer<QTextDocument> document;
    /*!
     * \brief what the server answers about one of the tracked words: the word the
     * text cursor sits on, which gains an underline, and the word the mouse rests
     * on, which gains the icon of the jump as its mouse cursor
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
      /*!\brief where the server says the word is defined, empty when nothing was
       * found: the places a \c Ctrl-Left click on the word jumps to, which is why
       * the answers are kept and not only counted
       */
      QVector<LspLocation> definitions;
      //! the same, for the declarations of the word
      QVector<LspLocation> declarations;
    };
    //! the word under the text cursor, as far as it was resolved
    TrackedWord cursor_word;
    //! the word under the mouse, as far as it was resolved
    TrackedWord hovered_word;
    //! the viewport position the mouse last visited
    QPoint hovered_point;
    /*!
     * \brief what was last echoed of a diagnostic, message and start of its range
     * glued; empty when nothing stands under the cursor to be told
     */
    QString echoed_diagnostic;
    //! the timer after which a settled position is asked about
    QTimer *navigation_timer = nullptr;
    /*!
     * \brief the timer after which a typed trigger character asks for the
     * signature of the call it belongs to
     *
     * Its own, and not the navigation one: a settling cursor asks about a word,
     * which is a different question from the one a typed `(` asks, and the two
     * answers belong in different places — an underline, and the minibuffer
     */
    QTimer *signature_timer = nullptr;
    /*!\brief counts the signature questions, so that the answer of one which is
     * still on its way cannot overwrite the message of a later one
     */
    int signature_request = 0;
    //! true while the word under the cursor is to be asked for
    bool cursor_tracking = false;
    //! true while the word under the mouse is to be asked for
    bool hover_tracking = false;
    /*!
     * \brief the timer after which the identifiers of the file are asked what they
     * are, the semantic tokens of the protocol
     *
     * Its own, and much longer than the two others: the question is about the
     * whole file, not about a word, so it is worth a pause the size of a
     * keystroke, and the answer is the color of the file rather than anything the
     * person typing waits to be told
     */
    QTimer *tokens_timer = nullptr;
    /*!\brief counts the questions for the tokens of the file, so that the answer
     * of one which is still on its way cannot color the buffer with what the
     * server said of a text which is no longer on screen
     */
    int tokens_request = 0;
    //! the completer the completions of the server are shown with, none until asked
    QCompleter *lspCompleter = nullptr;
    //! true while the popup of that completer is what the \c Tab key is for
    bool lspCompletionActive = false;
    /*!
     * \brief true while the press of the mouse is one the session answered with a
     * jump, which is what makes the release of that click go with the press
     */
    bool mouse_press_consumed = false;
    /*!
     * \brief underline the diagnostics the server last published, as the
     * contribution of this session to the extra selections of the text edit
     */
    void applyDiagnostics();
    /*!
     * \brief ask what the identifiers of the file are and color them with the
     * answer, the semantic tokens of the protocol
     *
     * \note silent on every path: this is the editor coloring a buffer on its own,
     * not answering a question somebody asked, and a minibuffer which talks about
     * the colors of the text would talk over everything else
     */
    void refreshSemanticTokens();
    /*!
     * \brief draw the given tokens on the buffer
     * \param[in] tokens: what the server said the identifiers are
     * \param[in] sequence: which question this is the answer of
     * \param[in] revision: the revision the question was asked at, which the
     * document has to still be at for the answer to describe its text
     */
    void applySemanticTokens(const QVector<LspSemanticToken> &,
                             int sequence,
                             qint64 revision);
    /*!
     * \brief forget the tracked words, the underline and the mouse cursor:
     * another file, or another session, knows nothing of them
     */
    void resetNavigation();
    /*!
     * \brief ask the new name of the symbol under the cursor in the minibuffer,
     * and hand it to \c renameToAt once it is answered
     */
    void renameSymbol();
    /*!
     * \brief apply the edits of a workspace edit to the documents they name,
     * opening the files which are not visited yet and coming back to the buffer
     * of this session afterwards
     * \return the number of edits applied
     *
     * \note what a rename answers, what a code action carries and what a format
     * answers are the same `WorkspaceEdit` of the protocol, so the same code
     * applies all three, and a quick fix which touches three files is applied to
     * three files exactly as a rename would be
     *
     * \note an edit whose range is the whole document — which is what a rename by
     * \c jedi and the formatting of a file are — is narrowed to the characters
     * which actually differ first: the text is the same either way, and applying
     * it as it comes throws the cursor to the end of the file
     */
    int applyWorkspaceEdit(const QMap<QString, QVector<LspTextEdit>> &);
    //! ask about the tracked words, once the position has settled
    void refreshTrackedWords();
    /*!
     * \brief ask whether the word of one of the two tracked positions is defined
     * or declared somewhere
     * \param[in] track_cursor: ask for the word under the text cursor, else for
     * the one under the mouse
     */
    void askTrackedWord(bool track_cursor);
    //! the underline of the word under the cursor, once it is resolved
    void applyTrackedCursor();
    //! the mouse cursor over the word it rests on, once it is resolved
    void applyTrackedHover();
    /*!\brief jump to the definition of the symbol at the given position of the
     * document, or to its declaration when nothing defines it, as the \c Ctrl-Left
     * click of the mouse does
     * \param[in] position: where the click was pressed, in the document
     * \return true when the cursor went somewhere, false when the symbol has
     * nowhere to go, in which case the click is a plain click
     *
     * \note the two questions are asked at once and waited for together, as the
     * context menu asks its two, and the answers the tracking of the mouse already
     * holds are used without asking again: the click which follows the icon of the
     * jump is the common case and shall not wait on a server which is busy parsing
     * the file
     */
    bool jumpFromMouse(const int position);
    /*!\brief echo the message of the diagnostic the cursor stands on, if any
     *
     * \c report is called only when the diagnostic under the cursor differs from
     * the one last echoed, so a settling timer restarted by editing in the middle
     * of a reported line does not repeat the message
     */
    void echoDiagnosticAtCursor();
    /*!\return the message of the diagnostic whose range visits the given document
     * position, empty when none does; the text the minibuffer and the tooltip show,
     * in the same words
     */
    QString diagnosticAt(const QTextDocument &, int position) const;
    /*!
     * \brief watch the mouse moves, the mouse leaving the text and the release of
     * a click the session answered; installed on the viewport of the text edit by
     * the constructor
     */
    bool eventFilter(QObject *const, QEvent *const) override;
    /*!
     * \return true when requests can be sent, waiting for the handshake of a
     * session which is running when it is not over yet, and saying in the
     * minibuffer where to ask for a session when none is running
     */
    bool serverIsReachable();
    /*!\return the message of a question asked of a server which is up but has
     * not answered, in the name the minibuffer gives that server
     */
    QString notAnswering() const;
    //! ask the server for the completions at the cursor, synchronously
    void completeAtCursor();
    //! ask the server where the symbol under the cursor is defined
    void jumpToDefinition();
    /*!\brief ask the server for every use of the symbol under the cursor and pop a
     * menu of them to jump to, the references list of the protocol
     */
    void showReferences();
    /*!\brief ask the server what call the cursor is inside of and echo its
     * parameter list in the minibuffer, the active argument picked out of it
     * \param[in] asked_by_user: false when a typed character raised the question,
     * which stays silent when the server sees no call there rather than reporting
     * one on every keystroke
     */
    void showSignatureHelp(const bool asked_by_user);
    /*!\brief place the cursor at the place a server names, opening the file which
     * holds it when it is not visited yet
     */
    void jumpTo(const LspLocation &location);
    //! ask the server what stands behind the symbol under the cursor
    void showHover();
  };  // end of struct LspNavigationSession

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPNAVIGATIONSESSION_HXX */
