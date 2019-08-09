/*!
 * \file   MFrontBehaviourPage.cxx
 * \brief
 * \author th202608
 * \date   27/07/2019
 */

#include <QtCore/QTemporaryFile>
#include <QtGui/QTextDocumentWriter>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#include "TFEL/Material/FiniteStrainBehaviourTangentOperator.hxx"
#include "MFront/DSLFactory.hxx"
#include "MFront/AbstractBehaviourDSL.hxx"
#include "MFront/BehaviourDescription.hxx"
#include "MFront/BehaviourBrickFactory.hxx"
#include "MFront/NonLinearSystemSolverFactory.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MFrontBehaviourPage.hxx"

namespace tfel {

  namespace gui {

    static QString mark_disabled(const char *const s) {
      return "<i>" + QObject::tr(s) + QObject::tr(" (disabled)") +
             "</i>";
    }

    MFrontBehaviourPage::MFrontBehaviourPage(EditorWidget &w,
                                             TextEditBase &cd,
                                             QWizard *const p)
        : QWizardPage(p),
          bts(new QComboBox()),
          iss(new QComboBox()),
          dsls(new QComboBox()),
          algorithms_label(new QLabel(QObject::tr("Algorithm"))),
          algorithms(new QComboBox()),
          hypotheses_label(new QLabel(
              QObject::tr("Supported modelling hypotheses"))),
          hypotheses(new QComboBox()),
          symmetries_label(new QLabel(QObject::tr("Symmetry"))),
          symmetries(new QComboBox()),
          strain_measures_label(
              new QLabel(QObject::tr("Strain measure"))),
          strain_measures(new QComboBox()),
          bricks_label(new QLabel(QObject::tr("Brick"))),
          bricks(new QComboBox()),
          elastic_properties_label(
              new QLabel(QObject::tr("Elastic properties"))),
          elastic_properties(new QComboBox()),
          tangent_operators_label(
              new QLabel(QObject::tr("Tangent Operator"))),
          tangent_operators(new QComboBox()),
          convergence_criterion_label(
              new QLabel(QObject::tr("Convergence criterion"))),
          convergence_criterion(new LineEdit(w)),
          perturbation_value_label(
              new QLabel(QObject::tr("Perturbation value"))),
          perturbation_value(new LineEdit(w)),
          editor(w),
          d(cd) {
      using tfel::material::ModellingHypothesis;
      this->setTitle(
          QObject::tr("DSL selection, behaviour characteristics, "
                      "main numerical parameters"));
      this->setSubTitle(QObject::tr(
          "Selection of the Domain Specific Language "
          "for the new behaviour. This page also allows "
          "specifying various details about the behaviour and "
          "the numerical algorithm."));
      this->bts->addItems(QStringList() << "Strain based behaviour"
                                        << "Finite strain behaviour"
                                        << "Cohesive zone model"
                                        << "Generic behaviour");
      // also calls `updateDSLList`, `updateBrickList`, and
      // `updateElasticPropertyList`, `updateAlgorithmList`
      this->updateIntegrationSchemeList();
      this->updateTangentOperatorList();
      QObject::connect(this->bts, static_cast<void (QComboBox::*)(int)>(
                                      &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateIntegrationSchemeList();
                         this->updateTangentOperatorList();
                       });
      QObject::connect(this->iss, static_cast<void (QComboBox::*)(int)>(
                                      &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateDSLList();
                         this->updateAlgorithmList();
                       });
      QObject::connect(this->dsls,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] { this->updateStrainMeasureList(); });
      QObject::connect(this->algorithms,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] { this->updateNumericalCriteria(); });
      // modelling hypotheses
      QStringList mhs;
      mhs << "Default"
          << "All";
      for (const auto &h :
           ModellingHypothesis::getModellingHypotheses()) {
        mhs << QString::fromStdString(ModellingHypothesis::toString(h));
      }
      this->hypotheses->addItems(mhs);
      this->hypotheses->setCurrentText("Default");
      // behaviour symmetry
      this->symmetries->addItems(QStringList() << "Isotropic"
                                               << "Orthotropic (Pipe)"
                                               << "Orthotropic (Plate)"
                                               << "Orthotropic");
      this->symmetries->setCurrentText("Isotropic");
      // numerical parameters
      auto *const eps = new QDoubleValidator();
      eps->setBottom(0.);
      this->convergence_criterion->setValidator(eps);
      this->convergence_criterion->setText("1.e-14");
      auto *const pv = new QDoubleValidator();
      pv->setBottom(0.);
      this->perturbation_value->setValidator(pv);
      this->perturbation_value->setText("1.e-7");
      // associating labels and combo-boxes/line edits
      this->algorithms_label->setBuddy(this->algorithms);
      this->hypotheses_label->setBuddy(this->hypotheses);
      this->symmetries_label->setBuddy(this->symmetries);
      this->strain_measures_label->setBuddy(this->strain_measures);
      this->bricks_label->setBuddy(this->bricks);
      this->elastic_properties_label->setBuddy(this->elastic_properties);
      this->tangent_operators_label->setBuddy(this->tangent_operators);
      this->convergence_criterion_label->setBuddy(this->convergence_criterion);
      this->perturbation_value_label->setBuddy(this->perturbation_value);
      // alignements
      this->convergence_criterion->setAlignment(Qt::AlignCenter);
      this->perturbation_value->setAlignment(Qt::AlignCenter);
      // final layout
      auto *const gl = new QGridLayout;
      auto *bt_label = new QLabel(QObject::tr("Behaviour type"));
      bt_label->setBuddy(this->bts);
      gl->addWidget(bt_label, 0, 0);
      gl->addWidget(this->bts, 0, 1);
      auto *const behaviourTypeHelp =
          new QPushButton(QIcon::fromTheme("help-contents"), "");
      gl->addWidget(behaviourTypeHelp, 0, 2);
      behaviourTypeHelp->setToolTip(
          "Show a description of available "
          "behaviours' type");
      QObject::connect(behaviourTypeHelp, &QPushButton::clicked, [p] {
        QMessageBox::information(
            p, QObject::tr("Description of MFront behaviours' type"),
            QObject::tr("MFront distinguishes several "
                        "classes of behaviours:\n"
                        "- Standard strain based behaviours\n"
                        "- Standard finite strain behaviours\n"
                        "- Cohesive zone models\n"
                        "- Generic behaviours"));
      });
      auto *const isl = new QLabel(QObject::tr("Integration scheme"));
      isl->setBuddy(this->iss);
      gl->addWidget(isl, 1, 0);
      gl->addWidget(this->iss, 1, 1);
      auto *const iml =
          new QLabel(QObject::tr("Domain Specific Language"));
      iml->setBuddy(this->dsls);
      gl->addWidget(iml, 2, 0);
      gl->addWidget(this->dsls, 2, 1);
      auto *const l1 = new QFrame();
      l1->setFrameShape(QFrame::HLine);
      l1->setFrameShadow(QFrame::Sunken);
      gl->addWidget(l1, 3, 0, 1, 3);
      gl->addWidget(this->hypotheses_label, 4, 0);
      gl->addWidget(this->hypotheses, 4, 1);
      gl->addWidget(this->symmetries_label, 5, 0);
      gl->addWidget(this->symmetries, 5, 1);
      gl->addWidget(this->strain_measures_label, 6, 0);
      gl->addWidget(this->strain_measures, 6, 1);
      gl->addWidget(this->bricks_label, 7, 0);
      gl->addWidget(this->bricks, 7, 1);
      gl->addWidget(this->elastic_properties_label, 8, 0);
      gl->addWidget(this->elastic_properties, 8, 1);
      gl->addWidget(this->elastic_properties_label, 8, 0);
      gl->addWidget(this->elastic_properties, 8, 1);
      gl->addWidget(this->tangent_operators_label, 9, 0);
      gl->addWidget(this->tangent_operators, 9, 1);
      auto *const l2 = new QFrame();
      l2->setFrameShape(QFrame::HLine);
      l2->setFrameShadow(QFrame::Sunken);
      gl->addWidget(l2, 10, 0, 1, 3);
      gl->addWidget(this->algorithms_label, 11, 0);
      gl->addWidget(this->algorithms, 11, 1);
      gl->addWidget(this->convergence_criterion_label, 12, 0);
      gl->addWidget(this->convergence_criterion, 12, 1);
      gl->addWidget(this->perturbation_value_label, 13, 0);
      gl->addWidget(this->perturbation_value, 13, 1);
      this->setLayout(gl);
    }  // end of MFrontBehaviourPage::MFrontBehaviourPage

    bool MFrontBehaviourPage::validatePage() {
      QTemporaryFile tmp("mfront-file");
      if (!tmp.open()) {
        this->editor.displayInformativeMessage(
            QObject::tr("can't open temporary file"));
        return true;
      }
      QTextDocument t;
      QTextCursor tc(&t);
      tc.insertText("@Behaviour Test;\n");
      this->write(t);
      tc.movePosition(QTextCursor::End);
      tc.insertText("\n@Integrator{}\n");
      QTextDocumentWriter writer(&tmp, "plaintext");
      if (!writer.write(&t)) {
        this->editor.displayInformativeMessage(
            QObject::tr("can't write buffer in temporary file"));
        return true;
      }
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        const auto &n = this->getSelectedDomainSpecificLanguage();
        const auto dsl = f.createNewDSL(n.toStdString());
        dsl->analyseFile(tmp.fileName().toStdString(), {}, {});
      } catch (std::exception &e) {
        QMessageBox::warning(&(this->editor), "Inconsistent choice",
                             QString(e.what()));
        return false;
      }
      return true;
    }

    int MFrontBehaviourPage::nextId() const {
      return 2;
    }

    mfront::BehaviourDescription::BehaviourType
    MFrontBehaviourPage::getSelectedBehaviourType() const {
      if (this->bts->currentText() == "Strain based behaviour") {
        return BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR;
      } else if (this->bts->currentText() == "Finite strain behaviour") {
        return BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR;
      } else if (this->bts->currentText() == "Cohesive zone model") {
        return BehaviourDescription::COHESIVEZONEMODEL;
      }
      return BehaviourDescription::GENERALBEHAVIOUR;
    }  // end of MFrontBehaviourPage::getSelectedBehaviourType

    mfront::BehaviourDescription::IntegrationScheme
    MFrontBehaviourPage::getSelectedIntegrationScheme() const {
      if (this->iss->currentText() == "Implicit") {
        return BehaviourDescription::IMPLICITSCHEME;
      } else if (this->iss->currentText() == "Explicit (Runge-Kutta)") {
        return BehaviourDescription::EXPLICITSCHEME;
      } else if (this->iss->currentText() == "Specialized") {
        return BehaviourDescription::SPECIFICSCHEME;
      }
      return BehaviourDescription::USERDEFINEDSCHEME;
    }  // end of MFrontBehaviourPage::getSelectedIntegrationScheme

    void MFrontBehaviourPage::updateIntegrationSchemeList() {
      const auto cis = this->iss->currentText();
      this->iss->clear();
      const auto bt = this->getSelectedBehaviourType();
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        QStringList ischemes;
        for (const auto &n : f.getRegistredParsers(false)) {
          const auto dsl = f.createNewDSL(n);
          if (dsl->getTargetType() ==
              mfront::AbstractDSL::BEHAVIOURDSL) {
            const auto &bdsl =
                dynamic_cast<const mfront::AbstractBehaviourDSL &>(
                    *dsl);
            const auto &bd = bdsl.getBehaviourDescription();
            if (bd.getBehaviourType() == bt) {
              const auto is = [&bd] {
                const auto i = bd.getIntegrationScheme();
                if (i == BehaviourDescription::IMPLICITSCHEME) {
                  return "Implicit";
                } else if (i == BehaviourDescription::EXPLICITSCHEME) {
                  return "Explicit (Runge-Kutta)";
                } else if (i == BehaviourDescription::SPECIFICSCHEME) {
                  return "Specialized";
                }
                return "Other (user defined)";
              }();
              if (!ischemes.contains(is)) {
                ischemes << is;
              }
            }
          }
        }
        this->iss->addItems(ischemes);
        if (ischemes.contains(cis)) {
          this->iss->setCurrentText(cis);
        } else {
          if ((bt ==
               BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) ||
              (bt ==
               BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR) ||
              (bt == BehaviourDescription::GENERALBEHAVIOUR)) {
            if (ischemes.contains("Implicit")) {
              this->iss->setCurrentText("Implicit");
            }
          }
        }
      } catch (...) {
      }
      this->updateDSLList();
      this->updateBrickList();
      this->updateElasticPropertyList();
      this->updateAlgorithmList();
    }  // end of MFrontBehaviourPage::updateIntegrationSchemeList()

    void MFrontBehaviourPage::updateDSLList() {
      using mfront::BehaviourDescription;
      this->dsls->clear();
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        const auto bt = this->getSelectedBehaviourType();
        const auto it = this->getSelectedIntegrationScheme();
        for (const auto &n : f.getRegistredParsers(false)) {
          const auto dsl = f.createNewDSL(n);
          if (dsl->getTargetType() ==
              mfront::AbstractDSL::BEHAVIOURDSL) {
            const auto &bdsl =
                dynamic_cast<const mfront::AbstractBehaviourDSL &>(
                    *dsl);
            const auto &bd = bdsl.getBehaviourDescription();
            if ((bd.getBehaviourType() == bt) &&
                (bd.getIntegrationScheme() == it)) {
              this->dsls->addItem(QString::fromStdString(n));
            }
          }
        }
      } catch (...) {
      }
      this->updateStrainMeasureList();
    }  // end of updateDSLList

    void MFrontBehaviourPage::updateBrickList() {
      const auto t = this->getSelectedBehaviourType();
      const auto i = this->getSelectedIntegrationScheme();
      const auto &pb = this->bricks->currentText();
      this->bricks->clear();
      const auto &f = mfront::BehaviourBrickFactory::getFactory();
      auto bs = QStringList{};
      for (const auto &n : f.getRegistredBricks(t, i)) {
        bs << QString::fromStdString(n);
      }
      if (bs.isEmpty()) {
        this->bricks_label->setText(mark_disabled("Brick"));
        this->bricks->setDisabled(true);
      } else {
        this->bricks->addItem("None");
        this->bricks->addItems(bs);
        if (bs.contains(pb)) {
          this->bricks->setCurrentText(pb);
        } else {
          this->bricks->setCurrentText("None");
        }
        this->bricks_label->setText(QObject::tr("Brick"));
        this->bricks->setEnabled(true);
      }
    }  // end of MFrontBehaviourPage::updateBrickList

    void MFrontBehaviourPage::updateAlgorithmList() {
      this->algorithms->clear();
      try {
        const auto i = this->getSelectedIntegrationScheme();
        if (i == BehaviourDescription::IMPLICITSCHEME) {
          const auto &f = mfront::NonLinearSystemSolverFactory::
              getNonLinearSystemSolverFactory();
          auto solvers = QStringList{};
          for (const auto &s : f.getRegistredSolvers()) {
            solvers << QString::fromStdString(s);
          }
          this->algorithms->addItems(solvers);
          if (solvers.contains("NewtonRaphson")) {
            this->algorithms->setCurrentText("NewtonRaphson");
            this->updateNumericalCriteria();
          }
        } else if (i == BehaviourDescription::EXPLICITSCHEME) {
          this->algorithms->addItems(QStringList() << "rk54"
                                                   << "rk42"
                                                   << "rkCastem"
                                                   << "rk4"
                                                   << "rk2"
                                                   << "euler");
        }
      } catch (...) {
      }
      if (this->algorithms->count() != 0) {
        this->algorithms_label->setText(QObject::tr("Algorithm"));
        this->algorithms->setEnabled(true);
      } else {
        this->algorithms_label->setText(mark_disabled("Algorithm"));
        this->algorithms->setDisabled(true);
      }
    }  // end of MFrontBehaviourPage::updateAlgorithmList

    void MFrontBehaviourPage::updateStrainMeasureList() {
      this->strain_measures->clear();
      const auto bt = this->getSelectedBehaviourType();
      if (bt != BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) {
        this->strain_measures_label->setText(mark_disabled("Strain measure"));
        this->strain_measures->setDisabled(true);
        return;
      }
      this->strain_measures->addItems(QStringList()
                                      << "Unspecified"
                                      << "Hencky"
                                      << "Green-Lagragrange"
                                      << "Linearized");
      this->strain_measures_label->setText(
          QObject::tr("Strain measure"));
      this->strain_measures->setEnabled(true);
    }  // end of MFrontBehaviourPage::updateStrainMeasureList()

    void MFrontBehaviourPage::updateElasticPropertyList() {
      const auto p = this->elastic_properties->currentText();
      const auto bt = this->getSelectedBehaviourType();
      this->elastic_properties->clear();
      if (!((bt ==
             BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) ||
            (bt ==
             BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR))) {
        this->elastic_properties_label->setText(
            mark_disabled("Elastic properties"));
        return;
      }
      const auto i = this->getSelectedIntegrationScheme();
      this->elastic_properties_label->setText("Elastic properties");
auto emps = QStringList{};
      if (i == BehaviourDescription::SPECIFICSCHEME) {
        emps << "Undefined"
             << "@ElasticMaterialProperties";
      } else {
        emps << "Undefined"
             << "@ElasticMaterialProperties"
             << "@ComputeStiffnessTensor<Altered>"
             << "@ComputeStiffnessTensor<UnAltered>"
             << "@RequireStiffnessTensor<Altered>"
             << "@RequireStiffnessTensor<UnAltered>";
      }
      this->elastic_properties->addItems(emps);
      if (emps.contains(p)) {
        this->elastic_properties->setCurrentText(p);
      } else {
        this->elastic_properties->setCurrentText("Undefined");
      }
    }  // end of MFrontBehaviourPage::updateElasticPropertyList

    void MFrontBehaviourPage::updateTangentOperatorList() {
      using namespace tfel::material;
      const auto bt = this->getSelectedBehaviourType();
      this->tangent_operators->clear();
      if (bt != BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR) {
        this->tangent_operators_label->setText(
            mark_disabled("Tangent Operator"));
        this->tangent_operators->setDisabled(true);
        return;
      }
      this->tangent_operators_label->setText(
          QObject::tr("Tangent Operator"));
      this->tangent_operators->setEnabled(true);
      for (const auto to :
           getFiniteStrainBehaviourTangentOperatorFlags()) {
        this->tangent_operators->addItem(QString::fromStdString(
            convertFiniteStrainBehaviourTangentOperatorFlagToString(
                to)));
      }
    }  // end of MFrontBehaviourPage::updateTangentOperatorList

    void MFrontBehaviourPage::updateNumericalCriteria() {
      auto disable_perturbation_value = [this] {
        this->perturbation_value_label->setText(
            mark_disabled("Perturbation value"));
        this->perturbation_value->setDisabled(true);
      };
      const auto i = this->getSelectedIntegrationScheme();
      if ((i == BehaviourDescription::IMPLICITSCHEME) ||
          (i == BehaviourDescription::EXPLICITSCHEME)) {
        this->convergence_criterion_label->setText(
            QObject::tr("Convergence criterion"));
        this->convergence_criterion->setEnabled(true);
      } else {
        this->convergence_criterion_label->setText(
            mark_disabled("Convergence criterion"));
        this->convergence_criterion->setDisabled(true);
      }
      if (i == BehaviourDescription::IMPLICITSCHEME) {
        try {
          const auto &f = mfront::NonLinearSystemSolverFactory::
              getNonLinearSystemSolverFactory();
          const auto &a = this->algorithms->currentText();
          const auto s = f.getSolver(a.toStdString());
          if ((s->usesJacobian()) && (s->requiresNumericalJacobian())) {
            this->perturbation_value_label->setText(
                "Perturbation value");
            this->perturbation_value->setEnabled(true);
          } else {
            disable_perturbation_value();
          }
        } catch (...) {
          disable_perturbation_value();
        }
      } else {
        disable_perturbation_value();
      }
    }  // end of MFrontBehaviourPage::updateNumericalCriteria

    QString MFrontBehaviourPage::getSelectedDomainSpecificLanguage()
        const {
      return this->dsls->currentText();
    }  // end of
       // MFrontBehaviourPage::getSelectedDomainSpecificLanguage()

    void MFrontBehaviourPage::write(QTextDocument &t) const {
      auto tc = QTextCursor(&t);
      const auto bt = this->getSelectedBehaviourType();
      const auto i = this->getSelectedIntegrationScheme();
      const auto h = this->hypotheses->currentText();
      const auto s = this->symmetries->currentText();
      if (h != "Default") {
        if (h == "All") {
          tc.insertText("@ModellingHypotheses {\".+\"};\n\n");
        } else {
          tc.insertText("@ModellingHypothesis " + h +
                                  ";\n\n");
        }
      }
      if (s == "Orthotropic (Plate)") {
        tc.insertText("@OrthotropicBehaviour<Plate>;\n\n");
      } else if (s == "Orthotropic (Pipe)") {
        tc.insertText("@OrthotropicBehaviour<Pipe>;\n\n");
      } else if (s == "Orthotropic") {
        tc.insertText("@OrthotropicBehaviour;\n\n");
      }
      if (bt == BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) {
        const auto &sm = this->strain_measures->currentText();
        if (sm == "Green-Lagragrange") {
          tc.insertText("@StrainMeasure GreenLagrange;\n\n");
        } else if ((sm == "Hencky")||(sm == "Linearized")) {
          tc.insertText("@StrainMeasure " + sm + ";\n\n");
        }
      }
      const auto &b = this->bricks->currentText();
      if ((!b.isEmpty()) && (b != "None")) {
        tc.insertText("@Brick " + b + ";\n\n");
      }
      if ((i == BehaviourDescription::IMPLICITSCHEME) ||
          (i == BehaviourDescription::EXPLICITSCHEME)) {
        try {
          const auto &f = mfront::NonLinearSystemSolverFactory::
              getNonLinearSystemSolverFactory();
          const auto &a = this->algorithms->currentText();
          const auto &eps = this->convergence_criterion->text();
          const auto &pv = this->perturbation_value->text();
          if (!a.isEmpty()) {
            tc.insertText("@Algorithm " + a + ";\n");
          }
          if (!eps.isEmpty()) {
            tc.insertText("@Epsilon " + eps + ";\n");
          }
          const auto so = f.getSolver(a.toStdString());
          if ((so->usesJacobian()) && (so->requiresNumericalJacobian())) {
            if (!pv.isEmpty()) {
              tc.insertText(
                  "@PerturbationValueFor"
                  "NumericalJacobianComputation " +
                  pv + ";\n");
            }
          }
          tc.insertText("\n");
        } catch (...) {
        }
      }
    }  // end of MFrontBehaviourPage::write()

    void MFrontBehaviourPage::write() const {
      this->write(*(this->d.document()));
    }  // end of MFrontBehaviourPage::write()

    MFrontBehaviourPage::~MFrontBehaviourPage() = default;

  }  // end of namespace gui

}  // end of namespace tfel
