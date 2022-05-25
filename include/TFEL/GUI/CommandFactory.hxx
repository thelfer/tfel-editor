/*!
 * \file  CommandFactory.hxx
 * \brief
 * \author Helfer Thomas
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

  struct TFEL_GUI_VISIBILITY_EXPORT CommandProxy {
    virtual QString getName() const = 0;
    virtual Command* getCommand(EditorWidget&) const = 0;
    /*!
     * destructor
     */
    virtual ~CommandProxy();
  };  // end of struct CommandProxy

  /*!
   * a standard implementation of a proxy
   * \param T : major mode
   */
  template <typename T>
  struct StandardCommandProxy : public CommandProxy {
    /*!
     * \param[in] n : name of the major mode
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

    static CommandFactory& getCommandFactory();

    void loadLibrary(const QString&);

    /*!
     * \return a command according to the given name or NULL if no
     * command is found
     *
     * \param[in] n : name \param[in] w : editor
     * widget
     */
    Command* getCommand(const QString&, EditorWidget&) const;

    void addCommand(const CommandProxyPtr);

    QStringList getAvailableCommandsNames() const;

    bool hasCommand(const QString&) const;

   private:
    CommandFactory();

    ~CommandFactory();

    std::map<QString, CommandProxyPtr> proxies;

  };  // end of struct CommandFactory

}  // end of namespace tfel::gui

#include "TFEL/GUI/CommandFactory.ixx"

#endif /* LIB_TFEL_GUI_COMMANDFACTORY_HXX */
