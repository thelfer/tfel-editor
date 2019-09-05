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
#include "TFEL/GUI/MFrontBehaviourWizardPages.hxx"
#include "TFEL/GUI/MFrontMetaDataPage.hxx"
#include "TFEL/GUI/MFrontBehaviourPage.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"
#include "TFEL/GUI/MFrontStandardElastoViscoPlasticityBrickPage.hxx"
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
          variables(new MFrontAddVariablesPage(w, cd, this)),
          sevpbp(new MFrontStandardElastoViscoPlasticityBrickPage(
              w, cd, this)),
          d(cd) {
      this->setWindowTitle(QObject::tr("Behaviour wizard"));
      this->setPage(MFrontBehaviourWizardPages::METADATA, this->md);
      this->setPage(MFrontBehaviourWizardPages::BEHAVIOUR, this->im);
      this->setPage(
          MFrontBehaviourWizardPages::STANDARDELASTOVISCOPLASTICITY,
          this->sevpbp);
      this->setPage(MFrontBehaviourWizardPages::ADDVARIABLES,
                    this->variables);
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
    this->variables->complete(*dsl);
    auto dsld = dsl->getBehaviourDSLDescription();
    const auto bd = dsl->getBehaviourDescription();
    this->d.insertPlainText("@DSL " + n + ";\n\n");
    this->md->write();
    this->im->write();
    this->variables->write();
    // local variables
    const auto lvs = this->variables->getLocalVariables();
    if (!lvs.empty()) {
      this->d.insertPlainText("@InitLocalVariables{\n");
      for (const auto& lv : lvs) {
        const auto vn = QString::fromStdString(mfront::displayName(lv));
        this->d.insertPlainText(vn + " = ;\n");
      }
      this->d.insertPlainText("}\n\n");
    }
    const auto cbs = dsld.typicalCodeBlocks;
    for (const auto& c : cbs) {
      if (!bd.hasCode(h, c)) {
        auto t =
            QString::fromStdString(dsl->getCodeBlockTemplate(c, true));
        if (t.startsWith("@TangentOperator")) {
          continue;
        }
        if (t.startsWith("@PredictionOperator")) {
          const auto to = this->im->getSelectedTangentOperator();
          if(!to.isEmpty()){
            t.insert(std::strlen("@PredictionOperator"),
                     "<" + to + ">");
          }
        }
        if (!t.isEmpty()) {
          this->d.insertPlainText(t + "\n");
        }
      }
    }

    // @UpdateAuxiliaryStateVariables
    const auto asvs = this->variables->getAuxiliaryStateVariables();
    if (!asvs.empty()) {
      this->d.insertPlainText("@UpdateAuxiliaryStateVariables{\n");
      for (const auto& asv : asvs) {
        const auto vn = QString::fromStdString(mfront::displayName(asv));
        this->d.insertPlainText(vn + " = ;\n");
      }
      this->d.insertPlainText("}\n\n");
    }

    for (const auto& c : cbs) {
      if (!bd.hasCode(h, c)) {
        auto t =
            QString::fromStdString(dsl->getCodeBlockTemplate(c, true));
        if (!t.startsWith("@TangentOperator")) {
          continue;
        }
        const auto to = this->im->getSelectedTangentOperator();
        if (!to.isEmpty()) {
          t.insert(std::strlen("@TangentOperator"), "<" + to + ">");
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
