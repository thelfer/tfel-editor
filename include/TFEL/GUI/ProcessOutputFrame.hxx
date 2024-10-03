/*!
 * \file  ProcessOutputFrame.hxx
 * \brief
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
     */
    ProcessOutputFrame(EditorWidget&, Buffer&);
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

    virtual void displayProcessOutput();

    virtual void processFinished(int, QProcess::ExitStatus);

   protected:
    //! the underlying process
    QProcess* process;

   private:
    Q_OBJECT

  };  // end of ProcessOutputFrame

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PROCESSOUTPUTFRAME_HXX */
