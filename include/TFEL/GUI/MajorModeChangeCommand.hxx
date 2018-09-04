/*! 
 * \file  MajorModeChangeCommand.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODECHANGECOMMAND_HXX
#define LIB_TFEL_GUI_MAJORMODECHANGECOMMAND_HXX 

#include"TFEL/GUI/Command.hxx"

namespace tfel{

  namespace gui
{

  /*!
   * A base class for command which action is to change the major mode
   * of the current main frame
   */
  struct MajorModeChangeCommand
    : public Command
  {
    
    MajorModeChangeCommand(EditorWidget&,
				 const QString&);
    
    void execute() override;
    
    ~MajorModeChangeCommand() override;

  protected:

    //! major mode name
    const QString& mode;

  }; // end of MajorModeChangeCommand

  struct MajorModeChangeCommandProxy
    : public CommandProxy
  {
    /*!
     * \param[in] c : command name
     * \param[in] m : mode name
     */
    MajorModeChangeCommandProxy(const QString&,
				      const QString&);
    QString getName() const override;
    Command* getCommand(EditorWidget&) const override;
    /*!
     * destructor
     */
    ~MajorModeChangeCommandProxy() override;
  protected:
    const QString name;
    const QString mode;
  }; // end of struct CommandProxy


} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MAJORMODECHANGECOMMAND_HXX */

