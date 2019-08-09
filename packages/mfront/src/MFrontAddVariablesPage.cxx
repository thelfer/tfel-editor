/*!
 * \file   MFrontAddVariablesPage.cxx
 * \brief
 * \author th202608
 * \date   07/08/2019
 */

#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include "TFEL/GUI/MFrontAddVariableDialog.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"

namespace tfel {

  namespace gui {

    MFrontAddVariablesPage::MFrontAddVariablesPage(EditorWidget &w,
                                                   QWizard *const p)
        : QWizardPage(p) {
      this->setTitle(
          QObject::tr("Add variables."));
      this->setSubTitle(
          QObject::tr("This page allows the selection of material "
                      "properties, parameters, internal "
                      "state variables, etc.."));
      auto *const amp =
          new QPushButton(QObject::tr("Add material property"));
      QObject::connect(amp, &QPushButton::pressed, [this, &w, p] {
         MFrontAddVariableDialog d(
             w, MFrontAddVariableDialog::MATERIALPROPERTY, p);
         if(d.exec()== QDialog::Accepted){
           this->addVariable<
               &mfront::BehaviourDescription::addMaterialProperty>(
               this->mps, d.getVariableDescription());
         }
      });
      auto *const vl = new QVBoxLayout();
      vl->addWidget(amp);
      this->setLayout(vl);
    }  // end of MFrontAddVariablesPage::MFrontAddVariablesPage

    bool MFrontAddVariablesPage::validatePage() { return true; }

    int MFrontAddVariablesPage::nextId() const { return -1; }

    void MFrontAddVariablesPage::write() const {
    }  // end of MFrontAddVariablesPage::write()

    template <void (mfront::BehaviourDescription::*f)(
        const mfront::BehaviourDescription::Hypothesis,
        const mfront::VariableDescription &,
        const mfront::BehaviourData::RegistrationStatus)>
    void MFrontAddVariablesPage::addVariable(
        mfront::VariableDescriptionContainer &variables,
        const mfront::VariableDescription &v) {
      //       constexpr const auto h =
      //           tfel::material::ModellingHypothesis::UNDEFINEDHYPOTHESIS;
      variables.push_back(v);
    }  // end of MFrontAddVariablesPage::addVariable

    MFrontAddVariablesPage::~MFrontAddVariablesPage() = default;

  }  // end of namespace gui

}  // end of namespace tfel
