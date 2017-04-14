/*! 
 * \file  QEmacsCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSCOMMAND_H_
#define _LIB_QEMACS_QEMACSCOMMAND_H_ 

#include<QtCore/QObject>

namespace qemacs
{

  //! forward declaration
  class QEmacsWidget;

  /*!
   * Abstract class for all commands
   */
  class QEmacsCommand
    : public QObject
  {

    Q_OBJECT

  public:

    QEmacsCommand(QEmacsWidget&);

    /*!
     * execute the command
     */
    virtual void
    execute() = 0;

    /*!
     * destructor
     */
    virtual ~QEmacsCommand();

  }; // end of class QEmacsCommand

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSCOMMAND_H */

