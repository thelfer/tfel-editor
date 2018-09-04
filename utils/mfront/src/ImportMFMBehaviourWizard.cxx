/*!
 * \file   ImportMFMBehaviourWizard.cxx
 * \brief
 * \author Thomas Helfer
 * \date   02/06/2017
 */

#include <QtCore/QStringListModel>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QScrollArea>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/Material/ModellingHypothesis.hxx"
#include "MFront/TargetsDescription.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/SelectMFMBehaviourPage.hxx"
#include "TFEL/GUI/MaterialPropertiesSelectionPage.hxx"
#include "TFEL/GUI/BehaviourSummaryPage.hxx"
#include "TFEL/GUI/ImportMFMBehaviourWizard.hxx"

namespace tfel{

  namespace gui{

  ImportMFMBehaviourWizard::ImportMFMBehaviourWizard(
      TextEditBase& t, const Options& o)
      : QWizard(&t),
        sb(new SelectMFMBehaviourPage(t.getEditorWidget(), o)),
        mp(new MaterialPropertiesSelectionPage(t.getEditorWidget())),
        c(new BehaviourSummaryPage) {
    this->setWindowTitle(QObject::tr("Import MFMBehaviour"));
    this->setPage(0, this->sb);
    this->setPage(1, this->mp);
    this->setPage(2, this->c);
    QObject::connect(
        this->sb, &SelectMFMBehaviourPage::behaviourDescriptionChanged,
        this->mp,
        &MaterialPropertiesSelectionPage::updateMaterialPropertiesList);
    QObject::connect(this->sb, &SelectMFMBehaviourPage::doubleClicked,
                     this, [this](const BehaviourDescription& bd) {
                       this->mp->updateMaterialPropertiesList(bd);
                       this->next();
                     });
    this->mp->updateMaterialPropertiesList(
        this->sb->getBehaviourDescription());
    this->setStartId(0);
  }  // end of ImportMFMBehaviourWizard::ImportMFMBehaviourWizard

  BehaviourDescription ImportMFMBehaviourWizard::getSelectedBehaviour()
      const {
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    auto b = this->sb->getBehaviourDescription();
    const auto l = b.library.toStdString();
    const auto f = b.behaviour.toStdString();
    try {
      b.minterface = QString::fromStdString(elm.getInterface(l, f));
    } catch (std::exception&) {
      return {};
    } catch (...) {
      return {};
    }
    b.material_properties = this->mp->getMaterialPropertyDescriptions();
    return b;
  }  // end of ImportMFMBehaviourWizard::getMFMBehaviour

  ImportMFMBehaviourWizard::~ImportMFMBehaviourWizard() = default;

}  // end of namespace gui
}// end of namespace tfel
