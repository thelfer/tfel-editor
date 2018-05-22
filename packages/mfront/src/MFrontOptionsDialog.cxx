/*!
 * \file   packages/mfront/src/MFrontOptionsDialog.cxx
 * \brief
 * \author Thomas Helfer
 * \date   04/06/2012
 */

#include <cmath>
#include <QtCore/QDebug>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDialogButtonBox>
#include "MFront/MaterialPropertyInterfaceFactory.hxx"
#include "MFront/BehaviourInterfaceFactory.hxx"
#include "MFront/ModelInterfaceFactory.hxx"
#include "MFront/DSLFactory.hxx"
#include "MFront/ImplicitDSLBase.hxx"
#include "MFront/NonLinearSystemSolver.hxx"
#include "QEmacs/Debug.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/MFrontOptions.hxx"
#include "QEmacs/MFrontOptionsDialog.hxx"

namespace qemacs {

  MFrontOptionsDialog::MFrontOptionsDialog(
      QEmacsWidget &q,
      MFrontOptions &o,
      const MaterialKnowledgeType t,
      QWidget *const p)
      : QDialog(p), opts(o) {
    auto *vl = new QVBoxLayout();
    auto *gl1 = new QGridLayout();
    auto *gl2 = new QGridLayout();
    auto *interfaces = new QComboBox;
    if (t == MATERIALPROPERTY) {
      const auto &f = mfront::MaterialPropertyInterfaceFactory::
          getMaterialPropertyInterfaceFactory();
      for (const auto &i : f.getRegistredInterfaces()) {
        interfaces->addItem(QString::fromStdString(i));
      }
    } else if (t == BEHAVIOUR) {
      const auto &f = mfront::BehaviourInterfaceFactory::
          getBehaviourInterfaceFactory();
      for (const auto &i : f.getRegistredInterfaces()) {
        interfaces->addItem(QString::fromStdString(i));
      }
    } else {
      const auto &f =
          mfront::ModelInterfaceFactory::getModelInterfaceFactory();
      for (const auto &i : f.getRegistredInterfaces()) {
        interfaces->addItem(QString::fromStdString(i));
      }
    }
    gl1->addWidget(new QLabel(QObject::tr("Interface")), 0, 0);
    if (!o.i.isEmpty()) {
      interfaces->setCurrentText(o.i);
    }
    o.i = interfaces->currentText();
    gl1->addWidget(interfaces, 0, 1);
    auto *atype = new QComboBox;
    atype->addItems(QStringList() << "Build"
                                  << "Analyse"
                                  << "Generate");
    atype->setCurrentIndex(0);
    gl1->addWidget(new QLabel(QObject::tr("Type of analsys")), 1, 0);
    gl1->addWidget(atype, 1, 1);
    auto *olvl = new QComboBox;
    olvl->addItems(QStringList() << "level0"
                                 << "level1"
                                 << "level2");
    olvl->setCurrentIndex(1);
    gl1->addWidget(new QLabel(QObject::tr("Optimisation level")), 2, 0);
    gl1->addWidget(olvl, 2, 1);
    auto *vlv = new QComboBox;
    vlv->addItems(QStringList() << "quiet"
                                << "level0"
                                << "level1"
                                << "level2"
                                << "level3"
                                << "debug"
                                << "full");
    vlv->setCurrentIndex(2);
    gl1->addWidget(new QLabel(QObject::tr("Verbose level")), 3, 0);
    gl1->addWidget(vlv, 3, 1);
    // advanced options
    auto *ao = new QGroupBox(QObject::tr("Advanced options"));
    ao->setAlignment(Qt::AlignHCenter);
    // debug
    auto *rdebug = new QCheckBox();
    rdebug->setCheckState(Qt::Unchecked);
    gl2->addWidget(new QLabel(QObject::tr("Debug mode")), 1, 0);
    gl2->addWidget(rdebug, 1, 1, Qt::AlignHCenter);
    // warnings
    auto *rwarnings = new QCheckBox();
    rwarnings->setCheckState(Qt::Unchecked);
    gl2->addWidget(new QLabel(QObject::tr("Extra warnings")), 2, 0);
    gl2->addWidget(rwarnings, 2, 1, Qt::AlignHCenter);
    // pedantic warnings
    auto *rpedantic = new QCheckBox();
    rpedantic->setCheckState(Qt::Unchecked);
    gl2->addWidget(new QLabel(QObject::tr("Pedantic warnings")), 3, 0);
    gl2->addWidget(rpedantic, 3, 1, Qt::AlignHCenter);
    // profiling
    if (t == BEHAVIOUR) {
      auto *rprofiling = new QCheckBox();
      rprofiling->setCheckState(Qt::Unchecked);
      gl2->addWidget(new QLabel(QObject::tr("Performance profiling")),
                     4, 0);
      gl2->addWidget(rprofiling, 4, 1, Qt::AlignHCenter);
      QObject::connect(rprofiling, &QCheckBox::stateChanged, this,
                       [this](const int s) {
                         this->opts.profiling = s == Qt::Checked;
                       });
    }
    //
    // actions
    QObject::connect(
        interfaces, static_cast<void (QComboBox::*)(const QString &)>(
                        &QComboBox::activated),
        this, [this](const QString &v) { this->opts.i = v; });
    QObject::connect(
        atype, static_cast<void (QComboBox::*)(const QString &)>(
                   &QComboBox::activated),
        this,
        [this](const QString &v) { this->opts.analysis_type = v; });
    QObject::connect(
        olvl, static_cast<void (QComboBox::*)(const QString &)>(
                  &QComboBox::activated),
        this, [this](const QString &v) { this->opts.olvl = v; });
    QObject::connect(
        vlv, static_cast<void (QComboBox::*)(const QString &)>(
                 &QComboBox::activated),
        this, [this](const QString &v) { this->opts.vlvl = v; });
    QObject::connect(
        rwarnings, &QCheckBox::stateChanged, this,
        [this](const int s) { this->opts.warning = s == Qt::Checked; });
    QObject::connect(
        rdebug, &QCheckBox::stateChanged, this,
        [this](const int s) { this->opts.debug = s == Qt::Checked; });
    QObject::connect(rpedantic, &QCheckBox::stateChanged, this,
                     [this](const int s) {
                       this->opts.pedantic = s == Qt::Checked;
                     });
    vl->addLayout(gl1);
    // specific options for the Implicit DSL
    const auto uses_analytic_jacobian = [&t, &o] {
      try {
        if (t == BEHAVIOUR) {
          if ((o.dsl == "Implicit") || (o.dsl == "ImplicitII") ||
              (o.dsl == "ImplicitFiniteStrain")) {
            auto &f = mfront::DSLFactory::getDSLFactory();
            auto dsl = f.createNewDSL(o.dsl.toStdString());
            dsl->analyseFile(o.file.toStdString(), {}, {});
            const auto &idsl = dynamic_cast<mfront::ImplicitDSLBase &>(*dsl);
            const auto& solver = idsl.getSolver();
            return (solver.usesJacobian() &&
                    (!solver.requiresNumericalJacobian()));
          }
        }
      } catch (std::exception &e) {
        debug(e.what());
      } catch (...) {
      }
      return false;
    }();
    if (uses_analytic_jacobian) {
      // build a QEmacsLineEdit for double values
      auto newDoubleLineEdit = [&q] {
        auto *const le = new QEmacsLineEdit(q);
        auto *const vv = new QDoubleValidator;
        vv->setNotation(QDoubleValidator::ScientificNotation);
        vv->setBottom(0);
        vv->setLocale(QLocale(QLocale::C));
        le->setValidator(vv);
        le->setAlignment(Qt::AlignRight);
        return le;
      };
      // enabling comparison to numerical jacobian
      auto *const njcl1 =
          new QLabel(QObject::tr("Compare to numerical jacobian"));
      auto *const njccb1 = new QCheckBox();
      njccb1->setCheckState(Qt::Unchecked);
      gl2->addWidget(njcl1, 5, 0);
      gl2->addWidget(njccb1, 5, 1, Qt::AlignHCenter);
      // criterion
      auto *const njcl2 =
          new QLabel(QObject::tr("Comparison criterion"));
      gl2->addWidget(njcl2, 6, 0);
      auto *const cle = newDoubleLineEdit();
      gl2->addWidget(cle, 6, 1);
      // perturbation default value
      auto *const njcl3 = new QLabel(QObject::tr("Perturbation value"));
      gl2->addWidget(njcl3, 7, 0);
      auto *const ple = newDoubleLineEdit();
      gl2->addWidget(ple, 7, 1);
      QObject::connect(
          njccb1, &QCheckBox::stateChanged, this,
          [this, cle, ple](const int s) {
            if (s == Qt::Unchecked) {
              cle->clear();
              ple->clear();
              this->opts.numerical_jacobian_criterion.clear();
              this->opts.numerical_jacobian_perturbation.clear();
            }
            this->opts.compare_to_numerical_jacobian = s == Qt::Checked;
          });
      QObject::connect(cle, &QEmacsLineEdit::textChanged, cle,
                       [this](const QString &n) {
                         this->opts.numerical_jacobian_criterion = n;
                       });
      QObject::connect(ple, &QEmacsLineEdit::textChanged, ple,
                       [this](const QString &n) {
                         this->opts.numerical_jacobian_perturbation = n;
                       });
    }
    ao->setLayout(gl2);
    ao->setFlat(true);
    vl->addWidget(ao);
    // buttons
    auto *const hl = new QHBoxLayout();
    auto *const dbb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                           QDialogButtonBox::Cancel);
    QObject::connect(dbb, &QDialogButtonBox::accepted, this,
                     &MFrontOptionsDialog::accept);
    QObject::connect(dbb, &QDialogButtonBox::rejected, this,
                     &MFrontOptionsDialog::reject);
    hl->addWidget(dbb);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

}  // end of namespace qemacs
