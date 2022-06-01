/*!
 * \file   MFrontStandardElastoViscoPlasticityBrickPage.cxx
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
#include "MFront/AbstractBehaviourDSL.hxx"
#include "MFront/BehaviourBrick/OptionDescription.hxx"
#include "MFront/BehaviourBrick/StressPotential.hxx"
#include "MFront/BehaviourBrick/StressPotentialFactory.hxx"
#include "MFront/BehaviourBrick/StressCriterion.hxx"
#include "MFront/BehaviourBrick/StressCriterionFactory.hxx"
#include "MFront/BehaviourBrick/IsotropicHardeningRule.hxx"
#include "MFront/BehaviourBrick/IsotropicHardeningRuleFactory.hxx"
#include "MFront/BehaviourBrick/KinematicHardeningRule.hxx"
#include "MFront/BehaviourBrick/KinematicHardeningRuleFactory.hxx"
#include "MFront/BehaviourBrick/InelasticFlow.hxx"
#include "MFront/BehaviourBrick/InelasticFlowFactory.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontBehaviourWizardPages.hxx"
#include "TFEL/GUI/MFrontStandardElastoViscoPlasticityBrickPage.hxx"

namespace tfel::gui {

  MFrontStandardElastoViscoPlasticityBrickPage::
      MFrontStandardElastoViscoPlasticityBrickPage(
          EditorWidget &w, TextEditBase &cd, MFrontBehaviourWizard *const p)
      : QWizardPage(p), stress_potentials(new QComboBox), editor(w), d(cd) {
    this->setTitle(QObject::tr("The StandardElastoViscoPlasticity brick"));
    this->setSubTitle(
        QObject::tr("Specify a stress potential and some inelastic flows"));
    // populate stress potentials
    const auto &spf = mfront::bbrick::StressPotentialFactory::getFactory();
    for (const auto &sp : spf.getRegistredStressPotentials()) {
      this->stress_potentials->addItem(QString::fromStdString(sp));
    }
    this->stress_potentials->setCurrentText("Hooke");
    auto *const gl = new QGridLayout;
    auto *const spl = new QLabel(QObject::tr("Stress potential"));
    auto *const sph = new QPushButton(QIcon::fromTheme("help-contents"), "");
    QObject::connect(sph, &QPushButton::clicked, [p] {
      QMessageBox::information(
          p, QObject::tr("Stress potentials"),
          QObject::tr("The stress potentials describes the part of "
                      "the free energy related to computation of the "
                      "stress."));
    });
    sph->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    gl->addWidget(spl, 0, 0);
    gl->addWidget(this->stress_potentials, 0, 1);
    gl->addWidget(sph, 0, 2);
    auto *const l1 = new QFrame();
    l1->setFrameShape(QFrame::HLine);
    l1->setFrameShadow(QFrame::Sunken);
    gl->addWidget(l1, 2, 0, 1, 3);
    auto *const ifb = new QPushButton(QObject::tr("Add an inelastic flow"));
    QObject::connect(ifb, &QPushButton::pressed, [this, p] {
      MFrontAddInelasticFlowDialog dialog(p);
      if (dialog.exec() == QDialog::Accepted) {
        this->inelastic_flows.push_back(dialog.getInelasticFlowDescription());
      }
    });
    gl->addWidget(ifb, 3, 0, 1, 2);
    this->setLayout(gl);
  }  // end of
     // MFrontStandardElastoViscoPlasticityBrickPage::MFrontStandardElastoViscoPlasticityBrickPage

  bool MFrontStandardElastoViscoPlasticityBrickPage::validatePage() {
    return true;
  }

  int MFrontStandardElastoViscoPlasticityBrickPage::nextId() const {
    return MFrontBehaviourWizardPages::ADDVARIABLES;
  }

  void MFrontStandardElastoViscoPlasticityBrickPage::write() const {
    auto tc = this->d.textCursor();
    auto append = [&tc](const QString &text) { tc.insertText(text); };
    auto write_options =
        [&tc,
         &append](const std::vector<mfront::bbrick::OptionDescription> &opts) {
          if (opts.empty()) {
            return;
          }
          append("{\n");
          for (decltype(opts.size()) i = 0; i != opts.size();) {
            const auto &o = opts[i];
            if (++i != opts.size()) {
              append(QString::fromStdString(o.name + ": , //" + o.description +
                                            "\n"));
            } else {
              append(QString::fromStdString(o.name + ": //" + o.description +
                                            "\n"));
            }
          }
          append("}");
        };
    append("{\n");
    const auto spn = this->stress_potentials->currentText();
    append("stress_potential: \"" + spn + "\"");
    try {
      const auto &spf = mfront::bbrick::StressPotentialFactory::getFactory();
      const auto sp = spf.generate(spn.toStdString());
      auto *const p =
          dynamic_cast<MFrontBehaviourWizard *const>(this->wizard());
      if (p == nullptr) {
        QMessageBox::warning(&(this->editor), "Internal error",
                             "invalid wizard reference");
        return;
      }
      const auto o = [] {
        auto opts = MFrontBehaviourWizard::DSLGenerationOptions{};
        opts.with_brick = false;
        return opts;
      }();
      const auto dsl = p->getBehaviourDSL(o);
      const auto bd = dsl->getBehaviourDescription();
      write_options(sp->getOptions(bd, false));
    } catch (...) {
    }
    for (const auto &c : this->inelastic_flows) {
      append(",\ninelastic_flow: \"" + c.inelastic_flow + "\" {\n");
      append("criterion: \"" + c.stress_criterion + "\"");
      try {
        const auto &scf = mfront::bbrick::StressCriterionFactory::getFactory();
        const auto sc = scf.generate(c.stress_criterion.toStdString());
        write_options(sc->getOptions());
      } catch (...) {
      }
      if (!c.plastic_potential.isEmpty()) {
        append(",\nplastic_potential: \"" + c.plastic_potential + "\"");
        try {
          const auto &scf =
              mfront::bbrick::StressCriterionFactory::getFactory();
          const auto sc = scf.generate(c.plastic_potential.toStdString());
          write_options(sc->getOptions());
        } catch (...) {
        }
      }
      for (const auto ihrn : c.isotropic_hardening_rules) {
        append(",\nisotropic_hardening\n: \"" + ihrn + "\"");
        try {
          const auto &ihrf =
              mfront::bbrick::IsotropicHardeningRuleFactory::getFactory();
          const auto ihr = ihrf.generate(ihrn.toStdString());
          write_options(ihr->getOptions());
        } catch (...) {
        }
      }
      for (const auto khrn : c.kinematic_hardening_rules) {
        append(",\nkinematic_hardening\n: \"" + khrn + "\"");
        try {
          const auto &khrf =
              mfront::bbrick::KinematicHardeningRuleFactory::getFactory();
          const auto khr = khrf.generate(khrn.toStdString());
          write_options(khr->getOptions());
        } catch (...) {
        }
      }
      try {
        const auto &iff = mfront::bbrick::InelasticFlowFactory::getFactory();
        const auto iflow = iff.generate(c.inelastic_flow.toStdString());
        const auto opts = iflow->getOptions();
        bool first = true;
        for (decltype(opts.size()) i = 0; i != opts.size();) {
          const auto &o = opts[i];
          if ((o.name == "criterion") || (o.name == "stress_criterion") ||
              (o.name == "flow_criterion") || (o.name == "plastic_potential") ||
              (o.name == "isotropic_hardening") ||
              (o.name == "kinematic_hardening")) {
            ++i;
            continue;
          }
          if (first) {
            first = false;
            append(",\n");
          }
          if (++i != opts.size()) {
            append(QString::fromStdString(o.name + ": , //" + o.description +
                                          "\n"));
          } else {
            append(
                QString::fromStdString(o.name + ": //" + o.description + "\n"));
          }
        }
      } catch (...) {
      }
      append("}\n");
    }
    append("\n}");
  }  // end of MFrontStandardElastoViscoPlasticityBrickPage::write()

  MFrontStandardElastoViscoPlasticityBrickPage::
      ~MFrontStandardElastoViscoPlasticityBrickPage() = default;

}  // end of namespace tfel::gui
