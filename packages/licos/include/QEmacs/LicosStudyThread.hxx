/*! 
 * \file  LicosStudyThread.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 20 mai 2012
 */

#ifndef _LIB_QEMACS_LICOSSTUDYTHREAD_H_
#define _LIB_QEMACS_LICOSSTUDYTHREAD_H_ 

#include<QtCore/QProcess>
#include<QtCore/QString>
#include<QtCore/QStringList>
#include<QtCore/QThread>
#ifdef QEMACS_QT4
#include<QtGui/QPlainTextEdit>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QPlainTextEdit>
#endif /* QEMACS_QT5 */
#include<QtNetwork/QLocalServer>
#include<QtNetwork/QLocalSocket>

#include"QEmacs/LicosStudyOptions.hxx"

namespace qemacs
{

  class LicosStudy;

  class LicosOutputFrame;

  /*!
   * each studyThread is associated to a specific thread
   */
  class LicosStudyThread
    : public QThread
  {
    
    Q_OBJECT

    friend class LicosOutputFrame;

    /*!
     * \param [in] f : input file
     * \param [in] o : study options
     * \param [in] a : command line arguments
     * \param [in] p : parent (qt sense)
     */
    LicosStudyThread(const QString&,
		     const LicosStudyOptions&,
		     const QStringList&,
		     LicosOutputFrame * const);
    
    /*!
     * run the study
     */
    void run();

    /*!
     * return true if the study success
     */
    bool succeed() const;

    /*!
     * return the error messsage if the study failed
     */
    QString
    getErrorMessage(void) const;

    ~LicosStudyThread();
			
  signals:

    void finished(bool,QString);

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

  }; // end of LicosStudyThread

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LICOSSTUDYTHREAD_H */
