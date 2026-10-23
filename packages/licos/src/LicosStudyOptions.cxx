/*!
 * \file  LicosStudyOptions.cxx
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
#include "TFEL/GUI/LicosStudyOptions.hxx"

namespace tfel::gui {

  LicosStudyOptions::LicosStudyOptions()
      : vlvl("level1"),
        wlvl("level1"),
        allowMissingDependencies("no"),
        fpe("no"),
        printBackTrace("no"),
        debugMode("no") {}

  LicosStudyOptionsDialog::LicosStudyOptionsDialog(LicosStudyOptions &options,
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
    auto *wlv = new QComboBox;
    wlv->addItems(QStringList() << "level0"
                                << "level1"
                                << "level2"
                                << "debug"
                                << "full");
    wlv->setCurrentIndex(1);
    gl1->addWidget(new QLabel(QObject::tr("Verbose level")), 0, 0);
    gl1->addWidget(vlv, 0, 1);
    gl1->addWidget(new QLabel(QObject::tr("Warning level")), 1, 0);
    gl1->addWidget(wlv, 1, 1);
    // advanced options
    auto *ao = new QGroupBox(QObject::tr("Advanced options"));
    auto *amd = new QCheckBox;
    auto *fpe = new QCheckBox;
    auto *gdb = new QCheckBox;
    auto *dbg = new QCheckBox;
    gl2->addWidget(new QLabel(QObject::tr("Allow missing dependencies")), 0, 0);
    gl2->addWidget(amd, 0, 1, Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Trac floatting point exceptions")),
                   1, 0);
    gl2->addWidget(fpe, 1, 1, Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Launch gdb on crash")), 2, 0);
    gl2->addWidget(gdb, 2, 1, Qt::AlignHCenter);
    gl2->addWidget(new QLabel(QObject::tr("Debug mode")), 3, 0);
    gl2->addWidget(dbg, 3, 1, Qt::AlignHCenter);
    ao->setLayout(gl2);
    ao->setFlat(true);
    auto dbb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    hl->addWidget(dbb);
    QObject::connect(dbb, &QDialogButtonBox::accepted, this,
                     &LicosStudyOptionsDialog::accept);
    QObject::connect(dbb, &QDialogButtonBox::rejected, this,
                     &LicosStudyOptionsDialog::reject);
    QObject::connect(vlv, &QComboBox::currentTextChanged, this,
                     &LicosStudyOptionsDialog::verboseLevelChanged);
    QObject::connect(wlv, &QComboBox::currentTextChanged, this,
                     &LicosStudyOptionsDialog::warningLevelChanged);
    // the checkboxes are connected through the locals they were created in:
    // this used to re-read them from the layout with
    // qobject_cast<QCheckBox *>(gl2->itemAtPosition(r, c)->widget()), and
    // itemAtPosition() returns null for a vacant cell. The cells were
    // populated, so it worked, but inserting a row above one of these shifts
    // the index and the deref happens during dialog construction. A failed
    // qobject_cast passed as the sender of connect() is no better: it only
    // returns false and logs a warning, so the checkbox would silently stop
    // working instead of failing loudly.
    QObject::connect(amd, &QCheckBox::toggled, this,
                     &LicosStudyOptionsDialog::allowMissingDependenciesChecked);
    QObject::connect(fpe, &QCheckBox::toggled, this,
                     &LicosStudyOptionsDialog::fpeChecked);
    QObject::connect(gdb, &QCheckBox::toggled, this,
                     &LicosStudyOptionsDialog::printBackTraceChecked);
    QObject::connect(dbg, &QCheckBox::toggled, this,
                     &LicosStudyOptionsDialog::debugModeChecked);
    vl->addLayout(gl1);
    vl->addWidget(ao);
    vl->addLayout(hl);
    this->setLayout(vl);
  }

  void LicosStudyOptionsDialog::verboseLevelChanged(const QString &lvl) {
    o.vlvl = lvl;
  }

  void LicosStudyOptionsDialog::warningLevelChanged(const QString &lvl) {
    o.wlvl = lvl;
  }

  void LicosStudyOptionsDialog::allowMissingDependenciesChecked(bool b) {
    o.allowMissingDependencies = b ? "yes" : "no";
  }

  void LicosStudyOptionsDialog::fpeChecked(bool b) { o.fpe = b ? "yes" : "no"; }

  void LicosStudyOptionsDialog::printBackTraceChecked(bool b) {
    o.printBackTrace = b ? "yes" : "no";
  }

  void LicosStudyOptionsDialog::debugModeChecked(bool b) {
    o.debugMode = b ? "yes" : "no";
  }

}  // end of namespace tfel::gui
