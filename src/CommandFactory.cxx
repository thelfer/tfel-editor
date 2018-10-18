/*!
 * \file  CommandFactory.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include <utility>
#include <stdexcept>

#include <QtCore/QDebug>

#include "TFEL/System/ExternalLibraryManager.hxx"

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/CommandFactory.hxx"

namespace tfel{

  namespace gui{

  CommandProxy::~CommandProxy() = default;

  void CommandFactory::loadLibrary(const QString& lib) {
    using namespace tfel::system;
    auto& lm =
        ExternalLibraryManager::getExternalLibraryManager();
    lm.loadLibrary(lib.toStdString());
  }

  Command* CommandFactory::getCommand(
      const QString& n, EditorWidget& w) const {
    auto p = this->proxies.find(n);
    if (p == this->proxies.end()) {
      w.displayInformativeMessage(
          QObject::tr("no major mode named '%1' registred.").arg(n));
      return nullptr;
    }
    return p->second->getCommand(w);
  }  // end of CommandFactory::getCommandByName

  bool CommandFactory::hasCommand(const QString& n) const {
    return this->proxies.find(n) != this->proxies.end();
  }  // end of CommandFactory::hasCommandByExtension

  void CommandFactory::addCommand(
      const CommandFactory::CommandProxyPtr proxy) {
    using namespace std;
    const QString& name = proxy->getName();
    QStringList::const_iterator ps;
    if (this->proxies.find(name) != this->proxies.end()) {
      qDebug()
          << "CommandFactory::addCommand : "
          << QObject::tr(
                 "a command named '%1' has already been registred.")
                 .arg(name);
      return;
    }
    this->proxies.insert({name, proxy});
  }  // end of CommandFactory::getCommand

  QStringList CommandFactory::getAvailableCommandsNames()
      const {
    QStringList keys;
    for (const auto& p : this->proxies) {
      keys.append(p.first);
    }
    return keys;
  }  // end of CommandFactory::getAvailableCommandsNames()
     // const

  CommandFactory&
  CommandFactory::getCommandFactory() {
    static CommandFactory m;
    return m;
  }  // end of CommandFactory::~CommandFactory()

  CommandFactory::CommandFactory() = default;

  CommandFactory::~CommandFactory() = default;

}  // end of namespace gui
}// end of namespace tfel
