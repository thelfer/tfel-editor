/*!
 * \file  LicosOutputFrame.hxx
 * \brief LicosOutputFrame: frame showing the output of a licos study.
 * \author Thomas Helfer
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
    /*!
     * \brief constructor: build the frame and start the study at once
     * \param[in] w : editor widget the frame belongs to, which is its parent in
     * the Qt sense
     * \param[in] b : buffer the read only text edit of the frame is built for
     * \param[in] f : absolute path of the input file of the study
     * \param[in] options : options the study is run with
     *
     * \note the frame holds a read only text edit which shows the output of the
     * study and a progress bar whose maximum is one hundred
     *
     * \note the study runs in a \c LicosStudyThread which starts here and is a
     * child of this frame; the output, the progress and the end of the study
     * are forwarded to the slots below
     */
    LicosOutputFrame(EditorWidget &w,
                     Buffer &b,
                     const QString &,
                     const LicosStudyOptions &);

   protected:
    /*!
     * \brief ask the study to stop when the frame is closed
     * \param[in] e : close event, which is always accepted
     *
     * \note the thread is not waited for: `LicosStudyThread::requestStop` does
     * not block, so the study is terminated asynchronously and the user
     * interface stays responsive
     */
    void closeEvent(QCloseEvent *) override;

   signals:

    /*!
     * \brief emitted when the study is over
     * \param[in] s : true if the study succeeded
     * \param[in] e : error message of the study, empty on success
     *
     * \note \c LicosMajorMode::studyFinished is connected to this signal when
     * the frame is created
     */
    void finished(bool, QString);

   private slots:

    /*!
     * \brief show the outcome of the study in the text edit, hide the progress
     * bar, release the thread of the study and emit \c finished
     * \param[in] s : true if the study succeeded
     * \param[in] e : error message of the study, displayed line by line
     */
    void studyFinished(bool, QString);

    /*!
     * \brief show the progress of the study in the progress bar
     * \param[in] s : progress the study reports, the maximum of the bar being
     * one hundred
     */
    void newPeriod(int);

    /*!
     * \brief append the output of the study to the text edit
     * \param[in] o : text the study has just written
     */
    void displayProcessOutput(QString);

   private:
    //! read only text edit which shows the output of the study
    PlainTextEdit *textEdit;
    //! progress bar of the study, hidden once it is over
    QProgressBar *pbar;
    /*!
     * thread the study runs in, a child of this frame
     *
     * \note it is null once the study is over or the frame closed: the thread
     * is released with `deleteLater`, which does not null the pointer by itself
     */
    LicosStudyThread *licos;

    Q_OBJECT
  };  // end of struct LicosOutputFrame

}  // end of namespace tfel::gui

#endif /* LIB_LICOSOUTPUTFRAME_H */
