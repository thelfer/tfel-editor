/*!
 * \file  LicosOutputFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \date   01/08/2012
 */

#ifndef LIB_LICOSOUTPUTFRAME_HXX
#define LIB_LICOSOUTPUTFRAME_HXX

#include <QtWidgets/QProgressBar>
#include "TFEL/GUI/LicosStudyOptions.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  // forward declaration
  struct PlainTextEdit;
  // forward declaration
  struct LicosStudyThread;

  /*!
   * Frame displaying the licos output
   */
  struct LicosOutputFrame : public QWidget {
    LicosOutputFrame(EditorWidget &w,
                     Buffer &b,
                     const QString &,
                     const LicosStudyOptions &);

   protected:
    void closeEvent(QCloseEvent *) override;

   signals:

    void finished(bool, QString);

   private slots:

    void studyFinished(bool, QString);

    void newPeriod(int);

    void displayProcessOutput(QString);

   private:
    PlainTextEdit *textEdit;
    QProgressBar *pbar;
    LicosStudyThread *licos;

    Q_OBJECT
  };  // end of struct LicosOutputFrame

}  // end of namespace tfel::gui

#endif /* LIB_LICOSOUTPUTFRAME_H */
