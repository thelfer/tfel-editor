/*!
 * \file  MTestStudyOptions.cxx
 * \brief
 * \author Thomas Helfer
 * \date   04/06/2012
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDialogButtonBox>
#include "TFEL/GUI/MTestStudyOptions.hxx"

namespace tfel::gui {

  MTestStudyOptions::MTestStudyOptions() : vlvl("level1") {}

  MTestStudyOptionsDialog::MTestStudyOptionsDialog(MTestStudyOptions &options,
                                                   QWidget *const p)
      : QDialog(p), o(options) {
    auto *vl = new QVBoxLayout();
    auto *gl1 = new QGridLayout();
    auto *gl2 = new QGridLayout();
    auto *hl = new QHBoxLayout();
    auto *vlv = new QComboBox;
    vlv->addItems(QStringList() << "quiet"
                                << "level0"
                                << "level1"
                                << "level2"
                                << "level3"
                                << "debug"
                                << "full");
    vlv->setCurrentIndex(2);
    gl1->addWidget(new QLabel(QObject::tr("Verbose level")));
    gl1->addWidget(vlv, 0, 1);
    // advanced options
    auto *ao = new QGroupBox(QObject::tr("Advanced options"));
    auto *rf = new QCheckBox();
    rf->setCheckState(Qt::Checked);
    auto *xf = new QCheckBox;
    auto *fpe = new QCheckBox;
    auto *gdb = new QCheckBox;
    gl2->addWidget(new QLabel(QObject::tr("Result file")), 1, 0);
    gl2->addWidget(rf, 1, 1, Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("XML file")), 2, 0);
    gl2->addWidget(xf, 2, 1, Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Trac floatting point exceptions")),
                   3, 0);
    gl2->addWidget(fpe, 3, 1, Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Launch gdb on crash")), 4, 0);
    gl2->addWidget(gdb, 4, 1, Qt::AlignHCenter);
    ao->setLayout(gl2);
    ao->setFlat(true);
    auto *dbb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    hl->addWidget(dbb);
    QObject::connect(dbb, &QDialogButtonBox::accepted, this,
                     &MTestStudyOptionsDialog::accept);
    QObject::connect(dbb, &QDialogButtonBox::rejected, this,
                     &MTestStudyOptionsDialog::reject);
    QObject::connect(vlv, &QComboBox::currentTextChanged, this,
                     &MTestStudyOptionsDialog::verboseLevelChanged);
    // the checkboxes are connected through the locals they were created in:
    // this used to re-read them from the layout with
    // qobject_cast<QCheckBox *>(gl2->itemAtPosition(r, c)->widget()), and
    // itemAtPosition() returns null for a vacant cell. The cells were
    // populated, so it worked, but inserting a row above one of these shifts
    // the index and the deref happens during dialog construction. A failed
    // qobject_cast passed as the sender of connect() is no better: it only
    // returns false and logs a warning, so the checkbox would silently stop
    // working instead of failing loudly.
    QObject::connect(rf, &QCheckBox::toggled, this,
                     &MTestStudyOptionsDialog::resultFile);
    QObject::connect(xf, &QCheckBox::toggled, this,
                     &MTestStudyOptionsDialog::xmlFile);
    QObject::connect(fpe, &QCheckBox::toggled, this,
                     &MTestStudyOptionsDialog::fpeChecked);
    QObject::connect(gdb, &QCheckBox::toggled, this,
                     &MTestStudyOptionsDialog::printBackTraceChecked);
    vl->addLayout(gl1);
    vl->addWidget(ao);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

  void MTestStudyOptionsDialog::verboseLevelChanged(const QString &lvl) {
    o.vlvl = lvl;
  }

  void MTestStudyOptionsDialog::resultFile(bool b) { o.res = b; }

  void MTestStudyOptionsDialog::xmlFile(bool b) { o.xml = b; }

  void MTestStudyOptionsDialog::fpeChecked(bool b) { o.fpe = b; }

  void MTestStudyOptionsDialog::printBackTraceChecked(bool b) {
    o.printBackTrace = b;
  }

}  // end of namespace tfel::gui
