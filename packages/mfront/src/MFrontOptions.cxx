/*! 
 * \file  MTestStudyOptions.cxx
 * \brief
 * \author Helfer Thomas
 * \date   04/06/2012
 */

#include<QtWidgets/QLabel>
#include<QtWidgets/QComboBox>
#include<QtWidgets/QGridLayout>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QCheckBox>
#include<QtWidgets/QGroupBox>
#include<QtWidgets/QDialogButtonBox>
#include"MFront/MaterialPropertyInterfaceFactory.hxx"
#include"MFront/BehaviourInterfaceFactory.hxx"
#include"MFront/ModelInterfaceFactory.hxx"
#include"QEmacs/MFrontOptions.hxx"

namespace qemacs {

  MFrontOptions::MFrontOptions()
      : analysis_type("Build"),
        olvl("level1"),
        vlvl("level1") {}  // end of MFrontOptions::MFrontOptions

  MFrontOptionsDialog::MFrontOptionsDialog(
      MFrontOptions &o, const MaterialKnowledgeType t, QWidget *const p)
      : QDialog(p), opts(o) {
    auto *vl  = new QVBoxLayout();
    auto *gl1 = new QGridLayout();
    auto *gl2 = new QGridLayout();
    auto *hl  = new QHBoxLayout();
    auto *interfaces = new QComboBox;
    if(t==MATERIALPROPERTY){
      const auto &f = mfront::MaterialPropertyInterfaceFactory::
          getMaterialPropertyInterfaceFactory();
      for(const auto& i : f.getRegistredInterfaces()){
        interfaces->addItem(QString::fromStdString(i));
      }
    } else if (t == BEHAVIOUR) {
      const auto &f = mfront::BehaviourInterfaceFactory::
          getBehaviourInterfaceFactory();
      for(const auto& i : f.getRegistredInterfaces()){
        interfaces->addItem(QString::fromStdString(i));
      }
    } else {
      const auto &f =
          mfront::ModelInterfaceFactory::getModelInterfaceFactory();
      for(const auto& i : f.getRegistredInterfaces()){
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
    // debug
    auto *rdebug = new QCheckBox();
    rdebug->setCheckState(Qt::Unchecked);
    gl2->addWidget(new QLabel(QObject::tr("Debug Mode")), 1, 0);
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
    ao->setLayout(gl2);
    ao->setFlat(true);
    auto *dbb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);
    hl->addWidget(dbb);
    // actions
    QObject::connect(dbb, &QDialogButtonBox::accepted, this,
                     &MFrontOptionsDialog::accept);
    QObject::connect(dbb, &QDialogButtonBox::rejected, this,
                     &MFrontOptionsDialog::reject);
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
    vl->addWidget(ao);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

} // end of namespace qemacs
