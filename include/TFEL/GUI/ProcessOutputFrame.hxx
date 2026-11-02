/*!
 * \file  ProcessOutputFrame.hxx
 * \brief frame which displays the output of a `QProcess`.
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#ifndef LIB_TFEL_GUI_PROCESSOUTPUTFRAME_HXX
#define LIB_TFEL_GUI_PROCESSOUTPUTFRAME_HXX

#include <QtCore/QProcess>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"

namespace tfel::gui {

  /*!
   * \brief an helper class to display the output of a QProcess
   * within a read-only TextEdit
   */
  struct TFEL_GUI_VISIBILITY_EXPORT ProcessOutputFrame : public PlainTextEdit {
    /*!
     * \param[in] w : the editor widget
     * \param[in] b : the buffer
     *
     * The frame is read-only, undo/redo is disabled and the two channels of the
     * process are merged, so that the output and the errors of the command stay
     * in the order they are written.
     *
     * \note the frame owns the process and connects it to
     * \c displayProcessOutput and \c processFinished
     */
    ProcessOutputFrame(EditorWidget&, Buffer&);
    /*!
     * This method is reimplemented so that this frame can't be
     * writtable
     *
     * \note the requested state is simply ignored: the frame is always
     * read-only
     */
    void setReadOnly(bool) override;
    /*!
     * \return the underlying process
     *
     * \note the frame owns it: it is a child of the frame and it is terminated
     * when the frame is destroyed. It is what the callers start, and what a
     * \c ProcessOutputMajorModeBase restarts
     */
    QProcess& getProcess();

    /*!
     * \brief destructor
     *
     * \note the running process is terminated, then killed if it does not stop,
     * and the \c finished signal is disconnected first so that the frame is not
     * used as a reply is being handled
     */
    ~ProcessOutputFrame() override;

   protected slots:

    /*!
     * \brief append everything the process has written so far to the text edit
     *
     * \note the slot is triggered by \c QProcess::readyReadStandardOutput. It
     * disconnects itself from the process, reads and displays the output, then
     * connects itself back
     */
    virtual void displayProcessOutput();

    /*!
     * \brief react to the end of the process
     * \param[in] s : exit code of the process, 0 when it succeeds
     * \param[in] es : exit status of the process
     *
     * The icon of the secondary task shows whether the command succeeded or
     * failed. When the mode of the frame is a \c ProcessOutputMajorModeBase,
     * which is what displays the output of a command, that mode is called back
     * so that it can act on the end of the process.
     *
     * \note this slot is connected to \c QProcess::finished by the constructor
     */
    virtual void processFinished(int, QProcess::ExitStatus);

   protected:
    //! the underlying process
    QProcess* process;

   private:
    Q_OBJECT

  };  // end of ProcessOutputFrame

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PROCESSOUTPUTFRAME_HXX */
