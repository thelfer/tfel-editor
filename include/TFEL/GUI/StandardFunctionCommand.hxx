/*!
 * \file   StandardFunctionCommand.hxx
 * \brief
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_TFEL_GUI_STANDARDFUNCTIONCOMMAND_HXX
#define LIB_TFEL_GUI_STANDARDFUNCTIONCOMMAND_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/Command.hxx"
#include "TFEL/GUI/CommandFactory.hxx"

namespace tfel::gui {

  /*!
   * \brief a simple wrapper around a standard function
   * to easily create editor commands.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT StandardFunctionCommand : public Command {
    //! a simple alias
    using Fct = void (*)(EditorWidget&);
    /*!
     * \brief build a command from a function object
     * \tparam FunctionType: type of the function object
     * \param[in] w: editor widget (parent widget)
     * \param[in] f: function
     */
    StandardFunctionCommand(EditorWidget&, const Fct f);
    //! execute the command
    void execute() override;
    //! destructor
    ~StandardFunctionCommand() override;

   private:
    //! command
    const Fct c;
  };  // end of struct StandardFunctionCommand

  /*!
   * \brief a proxy for standard function
   * \tparam f: function pointer
   */
  template <void (*f)(EditorWidget&)>
  struct StandardFunctionCommandProxy : public CommandProxy {
    /*!
     * \param[in] n: name of the command
     * \param[in] b: if true, register the proxy
     */
    StandardFunctionCommandProxy(const QString&, const bool = true);
    //! \return the command name
    QString getName() const override;
    /*!
     * \return a new instance of the command associated with the
     * function.
     */
    Command* getCommand(EditorWidget&) const override;
    //! \brief destructor
    ~StandardFunctionCommandProxy() override;

   private:
    //! command name
    const QString name;
  };  // end of struct StandardFunctionCommandProxy

}  // end of namespace tfel::gui

#include "TFEL/GUI/StandardFunctionCommand.ixx"

#endif /* LIB_TFEL_GUI_STANDARDFUNCTIONCOMMAND_HXX */
