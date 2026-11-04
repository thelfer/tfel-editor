/*!
 * \file  LspClient.hxx
 * \brief client of a language server: the handshake, the documents, the
 *        diagnostics, completion, definition and hover.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPCLIENT_HXX
#define LIB_TFEL_GUI_LSPCLIENT_HXX

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
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
   * \note the documents are synchronized incrementally: the client keeps the
   * text it last sent for each of them and turns every change into the range
   * of that snapshot the change replaces. A server which is not ready yet
   * gets the whole text of a document once the handshake completes
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
     * \return the client of the given editor widget, creating it as a child of
     * that widget on the first call
     */
    static LspClient &get(EditorWidget &);
    //! \return true once the handshake with the server is complete
    bool isReady() const;
    /*!
     * \brief start the server and run the handshake, if that has not been
     * done yet; returns as soon as the server is started, the \c ready signal
     * says when the handshake is over
     * \param[out] error : why the server could not be started
     * \return false when the server can not be started at all
     */
    bool ensureStarted(QString &error);
    /*!
     * \brief start the server if needed and wait, at most \c timeout_ms, for
     * the handshake to complete
     * \return true when the client is ready
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
    void publishDiagnostics(const QJsonValue &);
    void showInformativeMessage(const QString &);
    //! send the whole text of every document which is not yet opened
    void flushDocuments();
    //! turn what the document holds now into a change of the snapshot
    void syncDocument(const QString &file);
    //! the editor widget to report to, may be null
    EditorWidget *editor;
    //! true once the destructor runs: the editor may be dying with us, and
    //! nothing may be displayed on it anymore
    bool shutting_down = false;
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
  };  // end of struct LspClient

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPCLIENT_HXX */
