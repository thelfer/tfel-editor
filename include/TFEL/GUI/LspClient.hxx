/*!
 * \file  LspClient.hxx
 * \brief client of a language server: the handshake, the documents, the
 *        diagnostics, completion, definition, declaration, references,
 *        rename, code actions, signature help, hover and the semantic tokens
 *        of a file.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPCLIENT_HXX
#define LIB_TFEL_GUI_LSPCLIENT_HXX

#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QChar>
#include <functional>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/LspProtocol.hxx"

// forward declarations
class QTextDocument;

namespace tfel::gui {

  struct EditorWidget;
  struct LspConnection;
  struct LspTransport;

  /*!
   * \brief a language server and the conversation the editor holds with it
   *
   * \note one client serves one editor widget, see \c get: several C++
   * buffers talk to a single server, which is what a server with a shared
   * index expects. The client is a child of the editor widget it serves and
   * dies with it
   *
   * \note the client speaks for \c clangd by default, which is what \c get
   * returns and what the C++ mode drives; a client used for another server
   * says so with \c setServerName — the name the minibuffer messages speak
   * of, and the object name which keeps the two kinds of clients of one
   * editor apart — and answers the questions of the handshake with its own
   * \c setLanguageId and \c setConfiguration
   *
   * \note the documents are synchronized incrementally: the client keeps the
   * text it last sent for each of them and turns every change into the range
   * of that snapshot the change replaces. A server which is not ready yet
   * gets the whole text of a document once the handshake completes
   *
   * \note a server which says it synchronizes its documents with the whole
   * text of each — `textDocumentSync: 1`, which is what `harper-ls` answers
   * — is sent the whole text on every change, whatever the diff: the range of
   * a change means nothing to a server which reads the text it is given as
   * the text of the document
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspClient : QObject {
    Q_OBJECT
   public:
    /*!
     * \brief constructor
     * \param[in] w : editor widget to report to, may be null; a client without
     * one is silent, which is what a test wants
     * \param[in] p : parent object
     */
    explicit LspClient(EditorWidget *const w = nullptr,
                       QObject *const p = nullptr);
    /*!
     * \brief destructor: shuts the server down gracefully, then terminates it
     * if it lingers
     */
    ~LspClient() override;
    /*!
     * \return the client of \c clangd held by the given editor widget,
     * creating it as a child of that widget on the first call
     *
     * \note a client of another server is not the answer of this function,
     * even when it is a child of the same widget: it names itself with
     * \c setServerName, and the search here is by that name. The clients of
     * the grammar checkers are held by their \c GrammarCheckSession and
     * reached through it
     */
    static LspClient &get(EditorWidget &);
    /*!
     * \brief say what server this client speaks with: the name the
     * minibuffer messages of the conversation name, and the object name the
     * widget finds it by
     * \param[in] name : `clangd`, which a client of \c get speaks with, while
     * nobody says otherwise
     */
    void setServerName(const QString &name);
    /*!
     * \brief say what the files handed to the server are, in the sense of the
     * `languageId` of the protocol, which is what decides how a server parses
     * them: `cpp`, which is what a C++ mode sends, while nobody says
     * otherwise
     */
    void setLanguageId(const QString &id);
    /*!
     * \brief answer the settings the server asks for with \c settings
     * \param[in] settings : what a `workspace/configuration` question gets
     * for every scope it names; the settings of the server itself, in the
     * shape it reads them — `harper-ls` wants an object whose one key is its
     * own name
     *
     * \note a server which asks gets these settings on every analysis it
     * makes, so changing them is enough for the next change of a document to
     * be read under the new ones; \c recheckDocuments is what makes a server
     * look at the documents it already holds again, and a server which is not
     * up to answer yet reads them at the handshake
     */
    void setConfiguration(const QJsonValue &settings);
    /*!
     * \brief send the whole text of the documents on every change, whatever
     * the diff
     * \param[in] on : true for a server which reads the text it is given as the
     * text of the document and understands no range; false, the default, for a
     * server which applies the ranges of the changes it is handed
     *
     * \note the handshake says this by itself — `textDocumentSync: 1` —, and a
     * server which says so needs nothing here; it is for a server which is
     * silent about it, `harper-ls` answering no `textDocumentSync` at all, and
     * it stays set across the sessions of one client
     */
    void setFullTextSync(bool on);
    /*!
     * \brief tell the server that its settings changed and send it the whole
     * text of every document it holds again, so that what it says of them
     * says what it holds now
     *
     * \note the way to apply a change of the \c configuration under a running
     * session: the documents do not change of their own accord, and a server
     * which analyzes on changes alone would keep saying what it said under
     * the settings of the text
     */
    void recheckDocuments();
    //! \return true once the handshake with the server is complete
    bool isReady() const;
    /*!
     * \return true when no server has been launched: nothing asked for one.
     * The client never launches a server on its own: launching is a decision
     * of the user, see \c launch, because the options a session runs with, in
     * particular the compilation database, are not the editor's to guess
     */
    bool isIdle() const;
    /*!
     * \brief launch the server and run the handshake, shutting the session
     * which is still running down first, if any; returns as soon as the server
     * is started, the \c ready signal says when the handshake is over
     * \param[in] extra_arguments : arguments appended to those of the settings
     * \param[out] error : why the server could not be launched
     * \return false when the server can not be launched at all
     *
     * \note the documents the editor holds are re-opened on the new session
     * once its handshake is over, and the diagnostics of the previous one are
     * forgotten: they describe a server which has seen the text of the buffers
     * under other options, or not at all
     */
    bool launch(const QStringList &extra_arguments, QString &error);
    /*!
     * \brief launch a given program with the given arguments and run the
     * handshake, for a client which speaks with a server other than `clangd`
     * \param[in] program : the program to run, never looked for by this client
     * \param[in] arguments : the arguments to run it with, as the caller spells
     * them: a server of the editor takes them from the settings, see
     * \c LspHarperSupport::serverArguments
     * \param[out] error : why the program could not be launched
     * \return false when it can not be started at all
     *
     * \note what this client does with the session is the same either way: the
     * documents are re-opened once the handshake is over, and the diagnostics
     * of the previous session are forgotten
     */
    bool launch(const QString &program,
                const QStringList &arguments,
                QString &error);
    /*!
     * \brief end the session which is running, if any: the server is asked to
     * shut down and loses the argument if it lingers
     * \return false when no session was running
     *
     * \note the client goes back to the state of a session which was never
     * launched: the diagnostics of the ended session are forgotten and the
     * documents the editor holds are marked as never sent, so a later \c launch
     * re-sends their whole text, as it does when it replaces a session
     *
     * \note a session which exited of its own accord has nothing left to
     * dismiss: stopping it cleans the client up as well, but is reported as
     * not running
     */
    bool stop();
    /*!
     * \brief wait, at most \c timeout_ms, for the handshake of a server
     * launched with \c launch to complete
     * \return true when the client is ready, false when no server was launched
     * or when none is answering
     */
    bool waitUntilReady(int timeout_ms);
    /*!
     * \brief tell the server about a document; calling it again for the same
     * file, as the several frames of a buffer do, only counts the user up
     *
     * \note the changes are followed on `QTextDocument::contentsChange`, which
     * Qt only emits for a document attached to a text edit widget: a bare
     * document edited through a cursor stays silent. Every document of the
     * editor is attached to a `TextEditBase`, so the synchronization sees every
     * change; a caller handing over a detached document gets the text of the
     * moment of the `open`, and of each request, which syncs once more
     *
     * \note a document of a file which is not on disk is opened under its
     * buffer name all the same: the protocol allows a document the file
     * system does not have
     */
    void openDocument(const QString &file, QTextDocument *);
    /*!
     * \brief release a document opened with \c openDocument; the server is
     * told when the last user is gone
     */
    void closeDocument(const QString &file, QTextDocument *);
    //! \return true if a document is open for the given file
    bool isDocumentOpen(const QString &file) const;
    /*!
     * \return the text the server is supposed to hold for the document: the
     * snapshot the client last sent
     */
    QString snapshotOf(const QString &file) const;
    /*!
     * \brief the labels and the details of the completions at a position; the
     * labels are what to insert, the details what to show next to them
     */
    using CompletionReply = std::function<void(const QStringList &labels,
                                               const QStringList &details)>;
    /*!
     * \brief ask for the completions at a position of a document; the reply
     * comes on the answer, or at once, empty, when the question can not be
     * asked
     */
    void requestCompletion(const QString &file,
                           const QTextDocument &,
                           int position,
                           const CompletionReply &);
    //! the places a position points to
    using DefinitionReply = std::function<void(const QVector<LspLocation> &)>;
    //! ask for the definitions of the symbol at a position
    void requestDefinition(const QString &file,
                           const QTextDocument &,
                           int position,
                           const DefinitionReply &);
    /*!
     * \brief ask for the declarations of the symbol at a position: \c clangd
     * answers the prototype, which is not the definition of a function whose
     * body lives elsewhere
     */
    void requestDeclaration(const QString &file,
                            const QTextDocument &,
                            int position,
                            const DefinitionReply &);
    /*!
     * \brief ask for every use of the symbol at a position, its declaration
     * included; the answer can be long, which is what a list to choose from
     * is made of
     */
    void requestReferences(const QString &file,
                           const QTextDocument &,
                           int position,
                           const DefinitionReply &);
    /*!
     * \brief the files a rename changes and the edits of each; an empty map
     * says nothing changed, \c error then tells whether the server refused
     * the name or simply had nothing to do
     */
    using RenameReply =
        std::function<void(const QMap<QString, QVector<LspTextEdit>> &edits,
                           const QString &error)>;
    /*!
     * \brief ask to rename the symbol at a position; the answer is a whole
     * workspace edit, since a symbol is used in files the cursor is not on
     */
    void requestRename(const QString &file,
                       const QTextDocument &,
                       int position,
                       const QString &newName,
                       const RenameReply &);
    /*!
     * \brief the signatures a call at a position can be, the one the server
     * thinks the user means, and which of its arguments is being typed; all
     * three are empty or zero when the server sees no call there
     */
    using SignatureHelpReply =
        std::function<void(const QVector<LspSignature> &signatures,
                           int active_signature,
                           int active_parameter)>;
    /*!
     * \brief ask what call the cursor is inside of, which is what typing an
     * argument deserves to be told
     */
    void requestSignatureHelp(const QString &file,
                              const QTextDocument &,
                              int position,
                              const SignatureHelpReply &);
    /*!
     * \return the characters the server declares a call is being typed
     * around, which is where the mode asks \c requestSignatureHelp without
     * being asked for it
     *
     * \note the answer of the handshake, or `(` and `,` when the server
     * declares no list at all: a server which declares nothing still answers
     * the question, it just leaves the client to choose its moments
     */
    QVector<QChar> signatureHelpTriggers() const;
    /*!
     * \brief the actions a server offers on a range: the quick fixes of the
     * diagnostics published there, the refactors of the code there
     */
    using CodeActionsReply =
        std::function<void(const QVector<LspCodeAction> &)>;
    /*!
     * \brief ask what can be done about the code at a range
     * \param[in] file: the file the range is in
     * \param[in] doc: the document the range is in, to turn the offsets into
     * positions of the protocol
     * \param[in] position: first offset of the range
     * \param[in] end: past the last offset of the range; the same as
     * \c position when the question is about the cursor, not about a selection
     *
     * \note the diagnostics the client holds for the file are handed to the
     * question: `clangd` answers the fixes of the diagnostics it is *given*,
     * not those it happens to have computed, and returns nothing at all when
     * the context is empty
     */
    void requestCodeActions(const QString &file,
                            const QTextDocument &,
                            int position,
                            int end,
                            const CodeActionsReply &);
    /*!
     * \brief the overview of a file: the symbols it holds, in the order the
     * file spells them
     */
    using DocumentSymbolsReply =
        std::function<void(const QVector<LspDocumentSymbol> &)>;
    /*!\brief ask for the overview of a file
     * \param[in] file: the file to look at
     * \param[in] reply: what to do with the answer
     *
     * \note this is the one question of this client which takes no position:
     * the answer is the whole file, and the mode asks it once and keeps the
     * list while the document stays unchanged
     */
    void requestDocumentSymbols(const QString &file,
                                const DocumentSymbolsReply &);
    /*!
     * \brief the identifiers of a file and what the server says they are: the
     * semantic tokens of the protocol, which is what colors a name a regular
     * expression can only guess at
     */
    using SemanticTokensReply =
        std::function<void(const QVector<LspSemanticToken> &)>;
    /*!\brief ask what the identifiers of a file are, to the server's knowledge
     * \param[in] file: the file to look at
     * \param[in] reply: what to do with the answer
     *
     * \note the whole file, since a server which supports the question at all
     * is `clangd`, and `clangd` supports no range of it: what comes back is
     * every identifier it resolved, in the order of the file
     *
     * \note the numbers of the answer are named by the legend the server sent
     * with its capabilities, so the reply holds names, not numbers, and an
     * empty one says either that the server announced no legend, or that it
     * resolved nothing
     */
    void requestSemanticTokens(const QString &file,
                               const SemanticTokensReply &);
    /*!
     * \return true when the server announced semantic tokens with a legend to
     * read them by, which is what \c requestSemanticTokens needs
     */
    bool hasSemanticTokens() const;
    /*!
     * \return the names the legend of the server gives the kinds of tokens it
     * answers, empty when it announced none
     */
    QStringList semanticTokenKinds() const;
    /*!
     * \brief the text behind a position, rendered as plain text or as
     * markdown, empty when there is nothing to show
     */
    using HoverReply =
        std::function<void(const QString &text, bool is_markdown)>;
    //! ask for the hover information of a position
    void requestHover(const QString &file,
                      const QTextDocument &,
                      int position,
                      const HoverReply &);
    //! \return the diagnostics the server last published for a file
    QVector<LspDiagnostic> diagnosticsFor(const QString &file) const;
   signals:
    //! the handshake is over, the server answers
    void ready();
    //! the server went away
    void stopped();
    //! the server published new diagnostics for a file
    void diagnosticsUpdated(const QString &file);

   private slots:
    void handleNotification(const QString &, const QJsonValue &);

   private:
    //! a document held by the server
    struct Document {
      //! the document, null once the buffer it belongs to is gone
      QPointer<QTextDocument> doc;
      //! the text the server holds
      QString text;
      //! version sent with the last change; 0 while the server has never
      //! been told about the document at all
      int version = 0;
      //! how many users asked for this document
      int refs = 0;
    };
    void handleInitializeAnswer(const QJsonValue &, const QString &);
    /*!
     * \brief the body of \c requestDefinition, \c requestDeclaration and
     * \c requestReferences, which differ by the method they call and by the
     * few parameters the method needs beyond position and document
     */
    void requestLocations(const QString &method,
                          const QString &file,
                          const QTextDocument &,
                          int position,
                          const DefinitionReply &,
                          const QJsonObject &extra = QJsonObject());
    void publishDiagnostics(const QJsonValue &);
    void showInformativeMessage(const QString &);
    /*!
     * \brief ask the running server, if any, to shut down and give it the
     * grace period of the transport to do so; used when a session is replaced
     * and when the client dies
     */
    void endSession();
    //! forget the diagnostics published by the session which is ending
    void clearDiagnostics();
    //! send the whole text of every document which is not yet opened
    void flushDocuments();
    //! turn what the document holds now into a change of the snapshot
    void syncDocument(const QString &file);
    //! the editor widget to report to, may be null
    EditorWidget *editor;
    //! true once the destructor runs: the editor may be dying with us, and
    //! nothing may be displayed on it anymore
    bool shutting_down = false;
    /*!
     * \brief true while the client ends a session of its own accord, at which
     * point the exit of the process is neither an error nor news
     */
    bool ending_session = false;
    //! the server process
    LspTransport *transport;
    //! the conversation with it
    LspConnection *connection;
    //! the states of the handshake
    enum class State { Idle, Starting, Ready, Dead };
    //! where the client is with the server
    State state;
    //! the documents the server knows about, by file name
    QMap<QString, Document> documents;
    //! the last diagnostics published, by file name
    QMap<QString, QVector<LspDiagnostic>> diagnostics;
    /*!
     * \brief the characters the server says a call is typed around, from the
     * handshake; `(` and `,` while no handshake has been answered
     */
    QVector<QChar> signature_triggers{QLatin1Char('('), QLatin1Char(',')};
    /*!
     * \brief the legend the server announced with its capabilities: the names
     * the numbers of a semantic tokens answer are read against, invalid when
     * none was announced
     */
    LspSemanticLegend semantic_legend;
    /*!
     * \brief what server this client speaks with: the name the messages of the
     * conversation give it, and the object name \c get searches by
     */
    QString server_name{QLatin1String("clangd")};
    /*!
     * \brief the `languageId` the documents are opened under, which is what
     * decides how the server parses them
     */
    QString language_id{QLatin1String("cpp")};
    /*!
     * \brief what a `workspace/configuration` question is answered with, as the
     * server reads it; undefined for a server which is asked no settings
     */
    QJsonValue configuration;
    /*!
     * \brief true when the server says it synchronizes its documents with their
     * whole text, which is what is sent on every change then
     */
    bool full_text_sync = false;
  };  // end of struct LspClient

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPCLIENT_HXX */
