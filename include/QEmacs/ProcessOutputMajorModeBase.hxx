/*!
 * \file   ProcessOutputMajorModeBase.hxx
 * \brief
 * \author Thomas Helfer
 * \date   7/02/2018
 */

#ifndef LIB_QEMACS_PROCESSOUTPUTMAJORMODEBASE_HXX
#define LIB_QEMACS_PROCESSOUTPUTMAJORMODEBASE_HXX

#include <QtCore/QProcess>
#include "QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs {

  /*!
   * \brief A base class for major mode associated with external process
   * output.
   */
  struct ProcessOutputMajorModeBase : QEmacsMajorModeBase {
    //! constructor
    ProcessOutputMajorModeBase(QEmacsWidget&,
			       QEmacsBuffer&,
			       QEmacsTextEditBase&,
			       QWidget* const);
    /*!
     * \brief method called when the process is finished
     * \param[in] s: exit status
     * \param[in] e: exit status
     */
    virtual void processFinished(int, QProcess::ExitStatus) = 0;
    //! destructor
    ~ProcessOutputMajorModeBase() override;
  private:
    Q_OBJECT
  }; // end of struct ProcessOutputMajorModeBase

}  // end of namespace qemacs

#endif /* LIB_QEMACS_PROCESSOUTPUTMAJORMODEBASE_HXX */
