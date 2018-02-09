/*! 
 * \file  ProcessInteractionFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \date 01/08/2012
 */

#ifndef LIB_QEMACS_PROCESSINTERACTIONFRAME_HXX
#define LIB_QEMACS_PROCESSINTERACTIONFRAME_HXX 

#include<QtCore/QProcess>

#include"QEmacs/Config.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"

namespace qemacs {

  /*!
   * \brief an helper class to interact with a QProcess
   * and displaying its outpout within a QEmacsPlainText
   */
 struct QEMACS_VISIBILITY_EXPORT ProcessInteractionFrame
    : public ProcessOutputFrame
  {
    /*!
     * \param[in] w : the qemacs widget
     * \param[in] b : the buffer
     */
    ProcessInteractionFrame(QEmacsWidget&, QEmacsBuffer&);
    //! destructor
    ~ProcessInteractionFrame();

   private:
    Q_OBJECT
  }; // end of ProcessInteractionFrame

} // end of namespace qemacs

#endif /* LIB_QEMACS_PROCESSINTERACTIONFRAME_HXX */

