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
#include"QEmacs/MFrontOptions.hxx"

namespace qemacs
{

  MFrontOptions::MFrontOptions()
    : vlvl("level1")
  {}
  
  MFrontOptionsDialog::MFrontOptionsDialog(MFrontOptions& options,
						   QWidget * const p)
    : QDialog(p),
      o(options)
  {
    auto *vl  = new QVBoxLayout();
    auto *gl1 = new QGridLayout();
    auto *gl2 = new QGridLayout();
    auto *hl  = new QHBoxLayout();
    auto *vlv = new QComboBox;
    vlv->addItems(QStringList() << "quiet"
                                << "level0"
                                << "level1"
                                << "level2"
                                << "level3"
                                << "debug"
                                << "full");
    vlv->setCurrentIndex(2);
    gl1->addWidget(new QLabel(QObject::tr("Verbose level"),nullptr,nullptr));
    gl1->addWidget(vlv,0,1);
    // advanced options
    auto *ao = new QGroupBox(QObject::tr("Advanced options"));
    auto *rf = new QCheckBox();
    rf->setCheckState(Qt::Unchecked);
    gl2->addWidget(new QLabel(QObject::tr("Debug Mode")),1,0);
    gl2->addWidget(rf,1,1,Qt::AlignHCenter);
    ao->setLayout(gl2);
    ao->setFlat(true);
    auto *dbb = new QDialogButtonBox(QDialogButtonBox::Ok |
				     QDialogButtonBox::Cancel);
    hl->addWidget(dbb);
    QObject::connect(dbb,&QDialogButtonBox::accepted,
		     this,&MFrontOptionsDialog::accept);
    QObject::connect(dbb,&QDialogButtonBox::rejected,
		     this,&MFrontOptionsDialog::reject);
    QObject::connect(
        vlv, static_cast<void (QComboBox::*)(const QString &)>(
                 &QComboBox::activated),
        this, [this](const QString &v) { this->o.vlvl = v; });
    QObject::connect(rf, &QCheckBox::stateChanged, this,
                     [this](const int s) { this->o.debug = s==Qt::Checked; });
    vl->addLayout(gl1);
    vl->addWidget(ao);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

} // end of namespace qemacs
