/*! 
 * \file  QEmacsMajorModeChangeCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_QEMACS_QEMACSMAJORMODECHANGECOMMAND_HXX
#define LIB_QEMACS_QEMACSMAJORMODECHANGECOMMAND_HXX 

#include"QEmacs/QEmacsCommand.hxx"

namespace qemacs
{

  /*!
   * A base class for command which action is to change the major mode
   * of the current main frame
   */
  struct QEmacsMajorModeChangeCommand
    : public QEmacsCommand
  {
    
    QEmacsMajorModeChangeCommand(QEmacsWidget&,
				 const QString&);
    
    void execute() override;
    
    ~QEmacsMajorModeChangeCommand() override;

  protected:

    //! major mode name
    const QString& mode;

  }; // end of QEmacsMajorModeChangeCommand

  struct QEmacsMajorModeChangeCommandProxy
    : public QEmacsCommandProxy
  {
    /*!
     * \param[in] c : command name
     * \param[in] m : mode name
     */
    QEmacsMajorModeChangeCommandProxy(const QString&,
				      const QString&);
    QString getName() const override;
    QEmacsCommand* getQEmacsCommand(QEmacsWidget&) const override;
    /*!
     * destructor
     */
    ~QEmacsMajorModeChangeCommandProxy() override;
  protected:
    const QString name;
    const QString mode;
  }; // end of struct QEmacsCommandProxy


} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSMAJORMODECHANGECOMMAND_HXX */

