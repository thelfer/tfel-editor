/*! 
 * \file  QEmacsCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#ifndef LIB_QEMACS_QEMACSCOMMAND_HXX
#define LIB_QEMACS_QEMACSCOMMAND_HXX 

#include<QtCore/QObject>
#include"QEmacs/Config.hxx"

namespace qemacs {

  //! forward declaration
  struct QEmacsWidget;

  //! \brief abstract class for all qemacs commands
  struct QEMACS_VISIBILITY_EXPORT QEmacsCommand : public QObject {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    QEmacsCommand(QEmacsWidget&);
    //! execute the command
    virtual void execute() = 0;
    //! destructor
    virtual ~QEmacsCommand();

   private:
    Q_OBJECT
  }; // end of class QEmacsCommand

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSCOMMAND_HXX */
