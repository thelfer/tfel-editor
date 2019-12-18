/*!
 * \file   SelectMFMBehaviourPage.cxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#include <QtCore/QDebug>

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFileDialog>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "MFront/TargetsDescription.hxx"
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/SelectMFMBehaviour.hxx"
#include "TFEL/GUI/SelectMFMBehaviourPage.hxx"

namespace tfel {

  namespace gui {

    SelectMFMBehaviourPage::SelectMFMBehaviourPage(EditorWidget& q,
                                                   const Options& o)
        : sb(new SelectMFMBehaviour(q, o)) {
      this->setTitle("Select a behaviour");
      auto* l = new QVBoxLayout;
      l->addWidget(this->sb);
      QObject::connect(this->sb,
                       &SelectMFMBehaviour::behaviourDescriptionChanged,
                       this, [this](const BehaviourDescription& bd) {
                         emit behaviourDescriptionChanged(bd);
                       });
      QObject::connect(this->sb, &SelectMFMBehaviour::doubleClicked,
                       [this](const BehaviourDescription& bd) {
                         emit behaviourDescriptionChanged(bd);
                         emit doubleClicked(bd);
                       });
      this->setLayout(l);
    }

    BehaviourDescription
    SelectMFMBehaviourPage::getBehaviourDescription() const {
      return this->sb->getSelectedBehaviour();
    }  // end of SelectMFMBehaviourPage::getSelectedBehaviour

    bool SelectMFMBehaviourPage::validatePage() {
      const auto b = this->sb->getSelectedBehaviour();
      return b.generate() != nullptr;
    }  // end of SelectLibrary::validatePage

    int SelectMFMBehaviourPage::nextId() const {
      auto b = this->sb->getSelectedBehaviour().generate();
      if (b) {
        if (b->getMaterialPropertiesNames().empty()) {
          return 2;
        }
        return 1;
      }
      return 2;
    }  // end of SelectBehaviourPage::nextId

    SelectMFMBehaviourPage::~SelectMFMBehaviourPage() = default;

  }  // end of namespace gui
}  // end of namespace tfel
