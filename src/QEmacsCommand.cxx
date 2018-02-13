/*! 
 * \file  QEmacsCommand.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsCommand.hxx"

namespace qemacs {

  QEmacsCommand::QEmacsCommand(QEmacsWidget& t)
    : QObject(&t)
  {} // end of QEmacsCommand::QEmacsCommand

  QEmacsCommand::~QEmacsCommand() = default;

} // end of namespace qemacs
