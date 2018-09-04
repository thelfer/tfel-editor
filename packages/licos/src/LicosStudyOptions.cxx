/*! 
 * \file  LicosStudyOptions.cxx
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
#include"TFEL/GUI/LicosStudyOptions.hxx"

namespace tfel{

  namespace gui
{

  LicosStudyOptions::LicosStudyOptions()
    : vlvl("level1"),
      wlvl("level1"),
      allowMissingDependencies("no"),
      fpe("no"),
      printBackTrace("no"),
      debugMode("no")
  {}
  
  LicosStudyOptionsDialog::LicosStudyOptionsDialog(LicosStudyOptions& options,
						   QWidget * const p)
    : QDialog(p),
      o(options)
  {
    auto *vl  = new QVBoxLayout();
    auto *gl1 = new QGridLayout();
    auto *gl2 = new QGridLayout();
    auto *hl  = new QHBoxLayout();
    auto *vlv = new QComboBox;
    vlv->addItems(QStringList() << "quiet"  << "level0"
		  << "level1"   << "level2" << "level3"
		  << "debug" << "full");
    vlv->setCurrentIndex(2);
    auto *wlv = new QComboBox;
    wlv->addItems(QStringList() << "level0" << "level1"
		  << "level2"   << "debug"  << "full");
    wlv->setCurrentIndex(1);
    gl1->addWidget(new QLabel(QObject::tr("Verbose level"),0,0));
    gl1->addWidget(vlv,0,1);
    gl1->addWidget(new QLabel(QObject::tr("Warning level")),1,0);
    gl1->addWidget(wlv,1,1);
    // advanced options
    auto *ao  = new QGroupBox(QObject::tr("Advanced options"));
    gl2->addWidget(new QLabel(QObject::tr("Allow missing dependencies")),0,0);
    gl2->addWidget(new QCheckBox,0,1,Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Trac floatting point exceptions")),1,0);
    gl2->addWidget(new QCheckBox,1,1,Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Launch gdb on crash")),2,0);
    gl2->addWidget(new QCheckBox,2,1,Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Debug mode")),3,0);
    gl2->addWidget(new QCheckBox,3,1,Qt::AlignHCenter);
    ao->setLayout(gl2);
    ao->setFlat(true);
    auto dbb =  new QDialogButtonBox(QDialogButtonBox::Ok |
				     QDialogButtonBox::Cancel);
    hl->addWidget(dbb);
    QObject::connect(dbb,&QDialogButtonBox::accepted,
		     this,&LicosStudyOptionsDialog::accept);
    QObject::connect(dbb,&QDialogButtonBox::rejected,
		     this,&LicosStudyOptionsDialog::reject);
    QObject::connect(vlv,static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),
		     this,&LicosStudyOptionsDialog::verboseLevelChanged);
    QObject::connect(wlv,static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),
		     this,&LicosStudyOptionsDialog::warningLevelChanged);
    QObject::connect(qobject_cast<QCheckBox*>(gl2->itemAtPosition(0,1)->widget()),&QCheckBox::toggled,
		     this,&LicosStudyOptionsDialog::allowMissingDependenciesChecked);
    QObject::connect(qobject_cast<QCheckBox*>(gl2->itemAtPosition(1,1)->widget()),&QCheckBox::toggled,
		     this,&LicosStudyOptionsDialog::fpeChecked);
    QObject::connect(qobject_cast<QCheckBox*>(gl2->itemAtPosition(2,1)->widget()),&QCheckBox::toggled,
		     this,&LicosStudyOptionsDialog::printBackTraceChecked);
    QObject::connect(qobject_cast<QCheckBox*>(gl2->itemAtPosition(3,1)->widget()),&QCheckBox::toggled,
		     this,&LicosStudyOptionsDialog::debugModeChecked);
    vl->addLayout(gl1);
    vl->addWidget(ao);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

  void LicosStudyOptionsDialog::verboseLevelChanged(const QString& lvl)
  {
    o.vlvl = lvl;
  }
  
  void LicosStudyOptionsDialog::warningLevelChanged(const QString& lvl)
  {
    o.wlvl = lvl;
  }
  
  void LicosStudyOptionsDialog::allowMissingDependenciesChecked(bool b)
  {
    o.allowMissingDependencies = b ? "yes" : "no";
  }
  
  void LicosStudyOptionsDialog::fpeChecked(bool b)
  {
    o.fpe = b ? "yes" : "no";
  }
  
  void LicosStudyOptionsDialog::printBackTraceChecked(bool b)
  {
    o.printBackTrace = b ? "yes" : "no";
  }
  
  void LicosStudyOptionsDialog::debugModeChecked(bool b)
  {
    o.debugMode = b ? "yes" : "no";
  }

} // end of namespace gui
}// end of namespace tfel
