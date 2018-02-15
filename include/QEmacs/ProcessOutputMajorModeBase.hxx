/*!
 * \file   ProcessOutputMajorModeBase.hxx
 * \brief
 * \author Thomas Helfer
 * \date   7/02/2018
 */

#ifndef LIB_QEMACS_PROCESSOUTPUTMAJORMODEBASE_HXX
#define LIB_QEMACS_PROCESSOUTPUTMAJORMODEBASE_HXX

#include <QtCore/QProcess>
#include "QEmacs/Config.hxx"
#include "QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs {

  /*!
   * \brief a base class for major mode associated with external process
   * output.
   */
  struct QEMACS_VISIBILITY_EXPORT ProcessOutputMajorModeBase
      : QEmacsMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: widget
     * \param[in] b: buffer
     * \param[in] t: text edit widget
     * \param[in] p: parent widget
     */
    ProcessOutputMajorModeBase(QEmacsWidget&,
                               QEmacsBuffer&,
                               QEmacsTextEditBase&,
                               QWidget* const);
    void setDirectory(const QString&);
    void setCommand(const QString&);
    void setArguments(const QStringList&);
    void setMajorMode(const QString&);
    /*!
     * \brief handle key event
     * \param[in] e: event
     */
    bool keyPressEvent(QKeyEvent* const) override;
    /*!
     * \brief method called when the process is finished
     * \param[in] s: exit status
     * \param[in] e: exit status
     */
    virtual void processFinished(int, QProcess::ExitStatus) = 0;
    //! destructor
    ~ProcessOutputMajorModeBase() override;

   protected:
    QString directory;
    QString command;
    QStringList arguments;
    QString mode;

   private:
    Q_OBJECT
  };  // end of struct ProcessOutputMajorModeBase

}  // end of namespace qemacs

#endif /* LIB_QEMACS_PROCESSOUTPUTMAJORMODEBASE_HXX */
