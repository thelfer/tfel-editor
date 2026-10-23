/*!
 * \file  LicosStudyThread.hxx
 * \brief
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
   */
  struct LicosStudyThread : public QThread {
    friend struct LicosOutputFrame;

    /*!
     * \param [in] f : input file
     * \param [in] o : study options
     * \param [in] a : command line arguments
     * \param [in] p : parent (qt sense)
     */
    LicosStudyThread(const QString &,
                     const LicosStudyOptions &,
                     const QStringList &,
                     LicosOutputFrame *const);
    /*!
     * run the study
     */
    void run() override;

    /*!
     * return true if the study success
     */
    bool succeed() const;

    /*!
     * return the error messsage if the study failed
     */
    QString getErrorMessage() const;

    /*!
     * \brief ask the running study to stop, without blocking the caller.
     *
     * The request is delivered through the thread's own event loop, so the
     * study is terminated asynchronously and the caller stays responsive.
     */
    void requestStop();

    ~LicosStudyThread() override;

   signals:

    void finished(bool, QString);

    void newProcessOutput(QString);

    void newPeriod(int);

    void killProcess();

   private slots:

    void studyFinished();

    void forwardProcessOutput(QString);

    void forwardNewPeriod(int);

   private:
    QString inputFile;

    QStringList args;

    QString errorMessage;

    LicosStudyOptions options;

    LicosStudy *study;

    bool success;

    Q_OBJECT

  };  // end of LicosStudyThread

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSSTUDYTHREAD_H */
