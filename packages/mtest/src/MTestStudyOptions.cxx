/*! 
 * \file  MTestStudyOptions.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 04 juin 2012
 */

#ifdef QEMACS_QT4
#include<QtGui/QLabel>
#include<QtGui/QComboBox>
#include<QtGui/QGridLayout>
#include<QtGui/QHBoxLayout>
#include<QtGui/QCheckBox>
#include<QtGui/QGroupBox>
#include<QtGui/QDialogButtonBox>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QLabel>
#include<QtWidgets/QComboBox>
#include<QtWidgets/QGridLayout>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QCheckBox>
#include<QtWidgets/QGroupBox>
#include<QtWidgets/QDialogButtonBox>
#endif /* QEMACS_QT5 */

#include"QEmacs/MTestStudyOptions.hxx"

namespace qemacs
{

  MTestStudyOptions::MTestStudyOptions()
    : vlvl("level1")
  {}
  
  MTestStudyOptionsDialog::MTestStudyOptionsDialog(MTestStudyOptions& options,
						   QWidget * const p)
    : QDialog(p),
      o(options)
  {
    QVBoxLayout *vl  = new QVBoxLayout();
    QGridLayout *gl1 = new QGridLayout();
    QGridLayout *gl2 = new QGridLayout();
    QHBoxLayout *hl  = new QHBoxLayout();
    QComboBox * vlv  = new QComboBox;
    vlv->addItems(QStringList() << "quiet"  << "level0"
		  << "level1"   << "level2" << "level3"
		  << "debug" << "full");
    vlv->setCurrentIndex(2);
    gl1->addWidget(new QLabel(QObject::tr("Verbose level"),0,0));
    gl1->addWidget(vlv,0,1);
    // advanced options
    QGroupBox   *ao  = new QGroupBox(QObject::tr("Advanced options"));
    auto* rf = new QCheckBox();
    rf->setCheckState(Qt::Checked);
    gl2->addWidget(new QLabel(QObject::tr("Result file")),1,0);
    gl2->addWidget(rf,1,1,Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("XML file")),2,0);
    gl2->addWidget(new QCheckBox,2,1,Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Trac floatting point exceptions")),3,0);
    gl2->addWidget(new QCheckBox,3,1,Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Launch gdb on crash")),4,0);
    gl2->addWidget(new QCheckBox,4,1,Qt::AlignHCenter);
    ao->setLayout(gl2);
    ao->setFlat(true);

    QDialogButtonBox  *dbb;
    dbb =  new QDialogButtonBox(QDialogButtonBox::Ok |
				QDialogButtonBox::Cancel);
    hl->addWidget(dbb);
    QObject::connect(dbb, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(dbb, SIGNAL(rejected()), this, SLOT(reject()));
    QObject::connect(vlv, SIGNAL(activated(const QString&)),
		     this, SLOT(verboseLevelChanged(const QString&)));
    QObject::connect(gl2-> itemAtPosition(1,1)->widget(), SIGNAL(toggled(bool)),
		     this, SLOT(resultFile(bool)));
    QObject::connect(gl2-> itemAtPosition(2,1)->widget(), SIGNAL(toggled(bool)),
		     this, SLOT(xmlFile(bool)));
    QObject::connect(gl2-> itemAtPosition(3,1)->widget(), SIGNAL(toggled(bool)),
		     this, SLOT(fpeChecked(bool)));
    QObject::connect(gl2-> itemAtPosition(4,1)->widget(), SIGNAL(toggled(bool)),
		     this, SLOT(printBackTraceChecked(bool)));
    vl->addLayout(gl1);
    vl->addWidget(ao);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

  void MTestStudyOptionsDialog::verboseLevelChanged(const QString& lvl)
  {
    o.vlvl = lvl;
  }

  void MTestStudyOptionsDialog::resultFile(bool b)
  {
    o.res = b;
  }

  void MTestStudyOptionsDialog::xmlFile(bool b)
  {
    o.xml = b;
  }
  
  void MTestStudyOptionsDialog::fpeChecked(bool b)
  {
    o.fpe = b;
  }
  
  void MTestStudyOptionsDialog::printBackTraceChecked(bool b)
  {
    o.printBackTrace = b;
  }

} // end of namespace qemacs
