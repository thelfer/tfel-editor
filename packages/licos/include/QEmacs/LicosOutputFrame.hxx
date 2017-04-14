/*! 
 * \file  LicosOutputFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#ifndef _LIB_LICOSOUTPUTFRAME_H_
#define _LIB_LICOSOUTPUTFRAME_H_ 

#ifdef QEMACS_QT4
#include<QtGui/QProgressBar>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QProgressBar>
#endif /* QEMACS_QT5 */

#include"QEmacs/LicosStudyOptions.hxx"

namespace qemacs
{
  
  class QEmacsWidget;

  class QEmacsBuffer;

  class QEmacsPlainTextEdit;

  class LicosStudyThread;

  /*!
   * Frame displaying the licos output
   */
  class LicosOutputFrame
    : public QWidget
  {
     Q_OBJECT

  public:

    LicosOutputFrame(QEmacsWidget& w,
		     QEmacsBuffer& b,
		     const QString&,
		     const LicosStudyOptions&);

  protected:

    void closeEvent(QCloseEvent *);

  signals:
      
    void finished(bool,QString);

  private slots:

    void studyFinished(bool,QString);

    void newPeriod(int);

    void displayProcessOutput(QString);

  private:

    QEmacsPlainTextEdit *textEdit;
    QProgressBar        *pbar;
    LicosStudyThread    *licos;

  }; // end of struct LicosOutputFrame

} // end of namespace qemacs

#endif /* _LIB_LICOSOUTPUTFRAME_H */

