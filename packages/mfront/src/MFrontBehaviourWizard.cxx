/*!
 * \file   MFrontBehaviourWizard.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGridLayout>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontMetaDataPage.hxx"
#include "TFEL/GUI/MFrontBehaviourPage.hxx"
#include "TFEL/GUI/MFrontAddVariablesPage.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"

namespace tfel{

  namespace gui{

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

  void MFrontBehaviourWizard::write() const{
    const auto& dsl = this->im->getSelectedDomainSpecificLanguage();
    const auto i = this->im->getSelectedIntegrationScheme();
    this->d.insertPlainText("@DSL " + dsl + ";\n\n");
    this->md->write();
    this->im->write();
    if (i == mfront::BehaviourDescription::IMPLICITSCHEME) {
      this->d.insertPlainText("@InitLocalVariables {\n}\n\n");
      this->d.insertPlainText("@PredictionOperator{\n}\n\n");
      this->d.insertPlainText("@Predictor{\n}\n\n");
      this->d.insertPlainText("@ComputeStress{\n}\n\n");
      this->d.insertPlainText("@Integrator{\n}\n\n");
      this->d.insertPlainText("@TangentOperator{\n}\n\n");
    }
  }  // end of MFrontBehaviourWizard::write

  MFrontBehaviourWizard::~MFrontBehaviourWizard() = default;

  }  // end of namespace gui

}// end of namespace tfel
