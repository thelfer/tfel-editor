/*!
 * \file   MFrontBehaviourWizard.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#include <cstring>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGridLayout>
#include "MFront/AbstractBehaviourDSL.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontMetaDataPage.hxx"
#include "TFEL/GUI/MFrontBehaviourPage.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"

namespace tfel {

  namespace gui {

    MFrontBehaviourWizard::MFrontBehaviourWizard(EditorWidget& w,
                                                 TextEditBase& cd,
                                                 QWidget* const p)
        : QWizard(p),
          md(new MFrontMetaDataPage(
              w, cd, MFrontMetaDataPage::BEHAVIOUR, 1, this)),
          im(new MFrontBehaviourPage(w, cd, this)),
          variables(new MFrontAddVariablesPage(w, this)),
          d(cd) {
      this->setWindowTitle(QObject::tr("Behaviour wizard"));
      this->setPage(0, this->md);
      this->setPage(1, this->im);
      this->setPage(2, this->variables);
      this->setStartId(0);
  }  // end of MFrontBehaviourWizard::MFrontBehaviourWizard

  std::shared_ptr<mfront::AbstractBehaviourDSL>
  MFrontBehaviourWizard::getBehaviourDSL() const {
    return this->im->getBehaviourDSL();
  }  // end of MFrontBehaviourWizard::getBehaviourDSL

  void MFrontBehaviourWizard::write() const{
    constexpr const auto h =
        tfel::material::ModellingHypothesis::UNDEFINEDHYPOTHESIS;
    const auto& n = this->im->getSelectedDomainSpecificLanguage();
    const auto dsl = this->im->getBehaviourDSL();
    if (dsl == nullptr) {
      return;
    }
    const auto& bd = dsl->getBehaviourDescription();
    const auto& dsld = dsl->getBehaviourDSLDescription();
    this->d.insertPlainText("@DSL " + n + ";\n\n");
    this->md->write();
    this->im->write();
    auto write_variables = [this](
        const QString& t,
        const mfront::VariableDescriptionContainer& vars) {
      for (const auto& v : vars) {
        this->d.insertPlainText(t + " " +
                                QString::fromStdString(v.type) + " " +
                                QString::fromStdString(v.name) + ";\n");
        if (v.hasGlossaryName()) {
          this->d.insertPlainText(
              QString::fromStdString(v.name) + ".setGlossaryName(\"" +
              QString::fromStdString(v.getExternalName()) + "\");\n");
        }
        if (v.hasEntryName()) {
          this->d.insertPlainText(
              QString::fromStdString(v.name) + ".setEntryName(\"" +
              QString::fromStdString(v.getExternalName()) + "\");\n");
        }
        this->d.insertPlainText("\n");
      }
    };
    write_variables("@MaterialProperty",
                    this->variables->getMaterialProperties());
    write_variables("@Parameter", this->variables->getParameters());
    write_variables("@StateVariable", this->variables->getStateVariables());
    write_variables("@AuxiliaryStateVariable",
                    this->variables->getAuxiliaryStateVariables());
    write_variables("@IntegrationVariable",
                    this->variables->getIntegrationVariables());
    write_variables("@ExternalStateVariable",
                    this->variables->getExternalStateVariables());
    write_variables("@LocalVariable",
                    this->variables->getLocalVariables());
    for (const auto& c : dsld.typicalCodeBlocks) {
      if (!bd.hasCode(h, c)) {
        auto t =
            QString::fromStdString(dsl->getCodeBlockTemplate(c, true));
        if ((t.startsWith("@PredictionOperator")) ||
            (t.startsWith("@TangentOperator"))) {
          const auto to = this->im->getSelectedTangentOperator();
          if (t.startsWith("@PredictionOperator")) {
            t.insert(std::strlen("@PredictionOperator"), "<" + to + ">");
          }
          if (t.startsWith("@TangentOperator")) {
            t.insert(std::strlen("@TangentOperator"), "<" + to + ">");
          }
        }
        if (!t.isEmpty()) {
          this->d.insertPlainText(t + "\n");
        }
      }
    }
  }  // end of MFrontBehaviourWizard::write

  MFrontBehaviourWizard::~MFrontBehaviourWizard() = default;

  }  // end of namespace gui

}// end of namespace tfel
