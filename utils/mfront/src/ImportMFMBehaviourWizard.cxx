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
#include "QEmacs/Debug.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/SelectMFMBehaviourPage.hxx"
#include "QEmacs/MaterialPropertiesSelectionPage.hxx"
#include "QEmacs/ImportMFMBehaviourWizard.hxx"

namespace qemacs {

  ImportMFMBehaviourWizard::ConclusionPage::ConclusionPage(
      ImportMFMBehaviourWizard& w)
      : wizard(w) {}  // end of
  // ImportMFMBehaviourWizard::ConclusionPage::ConclusionPage

  int ImportMFMBehaviourWizard::ConclusionPage::nextId() const {
    return -1;
  }

  ImportMFMBehaviourWizard::ImportMFMBehaviourWizard(
      QEmacsTextEditBase& t)
      : QWizard(&t),
        sb(new SelectMFMBehaviourPage(t.getQEmacsWidget())),
        mp(new MaterialPropertiesSelectionPage(t.getQEmacsWidget())),
        c(new ConclusionPage(*this)) {
    this->setWindowTitle(QObject::tr("Import MFMBehaviour"));
    this->setPage(0, this->sb);
    this->setPage(1, this->mp);
    this->setPage(2, this->c);
    QObject::connect(
        this->sb,
        &SelectMFMBehaviourPage::behaviourDescriptionChanged,
        this->mp,
        &MaterialPropertiesSelectionPage::updateMaterialPropertiesList);
    this->mp->updateMaterialPropertiesList(this->sb->getBehaviourDescription());
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

}  // end of namespace qemacs
