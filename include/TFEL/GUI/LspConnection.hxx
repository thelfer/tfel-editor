/*!
 * \file  LspConnection.hxx
 * \brief JSON-RPC requests, responses and notifications on top of a
 *        language server transport.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPCONNECTION_HXX
#define LIB_TFEL_GUI_LSPCONNECTION_HXX

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <functional>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  struct LspTransport;

  /*!
   * \brief the JSON-RPC conversation of a client with a language server, over
   * a transport
   *
   * \note the requests the server sends to the client are answered here and
   * never reach the client: a client which leaves them unanswered stalls the
   * server, and none of them is anything the editor has an opinion about yet
   * (`workspace/configuration` gets an array of nulls, that is the default
   * settings for every item asked for; `client/registerCapability` and
   * `window/workDoneProgress/create` get a null result, that is accepted
   * without anything being registered or tracked)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspConnection : QObject {
    Q_OBJECT
   public:
    /*!
     * \brief the answer to a request: the result it asked for, or a message
     * when it failed; exactly one of the two is meaningful
     */
    using Reply =
        std::function<void(const QJsonValue &result, const QString &error)>;
    /*!
     * \brief constructor
     * \param[in] t : transport the conversation is held on; the connection
     * does not own it
     * \param[in] p : parent object
     */
    explicit LspConnection(LspTransport &t, QObject *const p = nullptr);
    ~LspConnection() override;
    //! \return true while the transport runs
    bool isRunning() const;
    /*!
     * \brief send a request; the reply is called when the answer arrives, on
     * the thread of the connection
     *
     * \note when the server dies, the pending replies are dropped: there is
     * nobody left to answer them and the editor is told by \c stopped
     */
    void request(const QString &method,
                 const QJsonObject &params,
                 const Reply &);
    //! send a notification, which expects no answer
    void notify(const QString &method, const QJsonObject &params = {});
   signals:
    //! a notification the server sent and the connection does not handle
    void notification(const QString &method, const QJsonValue &params);
    //! the server went away; pending replies are dropped
    void stopped();

   private slots:
    void handleMessage(const QJsonObject &);

   private:
    //! answer a request the server sent
    void respond(const QJsonValue &id, const QJsonValue &result);
    //! the transport the conversation is held on
    LspTransport *transport;
    //! next request id
    int next_id;
    //! the replies waiting for their answer, by request id
    QMap<int, Reply> replies;
  };  // end of struct LspConnection

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPCONNECTION_HXX */
