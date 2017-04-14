/*! 
 * \file  ProcessInteractionFrame.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#ifndef _LIB_QEMACS_PROCESSINTERACTIONFRAME_H_
#define _LIB_QEMACS_PROCESSINTERACTIONFRAME_H_ 

#include<QtCore/QProcess>

#include"QEmacs/Config.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"

namespace qemacs
{

  /*!
   * \brief an helper class to interact with a QProcess
   * and displaying its outpout within a QEmacsPlainText
   */
  class QEMACS_VISIBILITY_EXPORT ProcessInteractionFrame
    : public ProcessOutputFrame
  {

    Q_OBJECT

  public:

    /*!
     * \param[in] w : the qemacs widget
     * \param[in] b : the buffer
     */
    ProcessInteractionFrame(QEmacsWidget&,
			    QEmacsBuffer&);

  }; // end of ProcessInteractionFrame

} // end of namespace qemacs

#endif /* _LIB_QEMACS_PROCESSINTERACTIONFRAME_H */

