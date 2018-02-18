/*!
 * \file   QEmacsStandardFunctionCommand.hxx
 * \brief
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_QEMACS_QEMACSSTANDARDFUNCTIONCOMMAND_HXX
#define LIB_QEMACS_QEMACSSTANDARDFUNCTIONCOMMAND_HXX

#include "QEmacs/Config.hxx"
#include "QEmacs/QEmacsCommand.hxx"
#include "QEmacs/QEmacsCommandFactory.hxx"

namespace qemacs {

  /*!
   * \brief a simple wrapper around a standard function
   * to easily create qemacs commands.
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsStandardFunctionCommand
      : public QEmacsCommand {
    //! a simple alias
    using Fct = void (*)(QEmacsWidget&);
    /*!
     * \brief build a command from a function object
     * \tparam FunctionType: type of the function object
     * \param[in] w: qemacs widget (parent widget)
     * \param[in] f: function
     */
    QEmacsStandardFunctionCommand(QEmacsWidget&, const Fct f);
    //! execute the command
    void execute() override;
    //! destructor
    ~QEmacsStandardFunctionCommand() override;

   private:
    //! command
    const Fct c;
  };  // end of struct QEmacsStandardFunctionCommand

  /*!
   * \brief a proxy for standard function
   * \tparam f: function pointer
   */
  template <void (*f)(QEmacsWidget&)>
  struct QEmacsStandardFunctionCommandProxy
      : public QEmacsCommandProxy {
    /*!
     * \param[in] n: name of the command
     * \param[in] b: if true, register the proxy
     */
    QEmacsStandardFunctionCommandProxy(const QString&, const bool = true);
    //! \return the command name
    QString getName() const override;
    /*!
     * \return a new instance of the command associated with the
     * function.
     */
    QEmacsCommand* getQEmacsCommand(QEmacsWidget&) const override;
    //! \brief destructor
    ~QEmacsStandardFunctionCommandProxy() override;

   private:
    //! command name
    const QString name;
  };  // end of struct QEmacsStandardFunctionCommandProxy

}  // end of namespace qemacs

#include"QEmacs/QEmacsStandardFunctionCommand.ixx"

#endif /* LIB_QEMACS_QEMACSSTANDARDFUNCTIONCOMMAND_HXX */
