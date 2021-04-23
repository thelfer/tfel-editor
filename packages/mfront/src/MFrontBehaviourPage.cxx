/*!
 * \file   MFrontBehaviourPage.cxx
 * \brief
 * \author th202608
 * \date   27/07/2019
 */

#include <QtCore/QTemporaryFile>
#include <QtGui/QTextCursor>
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
#include "MFront/BehaviourBrickDescription.hxx"
#include "MFront/BehaviourBrickFactory.hxx"
#include "MFront/NonLinearSystemSolverFactory.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontBehaviourWizardPages.hxx"
#include "TFEL/GUI/MFrontBehaviourPage.hxx"

namespace tfel {

  namespace gui {

    static QString mark_disabled(const char *const s) {
      return "<i>" + QObject::tr(s) + QObject::tr(" (disabled)") + "</i>";
    }

    MFrontBehaviourPage::MFrontBehaviourPage(EditorWidget &w,
                                             TextEditBase &cd,
                                             MFrontBehaviourWizard *const p)
        : QWizardPage(p),
          bts(new QComboBox()),
          iss(new QComboBox()),
          dsls(new QComboBox()),
          algorithms_label(new QLabel(QObject::tr("Algorithm"))),
          algorithms(new QComboBox()),
          hypotheses_label(
              new QLabel(QObject::tr("Supported modelling hypotheses"))),
          hypotheses(new QComboBox()),
          symmetries_label(new QLabel(QObject::tr("Symmetry"))),
          symmetries(new QComboBox()),
          strain_measures_label(new QLabel(QObject::tr("Strain measure"))),
          strain_measures(new QComboBox()),
          bricks_label(new QLabel(QObject::tr("Brick"))),
          bricks(new QComboBox()),
          elastic_properties_label(
              new QLabel(QObject::tr("Elastic properties"))),
          elastic_properties(new QComboBox()),
          crystal_structures_label(
              new QLabel(QObject::tr("Crystal structure"))),
          crystal_structures(new QComboBox()),
          tangent_operators_label(new QLabel(QObject::tr("Tangent Operator"))),
          tangent_operators(new QComboBox()),
          convergence_criterion_label(
              new QLabel(QObject::tr("Convergence criterion"))),
          convergence_criterion(new LineEdit(w)),
          perturbation_value_label(
              new QLabel(QObject::tr("Perturbation value"))),
          perturbation_value(new LineEdit(w)),
          editor(w),
          d(cd) {
      this->setTitle(
          QObject::tr("DSL selection, behaviour characteristics, "
                      "main numerical parameters"));
      this->setSubTitle(
          QObject::tr("Selection of the Domain Specific Language "
                      "for the new behaviour. This page also allows "
                      "specifying various details about the behaviour and "
                      "the numerical algorithm."));
      this->bts->addItems(QStringList() << "Strain based behaviour"
                                        << "Finite strain behaviour"
                                        << "Cohesive zone model"
                                        << "Generic behaviour");
      // also calls `updateDSLList`, `updateBrickList`, and
      // `updateElasticPropertyList`, `updateAlgorithmList`
      // `updateModellingHypothesisList`, `updateBehaviourSymmetryList`
      // `updateCrystalStructureList`
      this->updateIntegrationSchemeList();
      this->updateTangentOperatorList();
      QObject::connect(this->bts,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateIntegrationSchemeList();
                         this->updateTangentOperatorList();
                       });
      QObject::connect(this->iss,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateDSLList();
                         this->updateAlgorithmList();
                       });
      QObject::connect(this->dsls,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateStrainMeasureList();
                         this->updateBrickList();
                       });
      QObject::connect(this->bricks,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] {
                         this->updateModellingHypothesisList();
                         this->updateBehaviourSymmetryList();
                         this->updateCrystalStructureList();
                         this->updateElasticPropertyList();
                       });
      QObject::connect(this->hypotheses,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] { this->updateElasticPropertyList(); });
      QObject::connect(this->algorithms,
                       static_cast<void (QComboBox::*)(int)>(
                           &QComboBox::currentIndexChanged),
                       [this] { this->updateNumericalCriteria(); });
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
      this->crystal_structures_label->setBuddy(this->crystal_structures);
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
      auto *const iml = new QLabel(QObject::tr("Domain Specific Language"));
      iml->setBuddy(this->dsls);
      gl->addWidget(iml, 2, 0);
      gl->addWidget(this->dsls, 2, 1);
      gl->addWidget(this->bricks_label, 3, 0);
      gl->addWidget(this->bricks, 3, 1);
      auto *const l1 = new QFrame();
      l1->setFrameShape(QFrame::HLine);
      l1->setFrameShadow(QFrame::Sunken);
      gl->addWidget(l1, 4, 0, 1, 3);
      gl->addWidget(this->hypotheses_label, 5, 0);
      gl->addWidget(this->hypotheses, 5, 1);
      gl->addWidget(this->symmetries_label, 6, 0);
      gl->addWidget(this->symmetries, 6, 1);
      gl->addWidget(this->strain_measures_label, 7, 0);
      gl->addWidget(this->strain_measures, 7, 1);
      gl->addWidget(this->elastic_properties_label, 8, 0);
      gl->addWidget(this->elastic_properties, 8, 1);
      gl->addWidget(this->crystal_structures_label, 9, 0);
      gl->addWidget(this->crystal_structures, 9, 1);
      gl->addWidget(this->tangent_operators_label, 10, 0);
      gl->addWidget(this->tangent_operators, 10, 1);
      auto *const l2 = new QFrame();
      l2->setFrameShape(QFrame::HLine);
      l2->setFrameShadow(QFrame::Sunken);
      gl->addWidget(l2, 11, 0, 1, 3);
      gl->addWidget(this->algorithms_label, 12, 0);
      gl->addWidget(this->algorithms, 12, 1);
      gl->addWidget(this->convergence_criterion_label, 13, 0);
      gl->addWidget(this->convergence_criterion, 13, 1);
      gl->addWidget(this->perturbation_value_label, 14, 0);
      gl->addWidget(this->perturbation_value, 14, 1);
      this->setLayout(gl);
    }  // end of MFrontBehaviourPage::MFrontBehaviourPage

    std::shared_ptr<mfront::AbstractBehaviourDSL>
    MFrontBehaviourPage::getBehaviourDSL(const DSLGenerationOptions &o) const {
      QTemporaryFile tmp("mfront-file");
      if (!tmp.open()) {
        this->editor.displayInformativeMessage(
            QObject::tr("can't open temporary file"));
        return {};
      }
      QTextDocument t;
      QTextCursor tc(&t);
      tc.insertText("@Behaviour Test;\n");
      this->write(tc, o, MFrontBehaviourPage::PROCESSING_STAGE);
      tc.movePosition(QTextCursor::End);
      QTextDocumentWriter writer(&tmp, "plaintext");
      if (!writer.write(&t)) {
        this->editor.displayInformativeMessage(
            QObject::tr("can't write buffer in temporary file"));
        return {};
      }
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        const auto &n = this->getSelectedDomainSpecificLanguage();
        const auto dsl = f.createNewDSL(n.toStdString());
        dsl->importFile(tmp.fileName().toStdString(), {}, {});
        return std::dynamic_pointer_cast<mfront::AbstractBehaviourDSL>(dsl);
      } catch (std::exception &e) {
        QMessageBox::warning(&(this->editor), "Inconsistent choice",
                             QString(e.what()));
      }
      return {};
    }  // end of MFrontBehaviourPage::getBehaviourDSL

    bool MFrontBehaviourPage::validatePage() {
      QTemporaryFile tmp("mfront-file");
      if (!tmp.open()) {
        this->editor.displayInformativeMessage(
            QObject::tr("can't open temporary file"));
        return true;
      }
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        const auto &n = this->getSelectedDomainSpecificLanguage();
        const auto dsl =
            std::dynamic_pointer_cast<mfront::AbstractBehaviourDSL>(
                f.createNewDSL(n.toStdString()));
        const auto &dsld = dsl->getBehaviourDSLDescription();
        QTextDocument t;
        QTextCursor tc(&t);
        tc.insertText("@DSL " + n + ";\n");
        tc.insertText("@Behaviour Test;\n");
        this->write(tc, {}, MFrontBehaviourPage::VALIDATE_STAGE);
        tc.movePosition(QTextCursor::End);
        const auto b = dsld.minimalMFrontFileBody;
        tc.insertText("\n" + QString::fromStdString(b) + "\n");
        QTextDocumentWriter writer(&tmp, "plaintext");
        if (!writer.write(&t)) {
          this->editor.displayInformativeMessage(
              QObject::tr("can't write buffer in temporary file"));
          return true;
        }
        dsl->analyseFile(tmp.fileName().toStdString(), {}, {});
      } catch (std::exception &e) {
        QMessageBox::warning(&(this->editor), "Inconsistent choice",
                             QString(e.what()));
        return false;
      }
      return true;
    }

    int MFrontBehaviourPage::nextId() const {
      const auto b = this->bricks->currentText();
      if (b == "StandardElastoViscoPlasticity") {
        return MFrontBehaviourWizardPages::STANDARDELASTOVISCOPLASTICITY;
      } else {
        return MFrontBehaviourWizardPages::ADDVARIABLES;
      }
    }

    void MFrontBehaviourPage::updateIntegrationSchemeList() {
      const auto cis = this->iss->currentText();
      this->iss->clear();
      const auto bt = this->getSelectedBehaviourType();
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        QStringList ischemes;
        for (const auto &n : f.getRegistredParsers(false)) {
          const auto dsl = f.createNewDSL(n);
          if (dsl->getTargetType() == mfront::AbstractDSL::BEHAVIOURDSL) {
            const auto &bdsl =
                dynamic_cast<const mfront::AbstractBehaviourDSL &>(*dsl);
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
          if ((bt == BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) ||
              (bt == BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR) ||
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
          if (dsl->getTargetType() == mfront::AbstractDSL::BEHAVIOURDSL) {
            const auto &bdsl =
                dynamic_cast<const mfront::AbstractBehaviourDSL &>(*dsl);
            const auto &bd = bdsl.getBehaviourDescription();
            if ((bd.getBehaviourType() == bt) &&
                (bd.getIntegrationScheme() == it)) {
              this->dsls->addItem(QString::fromStdString(n));
            }
          }
        }
      } catch (...) {
      }
      this->updateBrickList();
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
      this->updateModellingHypothesisList();
      this->updateBehaviourSymmetryList();
      this->updateCrystalStructureList();
      this->updateElasticPropertyList();
    }  // end of MFrontBehaviourPage::updateBrickList

    void MFrontBehaviourPage::updateModellingHypothesisList() {
      using tfel::material::ModellingHypothesis;
      const auto ch = this->hypotheses->currentText();
      this->hypotheses->clear();
      const auto dsld = this->getCurrentBehaviourDSLDescription();
      const auto amhs = ModellingHypothesis::getModellingHypotheses();
      const auto &smhs = dsld.supportedModellingHypotheses;
      QStringList mhs;
      if (smhs.size() != 1u) {
        mhs << "Default";
      }
      if (smhs.size() == amhs.size()) {
        // paranoic check
        if (std::equal(smhs.begin(), smhs.end(), amhs.begin())) {
          mhs << "All";
        }
      }
      for (const auto &h : smhs) {
        mhs << QString::fromStdString(ModellingHypothesis::toString(h));
      }
      this->hypotheses->addItems(mhs);
      if ((ch.isEmpty()) && (mhs.contains("Default"))) {
        this->hypotheses->setCurrentText("Default");
      } else if (mhs.contains(ch)) {
        this->hypotheses->setCurrentText(ch);
      }
      this->updateElasticPropertyList();
    }  // end of MFrontBehaviourPage::updateModellingHypothesisList

    void MFrontBehaviourPage::updateBehaviourSymmetryList() {
      using tfel::material::ModellingHypothesis;
      const auto cs = this->symmetries->currentText();
      const auto dsld = this->getCurrentBehaviourDSLDescription();
      const auto &bhs = dsld.supportedBehaviourSymmetries;
      this->symmetries->clear();
      QStringList ss;
      if (std::find(bhs.cbegin(), bhs.cend(), mfront::ISOTROPIC) !=
          bhs.cend()) {
        ss << "Isotropic";
      }
      if (std::find(bhs.cbegin(), bhs.cend(), mfront::ORTHOTROPIC) !=
          bhs.cend()) {
        const auto &mhs = dsld.supportedModellingHypotheses;
        if ((mhs.size() == 1u) &&
            (mhs.front() == ModellingHypothesis::TRIDIMENSIONAL)) {
          ss << "Orthotropic";
        } else {
          ss << "Orthotropic (Pipe)"
             << "Orthotropic (Plate)"
             << "Orthotropic";
        }
      }
      this->symmetries->addItems(ss);
      if (ss.size() == 1) {
        this->symmetries_label->setText(mark_disabled("Symmetry"));
        this->symmetries->setDisabled(true);
      } else {
        this->symmetries_label->setText(QObject::tr("Symmetry"));
        this->symmetries->setEnabled(true);
      }
      if (cs.isEmpty()) {
        if (ss.contains("Isotropic")) {
          this->symmetries->setCurrentText("Isotropic");
        }
      } else {
        if (ss.contains(cs)) {
          this->symmetries->setCurrentText(cs);
        }
      }
    }  // end of MFrontBehaviourPage::updateBehaviourSymmetryList

    void MFrontBehaviourPage::updateCrystalStructureList() {
      const auto cs = this->crystal_structures->currentText();
      const auto dsld = this->getCurrentBehaviourDSLDescription();
      this->crystal_structures->clear();
      if (dsld.allowCrystalStructureDefinition) {
        QStringList css;
        if (!dsld.requireCrystalStructureDefinition) {
          css << "None";
        }
        css << "Cubic"
            << "Body-centered cubic"
            << "Face-centered cubic"
            << "Hexagonal close packing";
        this->crystal_structures->addItems(css);
        if (css.contains(cs)) {
          this->crystal_structures->setCurrentText(cs);
        }
        this->crystal_structures_label->setText(
            QObject::tr("Crystal structure"));
        this->crystal_structures->setEnabled(true);
      } else {
        this->crystal_structures_label->setText(
            mark_disabled("Crystal structure"));
        this->crystal_structures->setDisabled(true);
      }
    }  // end of MFrontBehaviourPage::updateCrystalStructureList

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
      this->strain_measures->addItems(QStringList() << "Unspecified"
                                                    << "Hencky"
                                                    << "Green-Lagragrange"
                                                    << "Linearized");
      this->strain_measures_label->setText(QObject::tr("Strain measure"));
      this->strain_measures->setEnabled(true);
    }  // end of MFrontBehaviourPage::updateStrainMeasureList()

    void MFrontBehaviourPage::updateElasticPropertyList() {
      using tfel::material::ModellingHypothesis;
      const auto p = this->elastic_properties->currentText();
      const auto bt = this->getSelectedBehaviourType();
      const auto dsld = this->getCurrentBehaviourDSLDescription();
      auto add_stiffness_tensor = [this, &dsld](QStringList &emps) {
        const auto h = this->hypotheses->currentText();
        const auto ps = QString::fromStdString(
            ModellingHypothesis::toString(ModellingHypothesis::PLANESTRESS));
        const auto agps = QString::fromStdString(ModellingHypothesis::toString(
            ModellingHypothesis::AXISYMMETRICALGENERALISEDPLANESTRESS));
        if ((h == "All") || (h == ps) || (h == agps)) {
          emps << "@ComputeStiffnessTensor<Altered>"
               << "@ComputeStiffnessTensor<UnAltered>"
               << "@RequireStiffnessTensor<Altered>"
               << "@RequireStiffnessTensor<UnAltered>";
        } else {
          emps << "@ComputeStiffnessTensor"
               << "@RequireStiffnessTensor";
        }
      };
      this->elastic_properties->clear();
      if (!((bt == BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) ||
            (bt == BehaviourDescription::STANDARDFINITESTRAINBEHAVIOUR))) {
        this->elastic_properties_label->setText(
            mark_disabled("Elastic properties"));
        return;
      }
      this->elastic_properties_label->setText("Elastic properties");
      auto emps = QStringList{};
      if (dsld.requireStiffnessTensorDefinition) {
        add_stiffness_tensor(emps);
      } else {
        emps << "Undefined";
        if (dsld.allowElasticPropertiesDefinition) {
          emps << "@ElasticMaterialProperties";
        }
        if (dsld.allowStiffnessTensorDefinition) {
          add_stiffness_tensor(emps);
        }
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
      this->tangent_operators_label->setText(QObject::tr("Tangent Operator"));
      this->tangent_operators->setEnabled(true);
      for (const auto to : getFiniteStrainBehaviourTangentOperatorFlags()) {
        this->tangent_operators->addItem(QString::fromStdString(
            convertFiniteStrainBehaviourTangentOperatorFlagToString(to)));
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
          (i == BehaviourDescription::EXPLICITSCHEME) ||
          (i == BehaviourDescription::SPECIFICSCHEME)) {
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
            this->perturbation_value_label->setText("Perturbation value");
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

    QString MFrontBehaviourPage::getSelectedDomainSpecificLanguage() const {
      return this->dsls->currentText();
    }  // end of
       // MFrontBehaviourPage::getSelectedDomainSpecificLanguage()

    void MFrontBehaviourPage::write(QTextCursor tc,
                                    const DSLGenerationOptions &o,
                                    const WriteMode m) const {
      const auto bt = this->getSelectedBehaviourType();
      const auto i = this->getSelectedIntegrationScheme();
      const auto h = this->hypotheses->currentText();
      const auto s = this->symmetries->currentText();
      const auto e = this->elastic_properties->currentText();
      const auto b = this->getSelectedBrick();
      const auto c = this->crystal_structures->currentText();
      auto append = [&tc](const QString &text) { tc.insertText(text); };
      auto append_if = [&append](const bool cond, const QString &text) {
        if (!cond) {
          append("// ");
        }
        append(text);
      };
      if (h != "Default") {
        if (h == "All") {
          append("@ModellingHypotheses {\".+\"};\n\n");
        } else {
          append("@ModellingHypothesis " + h + ";\n\n");
        }
      }
      if (bt == BehaviourDescription::STANDARDSTRAINBASEDBEHAVIOUR) {
        const auto &sm = this->strain_measures->currentText();
        if (sm == "Green-Lagragrange") {
          append("@StrainMeasure GreenLagrange;\n\n");
        } else if ((sm == "Hencky") || (sm == "Linearized")) {
          append("@StrainMeasure " + sm + ";\n\n");
        }
      }
      if (s == "Orthotropic (Plate)") {
        append("@OrthotropicBehaviour<Plate>;\n\n");
      } else if (s == "Orthotropic (Pipe)") {
        append("@OrthotropicBehaviour<Pipe>;\n\n");
      } else if (s == "Orthotropic") {
        append("@OrthotropicBehaviour;\n\n");
      }
      if ((!b.isEmpty()) && (o.with_brick)) {
        if (((m == VALIDATE_STAGE) || (m == PROCESSING_STAGE)) &&
            (b == "StandardElastoViscoPlasticity")) {
          append("@Brick " + b +
                 "{\n"
                 "stress_potential: \"Hooke\"{}\n"
                 "};\n\n");
        } else {
          append("@Brick " + b);
          const auto *const w =
              dynamic_cast<const MFrontBehaviourWizard *const>(this->wizard());
          if ((w != nullptr) && (m == FINAL_STAGE)) {
            w->writeBehaviourBrickOptions(b);
          }
          append(";\n\n");
        }
      }
      if (c == "Cubic") {
        append("@CrystalStructure Cubic;\n");
        append("@SlidingSystems {<0,1,-1>{1,1,1}};\n\n");
      } else if (c == "Body-centered cubic") {
        append("@CrystalStructure BCC;\n");
        append("@SlidingSystems {<0,1,-1>{1,1,1}};\n\n");
      } else if (c == "Face-centered cubic") {
        append("@CrystalStructure FCC;\n\n");
        append("@SlidingSystems {<0,1,-1>{1,1,1}};\n\n");
      } else if (c == "Hexagonal close packing") {
        append("@CrystalStructure HCP;\n\n");
        append("@SlidingSystems {<0,1,-1,0>{1,1,1,1}};\n\n");
      }
      if ((e == "@ElasticMaterialProperties") ||
          (e.startsWith("@ComputeStiffnessTensor"))) {
        if (s == "Isotropic") {
          append_if(m == VALIDATE_STAGE, e + "{150e9, 0.3};\n\n");
        } else {
          append_if(m == VALIDATE_STAGE, e + "{150e9, 150e9, 150e9,"
                                             " 0.3, 0.3, 0.3,"
                                             " 150e9, 150e9, 150e9};\n\n");
        }
      } else if (e.startsWith("@RequireStiffnessTensor")) {
        append(e + ";\n\n");
      }
      if (i == BehaviourDescription::IMPLICITSCHEME) {
        try {
          const auto &f = mfront::NonLinearSystemSolverFactory::
              getNonLinearSystemSolverFactory();
          const auto &a = this->algorithms->currentText();
          const auto &eps = this->convergence_criterion->text();
          const auto &pv = this->perturbation_value->text();
          auto bnl = false;
          if (!a.isEmpty()) {
            append("@Algorithm " + a + ";\n");
            bnl = true;
          }
          if (!eps.isEmpty()) {
            append("@Epsilon " + eps + ";\n");
            bnl = true;
          }
          const auto so = f.getSolver(a.toStdString());
          if ((so->usesJacobian()) && (so->requiresNumericalJacobian())) {
            if (!pv.isEmpty()) {
              append(
                  "@PerturbationValueFor"
                  "NumericalJacobianComputation " +
                  pv + ";\n");
              bnl = true;
            }
          }
          if (bnl) {
            append("\n");
          };
        } catch (...) {
        }
      }
    }  // end of MFrontBehaviourPage::write()

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

    QString MFrontBehaviourPage::getSelectedTangentOperator() const {
      return this->tangent_operators->currentText();
    }  // end of MFrontBehaviourPage::getSelectedTangentOperator

    void MFrontBehaviourPage::write() const {
      this->write(this->d.textCursor(), {}, MFrontBehaviourPage::FINAL_STAGE);
    }  // end of MFrontBehaviourPage::write()

    mfront::BehaviourDSLDescription
    MFrontBehaviourPage::getCurrentBehaviourDSLDescription() const {
      try {
        const auto &f = mfront::DSLFactory::getDSLFactory();
        const auto &n = this->getSelectedDomainSpecificLanguage();
        const auto dsl =
            std::dynamic_pointer_cast<mfront::AbstractBehaviourDSL>(
                f.createNewDSL(n.toStdString()));
        if (dsl == nullptr) {
          return {};
        }
        auto bd = dsl->getBehaviourDescription();
        auto dsldd = dsl->getBehaviourDSLDescription();
        const auto b = MFrontBehaviourPage::getSelectedBrick();
        if (b.isEmpty()) {
          return dsldd;
        }
        const auto &bbf = mfront::BehaviourBrickFactory::getFactory();
        const auto bb = bbf.get(b.toStdString(), *dsl, bd);
        const auto bbd = bb->getDescription();
        mfront::complete(dsldd, bbd);
        return dsldd;
      } catch (...) {
      }
      return {};
    }  // end of getCurrentBehaviourDSLDescription

    QString MFrontBehaviourPage::getSelectedBrick() const {
      const auto b = this->bricks->currentText();
      return b == "None" ? "" : b;
    }  // end of MFrontBehaviourPage::getSelectedBrick()

    MFrontBehaviourPage::~MFrontBehaviourPage() = default;

  }  // end of namespace gui

}  // end of namespace tfel
