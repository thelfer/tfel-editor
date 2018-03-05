/*!
 * \file  QEmacsMajorModeFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_QEMACS_QEMACSMAJORMODEFACTORY_IXX
#define LIB_QEMACS_QEMACSMAJORMODEFACTORY_IXX

#include <QtCore/QString>
#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/System/ExternalLibraryManager.hxx"

namespace qemacs {

  template <typename T>
  StandardQEmacsMajorModeProxy<T>::StandardQEmacsMajorModeProxy(
      const QString& n,
      const QVector<QRegExp>& e,
      const QString& i,
      const bool b)
      : name(n), rexp(e), icon(i) {
    if (b) {
      auto& f = QEmacsMajorModeFactory::getQEmacsMajorModeFactory();
      f.addQEmacsMajorMode(
          std::shared_ptr<QEmacsMajorModeProxy>(
              new StandardQEmacsMajorModeProxy(n, e, i, false)),
          e);
    }
  }  // end of
     // StandardQEmacsMajorModeProxy<T>::StandardQEmacsMajorModeProxy

  template <typename T>
  QString StandardQEmacsMajorModeProxy<T>::getName() const {
    return this->name;
  }  // end of StandardQEmacsMajorModeProxy<T>::getName() const

  template <typename T>
  QIcon StandardQEmacsMajorModeProxy<T>::getIcon() const {
    return QIcon(this->icon);
  }  // end of StandardQEmacsMajorModeProxy<T>::getIcon() const

  template <typename T>
  QEmacsMajorMode* StandardQEmacsMajorModeProxy<T>::getQEmacsMajorMode(
      QEmacsWidget& w, QEmacsBuffer& b, QEmacsTextEditBase& t) const {
    return new T(w, b, t);
  }  // end of StandardQEmacsMajorModeProxy<T>::getQEmacsMajorMode

  template <typename T>
  StandardQEmacsMajorModeProxy<T>::~StandardQEmacsMajorModeProxy() {
  }  // end of
     // StandardQEmacsMajorModeProxy::~StandardQEmacsMajorModeProxy

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSMAJORMODEFACTORY_IXX */
