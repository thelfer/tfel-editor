/*! 
 * \file  ProcessOutputFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#ifndef LIB_QEMACS_PROCESSOUTPUTFRAME_HXX
#define LIB_QEMACS_PROCESSOUTPUTFRAME_HXX 

#include<QtCore/QProcess>

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"

namespace qemacs
{

  /*!
   * \brief an helper class to display the output of a QProcess
   * within a read-only QEmacsTextEdit
   */
  struct QEMACS_VISIBILITY_EXPORT ProcessOutputFrame
      : public QEmacsPlainTextEdit {
    /*!
     * \param[in] w : the qemacs widget
     * \param[in] b : the buffer
     */
    ProcessOutputFrame(QEmacsWidget&,
		       QEmacsBuffer&);
    /*!
     * This method is reimplemented so that this frame can't be
     * writtable     
     */
    void setReadOnly(bool) override;
    /*!
     * \return the underlying process
     */
    QProcess& getProcess();

    //! destructor
    ~ProcessOutputFrame() override;

  protected slots:

    virtual void
    displayProcessOutput();

    virtual void
    processFinished(int,QProcess::ExitStatus);

  protected:

    //! the underlying process
    QProcess *process;

   private:
    Q_OBJECT

  }; // end of ProcessOutputFrame

} // end of namespace qemacs

#endif /* LIB_QEMACS_PROCESSOUTPUTFRAME_HXX */

