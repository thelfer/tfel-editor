/*!
 * \file  LicosStudyThread.hxx
 * \brief LicosStudyThread: runs a licos study in a worker thread.
 * \author Thomas Helfer
 * \date   20/05/2012
 */

#ifndef LIB_TFEL_GUI_LICOSSTUDYTHREAD_HXX
#define LIB_TFEL_GUI_LICOSSTUDYTHREAD_HXX

#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtWidgets/QPlainTextEdit>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include "TFEL/GUI/LicosStudyOptions.hxx"

namespace tfel::gui {

  // forward declaration
  struct LicosStudy;
  // forward declaration
  struct LicosOutputFrame;

  /*!
   * each studyThread is associated to a specific thread
   *
   * It runs a `LicosStudy` in a worker thread and forwards its signals to the
   * GUI thread, which is the only other user of this class: the study itself
   * is never exposed.
   *
   * \note the thread runs an event loop while the study is running, which is
   * what delivers the orders sent through `killProcess` and the signals the
   * study emits
   */
  struct LicosStudyThread : public QThread {
    friend struct LicosOutputFrame;

    /*!
     * \brief build the thread of the study of the given input file; the study
     * is only created when the thread is started
     * \param [in] f : input file
     * \param [in] o : study options
     * \param [in] a : command line arguments
     * \param [in] p : parent (qt sense)
     *
     * \note neither `a` nor the study options are used before `run`: they are
     * copies owned by this object, so the caller may modify the originals
     */
    LicosStudyThread(const QString &,
                     const LicosStudyOptions &,
                     const QStringList &,
                     LicosOutputFrame *const);
    /*!
     * \brief run the study: create it, connect its signals to the forwarding
     * slots of this thread and run its own event loop until it is over
     *
     * \note this is the body of the worker thread: call `start` instead of
     * calling it directly, which would run the study in the calling thread
     *
     * \note the study lives on the stack of this function, so the connections
     * to it die as soon as it returns
     */
    void run() override;

    /*!
     * \return true if the study success, that is, if it reached its end
     * without error
     *
     * \note the value is a copy of the outcome of the study, taken when the
     * `finished` signal is emitted; it is left unset before that and may be
     * read from another thread than the one which runs the study
     */
    bool succeed() const;

    /*!
     * \return the error messsage if the study failed, an empty string
     * otherwise; as `succeed` does, it is only filled once the `finished`
     * signal has been emitted
     */
    QString getErrorMessage() const;

    /*!
     * \brief ask the running study to stop, without blocking the caller.
     *
     * The request is delivered through the thread's own event loop, so the
     * study is terminated asynchronously and the caller stays responsive.
     */
    void requestStop();

    /*!
     * \brief stop the study and wait for the thread to end when it is still
     * running, which blocks the calling thread until the study stops
     *
     * \note this is why `LicosOutputFrame` calls `requestStop` and defers the
     * deletion with `deleteLater` instead of destroying a running thread
     */
    ~LicosStudyThread() override;

   signals:

    /*!
     * \brief emitted when the study is over, from the thread which has run
     * it, so the connection to a GUI object is queued
     * \param[in] s : true when the study has reached its end without error
     * \param[in] e : reason of the failure, empty on success; it may hold
     * several lines, the error message of the client being appended
     *
     * \note this signal hides the homonymous `QThread` one, which carries no
     * argument
     */
    void finished(bool, QString);

    /*!
     * \brief forwarded from the study with each new chunk of the standard
     * output of the client process
     * \param[in] o : text emitted by the study
     */
    void newProcessOutput(QString);

    /*!
     * \brief forwarded from the study when a new time step is reached
     * \param[in] s : completion of the computation, in percent, `100` being
     * sent when the study is over
     */
    void newPeriod(int);

    /*!
     * \brief request the study to stop; emitted by `requestStop` and
     * connected, in `run`, to `LicosStudy::stopComputations`
     *
     * \note it is delivered by the event loop of this thread: emitting it from
     * the GUI thread is what makes the stop asynchronous
     */
    void killProcess();

   private slots:

    /*!
     * \brief record the outcome of the study, leave its event loop and emit
     * the `finished` signal with the very same values
     */
    void studyFinished();

    //! forward the output of the study through `newProcessOutput`
    void forwardProcessOutput(QString);

    //! forward the progress of the study through `newPeriod`
    void forwardNewPeriod(int);

   private:
    //! path of the input file of the study
    QString inputFile;

    //! command line arguments, given to the study, which ignores them
    QStringList args;

    //! reason of the failure, filled when the study is over
    QString errorMessage;

    //! options the study is run with
    LicosStudyOptions options;

    /*!
     * \brief the study running in this thread, owned by the stack of `run`
     *
     * \note it is null before the thread is started and dangles once `run`
     * has returned; only the slots called from that thread use it
     */
    LicosStudy *study;

    /*!
     * \brief outcome of the study, filled when the study is over; the
     * constructor leaves it unset
     */
    bool success;

    Q_OBJECT

  };  // end of LicosStudyThread

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSSTUDYTHREAD_H */
