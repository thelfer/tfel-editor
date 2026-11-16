/*!
 * \file   MFrontStandardElastoViscoPlasticityBrickPage.cxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#include <algorithm>
#include <QtCore/QItemSelectionModel>
#include <QtGui/QStandardItemModel>
#include <QtGui/QValidator>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListView>
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

  namespace {

    /*!
     * \brief the name an inelastic flow is listed with by the page: the name of
     * the flow and the criterion it goes with, which is what tells two flows of
     * a brick apart at a glance
     */
    QString flow_label(
        const MFrontAddInelasticFlowDialog::InelasticFlowDescription &f) {
      if (f.plastic_potential.isEmpty()) {
        return f.inelastic_flow + " (criterion: " + f.stress_criterion + ")";
      }
      return f.inelastic_flow + " (criterion: " + f.stress_criterion +
             ", plastic potential: " + f.plastic_potential + ")";
    }  // end of flow_label

    /*!
     * \brief the whole description of an inelastic flow, as the tool tip of the
     * entry which names it: two flows can differ by their hardening rules only,
     * which the name of the flow and of its criterion says nothing about
     */
    QString flow_details(
        const MFrontAddInelasticFlowDialog::InelasticFlowDescription &f) {
      auto r = QStringList{};
      r << (QString("inelastic flow: ") + f.inelastic_flow);
      r << (QString("stress criterion: ") + f.stress_criterion);
      if (f.plastic_potential.isEmpty()) {
        r << QString("associated flow");
      } else {
        r << (QString("plastic potential: ") + f.plastic_potential);
      }
      for (const auto &ihr : f.isotropic_hardening_rules) {
        r << (QString("isotropic hardening rule: ") + ihr);
      }
      for (const auto &khr : f.kinematic_hardening_rules) {
        r << (QString("kinematic hardening rule: ") + khr);
      }
      return r.join("\n");
    }  // end of flow_details

    /*!
     * \brief the row of the flow the list selects, -1 when it selects none
     *
     * The enablement of the two buttons and the flow they work on are read with
     * this one function, which is what keeps them naming the same flow.
     */
    int selected_flow_row(const QListView *const v) {
      auto *const sm = v->selectionModel();
      if ((sm == nullptr) || (!sm->hasSelection())) {
        return -1;
      }
      return sm->selection().first().topLeft().row();
    }  // end of selected_flow_row

  }  // end of namespace

  MFrontStandardElastoViscoPlasticityBrickPage::
      MFrontStandardElastoViscoPlasticityBrickPage(
          EditorWidget &w, TextEditBase &cd, MFrontBehaviourWizard *const p)
      : QWizardPage(p),
        stress_potentials(new QComboBox),
        inelastic_flows_model(new QStandardItemModel(this)),
        inelastic_flows_list(new QListView),
        modify_inelastic_flow(new QPushButton(QObject::tr("Modify"))),
        remove_inelastic_flow(new QPushButton(QObject::tr("Remove"))),
        editor(w),
        d(cd) {
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
    // the inelastic flows which have been added are listed under the label
    // which names them and above the three buttons which work on that list: a
    // flow one can not see is one one can neither check nor take back
    auto *const ifl = new QLabel(QObject::tr("Inelastic flows"));
    gl->addWidget(ifl, 3, 0, 1, 3, Qt::AlignHCenter);
    auto *const ifv = this->inelastic_flows_list;
    // the page owns the model it hands to the view, which borrows the model
    // (issue #66)
    ifv->setModel(this->inelastic_flows_model);
    // one flow at a time is what the two buttons below the list work on
    ifv->setSelectionMode(QAbstractItemView::SingleSelection);
    ifv->setSelectionBehavior(QAbstractItemView::SelectRows);
    // an entry names a flow the page holds: typing into it would make the list
    // and the page disagree, and write() answers what the page holds
    ifv->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // an empty list is the state the page starts in, and one as tall as a
    // single row is a list nobody can see emptying
    const auto rh = std::max(1, ifv->sizeHintForRow(0));
    ifv->setMinimumHeight(6 * (rh + 2));
    gl->addWidget(ifv, 4, 0, 1, 3);
    auto *const ifb = new QPushButton(QObject::tr("Add"));
    gl->addWidget(ifb, 5, 0);
    gl->addWidget(this->modify_inelastic_flow, 5, 1);
    gl->addWidget(this->remove_inelastic_flow, 5, 2);
    QObject::connect(ifb, &QPushButton::pressed, this,
                     &MFrontStandardElastoViscoPlasticityBrickPage::
                         addInelasticFlow);
    QObject::connect(this->modify_inelastic_flow, &QPushButton::pressed, this,
                     &MFrontStandardElastoViscoPlasticityBrickPage::
                         modifyInelasticFlow);
    QObject::connect(this->remove_inelastic_flow, &QPushButton::pressed, this,
                     &MFrontStandardElastoViscoPlasticityBrickPage::
                         removeInelasticFlow);
    // a modification and a removal work on the selected flow, so it is the
    // selection which decides whether they have anything to work on
    QObject::connect(ifv->selectionModel(),
                     &QItemSelectionModel::selectionChanged, this,
                     &MFrontStandardElastoViscoPlasticityBrickPage::
                         updateInelasticFlowsButtons);
    // the list is empty and the two buttons which work on the selected flow
    // have nothing to work on
    this->updateInelasticFlowsList(-1);
    this->setLayout(gl);
  }  // end of
     // MFrontStandardElastoViscoPlasticityBrickPage::MFrontStandardElastoViscoPlasticityBrickPage

  void MFrontStandardElastoViscoPlasticityBrickPage::updateInelasticFlowsList(
      const int i) {
    auto *const m = this->inelastic_flows_model;
    // clear() takes the rows and the selection with them, which is why the
    // buttons are dealt with at the end of this function and not only by the
    // signal of the selection
    m->clear();
    for (const auto &f : this->inelastic_flows) {
      auto *const item = new QStandardItem(flow_label(f));
      // two flows can differ by their hardening rules only, which the name says
      // nothing about: the whole description goes in the tool tip
      item->setToolTip(flow_details(f));
      // the model owns the item from here on
      m->appendRow(item);
    }
    if (i >= 0) {
      // the row is made current *and* selected: what the two buttons work on is
      // the selection, so it is named rather than left to the default command a
      // view answers a programmatic change of the current row with
      const auto idx = m->index(i, 0);
      this->inelastic_flows_list->selectionModel()->setCurrentIndex(
          idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      this->inelastic_flows_list->scrollTo(idx);
    }
    this->updateInelasticFlowsButtons();
  }  // end of
     // MFrontStandardElastoViscoPlasticityBrickPage::updateInelasticFlowsList

  void MFrontStandardElastoViscoPlasticityBrickPage::
      updateInelasticFlowsButtons() {
    // a modification and a removal work on the selected flow: an empty list,
    // and a list whose entries are all deselected, leaves them with nothing to
    // do
    const auto selected = selected_flow_row(this->inelastic_flows_list) >= 0;
    this->modify_inelastic_flow->setEnabled(selected);
    this->remove_inelastic_flow->setEnabled(selected);
  }  // end of
     // MFrontStandardElastoViscoPlasticityBrickPage::updateInelasticFlowsButtons

  void MFrontStandardElastoViscoPlasticityBrickPage::addInelasticFlow() {
    auto *const p = dynamic_cast<MFrontBehaviourWizard *const>(this->wizard());
    if (p == nullptr) {
      QMessageBox::warning(&(this->editor), "Internal error",
                           "invalid wizard reference");
      return;
    }
    MFrontAddInelasticFlowDialog dialog(p);
    if (dialog.exec() != QDialog::Accepted) {
      return;
    }
    this->inelastic_flows.push_back(dialog.getInelasticFlowDescription());
    // the flow which has just been added is the one the user is looking at
    this->updateInelasticFlowsList(
        static_cast<int>(this->inelastic_flows.size()) - 1);
  }  // end of MFrontStandardElastoViscoPlasticityBrickPage::addInelasticFlow

  void MFrontStandardElastoViscoPlasticityBrickPage::modifyInelasticFlow() {
    // the list and the vector are kept in step, so the row of one is the same
    // flow as the entry of the other of the same index
    const auto i = selected_flow_row(this->inelastic_flows_list);
    if ((i < 0) ||
        (static_cast<decltype(inelastic_flows.size())>(i) >=
         this->inelastic_flows.size())) {
      // the button of the page is disabled when the list selects no flow
      return;
    }
    auto *const p = dynamic_cast<MFrontBehaviourWizard *const>(this->wizard());
    if (p == nullptr) {
      QMessageBox::warning(&(this->editor), "Internal error",
                           "invalid wizard reference");
      return;
    }
    const auto n = static_cast<decltype(inelastic_flows.size())>(i);
    MFrontAddInelasticFlowDialog dialog(p);
    dialog.setInelasticFlowDescription(this->inelastic_flows[n]);
    if (dialog.exec() != QDialog::Accepted) {
      // the flow is left as it was
      return;
    }
    this->inelastic_flows[n] = dialog.getInelasticFlowDescription();
    // the entry which names the modified flow keeps its place and the selection
    this->updateInelasticFlowsList(i);
  }  // end of MFrontStandardElastoViscoPlasticityBrickPage::modifyInelasticFlow

  void MFrontStandardElastoViscoPlasticityBrickPage::removeInelasticFlow() {
    const auto i = selected_flow_row(this->inelastic_flows_list);
    if ((i < 0) ||
        (static_cast<decltype(inelastic_flows.size())>(i) >=
         this->inelastic_flows.size())) {
      // the button of the page is disabled when the list selects no flow
      return;
    }
    this->inelastic_flows.erase(this->inelastic_flows.begin() + i);
    // the entry which takes the place of the removed one is selected, the last
    // entry of the list when the removed one was the last one
    const auto n = static_cast<int>(this->inelastic_flows.size());
    this->updateInelasticFlowsList((n == 0) ? -1 : std::min(i, n - 1));
  }  // end of MFrontStandardElastoViscoPlasticityBrickPage::removeInelasticFlow

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
      // getBehaviourDSL() returns a null shared_ptr when the choices made in
      // the behaviour page are inconsistent or when the temporary file cannot
      // be written. getBehaviourDescription() is pure virtual, so calling it
      // on a null pointer loads the vptr from address 0: a SIGSEGV, which the
      // catch below cannot catch because a signal is not an exception.
      if (dsl == nullptr) {
        return;
      }
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
      for (const auto &ihrn : c.isotropic_hardening_rules) {
        append(",\nisotropic_hardening\n: \"" + ihrn + "\"");
        try {
          const auto &ihrf =
              mfront::bbrick::IsotropicHardeningRuleFactory::getFactory();
          const auto ihr = ihrf.generate(ihrn.toStdString());
          write_options(ihr->getOptions());
        } catch (...) {
        }
      }
      for (const auto &khrn : c.kinematic_hardening_rules) {
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
