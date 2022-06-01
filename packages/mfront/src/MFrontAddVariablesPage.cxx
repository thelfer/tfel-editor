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
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontBehaviourWizardPages.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontAddVariableDialog.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"

namespace tfel::gui {

  MFrontAddVariablesPage::MFrontAddVariablesPage(EditorWidget &w,
                                                 TextEditBase &cd,
                                                 MFrontBehaviourWizard *const p)
      : QWizardPage(p), editor(w), d(cd) {
    this->setTitle(QObject::tr("Add variables."));
    this->setSubTitle(
        QObject::tr("This page allows the selection of material "
                    "properties, parameters, internal "
                    "state variables, etc.."));
  }  // end of MFrontAddVariablesPage::MFrontAddVariablesPage

  void MFrontAddVariablesPage::initializePage() {
    auto *const vl = new QVBoxLayout();
    auto *const p = dynamic_cast<MFrontBehaviourWizard *const>(this->wizard());
    if (p == nullptr) {
      QMessageBox::warning(&(this->editor), "Internal error",
                           "invalid wizard reference");
      return;
    }
    const auto dsl = p->getBehaviourDSL({});
    if (dsl == nullptr) {
      vl->addWidget(
          new QLabel(QObject::tr("Invalid domain specific language")));
      this->setLayout(vl);
      return;
    }
    const auto dsld = p->getBehaviourDSLDescription();
    // material properties
    if (dsld.allowUserDefinedMaterialProperties) {
      auto *const b = new QPushButton(QObject::tr("Add a material property"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(
            this->editor, MFrontAddVariableDialog::MATERIALPROPERTY, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<&mfront::BehaviourDescription::addMaterialProperty>(
              this->materialProperties, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    // parameters
    if (dsld.allowUserDefinedParameters) {
      auto *const b = new QPushButton(QObject::tr("Add a parameter"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(this->editor,
                                       MFrontAddVariableDialog::PARAMETER, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<&mfront::BehaviourDescription::addParameter>(
              this->parameters, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    // local variables
    if (dsld.allowUserDefinedLocalVariables) {
      auto *const b = new QPushButton(QObject::tr("Add a local variable"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(
            this->editor, MFrontAddVariableDialog::LOCALVARIABLE, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<&mfront::BehaviourDescription::addLocalVariable>(
              this->localVariables, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    // state variables
    if (dsld.allowUserDefinedStateVariables) {
      auto *const b = new QPushButton(QObject::tr("Add a state variable"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(
            this->editor, MFrontAddVariableDialog::STATEVARIABLE, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<&mfront::BehaviourDescription::addStateVariable>(
              this->stateVariables, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    // auxiliary state variables
    if (dsld.allowUserDefinedAuxiliaryStateVariables) {
      auto *const b =
          new QPushButton(QObject::tr("Add an auxiliary state variable"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(
            this->editor, MFrontAddVariableDialog::AUXILIARYSTATEVARIABLE, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<
              &mfront::BehaviourDescription::addAuxiliaryStateVariable>(
              this->auxiliaryStateVariables, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    // external state variables
    if (dsld.allowUserDefinedExternalStateVariables) {
      auto *const b =
          new QPushButton(QObject::tr("Add an external state variable"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(
            this->editor, MFrontAddVariableDialog::EXTERNALSTATEVARIABLE, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<
              &mfront::BehaviourDescription::addExternalStateVariable>(
              this->externalStateVariables, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    // integration variables
    if (dsld.allowUserDefinedIntegrationVariables) {
      auto *const b =
          new QPushButton(QObject::tr("Add an integration variable"));
      QObject::connect(b, &QPushButton::pressed, [this, p] {
        MFrontAddVariableDialog dialog(
            this->editor, MFrontAddVariableDialog::INTEGRATIONVARIABLE, p);
        if (dialog.exec() == QDialog::Accepted) {
          this->addVariable<
              &mfront::BehaviourDescription::addIntegrationVariable>(
              this->integrationVariables, dialog.getVariableDescription());
        }
      });
      vl->addWidget(b);
    }
    this->setLayout(vl);
  }  // end of MFrontAddVariablesPage::initializePage

  bool MFrontAddVariablesPage::validatePage() { return true; }

  int MFrontAddVariablesPage::nextId() const {
    return MFrontBehaviourWizardPages::TEMPLATEGENERATIONOPTIONS;
  }  // end of MFrontAddVariablesPage::nextId

  void MFrontAddVariablesPage::complete(
      mfront::AbstractBehaviourDSL &dsl) const {
    if (!this->materialProperties.empty()) {
      dsl.addMaterialProperties(this->materialProperties);
    }
    if (!this->parameters.empty()) {
      dsl.addParameters(this->parameters);
    }
    if (!this->stateVariables.empty()) {
      dsl.addStateVariables(this->stateVariables);
    }
    if (!this->auxiliaryStateVariables.empty()) {
      dsl.addAuxiliaryStateVariables(this->auxiliaryStateVariables);
    }
    if (!this->externalStateVariables.empty()) {
      dsl.addExternalStateVariables(this->externalStateVariables);
    }
    if (!this->localVariables.empty()) {
      dsl.addLocalVariables(this->localVariables);
    }
    if (!this->integrationVariables.empty()) {
      dsl.addIntegrationVariables(this->integrationVariables);
    }
  }  // end of MFrontAddVariablesPage::complete

  void MFrontAddVariablesPage::write() const {
    auto write_variables =
        [this](const QString &t,
               const mfront::VariableDescriptionContainer &vars) {
          for (const auto &v : vars) {
            const auto vn = QString::fromStdString(mfront::displayName(v));
            const auto as = v.arraySize;
            if (as == 1) {
              this->d.insertPlainText(t + " " + QString::fromStdString(v.type) +
                                      " " + vn + ";\n");
            } else {
              this->d.insertPlainText(t + " " + QString::fromStdString(v.type) +
                                      " " + vn + "[" + QString::number(as) +
                                      "];\n");
            }
            if (v.hasGlossaryName()) {
              this->d.insertPlainText(
                  vn + ".setGlossaryName(\"" +
                  QString::fromStdString(v.getExternalName()) + "\");\n");
            }
            if (v.hasEntryName()) {
              this->d.insertPlainText(
                  vn + ".setEntryName(\"" +
                  QString::fromStdString(v.getExternalName()) + "\");\n");
            }
            this->d.insertPlainText("\n");
          }
        };
    write_variables("@MaterialProperty", this->getMaterialProperties());
    write_variables("@Parameter", this->getParameters());
    write_variables("@StateVariable", this->getStateVariables());
    write_variables("@AuxiliaryStateVariable",
                    this->getAuxiliaryStateVariables());
    write_variables("@IntegrationVariable", this->getIntegrationVariables());
    write_variables("@ExternalStateVariable",
                    this->getExternalStateVariables());
    write_variables("@LocalVariable", this->getLocalVariables());
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
        dynamic_cast<const MFrontBehaviourWizard *const>(this->wizard());
    if (p == nullptr) {
      QMessageBox::warning(&(this->editor), "Internal error",
                           "invalid wizard reference");
      return;
    }
    const auto dsl = p->getBehaviourDSL({});
    if (dsl == nullptr) {
      QMessageBox::warning(&(this->editor), "Internal error",
                           "invalid DSL object");
      return;
    }
    try {
      auto bd = dsl->getBehaviourDescription();
      for (const auto cv : variables) {
        (bd.*f)(h, cv, mfront::BehaviourData::UNREGISTRED);
      }
      (bd.*f)(h, v, mfront::BehaviourData::UNREGISTRED);
    } catch (std::exception &e) {
      QMessageBox::warning(&(this->editor), "Variable insertion failed",
                           e.what());
    } catch (...) {
      QMessageBox::warning(&(this->editor), "Variable insertion failed",
                           "unsupported exception caught");
    }
    variables.push_back(v);
  }  // end of MFrontAddVariablesPage::addVariable

  mfront::VariableDescriptionContainer
  MFrontAddVariablesPage::getMaterialProperties() const {
    return this->materialProperties;
  }  // end of MFrontAddVariablesPage::getMaterialProperties

  mfront::VariableDescriptionContainer MFrontAddVariablesPage::getParameters()
      const {
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

}  // end of namespace tfel::gui
