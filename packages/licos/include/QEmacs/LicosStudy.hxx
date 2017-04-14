/*! 
 * \file  LicosStudy.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 20 mai 2012
 */

#ifndef _LIB_QEMACS_LICOSSTUDY_H_
#define _LIB_QEMACS_LICOSSTUDY_H_ 

#include<QtCore/QProcess>
#include<QtCore/QString>
#include<QtCore/QStringList>
#include<QtCore/QObject>

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

  /*!
   * each study is associated to a specific thread
   */
  class LicosStudy
    : public QObject
  {
    
    Q_OBJECT

  public:

    /*!
     * \param [in] f : input file
     * \param [in] o : options
     * \param [in] a : command line arguments
     */
    LicosStudy(const QString&,
	       const LicosStudyOptions&,
	       const QStringList&);

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

    ~LicosStudy();

  public slots:

    void stopComputations();

  signals:
      
    void finished(void);

    void newPeriod(int);

    void newProcessOutput(QString);

  private slots:

    void processInitialised(void);

    void processReachedNextStage(void);

    void processError(QProcess::ProcessError);

    void processFinished(const int,QProcess::ExitStatus);

    void displayInputSocketError(QLocalSocket::LocalSocketError);

    void displayOutputSocketError(QLocalSocket::LocalSocketError);

    void displayProcessOutput();

  private:

    void displaySocketError(QLocalSocket *,
			    QLocalSocket::LocalSocketError);
    
    void fails(const QString&);

    void sendOption(const QString&,
		    const QString&);

    void send(const QString&);

    void send(const char* const);

    void quit();

    template<typename T>
    T receive(void);

    QLocalServer *server;

    QLocalSocket *in;

    QLocalSocket *out;
    
    QPlainTextEdit *output;

    QProcess *process;

    QString inputFile;

    QStringList args;

    QString inServer;

    QString errorMessage;

    LicosStudyOptions options;

    volatile bool success;

  }; // end of LicosStudy

  template<>
  QString
  LicosStudy::receive<QString>(void);

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LICOSSTUDY_H */

