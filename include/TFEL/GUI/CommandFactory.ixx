/*! 
 * \file  CommandFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_COMMANDFACTORY_IXX
#define LIB_TFEL_GUI_COMMANDFACTORY_IXX

#include <map>
#include <QtCore/QString>
#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/System/ExternalLibraryManager.hxx"

namespace tfel{

  namespace gui{

  template <typename T>
  StandardCommandProxy<T>::StandardCommandProxy(
      const QString& n, const bool b)
      : name(n) {
    if (b) {
      auto& f = CommandFactory::getCommandFactory();
      f.addCommand(std::shared_ptr<CommandProxy>(
          new StandardCommandProxy(n, false)));
    }
  }  // end of StandardCommandProxy<T>::StandardCommandProxy

  template <typename T>
  QString StandardCommandProxy<T>::getName() const {
    return this->name;
  }  // end of StandardCommandProxy<T>::getName() const

  template <typename T>
  Command* StandardCommandProxy<T>::getCommand(
      EditorWidget& w) const {
    return new T(w);
  }  // end of StandardCommandProxy<T>::getCommand

  template <typename T>
  StandardCommandProxy<T>::~StandardCommandProxy() {
  }  // end of StandardCommandProxy::~StandardCommandProxy

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_COMMANDFACTORY_IXX */
