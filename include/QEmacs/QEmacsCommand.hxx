/*! 
 * \file  QEmacsCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#ifndef LIB_QEMACS_QEMACSCOMMAND_HXX
#define LIB_QEMACS_QEMACSCOMMAND_HXX 

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

#endif /* LIB_QEMACS_QEMACSCOMMAND_HXX */

