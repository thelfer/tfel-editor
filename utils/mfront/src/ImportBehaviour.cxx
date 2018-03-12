/*!
 * \file   ImportBehaviour.cxx
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
#include "QEmacs/SelectBehaviourPage.hxx"
#include "QEmacs/MaterialPropertiesSelectionPage.hxx"
#include "QEmacs/ImportBehaviour.hxx"

namespace qemacs {

  ImportBehaviour::ConclusionPage::ConclusionPage(ImportBehaviour& w)
      : wizard(w) {}  // end of
  // ImportBehaviour::ConclusionPage::ConclusionPage

  int ImportBehaviour::ConclusionPage::nextId() const {
    return -1;
  }

  ImportBehaviour::ImportBehaviour(QEmacsTextEditBase& t)
      : QWizard(&t),
        sb(new SelectBehaviourPage(t.getQEmacsWidget())),
        mp(new MaterialPropertiesSelectionPage(t.getQEmacsWidget())),
        c(new ConclusionPage(*this)) {
    this->setWindowTitle(QObject::tr("Import Behaviour"));
    this->setPage(0, this->sb);
    this->setPage(1, this->mp);
    this->setPage(2, this->c);
    QObject::connect(
        this->sb, &SelectBehaviourPage::behaviourDescriptionChanged,
        this->mp,
        &MaterialPropertiesSelectionPage::updateMaterialPropertiesList);
    this->setStartId(0);
  }  // end of ImportBehaviour::ImportBehaviour

  BehaviourDescription ImportBehaviour::getSelectedBehaviour() const {
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    BehaviourDescription b;
    b.library    = this->field("Library").toString();
    b.behaviour  = this->field("Behaviour").toString();
    b.hypothesis = this->field("Hypothesis").toString();
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
  }  // end of ImportBehaviour::getBehaviour

  ImportBehaviour::~ImportBehaviour() = default;

}  // end of namespace qemacs
