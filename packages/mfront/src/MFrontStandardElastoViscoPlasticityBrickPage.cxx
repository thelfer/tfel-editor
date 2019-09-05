/*!
 * \file   MFrontMetaDataPage.cxx
 * \brief    
 * \author th202608
 * \date   27/07/2019
 */

#include <QtGui/QValidator>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include "MFront/BehaviourBrick/StressPotentialFactory.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontBehaviourWizardPages.hxx"
#include "TFEL/GUI/MFrontAddInelasticFlowDialog.hxx"
#include "TFEL/GUI/MFrontStandardElastoViscoPlasticityBrickPage.hxx"

namespace tfel{

  namespace gui{

    MFrontStandardElastoViscoPlasticityBrickPage::
        MFrontStandardElastoViscoPlasticityBrickPage(EditorWidget &w,
                                                     TextEditBase &cd,
                                                     QWizard *const p)
        : QWizardPage(p), stress_potentials(new QComboBox), d(cd) {
      this->setTitle(
          QObject::tr("The StandardElastoViscoPlasticity brick"));
      this->setSubTitle(QObject::tr(
          "Specify a stress potential and some inelastic flows"));
      // populate stress potentials
      const auto &spf = mfront::bbrick::StressPotentialFactory::getFactory();
      for (const auto &sp : spf.getRegistredStressPotentials()) {
        this->stress_potentials->addItem(QString::fromStdString(sp));
      }
      this->stress_potentials->setCurrentText("Hooke");
      auto *const gl = new QGridLayout;
      auto *const spl = new QLabel(QObject::tr("Stress potential"));
      auto *const sph =
          new QPushButton(QIcon::fromTheme("help-contents"), "");
      QObject::connect(sph, &QPushButton::clicked, [p] {
        QMessageBox::information(
            p, QObject::tr("Stress potentials"),
            QObject::tr("The stress potentials describes the part of "
                        "the free energy related to computation of the "
                        "stress."));
      });
      gl->addWidget(spl, 0, 0);
      gl->addWidget(this->stress_potentials, 0, 1);
      gl->addWidget(sph, 0, 2);
      auto *const l1 = new QFrame();
      l1->setFrameShape(QFrame::HLine);
      l1->setFrameShadow(QFrame::Sunken);
      gl->addWidget(l1, 2, 0, 1, 3);
      auto *const ifb =
          new QPushButton(QObject::tr("Add an inelastic flow"));
      QObject::connect(ifb, &QPushButton::pressed, [this, p] {
        MFrontAddInelasticFlowDialog dialog(p);
        if (dialog.exec() == QDialog::Accepted) {
        }
      });
      gl->addWidget(ifb, 3, 0, 1, 3);
      this->setLayout(gl);
    }  // end of MFrontStandardElastoViscoPlasticityBrickPage::MFrontStandardElastoViscoPlasticityBrickPage

    bool MFrontStandardElastoViscoPlasticityBrickPage::validatePage() {
      return true;
    }

    int MFrontStandardElastoViscoPlasticityBrickPage::nextId() const {
      return MFrontBehaviourWizardPages::ADDVARIABLES;
    }

    void MFrontStandardElastoViscoPlasticityBrickPage::write() const {
    }  // end of MFrontStandardElastoViscoPlasticityBrickPage::write()

    MFrontStandardElastoViscoPlasticityBrickPage::~MFrontStandardElastoViscoPlasticityBrickPage() = default;

  }  // end of namespace gui

} // end of namespace tfel


