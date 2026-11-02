/*!
 * \file  LicosStudy.hxx
 * \brief LicosStudy: drives a licos study through an xlicos-client process.
 * \author Thomas Helfer
 * \date   20/06/2012
 */

#ifndef LIB_TFEL_GUI_LICOSSTUDY_HXX
#define LIB_TFEL_GUI_LICOSSTUDY_HXX

#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtWidgets/QPlainTextEdit>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include "TFEL/GUI/LicosStudyOptions.hxx"

namespace tfel::gui {

  /*!
   * each study is associated to a specific thread
   *
   * A study drives an `xlicos-client` sub-process through a pair of local
   * sockets: it sends the options, the input file and the orders the client
   * asks for and reads back the stages the computation reaches.
   *
   * \note the object lives in the thread which runs the study: it is created
   * on the stack of `LicosStudyThread::run`, so it and all its slots are
   * invoked from that worker thread, never from the GUI thread
   */
  struct LicosStudy : public QObject {
    /*!
     * \brief build the study of the given input file
     * \param [in] f : input file
     * \param [in] o : options
     * \param [in] a : command line arguments
     *
     * \note the server, the first socket and the process are created as
     * children of this object; the output socket stays null until the client
     * has connected
     *
     * \note `a` is only stored in `args`: `run` always starts
     * `xlicos-client` with the single argument `licos`
     */
    LicosStudy(const QString &, const LicosStudyOptions &, const QStringList &);
    /*!
     * \brief run the study: launch the client and connect the slots which
     * drive it to its end
     *
     * This makes the local server `licos.in` listen, then starts
     * `xlicos-client` in the directory of the input file. The study itself is
     * driven by the slots connected here, so this function returns as soon as
     * the process has been launched.
     *
     * \note when the server cannot be created, the study fails immediately
     * and the `finished` signal is emitted
     */
    void run();
    /*!
     * \return true when the study has reached its end without error
     *
     * \note the value is only meaningful once the `finished` signal has been
     * emitted: the constructor leaves it unset
     */
    bool succeed() const;
    /*!
     * \return the reason why the study failed, followed by the standard error
     * of the client process, or an empty string when no failure is recorded
     */
    QString getErrorMessage() const;

    /*!
     * \brief stop the computations of the study
     *
     * \note the sockets, the process and the server are released by Qt as
     * children of this object
     */
    ~LicosStudy() override;

   public slots:

    /*!
     * \brief disconnect the sockets and stop the client process
     *
     * The client is asked to end, is given 300 ms to do so and is terminated
     * otherwise. The socket file the client has created is removed and the
     * local server is closed.
     *
     * \note called by the destructor and through the `killProcess` signal of
     * `LicosStudyThread`, which is emitted from the GUI thread and delivered
     * here by the event loop of this thread
     *
     * \note the `finished` signal is not emitted
     */
    void stopComputations();

   signals:

    /*!
     * \brief emitted once the study is over, successfully or not
     *
     * \note emitted by `quit` only; the outcome is read back with `succeed`
     * and `getErrorMessage`
     */
    void finished();

    /*!
     * \brief emitted when the computation reaches a new time step
     * \param[in] s : completion of the computation, in percent, as sent by
     * the client; `100` is sent when the study is over
     */
    void newPeriod(int);

    /*!
     * \brief emitted with a new chunk of the standard output of the client
     * \param[in] o : text read from the process since the last emission
     */
    void newProcessOutput(QString);

   private slots:

    /*!
     * \brief handle the first connection of the client: complete the hand
     * shake, then send the options and the input file
     *
     * Connected to the `newConnection` signal of the local server and
     * disconnected at once, so it is run only once. The pending connection
     * becomes the output socket, the input socket is connected to the server
     * the client has just named and the study is configured with the
     * `initialize`, `option` and `inputFile` messages.
     *
     * \note the study fails if the input socket cannot be connected or if the
     * client does not answer `ok`
     */
    void processInitialised();

    /*!
     * \brief read the stage the client has just reached and answer it
     *
     * Connected to the `readyRead` signal of the input socket, disconnected
     * while the message is handled and reconnected at the end, except when
     * the study is over, so a single activation reads a single stage.
     * `readInputFile` is answered by `run`, `newPeriod` by `continue` and
     * `finished` by `end`, which ends the study; `fails` carries the error
     * message of the client.
     *
     * \note an unknown stage makes the study fail
     */
    void processReachedNextStage();

    /*!
     * \brief report an error of the client process as a failed study
     * \param[in] e : kind of error reported by `QProcess`
     *
     * \note only the kind of error is used; the error string of the process
     * is never part of the message
     */
    void processError(QProcess::ProcessError);

    /*!
     * \brief record the end of the client process
     * \param[in] s : exit code of the process; a non-zero code fails the
     * study. The exit status which follows is ignored
     *
     * \note a zero exit code emits nothing: the end of a study is reported
     * through the socket, see `processReachedNextStage`
     */
    void processFinished(const int, QProcess::ExitStatus);

    //! report an error of the input socket, see `displaySocketError`
    void displayInputSocketError(QLocalSocket::LocalSocketError);

    //! report an error of the output socket, see `displaySocketError`
    void displayOutputSocketError(QLocalSocket::LocalSocketError);

    /*!
     * \brief forward the standard output of the client to the
     * `newProcessOutput` signal; connected to its `readyReadStandardOutput`
     * signal
     */
    void displayProcessOutput();

   private:
    /*!
     * \brief turn an error of the given socket into a failed study
     * \param[in] s : socket the error comes from, used for its error string
     * only
     * \param[in] e : kind of error
     *
     * \note a socket timeout is ignored; the other cases report a fixed
     * message which still evokes a host and a fortune server
     */
    void displaySocketError(QLocalSocket *, QLocalSocket::LocalSocketError);

    /*!
     * \brief record the given reason as the failure of the study and end it
     * \param[in] msg : reason of the failure
     *
     * \note the standard error of the process, when it is not empty, is
     * appended to the stored message; the `finished` signal is emitted by
     * `quit`
     */
    void fails(const QString &);

    /*!
     * \brief send an option to the client as three messages: `option`, the
     * name and the value
     * \param[in] o : name of the option, as understood by the client
     * \param[in] v : value of the option
     */
    void sendOption(const QString &, const QString &);

    /*!
     * \brief send a message to the client, prefixed by its size on two bytes
     * \param[in] msg : message to send
     *
     * \note the message is flushed immediately; the call writes to `out` and
     * is thus only valid once the client has connected
     */
    void send(const QString &);

    //! build a `QString` from the given C string and send it, see `send`
    void send(const char *const);

    /*!
     * \brief end the study: drop the connections to the sockets and to the
     * process, then emit the `finished` signal
     *
     * \note neither the sockets nor the client process are stopped here
     */
    void quit();

    /*!
     * \brief read a size-prefixed message sent by the client
     * \return the received message
     *
     * \note the call blocks the calling thread, polling the socket every
     * 10 ms, until at least one byte is available
     *
     * \note a truncated message is reported through `fails`, yet what has
     * been read is returned all the same
     *
     * \see the specialisation for `QString`, the only one provided
     */
    template <typename T>
    T receive();

    //! local server `licos.in`, the socket the client writes to
    QLocalServer *server;

    /*!
     * \brief socket connected to the `licos.out` server of the client: the
     * messages this object reads
     *
     * \note it is created by the constructor but the connection is only
     * established in `processInitialised`
     */
    QLocalSocket *in;

    /*!
     * \brief socket accepted on `server`: the messages this object writes
     *
     * \note it is null until the client has connected, so no message may be
     * sent before `processInitialised`
     */
    QLocalSocket *out;

    //! unused: never assigned nor read, the output is forwarded by a signal
    QPlainTextEdit *output;

    //! the `xlicos-client` sub-process the study communicates with
    QProcess *process;

    //! path of the input file, sent to the client after the options
    QString inputFile;

    //! command line arguments; stored only, `run` never uses them
    QStringList args;

    /*!
     * \brief name of the socket file the client has created, received during
     * the hand shake
     *
     * \note it is removed from the file system by `stopComputations`
     */
    QString inServer;

    //! reason of the failure, empty as long as the study has not failed
    QString errorMessage;

    //! options of the study, sent to the client at the beginning
    LicosStudyOptions options;

    /*!
     * \brief outcome of the study
     *
     * \note `volatile` as it is written while the event loop of the thread
     * runs; the constructor leaves it unset, so it is only meaningful once
     * the `finished` signal has been emitted
     */
    volatile bool success;

    Q_OBJECT

  };  // end of LicosStudy

  /*!
   * \brief the only specialisation of `receive`: it reads a message from the
   * input socket; defined in `LicosStudy.cxx`
   */
  template <>
  QString LicosStudy::receive<QString>();

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSSTUDY_H */
