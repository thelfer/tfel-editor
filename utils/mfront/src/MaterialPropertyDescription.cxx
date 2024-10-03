/*!
 * \file   MaterialPropertyDescription.cxx
 * \brief
 * \author Thomas Helfer
 * \date   12/03/2018
 */

#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/GUI/MaterialPropertyDescription.hxx"

namespace tfel::gui {

  static QStringList buildCastemMaterialPropertyArguments(const QString& l,
                                                          const QString& f) {
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    QStringList args;
    try {
      for (const auto& a :
           elm.getCastemFunctionVariables(l.toStdString(), f.toStdString())) {
        args.append(QString::fromStdString(a));
      }
    } catch (...) {
      return QStringList{};
    }
    return args;
  }  // end of buildCastemMaterialPropertyArguments

  CastemMaterialPropertyDescription::CastemMaterialPropertyDescription(
      const QString& n, const QString& l, const QString& f)
      : name(n),
        library(l),
        function(f),
        arguments(buildCastemMaterialPropertyArguments(l, f)) {
  }  // end
     // ofCastemMaterialPropertyDescription::CastemMaterialPropertyDescription

  CastemMaterialPropertyDescription::CastemMaterialPropertyDescription(
      CastemMaterialPropertyDescription&&) = default;

  CastemMaterialPropertyDescription::CastemMaterialPropertyDescription(
      const CastemMaterialPropertyDescription&) = default;

  CastemMaterialPropertyDescription&
  CastemMaterialPropertyDescription::operator=(
      CastemMaterialPropertyDescription&&) = default;

  CastemMaterialPropertyDescription&
  CastemMaterialPropertyDescription::operator=(
      const CastemMaterialPropertyDescription&) = default;

}  // end of namespace tfel::gui
