/*!
 * \file   ImportMFMMaterialProperty.cxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#include <stdexcept>
#include <QtCore/QRegExp>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/MFMDataBase.hxx"
#include "QEmacs/MFMFilterProxyModel.hxx"
#include "QEmacs/ImportMFMMaterialProperty.hxx"

namespace qemacs {

  ImportMFMMaterialProperty::ImportMFMMaterialProperty(QEmacsWidget& q,
                                                       const Options& o,
                                                       QWidget* const p)
      : QDialog(p), view(new QTreeView), isb(new QComboBox) {
    MFMDataBase db;
    auto* const bfpm = new MFMFilterProxyModel(this);
    bfpm->setMaterialKnowledgeType("MaterialProperty");
    bfpm->setDynamicSortFilter(true);
    bfpm->setSourceModel(db.load(this));
    this->view->setModel(bfpm);
    this->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->view->setAlternatingRowColors(true);
    this->view->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->view->selectionModel()->select(
        this->view->model()->index(0, 0),
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
    this->view->hideColumn(0);
    this->view->hideColumn(3);
    this->view->setRootIsDecorated(false);
    this->view->setAlternatingRowColors(true);
    this->view->setSortingEnabled(true);
    auto* const lv = new QVBoxLayout;
    /* filters */
    auto* const fg = new QGridLayout;
    // name filter
    auto* const nfl = new QLabel("Name filter");
    auto* const nfe = new QEmacsLineEdit(q);
    nfl->setBuddy(nfe);
    QObject::connect(nfe, &QEmacsLineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setNameFilter);
    if (!o.name.isEmpty()) {
      nfe->setText(o.name);
    }
    // material filter
    auto* const mfl = new QLabel(QObject::tr("Material filter"));
    auto* const mfe = new QEmacsLineEdit(q);
    mfl->setBuddy(mfe);
    QObject::connect(mfe, &QEmacsLineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setMaterialFilter);
    if (!o.material.isEmpty()) {
      mfe->setText(o.material);
    }
    fg->addWidget(nfl, 0, 0);
    fg->addWidget(nfe, 0, 1);
    fg->addWidget(mfl, 1, 0);
    fg->addWidget(mfe, 1, 1);
    // interface filter
    if (!o.minterface.isEmpty()) {
      bfpm->setInterfaceFilter(o.minterface);
      this->view->hideColumn(4);
    } else {
      auto* const isl = new QLabel(QObject::tr("Interface"));
      isl->setBuddy(this->isb);
      this->isb->addItem(".+");
      this->isb->addItems(db.getMaterialPropertiesInterfaces());
      QObject::connect(this->isb,
                       static_cast<void (QComboBox::*)(const QString&)>(
                           &QComboBox::activated),
                       bfpm, &MFMFilterProxyModel::setInterfaceFilter);
      fg->addWidget(isl, 2, 0);
      fg->addWidget(this->isb, 2, 1);
    }
    /* advanced options */
    auto* const sl = new QCheckBox(QObject::tr("Show library path"));
    sl->setChecked(false);
    QObject::connect(sl, &QCheckBox::stateChanged, this,
                     [this](const int s) {
                       if (s == Qt::Checked) {
                         this->view->showColumn(3);
                       } else {
                         this->view->hideColumn(3);
                       }
                     });
    // the author is not yet available
    //     auto* const sa = new QCheckBox(QObject::tr("Show author"));
    //     sa->setChecked(false);
    //     QObject::connect(sa, &QCheckBox::stateChanged, this,
    //                      [this](const int s) {
    //                        if (s == Qt::Checked) {
    //                          this->view->showColumn(5);
    //                        } else {
    //                          this->view->showColumn(5);
    //                        }
    //                      });
    /* buttons */
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // main layout
    lv->addWidget(this->view);
    lv->addLayout(fg);
    lv->addWidget(sl);
    lv->addWidget(bb);
    this->setLayout(lv);
  }  // end of ImportMFMMaterialProperty::ImportMFMMaterialProperty

  QString ImportMFMMaterialProperty::getLibrary() const {
    const auto indexes =
        this->view->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
      return {};
    }
    auto* const m = this->view->model();
    const auto il = m->index(indexes[0].row(), 3);
    return m->data(il).toString();
  } // end of ImportMFMMaterialProperty::getLibrary

  QString ImportMFMMaterialProperty::getFunction() const {
    const auto indexes =
        this->view->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
      return {};
    }
    auto* const m = this->view->model();
    const auto ifct = m->index(indexes[0].row(), 1);
    return m->data(ifct).toString();
  }  // end of ImportMFMMaterialProperty::getFunction

  ImportMFMMaterialProperty::~ImportMFMMaterialProperty() = default;

}  // end of namespace qemacs
