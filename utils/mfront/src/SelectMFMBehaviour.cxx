/*!
 * \file   SelectMFMBehaviour.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   12/03/2018
 */

#include <stdexcept>
#include <QtCore/QRegExp>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCheckBox>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/MFMDataBase.hxx"
#include "QEmacs/MFMFilterProxyModel.hxx"
#include "QEmacs/SelectMFMBehaviour.hxx"

namespace qemacs {

  SelectMFMBehaviour::SelectMFMBehaviour(QEmacsWidget& q,
                                         QWidget* const p)
      : QWidget(p),
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
    this->view->hideColumn(3);
    this->view->setRootIsDecorated(false);
    this->view->setAlternatingRowColors(true);
    this->view->setSortingEnabled(true);
    auto* const lv = new QVBoxLayout;
    auto* const fg = new QGridLayout;
    // name filter
    auto* const nfl = new QLabel("Name filter");
    auto* const nfe = new QEmacsLineEdit(q);
    nfl->setBuddy(nfe);
    QObject::connect(nfe, &QEmacsLineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setNameFilter);
    // material filter
    auto* const mfl = new QLabel(QObject::tr("Material filter"));
    auto* const mfe = new QEmacsLineEdit(q);
    mfl->setBuddy(mfe);
    QObject::connect(mfe, &QEmacsLineEdit::textChanged, bfpm,
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
    QObject::connect(this->view->selectionModel(),
                     &QItemSelectionModel::currentChanged, this,
                     [this](const QModelIndex& c, const QModelIndex&) {
                       const auto b = this->getSelectedBehaviour(c);
                       emit behaviourDescriptionChanged(b);
                     });
    fg->addWidget(nfl, 0, 0);
    fg->addWidget(nfe,0,1);
    fg->addWidget(mfl,1,0);
    fg->addWidget(mfe,1,1);
    fg->addWidget(isl,2,0);
    fg->addWidget(this->isb,2,1);
    fg->addWidget(hsl,3,0);
    fg->addWidget(this->hsb,3,1);
    // advanced options
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
    // main layout
    lv->addWidget(this->view);
    lv->addLayout(fg);
    lv->addWidget(sl);
    this->setLayout(lv);
  }  // end of SelectMFMBehaviour::SelectMFMBehaviour

  BehaviourDescription SelectMFMBehaviour::getSelectedBehaviour(
      const QModelIndex i) const {
    auto* const m = this->view->model();
    const auto il = m->index(i.row(), 3);
    const auto ib = m->index(i.row(), 1);
    const auto ii = m->index(i.row(), 4);
    BehaviourDescription b;
    b.behaviour = m->data(ib).toString();
    b.library = m->data(il).toString();
    b.minterface = m->data(ii).toString();
    b.hypothesis = this->hsb->currentText();
    return b;
  }  // end of SelectMFMBehaviour::getSelectedBehaviour

  BehaviourDescription SelectMFMBehaviour::getSelectedBehaviour()
      const {
    const auto indexes = this->view->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
      return {};
    }
    return this->getSelectedBehaviour(indexes[0]);
  }  // end of SelectMFMBehaviour::getSelectedBehaviour

  SelectMFMBehaviour::~SelectMFMBehaviour() = default;

}  // end of namespace qemacs

