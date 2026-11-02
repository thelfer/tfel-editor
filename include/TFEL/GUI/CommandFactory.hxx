/*!
 * \file  CommandFactory.hxx
 * \brief factory and proxies which build the commands available to the user.
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_COMMANDFACTORY_HXX
#define LIB_TFEL_GUI_COMMANDFACTORY_HXX

#include <map>
#include <memory>
#include <QtCore/QString>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/Command.hxx"

namespace tfel::gui {

  struct EditorWidget;

  /*!
   * \brief abstract class which knows how to build the commands of a given
   * name. The proxies are registered in the factory, which is what makes a
   * command available to the user
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CommandProxy {
    //! \return the name the command is registered under
    virtual QString getName() const = 0;
    /*!
     * \return a new instance of the command, built for the given editor
     * \param[in] w : editor widget the command is run on
     *
     * \note the caller takes the ownership of the returned command
     */
    virtual Command* getCommand(EditorWidget&) const = 0;
    /*!
     * destructor
     */
    virtual ~CommandProxy();
  };  // end of struct CommandProxy

  /*!
   * a standard implementation of a proxy
   * \param T : type of the command the proxy builds. It must be constructible
   * from the editor widget the command is run on
   */
  template <typename T>
  struct StandardCommandProxy : public CommandProxy {
    /*!
     * \param[in] n : name of the major mode
     * \param[in] b : if true, register a copy of this proxy in the command
     * factory. The constructor of a proxy which is itself a registration copy,
     * as the one the factory holds, must pass false not to register itself
     * again
     *
     * \note a command is usually registered by a static instance of this
     * template, defined at the bottom of the source file which defines the
     * command, so that loading the library which holds it is enough to make the
     * command known to the editor
     */
    StandardCommandProxy(const QString&, const bool = true);
    //! \return the name of the command
    QString getName() const override;
    //! \return a new instance of command
    Command* getCommand(EditorWidget&) const override;
    /*!
     * destructor
     */
    ~StandardCommandProxy() override;

   private:
    //! command name
    const QString name;
  };  // end of struct StandardCommandProxy

  //! \brief editor command factory
  struct TFEL_GUI_VISIBILITY_EXPORT CommandFactory {
    //! a simple alias
    using CommandPtr = std::shared_ptr<Command>;
    //! a simple alias
    using CommandProxyPtr = std::shared_ptr<CommandProxy>;

    /*!
     * \return the unique instance of the factory
     *
     * \note the instance is created on the first call and destroyed when the
     * program ends
     */
    static CommandFactory& getCommandFactory();

    /*!
     * \brief load the given library, whose static initialisers register the
     * commands it defines
     * \param[in] lib : name of the library
     *
     * \note the search of the library is left to the TFEL external library
     * manager
     */
    void loadLibrary(const QString&);

    /*!
     * \return a command according to the given name or NULL if no
     * command is found
     *
     * \param[in] n : name \param[in] w : editor
     * widget
     *
     * \note an unknown name is reported in the minibuffer of \c w. The caller
     * takes the ownership of the returned command
     */
    Command* getCommand(const QString&, EditorWidget&) const;

    /*!
     * \brief register a command proxy
     * \param[in] proxy : proxy which builds the command
     *
     * \note the factory keeps the proxy alive, through a shared pointer, for as
     * long as it lives. A proxy whose name is already used is not registered;
     * the clash is reported with \c qDebug only
     */
    void addCommand(const CommandProxyPtr);

    /*!
     * \return the names of all the registered commands, sorted by name
     */
    QStringList getAvailableCommandsNames() const;

    /*!
     * \return true if a command of the given name has been registered
     * \param[in] n : command name
     */
    bool hasCommand(const QString&) const;

   private:
    /*!
     * \brief builds an empty factory
     *
     * \note only `getCommandFactory` may build the single factory of the
     * application: the commands are registered by the static initialisers of
     * the `StandardCommandProxy` objects the commands declare.
     */
    CommandFactory();

    //! \brief destroys the factory and the registered proxies
    ~CommandFactory();

    //! registered commands, sorted by name
    std::map<QString, CommandProxyPtr> proxies;

  };  // end of struct CommandFactory

}  // end of namespace tfel::gui

#include "TFEL/GUI/CommandFactory.ixx"

#endif /* LIB_TFEL_GUI_COMMANDFACTORY_HXX */
