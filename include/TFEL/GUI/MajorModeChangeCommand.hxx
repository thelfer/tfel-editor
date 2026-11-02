/*!
 * \file  MajorModeChangeCommand.hxx
 * \brief base class of the commands which change the major mode.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODECHANGECOMMAND_HXX
#define LIB_TFEL_GUI_MAJORMODECHANGECOMMAND_HXX

#include "TFEL/GUI/Command.hxx"

namespace tfel::gui {

  /*!
   * A base class for command which action is to change the major mode
   * of the current main frame
   */
  struct MajorModeChangeCommand : public Command {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the command is run on, it is also the parent
     * of the command
     * \param[in] m : name of the major mode to switch to
     */
    MajorModeChangeCommand(EditorWidget&, const QString&);

    /*!
     * \brief set the major mode of the main frame of the current buffer
     *
     * \note nothing happens when the parent of the command is not an
     * \c EditorWidget. The mode is looked up by name: a name no proxy
     * registers is reported in the minibuffer and leaves the mode unchanged
     */
    void execute() override;
    //! \brief destructor
    ~MajorModeChangeCommand() override;

   protected:
    /*!
     * \brief major mode name
     *
     * \note a reference, not a copy: the string it designates is the one the
     * proxy which built this command holds and must outlive the command
     */
    const QString& mode;

  };  // end of MajorModeChangeCommand

  /*!
   * \brief a proxy which builds the command switching to a given major mode
   */
  struct MajorModeChangeCommandProxy : public CommandProxy {
    /*!
     * \param[in] c : command name
     * \param[in] m : mode name
     */
    MajorModeChangeCommandProxy(const QString&, const QString&);
    //! \return the name under which the command is registered
    QString getName() const override;
    /*!
     * \return a new instance of the command which switches to the mode of this
     * proxy
     * \param[in] w : editor widget the command is run on
     *
     * \note the caller takes ownership of the command
     */
    Command* getCommand(EditorWidget&) const override;
    //! \brief destructor
    ~MajorModeChangeCommandProxy() override;

   protected:
    //! command name, such as "cxx-mode"
    const QString name;
    //! name of the major mode the command switches to
    const QString mode;
  };  // end of struct CommandProxy

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAJORMODECHANGECOMMAND_HXX */
