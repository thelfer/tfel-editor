/*!
 * \file  LspConnection.cxx
 * \brief JSON-RPC requests, responses and notifications on top of a language
 *        server transport.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtCore/QJsonArray>

#include "TFEL/GUI/LspConnection.hxx"
#include "TFEL/GUI/LspTransport.hxx"

namespace tfel::gui {

  LspConnection::LspConnection(LspTransport &t, QObject *const p)
      : QObject(p), transport(&t), next_id(1) {
    QObject::connect(&t, &LspTransport::messageReceived, this,
                     &LspConnection::handleMessage);
    QObject::connect(&t, &LspTransport::finished, this, [this](const int) {
      this->replies.clear();
      emit stopped();
    });
  }  // end of LspConnection::LspConnection

  LspConnection::~LspConnection() = default;

  bool LspConnection::isRunning() const {
    return this->transport->isRunning();
  }  // end of LspConnection::isRunning

  void LspConnection::request(const QString &method,
                              const QJsonObject &params,
                              const Reply &reply) {
    if (!this->isRunning()) {
      reply(QJsonValue(), QStringLiteral("the language server is not running"));
      return;
    }
    const auto id = this->next_id++;
    this->replies.insert(id, reply);
    this->transport->send(QJsonObject{{"jsonrpc", "2.0"},
                                      {"id", id},
                                      {"method", method},
                                      {"params", params}});
  }  // end of LspConnection::request

  void LspConnection::notify(const QString &method, const QJsonObject &params) {
    if (!this->isRunning()) {
      return;
    }
    this->transport->send(QJsonObject{
        {"jsonrpc", "2.0"}, {"method", method}, {"params", params}});
  }  // end of LspConnection::notify

  void LspConnection::setConfigurationAnswer(const QJsonValue &answer) {
    this->configuration_answer = answer;
  }  // end of LspConnection::setConfigurationAnswer

  void LspConnection::handleMessage(const QJsonObject &m) {
    const auto id = m.value("id");
    if (m.contains("method")) {
      // a request or a notification the server sent: with an id it expects
      // an answer, without one it is a notification
      const auto method = m.value("method").toString();
      if (!id.isUndefined()) {
        if (method == QLatin1String("workspace/configuration")) {
          // one copy of what the client has to say per item asked for: while
          // nobody set an answer, that is an array of nulls, the default
          // settings of the server for every scope
          auto items = QJsonArray();
          const auto requested =
              m.value("params").toObject().value("items").toArray();
          for (const auto &i : requested) {
            items.append(this->configuration_answer);
          }
          respond(id, items);
        } else if ((method == QLatin1String("client/registerCapability")) ||
                   (method ==
                    QLatin1String("window/workDoneProgress/create"))) {
          // accepted, and nothing done about it: the editor does not track
          // the progress of the server
          respond(id, QJsonValue());
        } else {
          respond(id, QJsonValue());
        }
        return;
      }
      emit notification(method, m.value("params"));
      return;
    }
    if (!id.isUndefined()) {
      // a response to one of our requests; the id of the requests the server
      // sends lives in its own space, which is why a message is told apart by
      // the method it carries, never by its id
      const auto i = static_cast<int>(id.toDouble());
      const auto reply = this->replies.take(i);
      if (reply) {
        if (m.contains("error")) {
          const auto e = m.value("error").toObject();
          reply(QJsonValue(), e.value("message").toString());
        } else {
          reply(m.value("result"), QString());
        }
      }
    }
  }  // end of LspConnection::handleMessage

  void LspConnection::respond(const QJsonValue &id, const QJsonValue &result) {
    this->transport->send(
        QJsonObject{{"jsonrpc", "2.0"}, {"id", id}, {"result", result}});
  }  // end of LspConnection::respond

}  // end of namespace tfel::gui
