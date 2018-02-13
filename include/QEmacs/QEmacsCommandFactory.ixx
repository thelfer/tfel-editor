/*! 
 * \file  QEmacsCommandFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_QEMACS_QEMACSCOMMANDFACTORY_IXX
#define LIB_QEMACS_QEMACSCOMMANDFACTORY_IXX 

#include<QtCore/QMap>

#include<QtCore/QString>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/System/ExternalLibraryManager.hxx"

namespace qemacs {

  template <typename T>
  StandardQEmacsCommandProxy<T>::StandardQEmacsCommandProxy(
      const QString& n, const bool b)
      : name(n) {
    if (b) {
      auto& f = QEmacsCommandFactory::getQEmacsCommandFactory();
      f.addQEmacsCommand(std::shared_ptr<QEmacsCommandProxy>(
          new StandardQEmacsCommandProxy(n, false)));
    }
  }  // end of StandardQEmacsCommandProxy<T>::StandardQEmacsCommandProxy

  template <typename T>
  QString StandardQEmacsCommandProxy<T>::getName() const {
    return this->name;
  }  // end of StandardQEmacsCommandProxy<T>::getName() const

  template <typename T>
  QEmacsCommand* StandardQEmacsCommandProxy<T>::getQEmacsCommand(
      QEmacsWidget& w) const {
    return new T(w);
  }  // end of StandardQEmacsCommandProxy<T>::getQEmacsCommand

  template <typename T>
  StandardQEmacsCommandProxy<T>::~StandardQEmacsCommandProxy() {
  }  // end of StandardQEmacsCommandProxy::~StandardQEmacsCommandProxy

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSCOMMANDFACTORY_IXX */
