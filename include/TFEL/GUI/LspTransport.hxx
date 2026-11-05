/*!
 * \file  LspTransport.hxx
 * \brief process of a language server and the framed byte stream it speaks
 *        on its standard input and output.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPTRANSPORT_HXX
#define LIB_TFEL_GUI_LSPTRANSPORT_HXX

#include <QtCore/QByteArray>
#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include "TFEL/GUI/Config.hxx"

// forward declaration
class QProcess;

namespace tfel::gui {

  /*!
   * \brief the standard input, standard output and standard error of a
   * language server process
   *
   * \note the messages of the standard output may arrive by pieces or glued
   * together; \c LspProtocol::decode reassembles them and keeps the partial
   * tail of the stream for the next read
   *
   * \note the standard error is a channel of its own: a language server logs
   * there, and those logs must not interleave with the protocol. They are
   * forwarded line by line, see \c serverMessage
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspTransport : QObject {
    Q_OBJECT
   public:
    /*!
     * \brief constructor
     * \param[in] p : parent object; the transport kills the server when it is
     * destroyed with one
     */
    explicit LspTransport(QObject *const p = nullptr);
    //! destructor: terminates the server, see \c terminate
    ~LspTransport() override;
    /*!
     * \brief start the server
     * \return false if the program can not be started
     */
    bool start(const QString &program, const QStringList &arguments);
    //! \return true while the process runs
    bool isRunning() const;
    /*!
     * \brief frame a message and hand it to the standard input of the server;
     * nothing happens when the process does not run
     */
    void send(const QJsonObject &);
    /*!
     * \brief ask the process to terminate, then kill it should it still be
     * alive after the grace period
     */
    void terminate(int grace_period_ms = 2000);
   signals:
    //! a complete message was read on the standard output
    void messageReceived(const QJsonObject &);
    //! a line of the log the server wrote on its standard error
    void serverMessage(const QString &);
    //! the process exited
    void finished(int exit_code);
    //! the process could not be started
    void failedToStart();

   private slots:
    void handleReadyReadStandardOutput();
    void handleReadyReadStandardError();
    void handleFinished(int, QProcess::ExitStatus);

   private:
    //! the server process
    QProcess *process;
    //! the bytes read which do not make a complete message yet
    QByteArray in_buffer;
    //! the partial line of the log the last read ended with
    QByteArray err_buffer;
  };  // end of struct LspTransport

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPTRANSPORT_HXX */
