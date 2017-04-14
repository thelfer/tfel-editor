/*! 
 * \file  QEmacsMajorModeChangeCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSMAJORMODECHANGECOMMAND_H_
#define _LIB_QEMACS_QEMACSMAJORMODECHANGECOMMAND_H_ 

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
    
    virtual void
    execute(void) override;
    
    ~QEmacsMajorModeChangeCommand();

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
    virtual QString
    getName(void) const override;
    virtual QEmacsCommand*
    getQEmacsCommand(QEmacsWidget&) const override;
    /*!
     * destructor
     */
    virtual ~QEmacsMajorModeChangeCommandProxy();
  protected:
    const QString name;
    const QString mode;
  }; // end of struct QEmacsCommandProxy


} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSMAJORMODECHANGECOMMAND_H */

