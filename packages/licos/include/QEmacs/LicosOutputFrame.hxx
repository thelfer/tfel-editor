/*! 
 * \file  LicosOutputFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#ifndef LIB_LICOSOUTPUTFRAME_HXX
#define LIB_LICOSOUTPUTFRAME_HXX 

#include<QtWidgets/QProgressBar>
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

    void closeEvent(QCloseEvent *) override;

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

#endif /* LIB_LICOSOUTPUTFRAME_H */

