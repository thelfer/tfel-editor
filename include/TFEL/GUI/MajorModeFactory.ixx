/*!
 * \file  MajorModeFactory.hxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODEFACTORY_IXX
#define LIB_TFEL_GUI_MAJORMODEFACTORY_IXX

#include <QtCore/QString>
#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/System/ExternalLibraryManager.hxx"

namespace tfel{

  namespace gui{

  template <typename T>
  StandardMajorModeProxy<T>::StandardMajorModeProxy(
      const QString& n,
      const QVector<QRegExp>& e,
      const QString& i,
      const bool b)
      : name(n), rexp(e), icon(i) {
    if (b) {
      auto& f = MajorModeFactory::getMajorModeFactory();
      f.addMajorMode(
          std::shared_ptr<MajorModeProxy>(
              new StandardMajorModeProxy(n, e, i, false)),
          e);
    }
  }  // end of
     // StandardMajorModeProxy<T>::StandardMajorModeProxy

  template <typename T>
  QString StandardMajorModeProxy<T>::getName() const {
    return this->name;
  }  // end of StandardMajorModeProxy<T>::getName() const

  template <typename T>
  QIcon StandardMajorModeProxy<T>::getIcon() const {
    return QIcon(this->icon);
  }  // end of StandardMajorModeProxy<T>::getIcon() const

  template <typename T>
  MajorMode* StandardMajorModeProxy<T>::getMajorMode(
      EditorWidget& w, Buffer& b, TextEditBase& t) const {
    return new T(w, b, t);
  }  // end of StandardMajorModeProxy<T>::getMajorMode

  template <typename T>
  StandardMajorModeProxy<T>::~StandardMajorModeProxy() {
  }  // end of
     // StandardMajorModeProxy::~StandardMajorModeProxy

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MAJORMODEFACTORY_IXX */
