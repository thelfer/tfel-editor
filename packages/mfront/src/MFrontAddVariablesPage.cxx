/*!
 * \file   MFrontAddVariablesPage.cxx
 * \brief
 * \author th202608
 * \date   07/08/2019
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>
#include "MFront/AbstractBehaviourDSL.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontAddVariableDialog.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"

namespace tfel {

  namespace gui {

    MFrontAddVariablesPage::MFrontAddVariablesPage(
        EditorWidget &w, MFrontBehaviourWizard *const p)
        : QWizardPage(p), editor(w) {
      this->setTitle(QObject::tr("Add variables."));
      this->setSubTitle(
          QObject::tr("This page allows the selection of material "
                      "properties, parameters, internal "
                      "state variables, etc.."));
    }  // end of MFrontAddVariablesPage::MFrontAddVariablesPage

    void MFrontAddVariablesPage::initializePage() {
      auto *const vl = new QVBoxLayout();
      auto *const p = dynamic_cast<MFrontBehaviourWizard *const>(
          this->wizard());
      if (p == nullptr) {
        QMessageBox::warning(&(this->editor), "Internal error",
                             "invalid wizard reference");
        return;
      }
      const auto dsl = p->getBehaviourDSL();
      if (dsl == nullptr) {
        vl->addWidget(new QLabel(
            QObject::tr("Invalid domain specific language")));
        this->setLayout(vl);
        return;
      }
      const auto dsld = dsl->getBehaviourDSLDescription();
      // material properties
      if (dsld.allowUserDefinedMaterialProperties) {
        auto *const b =
            new QPushButton(QObject::tr("Add a material property"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor, MFrontAddVariableDialog::MATERIALPROPERTY,
              p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<
                &mfront::BehaviourDescription::addMaterialProperty>(
                this->materialProperties, d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      // parameters
      if (dsld.allowUserDefinedParameters) {
        auto *const b = new QPushButton(QObject::tr("Add a parameter"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor, MFrontAddVariableDialog::PARAMETER, p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<
                &mfront::BehaviourDescription::addParameter>(
                this->parameters, d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      // local variables
      if (dsld.allowUserDefinedLocalVariables) {
        auto *const b =
            new QPushButton(QObject::tr("Add a local variable"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor, MFrontAddVariableDialog::LOCALVARIABLE, p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<
                &mfront::BehaviourDescription::addLocalVariable>(
                this->localVariables, d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      // state variables
      if (dsld.allowUserDefinedStateVariables) {
        auto *const b =
            new QPushButton(QObject::tr("Add a state variable"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor, MFrontAddVariableDialog::STATEVARIABLE, p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<
                &mfront::BehaviourDescription::addStateVariable>(
                this->stateVariables, d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      // auxiliary state variables
      if (dsld.allowUserDefinedAuxiliaryStateVariables) {
        auto *const b = new QPushButton(
            QObject::tr("Add an auxiliary state variable"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor,
              MFrontAddVariableDialog::AUXILIARYSTATEVARIABLE, p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<&mfront::BehaviourDescription::
                                  addAuxiliaryStateVariable>(
                this->auxiliaryStateVariables,
                d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      // external state variables
      if (dsld.allowUserDefinedExternalStateVariables) {
        auto *const b = new QPushButton(
            QObject::tr("Add an external state variable"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor,
              MFrontAddVariableDialog::EXTERNALSTATEVARIABLE, p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<&mfront::BehaviourDescription::
                                  addExternalStateVariable>(
                this->externalStateVariables,
                d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      // integration variables
      if (dsld.allowUserDefinedIntegrationVariables) {
        auto *const b =
            new QPushButton(QObject::tr("Add an integration variable"));
        QObject::connect(b, &QPushButton::pressed, [this, p] {
          MFrontAddVariableDialog d(
              this->editor,
              MFrontAddVariableDialog::INTEGRATIONVARIABLE, p);
          if (d.exec() == QDialog::Accepted) {
            this->addVariable<
                &mfront::BehaviourDescription::addIntegrationVariable>(
                this->integrationVariables, d.getVariableDescription());
          }
        });
        vl->addWidget(b);
      }
      this->setLayout(vl);
    }  // end of MFrontAddVariablesPage::initializePage

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
      constexpr const auto h =
          tfel::material::ModellingHypothesis::UNDEFINEDHYPOTHESIS;
      const auto *const p =
          dynamic_cast<const MFrontBehaviourWizard *const>(
              this->wizard());
      if (p == nullptr) {
        QMessageBox::warning(&(this->editor), "Internal error",
                             "invalid wizard reference");
        return;
      }
      const auto dsl = p->getBehaviourDSL();
      if (dsl == nullptr) {
        QMessageBox::warning(&(this->editor), "Internal error",
                             "invalid DSL object");
        return;
      }
      try {
        auto d = dsl->getBehaviourDescription();
        for (const auto cv : variables) {
          (d.*f)(h, cv, mfront::BehaviourData::UNREGISTRED);
        }
        (d.*f)(h, v, mfront::BehaviourData::UNREGISTRED);
      } catch (std::exception &e) {
        QMessageBox::warning(&(this->editor),
                             "Variable insertion failed", e.what());
      } catch (...) {
        QMessageBox::warning(&(this->editor),
                             "Variable insertion failed",
                             "unsupported exception caught");
      }
      variables.push_back(v);
    }  // end of MFrontAddVariablesPage::addVariable

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getMaterialProperties() const {
      return this->materialProperties;
    }  // end of MFrontAddVariablesPage::getMaterialProperties

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getParameters() const {
      return this->parameters;
    }  // end of MFrontAddVariablesPage::getParameters

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getStateVariables() const {
      return this->stateVariables;
    }  // end of MFrontAddVariablesPage::getStateVariables

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getAuxiliaryStateVariables() const {
      return this->auxiliaryStateVariables;
    }  // end of MFrontAddVariablesPage::getAuxiliaryStateVariables

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getExternalStateVariables() const {
      return this->externalStateVariables;
    }  // end of MFrontAddVariablesPage::getExternalStateVariables

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getLocalVariables() const {
      return this->localVariables;
    }  // end of MFrontAddVariablesPage::getLocalVariables

    mfront::VariableDescriptionContainer
    MFrontAddVariablesPage::getIntegrationVariables() const {
      return this->integrationVariables;
    }  // end of MFrontAddVariablesPage::getIntegrationVariables

    MFrontAddVariablesPage::~MFrontAddVariablesPage() = default;

  }  // end of namespace gui

}  // end of namespace tfel
