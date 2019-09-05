/*!
 * \file   MFrontAddInelasticFlowDialog.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialogButtonBox>
#include "MFront/BehaviourBrick/StressCriterionFactory.hxx"
#include "MFront/BehaviourBrick/InelasticFlowFactory.hxx"
#include "TFEL/GUI/MFrontAddInelasticFlowDialog.hxx"

namespace tfel{

  namespace gui {

    MFrontAddInelasticFlowDialog::MFrontAddInelasticFlowDialog(
        QWidget* const p)
        : QDialog(p),
          inelastic_flows(new QComboBox),
          stress_criteria(new QComboBox) {
      this->setWindowTitle("Add a new inelastic flow");
      // inelastic flows
      const auto& iff =
          mfront::bbrick::InelasticFlowFactory::getFactory();
      auto* const ifl = new QLabel(QObject::tr("Inelastic flow"));
      for (const auto& iflow : iff.getRegistredInelasticFlows()) {
        this->inelastic_flows->addItem(QString::fromStdString(iflow));
      }
      // stress criteria
      const auto& scf =
          mfront::bbrick::StressCriterionFactory::getFactory();
      auto* const scl = new QLabel(QObject::tr("Stress criterion"));
      for (const auto& sc: scf.getRegistredStressCriteria()) {
        this->stress_criteria->addItem(QString::fromStdString(sc));
      }
      /* buttons */
      auto* const lv = new QVBoxLayout;
      auto* const bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                            QDialogButtonBox::Cancel);
      QObject::connect(bb, &QDialogButtonBox::accepted, this,
                       &QDialog::accept);
      QObject::connect(bb, &QDialogButtonBox::rejected, this,
                       &QDialog::reject);
      auto* const gl = new QGridLayout();
      gl->addWidget(ifl, 0, 0);
      gl->addWidget(this->inelastic_flows, 0, 1);
      gl->addWidget(scl, 1, 0);
      gl->addWidget(this->stress_criteria, 1, 1);
      lv->addLayout(gl);
      lv->addWidget(bb);
      this->setLayout(lv);
    }  // end of MFrontAddInelasticFlowDialog::MFrontAddInelasticFlowDialog

    MFrontAddInelasticFlowDialog::~MFrontAddInelasticFlowDialog() = default;

  }  // end of namespace gui

}  // end of namespace tfel
