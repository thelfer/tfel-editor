/*!
 * \file   ImplicitDSLWizard.cxx
 * \brief
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#include <QtWidgets/QLabel>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGridLayout>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/ImplicitDSLWizard.hxx"

namespace tfel::gui {

  BehaviourMetaDataPage::BehaviourMetaDataPage(EditorWidget &q,
                                               QWizard *const p)
      : QWizardPage(p) {
    // main grid layout
    auto *const mgl = new QGridLayout;
    auto *const bnl = new QLabel(QObject::tr("Behaviour name"));
    auto *const bne = new LineEdit(q);
    auto *const anl = new QLabel(QObject::tr("Author name"));
    auto *const ane = new LineEdit(q);
    auto *const dl = new QLabel(QObject::tr("Date"));
    auto *const de = new QDateEdit(QDate::currentDate());
    //    auto *const bd = new QGroupBox(QObject::tr("Description"));
    //     bd->addWidget();
    //     auto *const de = new QDateEdit(QDate::currentDate());
    mgl->addWidget(bnl, 0, 0);
    mgl->addWidget(bne, 0, 1);
    mgl->addWidget(anl, 1, 0);
    mgl->addWidget(ane, 1, 1);
    mgl->addWidget(dl, 2, 0);
    mgl->addWidget(de, 2, 1);
    this->setLayout(mgl);
  }  // end of BehaviourMetaDataPage::BehaviourMetaDataPage

  bool BehaviourMetaDataPage::validatePage() { return true; }

  int BehaviourMetaDataPage::nextId() const { return -1; }

  BehaviourMetaDataPage::~BehaviourMetaDataPage() = default;

  ImplicitDSLWizard::ImplicitDSLWizard(EditorWidget &q, QWidget *const p)
      : QWizard(p), md(new BehaviourMetaDataPage(q, this)) {
    this->setWindowTitle(QObject::tr("Implicit DSL Wizard"));
    this->setPage(0, this->md);
    this->setStartId(0);
  }  // end of ImplicitDSLWizard::ImplicitDSLWizard

  ImplicitDSLWizard::~ImplicitDSLWizard() = default;

}  // end of namespace tfel::gui
