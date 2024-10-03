/*!
 * \file   MaterialPropertiesSelectionPage.cxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QScrollArea>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/MaterialPropertySelectorWidget.hxx"
#include "TFEL/GUI/MaterialPropertiesSelectionPage.hxx"

namespace tfel::gui {

  MaterialPropertiesSelectionPage::MaterialPropertiesSelectionPage(
      EditorWidget& q)
      : editor(q) {}  // end of
  // MaterialPropertiesSelectionPage::MaterialPropertiesSelectionPage

  void MaterialPropertiesSelectionPage::updateMaterialPropertiesList(
      const BehaviourDescription& bd) {
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    const auto b = bd.generate();
    if (b == nullptr) {
      return;
    }
    const auto m = [&elm, &bd] {
      try {
        return QString::fromStdString(elm.getMaterial(
            bd.library.toStdString(), bd.behaviour.toStdString()));
      } catch (std::exception& e) {
        debug(
            "MaterialPropertiesSelectionPage::"
            "updateMaterialPropertiesList:",
            e.what());
      } catch (...) {
        debug(
            "MaterialPropertiesSelectionPage::"
            "updateMaterialPropertiesList:"
            "unknow exception");
      }
      return QString();
    }();

    auto* cw = new QWidget;
    auto* const vl = new QVBoxLayout;
    auto mpsws_new = std::vector<MaterialPropertySelectorWidget*>{};
    for (const auto& mp : b->getMaterialPropertiesNames()) {
      auto* const mpsw = new MaterialPropertySelectorWidget(
          this->editor, QString::fromStdString(mp), m);
      mpsws_new.push_back(mpsw);
      vl->addWidget(mpsw);
    }
    cw->setLayout(vl);
    auto* const sa = new QScrollArea;
    sa->setWidget(cw);
    sa->setWidgetResizable(true);
    //
    qDeleteAll(this->children());
    this->mpsws.swap(mpsws_new);
    auto* const l = this->layout();
    if (l != nullptr) {
      delete l;
    }
    //
    auto* const cl = new QVBoxLayout;
    cl->addWidget(sa);
    this->setLayout(cl);
  }  // end of
  // MaterialPropertiesSelectionPage::updateMaterialPropertiesList

  int MaterialPropertiesSelectionPage::nextId() const {
    return 2;
  }  // end of MaterialPropertiesSelectionPage::nextId

  std::vector<MaterialPropertyDescription>
  MaterialPropertiesSelectionPage::getMaterialPropertyDescriptions() const {
    std::vector<MaterialPropertyDescription> r;
    for (const auto& mpsw : this->mpsws) {
      const auto& d = mpsw->getMaterialProperty();
      if (!d.empty()) {
        r.push_back(d);
      }
    }
    return r;
  }  // end of
     // MaterialPropertiesSelectionPage::getMaterialPropertyDescriptions

  MaterialPropertiesSelectionPage::~MaterialPropertiesSelectionPage() = default;

}  // end of namespace tfel::gui
