/*! 
 * \file  QEmacsGrepCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#ifndef LIB_QEMACS_QEMACSGREPCOMMAND_HXX
#define LIB_QEMACS_QEMACSGREPCOMMAND_HXX 

#include"QEmacs/QEmacsCommand.hxx"

namespace qemacs
{

  /*!
   * A command based on the grep utility
   */
  struct QEmacsGrepCommand
    : public QEmacsCommand
  {
    
    QEmacsGrepCommand(QEmacsWidget&);
    
    void execute() override;
    
    ~QEmacsGrepCommand() override;

  }; // end of QEmacsGrepCommand

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSGREPCOMMAND_HXX */

