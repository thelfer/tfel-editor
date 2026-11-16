/*!
 * \file   MFrontAddInelasticFlowDialog.cxx
 * \brief
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#include <algorithm>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtCore/QStringListModel>
#include <QtWidgets/QDialogButtonBox>
#include "MFront/AbstractBehaviourDSL.hxx"
#include "MFront/BehaviourDescription.hxx"
#include "MFront/BehaviourBrick/StressCriterion.hxx"
#include "MFront/BehaviourBrick/StressCriterionFactory.hxx"
#include "MFront/BehaviourBrick/InelasticFlowFactory.hxx"
#include "MFront/BehaviourBrick/IsotropicHardeningRuleFactory.hxx"
#include "MFront/BehaviourBrick/KinematicHardeningRuleFactory.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontAddInelasticFlowDialog.hxx"

namespace tfel::gui {

  namespace {

    //! \brief select an option of a combo box
    void select_option(QComboBox* const c, const QString& option) {
      if (option.isEmpty()) {
        return;
      }
      const auto i = c->findText(option);
      if (i >= 0) {
        c->setCurrentIndex(i);
        return;
      }
      // the lists of the dialog are the registered bricks which are compatible
      // with the behaviour, so a description names an option such a list holds.
      // When it does not, the option is added and selected: an unknown option
      // shown as nothing selected would be dropped by the first Ok without the
      // user ever seeing that it was gone.
      c->addItem(option);
      c->setCurrentIndex(c->count() - 1);
    }  // end of select_option

  }  // end of namespace

  MFrontAddInelasticFlowDialog::MFrontAddInelasticFlowDialog(
      MFrontBehaviourWizard* const p)
      : QDialog(p),
        inelastic_flows(new QComboBox),
        stress_criteria(new QComboBox),
        plastic_potentials(new QComboBox),
        isotropic_hardening_rules_selected(new QListView(this)),
        isotropic_hardening_rules(new QComboBox),
        kinematic_hardening_rules_selected(new QListView(this)),
        kinematic_hardening_rules(new QComboBox) {
    this->setWindowTitle("Add a new inelastic flow");
    const auto o = [] {
      auto opts = MFrontBehaviourWizard::DSLGenerationOptions{};
      opts.with_brick = false;
      return opts;
    }();
    const auto dsl = p->getBehaviourDSL(o);
    auto* const vl = new QVBoxLayout;
    if (dsl == nullptr) {
      vl->addWidget(
          new QLabel(QObject::tr("Invalid domain specific language")));
      this->setLayout(vl);
      return;
    }
    const auto& bd = dsl->getBehaviourDescription();
    const auto s = bd.getSymmetryType();
    // inelastic flows
    const auto& iff = mfront::bbrick::InelasticFlowFactory::getFactory();
    auto* const ifl = new QLabel(QObject::tr("Inelastic flow"));
    for (const auto& iflow : iff.getRegistredInelasticFlows()) {
      this->inelastic_flows->addItem(QString::fromStdString(iflow));
    }
    // stress criteria and plastic potential
    this->plastic_potentials->addItem("Associated flow");
    const auto& scf = mfront::bbrick::StressCriterionFactory::getFactory();
    auto* const scl = new QLabel(QObject::tr("Stress criterion"));
    auto* const ppl = new QLabel(QObject::tr("Plastic Potential"));
    for (const auto& scn : scf.getRegistredStressCriteria()) {
      const auto sc = scf.generate(scn);
      const auto ss = sc->getSupportedBehaviourSymmetries();
      if (std::find(ss.begin(), ss.end(), s) != ss.end()) {
        this->stress_criteria->addItem(QString::fromStdString(scn));
        this->plastic_potentials->addItem(QString::fromStdString(scn));
      }
    }
    if (s == mfront::ISOTROPIC) {
      this->stress_criteria->setCurrentText("Mises");
    } else {
      this->stress_criteria->setCurrentText("Hill");
    }
    this->plastic_potentials->setCurrentText("Associated flow");
    /* buttons */
    auto* const bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    auto* const gl = new QGridLayout();
    gl->addWidget(ifl, 0, 0);
    gl->addWidget(this->inelastic_flows, 0, 1);
    gl->addWidget(scl, 1, 0);
    gl->addWidget(this->stress_criteria, 1, 1);
    gl->addWidget(ppl, 2, 0);
    gl->addWidget(this->plastic_potentials, 2, 1);
    //
    auto* const ihrb = new QPushButton(QObject::tr("Add"));
    QObject::connect(ihrb, &QPushButton::pressed, [this] {
      auto* m = dynamic_cast<QStringListModel*>(
          this->isotropic_hardening_rules_selected->model());
      if (m == nullptr) {
        // QAbstractItemView::setModel borrows: the dialog must own the model
        // (issue #66)
        m = new QStringListModel(this);
        this->isotropic_hardening_rules_selected->setModel(m);
      }
      auto ihrs = m->stringList();
      ihrs.append(this->isotropic_hardening_rules->currentText());
      m->setStringList(ihrs);
    });
    const auto& ihrf =
        mfront::bbrick::IsotropicHardeningRuleFactory::getFactory();
    for (const auto& ihrn : ihrf.getRegistredIsotropicHardeningRules()) {
      this->isotropic_hardening_rules->addItem(QString::fromStdString(ihrn));
    }
    gl->addWidget(this->isotropic_hardening_rules_selected, 3, 0, 1, 2);
    gl->addWidget(this->isotropic_hardening_rules, 4, 0);
    gl->addWidget(ihrb, 4, 1);
    //
    auto* const khrb = new QPushButton(QObject::tr("Add"));
    QObject::connect(khrb, &QPushButton::pressed, [this] {
      auto* m = dynamic_cast<QStringListModel*>(
          this->kinematic_hardening_rules_selected->model());
      if (m == nullptr) {
        // same as for the isotropic hardening rules: the view borrows the
        // model, the dialog owns it (issue #66)
        m = new QStringListModel(this);
        this->kinematic_hardening_rules_selected->setModel(m);
      }
      auto khrs = m->stringList();
      khrs.append(this->kinematic_hardening_rules->currentText());
      m->setStringList(khrs);
    });
    const auto& khrf =
        mfront::bbrick::KinematicHardeningRuleFactory::getFactory();
    for (const auto& khrn : khrf.getRegistredKinematicHardeningRules()) {
      this->kinematic_hardening_rules->addItem(QString::fromStdString(khrn));
    }
    gl->addWidget(this->kinematic_hardening_rules_selected, 5, 0, 1, 2);
    gl->addWidget(this->kinematic_hardening_rules, 6, 0);
    gl->addWidget(khrb, 6, 1);
    vl->addLayout(gl);
    vl->addWidget(bb);
    this->setLayout(vl);
  }  // end of
     // MFrontAddInelasticFlowDialog::MFrontAddInelasticFlowDialog

  void MFrontAddInelasticFlowDialog::setInelasticFlowDescription(
      const InelasticFlowDescription& d) {
    this->setWindowTitle(QObject::tr("Modify an inelastic flow"));
    select_option(this->inelastic_flows, d.inelastic_flow);
    select_option(this->stress_criteria, d.stress_criterion);
    if (d.plastic_potential.isEmpty()) {
      select_option(this->plastic_potentials, "Associated flow");
    } else {
      select_option(this->plastic_potentials, d.plastic_potential);
    }
    auto* ihrm = dynamic_cast<QStringListModel*>(
        this->isotropic_hardening_rules_selected->model());
    if (ihrm == nullptr) {
      // same as for the "Add" button: the view borrows the model, the dialog
      // owns it (issue #66)
      ihrm = new QStringListModel(this);
      this->isotropic_hardening_rules_selected->setModel(ihrm);
    }
    ihrm->setStringList(d.isotropic_hardening_rules);
    auto* khrm = dynamic_cast<QStringListModel*>(
        this->kinematic_hardening_rules_selected->model());
    if (khrm == nullptr) {
      // same as for the isotropic hardening rules
      khrm = new QStringListModel(this);
      this->kinematic_hardening_rules_selected->setModel(khrm);
    }
    khrm->setStringList(d.kinematic_hardening_rules);
  }  // end of
     // MFrontAddInelasticFlowDialog::setInelasticFlowDescription

  MFrontAddInelasticFlowDialog::InelasticFlowDescription
  MFrontAddInelasticFlowDialog::getInelasticFlowDescription() const {
    InelasticFlowDescription d;
    d.inelastic_flow = this->inelastic_flows->currentText();
    d.stress_criterion = this->stress_criteria->currentText();
    const auto pp = this->plastic_potentials->currentText();
    if (pp != "Associated flow") {
      d.plastic_potential = pp;
    }
    auto* ihrm = dynamic_cast<QStringListModel*>(
        this->isotropic_hardening_rules_selected->model());
    if (ihrm != nullptr) {
      d.isotropic_hardening_rules = ihrm->stringList();
    }
    auto* khrm = dynamic_cast<QStringListModel*>(
        this->kinematic_hardening_rules_selected->model());
    if (khrm != nullptr) {
      d.kinematic_hardening_rules = khrm->stringList();
    }
    return d;
  }  // end of
     // MFrontAddInelasticFlowDialog::getInelasticFlowDescription

  MFrontAddInelasticFlowDialog::~MFrontAddInelasticFlowDialog() = default;

}  // end of namespace tfel::gui
