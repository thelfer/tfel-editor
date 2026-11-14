/*!
 * \file  LanguageToolClient.hxx
 * \brief client of the LanguageTool server: the Java process which serves the
 *        checks, the requests of the buffers it reads, the errors it answers.
 * \author Thomas Helfer
 * \date   13 nov. 2026
 */

#ifndef LIB_TFEL_GUI_LANGUAGETOOLCLIENT_HXX
#define LIB_TFEL_GUI_LANGUAGETOOLCLIENT_HXX

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/LspProtocol.hxx"

// forward declarations
class QProcess;
class QTimer;
class QTextDocument;

namespace tfel::gui {

  struct EditorWidget;
  struct TextEditBase;

  /*!
   * \brief a LanguageTool server and the buffers it reads
   *
   * \note one client serves one editor widget, the way the client of `clangd`
   * does: the server is a Java virtual machine with the rules of every language
   * it knows loaded in it, which costs a second to start and a first request of
   * two seconds, and two buffers of one editor reading two files of one
   * document have nothing to gain from two of them. The client is a child of the
   * editor widget it serves and dies with it, and it ends the process with it
   *
   * \note the process is started when the first buffer asks for it and ended when
   * the last one is gone: a virtual machine which serves nobody is a hundred
   * megabytes of nothing, and the person who turns the checker off in a menu
   * means it
   *
   * \note the port is taken from the setting `language-tool/port`, and a free one
   * is looked for when that setting says 0. The editor is the one starting the
   * process, so it knows the port; a server somebody else started is a server
   * this client has no business talking to, and the setting is where a person who
   * wants exactly that writes the number it serves on
   *
   * \note the documents are read whole, on a timer which waits for the typing to
   * stop: the API has no notion of a change, it is handed a text and answers what
   * is wrong with it, so the change of a character is a whole new request. A text
   * too long for one request is cut into pieces, see
   * \c LanguageToolSupport::sliceStarts
   *
   * \note the answers come as the `matches` of a JSON body, which say what this
   * client publishes as the diagnostics of the protocol: see
   * \c LanguageToolSupport::diagnosticsOf. The underlines are drawn by whoever
   * holds this client, see \c GrammarCheckSession
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LanguageToolClient : QObject {
    Q_OBJECT
    public:
    /*!
     * \brief constructor
     * \param[in] w : editor widget to report to, may be null; a client without
     * one is silent, which is what a test wants
     * \param[in] p : parent object
     */
    explicit LanguageToolClient(EditorWidget *const w = nullptr,
                                QObject *const p = nullptr);
    /*!
     * \brief destructor: ends the server process, which is a child of this
     * client and would otherwise outlive the editor it was started for
     */
    ~LanguageToolClient() override;
    /*!
     * \return the client of the LanguageTool server of the given editor widget,
     * creating it as a child of that widget on the first call
     *
     * \note the clients of the grammar checkers are held by their
     * \c GrammarCheckSession, which is where the choice of the checker lives; the
     * server this client drives is shared because a shared server costs one
     * virtual machine
     */
    static LanguageToolClient &get(EditorWidget &);
    /*!
     * \return the client of \c w when one was created, null else: what a session
     * which has no server to talk to reads before creating one, so that a buffer
     * which is read by another checker holds no network manager, no timer and no
     * expectation of a port
     */
    static LanguageToolClient *heldBy(EditorWidget &);
    /*!
     * \return true when both a Java runtime and a LanguageTool installation are
     * to be found, which is what decides whether the choice of this checker is
     * offered at all
     */
    static bool isAvailable();
    /*!
     * \return true when no server was started and no buffer asked for one
     */
    bool isIdle() const;
    /*!
     * \return true once the server answers its check request, which is after the
     * handshake of a started process and after the process is there at all
     */
    bool isReady() const;
    /*!
     * \brief make sure a server runs, starting one if none does, and count this
     * call as a user of it; returns as soon as the process is started, the
     * \c ready signal says when it answers
     * \param[out] error : why no server could be started
     * \return false when the program can not be started at all
     *
     * \note a second call while a server runs asks nothing of the process: the
     * buffers of an editor read the same server, and the second buffer of a
     * project costs no Java virtual machine
     */
    bool startSession(QString &error);
    /*!
     * \brief end the use of the server by one buffer; the process is ended when no
     * user is left
     * \return false when no server was running
     */
    bool stopSession();
    /*!
     * \brief tell the server about a document; calling it again for the same file
     * only counts the user up, the way \c LspClient::openDocument does
     * \param[in] file : name the document is known by
     * \param[in] doc : document whose text is read
     * \param[in] frame : text edit the document is drawn on, whose mode says what
     * of the text a proofreader has any business reading; null for a buffer which
     * has no mode to ask, whose whole text is then sent
     */
    void openDocument(const QString &file,
                      QTextDocument *doc,
                      TextEditBase *frame = nullptr);
    /*!
     * \brief release a document opened with \c openDocument; its underlines go
     * away with it, since nobody is reading them anymore
     */
    void closeDocument(const QString &file, QTextDocument *doc);
    //! \return true if a document is open for the given file
    bool isDocumentOpen(const QString &file) const;
    /*!
     * \brief read every document again, from its text as it is now, under the
     * settings which are held now
     *
     * \note this is how a change of the language, of the categories of rules or of
     * the installation reaches the buffers which are on the screen: the request
     * reads its settings when it is built, so the documents have to be handed
     * over again for the new answer to be of any use to a person who is looking
     * at the old one
     *
     * \note a server which runs under a program, a port or Java options which are
     * gone with the change is ended by it, and \c checkPendingDocuments starts the
     * one the settings name now when the documents ask for it. What the person who
     * changed a field of the dialog reads is therefore the answer of the program
     * the dialog names, wherever the buffers were opened
     */
    void recheckDocuments();
    //! \return the errors the server last answered for a file
    QVector<LspDiagnostic> diagnosticsFor(const QString &file) const;
    //! \return the port the server answers on, 0 while none was started
    int port() const;
   signals:
    //! the server answers
    void ready();
    //! the server went away
    void stopped();
    //! the server answered errors for a file
    void diagnosticsUpdated(const QString &file);

   private:
    //! a document the server reads
    struct Document {
      //! the document, null once the buffer it belongs to is gone
      QPointer<QTextDocument> doc;
      /*!
       * \brief the text edit the document is drawn on, null when none was given
       *
       * A \c QPointer: the frame outlives the mode it is read with and outlives
       * this client too when the editor dies, and it is the mode of *that* frame
       * which says what of the text is prose. What of it is read is asked at the
       * moment of the request, never kept, because the mode of a frame is replaced
       * under a person who changes the mode of a buffer without changing the
       * buffer
       */
      QPointer<TextEditBase> frame;
      //! how many users asked for this document
      int refs = 0;
      /*!
       * \brief true when the text of the document is not the one the server last
       * read, or when nothing was ever read of it
       */
      bool pending = true;
      /*!
       * \brief what of the document the server has been told about
       *
       * The text is kept because the answers are places in it, and a range of a
       * reply which says nothing about the text of the moment is a range of a text
       * which is gone. It is also what keeps a document from being sent again
       * because a selection changed in it
       */
      QString text;
      /*!
       * \brief the generation of the requests which are on their way
       *
       * Bumped every time a new set of requests is sent, which is what voids the
       * replies of a set the text of the document left behind: a person who typed
       * a paragraph while the server read the previous one has no use for the
       * answer about that one
       */
      quint64 generation = 0;
      //! how many replies of the generation above are still expected
      int outstanding = 0;
      //! the errors of the replies of the generation above, in the order of the text
      QVector<LspDiagnostic> incoming;
      //! the errors the last complete set of replies answered
      QVector<LspDiagnostic> diagnostics;
      //! why the last set of requests failed, empty when it did not
      QString error;
    };
    //! start the process, or say why it can not be started
    bool startServer(QString &error);
    //! end the process, whatever the state it is in
    void endServer();
    /*!
     * \brief ask a port which nothing is listening to
     * \return the port, or -1 when none could be bound
     *
     * \note the port is looked for by binding it: \c QTcpServer binds port 0, the
     * kernel gives it a free one, and the server is closed again before Java is
     * asked for that number. The window between the two is small and a race lost
     * in it shows itself as a server which never answers, which the timeout of the
     * wait reports
     */
    static int freePort();
    //! the place the readiness of a server is checked, and its checks made
    QString endpoint() const;
    //! the timer which waits for the typing to stop has expired: read the documents
    void checkPendingDocuments();
    //! hand a document to the server, in as many pieces as its text needs
    void checkDocument(const QString &file);
    //! what of a document the checker reads, in the sense the mode of the frame gives
    QString textOf(const Document &) const;
    /*!
     * \brief a reply came in: gather it, and publish when nothing is expected
     * anymore
     * \param[in] file : the file the request was about
     * \param[in] generation : the generation the request was sent at; a reply
     * of a set the text left behind is dropped, unanswered of a server which
     * is slow is not
     * \param[in] offset : where the piece which was sent stands in the text of
     * the document, which is 0 for a text short enough for one request
     * \param[in] reply : the answer to read and to delete
     */
    void handleReply(const QString &file,
                     quint64 generation,
                     int offset,
                     QNetworkReply *);
    //! report on the minibuffer, unless there is nobody to read it
    void showInformativeMessage(const QString &);
    //! the editor widget to report to, may be null
    EditorWidget *editor;
    //! true once the destructor runs: nothing may be displayed anymore
    bool shutting_down = false;
    //! true while this client ends the process of its own accord
    bool ending_session = false;
    /*!
     * \brief true once the reason why no server could be started was displayed
     *
     * A program which is not there is a mistake of the configuration, and every
     * buffer which asks for the checker would repeat the same line: the person
     * who wrote the path reads it once and fixes it in the dialog. The flag is
     * spent when a start succeeds, so a server which is configured later in the
     * session reports a fresh failure of its own
     */
    bool failure_reported = false;
    //! how many buffers asked for the server
    int users = 0;
    //! the states of the conversation
    enum class State { Idle, Starting, Ready };
    //! where this client is with the server
    State state = State::Idle;
    //! the Java process which serves the checks, null while none runs
    QProcess *server = nullptr;
    //! the port the server serves on, 0 while none was started
    int server_port = 0;
    //! the requests are made through it
    QNetworkAccessManager network;
    /*!
     * \brief the timer which waits for the typing to stop before reading a text
     *
     * A single timer for all the documents: a person types in one buffer at a
     * time, and the documents which changed while it typed are all read when it
     * stops
     */
    QTimer *check_timer = nullptr;
    /*!
     * \brief the timer which waits for a started process to answer
     *
     * The server prints its banner and binds its port in about a second, and the
     * rules of a language are loaded by the first request which names it, so a
     * client which sent its first check before the port was bound would report a
     * failure of a server which was fine
     */
    QTimer *ready_timer = nullptr;
    //! when the process was started, to bound the wait for it
    qint64 started_at = 0;
    /*!
     * \brief what the settings named when the process was started: see
     * \c serverSettings, which is what \c recheckDocuments compares the settings
     * of the moment to
     */
    QString launch_settings;
    //! the documents the server reads, by file name
    QMap<QString, Document> documents;
  };  // end of struct LanguageToolClient

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LANGUAGETOOLCLIENT_HXX */
