/*! 
 * \file  QEmacsCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsCommand.hxx"

namespace qemacs
{

  QEmacsCommand::QEmacsCommand(QEmacsWidget& t)
    : QObject(&t)
  {} // end of QEmacsCommand::QEmacsCommand

  QEmacsCommand::~QEmacsCommand()
  {}

} // end of namespace qemacs
