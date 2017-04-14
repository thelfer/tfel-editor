/*! 
 * \file  QEmacsGrepCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSGREPCOMMAND_H_
#define _LIB_QEMACS_QEMACSGREPCOMMAND_H_ 

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
    
    virtual void
    execute(void) override;
    
    ~QEmacsGrepCommand();

  }; // end of QEmacsGrepCommand

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSGREPCOMMAND_H */

