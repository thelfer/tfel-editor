/*! 
 * \file  ProcessOutputFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#ifndef _LIB_QEMACS_PROCESSOUTPUTFRAME_H_
#define _LIB_QEMACS_PROCESSOUTPUTFRAME_H_ 

#include<QtCore/QProcess>

#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"

namespace qemacs
{

  /*!
   * \brief an helper class to display the output of a QProcess
   * within a read-only QEmacsTextEdit
   */
  class QEMACS_VISIBILITY_EXPORT ProcessOutputFrame
    : public QEmacsPlainTextEdit
  {
    Q_OBJECT

  public:

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
    virtual void 
    setReadOnly(bool) override;
    /*!
     * \return the underlying process
     */
    QProcess&
    getProcess();

    //! destructor
    ~ProcessOutputFrame();

  protected slots:

    virtual void
    displayProcessOutput();

    virtual void
    processFinished(int,QProcess::ExitStatus);

  protected:

    //! the underlying process
    QProcess *process;

  }; // end of ProcessOutputFrame

} // end of namespace qemacs

#endif /* _LIB_QEMACS_PROCESSOUTPUTFRAME_H */

