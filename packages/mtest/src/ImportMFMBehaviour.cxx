/*!
 * \file   ImportMFMBehaviour.cxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#include <stdexcept>
#include <QtCore/QRegExp>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QDialogButtonBox>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/MFMDataBase.hxx"
#include "QEmacs/MFMFilterProxyModel.hxx"
#include "QEmacs/ImportMFMBehaviour.hxx"

namespace qemacs {

  ImportMFMBehaviour::ImportMFMBehaviour(QEmacsWidget& w,
                                         QWidget* const p)
      : QDialog(p),
        qemacs(w),
        view(new QTreeView),
        isb(new QComboBox),
        hsb(new QComboBox) {
    MFMDataBase db;
    auto* const bfpm = new MFMFilterProxyModel(this);
    bfpm->setMaterialKnowledgeType("Behaviour");
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
//  this->view->resizeColumnToContents(1);
    this->view->setRootIsDecorated(false);
    this->view->setAlternatingRowColors(true);
    this->view->setSortingEnabled(true);
    auto* const lv = new QVBoxLayout;
    lv->addWidget(this->view);
    auto* const fg = new QGridLayout;
    // name filter
    auto* const nfl = new QLabel("Name filter");
    auto* const nfe = new QLineEdit;
    nfl->setBuddy(nfe);
    QObject::connect(nfe, &QLineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setNameFilter);
    // material filter
    auto* const mfl = new QLabel(QObject::tr("Material filter"));
    auto* const mfe = new QLineEdit;
    mfl->setBuddy(mfe);
    QObject::connect(mfe, &QLineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setMaterialFilter);
    // interface filter
    auto* const isl = new QLabel(QObject::tr("Interface"));
    isl->setBuddy(this->isb);
    this->isb->addItem(".+");
    this->isb->addItems(db.getBehavioursInterfaces());
    QObject::connect(this->isb,
                     static_cast<void (QComboBox::*)(const QString&)>(
                         &QComboBox::activated),
                     bfpm, &MFMFilterProxyModel::setInterfaceFilter);
    // hypothesis
    auto* const hsl = new QLabel(QObject::tr("Hypothesis"));
    QObject::connect(
        this->view->selectionModel(),
        &QItemSelectionModel::currentChanged, this->hsb,
        [this](const QModelIndex& index, const QModelIndex&) {
          using tfel::system::ExternalLibraryManager;
          auto& elm =
              ExternalLibraryManager::getExternalLibraryManager();
          this->hsb->clear();
          if (!index.isValid()) {
            return;
          }
          const auto* const m = index.model();
          const auto il = m->index(index.row(),3);
          const auto ib = m->index(index.row(),1);
          const auto l = m->data(il).toString().toStdString();
          const auto b = m->data(ib).toString().toStdString();
          try{
            const auto mh = elm.getSupportedModellingHypotheses(l, b);
            for (const auto& h : mh) {
              this->hsb->addItem(QString::fromStdString(h));
            }
          } catch (...) {
          }
        });
    fg->addWidget(nfl, 0, 0);
    fg->addWidget(nfe,0,1);
    fg->addWidget(mfl,1,0);
    fg->addWidget(mfe,1,1);
    fg->addWidget(isl,2,0);
    fg->addWidget(this->isb,2,1);
    fg->addWidget(hsl,3,0);
    fg->addWidget(this->hsb,3,1);
    lv->addLayout(fg);
    // button
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this,
            &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lv->addWidget(bb);
    this->setLayout(lv);
  }  // end of ImportMFMBehaviour::ImportMFMBehaviour

  ImportMFMBehaviour::BehaviourDescription
  ImportMFMBehaviour::getSelectedBehaviour() const {
    const auto indexes =
        this->view->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
      return {};
    }
    auto* const m = this->view->model();
    const auto il = m->index(indexes[0].row(), 3);
    const auto ib = m->index(indexes[0].row(), 1);
    const auto ii = m->index(indexes[0].row(), 4);

    BehaviourDescription b;
    b.behaviour = m->data(ib).toString();
    b.library = m->data(il).toString();
    b.mfront_interface = m->data(ii).toString();
    b.hypothesis = this->hsb->currentText();
    return b;
  }  // end of ImportMFMBehaviour::getSelectedBehaviour

  ImportMFMBehaviour::~ImportMFMBehaviour() = default;

}  // end of namespace qemacs
