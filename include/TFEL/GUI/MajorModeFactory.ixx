/*!
 * \file  MajorModeFactory.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MAJORMODEFACTORY_IXX
#define LIB_TFEL_GUI_MAJORMODEFACTORY_IXX

#include <QtCore/QObject>
#include <QtCore/QString>
#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/System/ExternalLibraryManager.hxx"

namespace tfel::gui {

  template <typename T>
  StandardMajorModeProxy<T>::StandardMajorModeProxy(
      const QString& n,
      const QVector<QRegularExpression>& e,
      const QString& i,
      const bool b)
      : name(n), rexp(e), icon(i) {
    if (b) {
      auto& f = MajorModeFactory::getMajorModeFactory();
      f.addMajorMode(std::shared_ptr<MajorModeProxy>(
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
  MajorMode* StandardMajorModeProxy<T>::getMajorMode(EditorWidget& w,
                                                     Buffer& b,
                                                     TextEditBase& t) const {
    return new T(w, b, t);
  }  // end of StandardMajorModeProxy<T>::getMajorMode

  template <typename T>
  QVector<SettingsGroup> StandardMajorModeProxy<T>::getSettingsGroups() const {
    if constexpr (requires { T::declareSettings(); }) {
      auto gs = T::declareSettings();
      for (auto& g : gs) {
        if (g.name.isEmpty()) {
          g.name = this->name;
        }
        // what says that this theme belongs to a major mode, and which one: a
        // theme named after a program the mode drives, `clangd` or `castem`,
        // would look like a theme of the editor to anything which only reads
        // its name
        g.mode = this->name;
        if (g.description.isEmpty()) {
          g.description =
              QObject::tr("Entries of the configuration the %1 major mode uses")
                  .arg(this->name);
        }
        if (g.icon.isNull()) {
          g.icon = this->getIcon();
        }
      }
      return gs;
    } else {
      return {};
    }
  }  // end of StandardMajorModeProxy<T>::getSettingsGroups

  template <typename T>
  StandardMajorModeProxy<T>::~StandardMajorModeProxy() = default;

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAJORMODEFACTORY_IXX */
