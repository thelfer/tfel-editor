/*!
 * \file   SelectMFMBehaviour.cxx
 * \brief
 * \author Thomas Helfer
 * \date   12/03/2018
 */

#include <stdexcept>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QCheckBox>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MFMDataBase.hxx"
#include "TFEL/GUI/MFMFilterProxyModel.hxx"
#include "TFEL/GUI/SelectMFMBehaviour.hxx"

namespace tfel::gui {

  SelectMFMBehaviour::Options::Options() = default;
  SelectMFMBehaviour::Options::Options(Options&&) = default;
  SelectMFMBehaviour::Options::Options(const Options&) = default;
  SelectMFMBehaviour::Options& SelectMFMBehaviour::Options::operator=(
      Options&&) = default;
  SelectMFMBehaviour::Options& SelectMFMBehaviour::Options::operator=(
      const Options&) = default;

  SelectMFMBehaviour::SelectMFMBehaviour(EditorWidget& q,
                                         const Options& o,
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
    auto* const nfe = new LineEdit(q);
    nfl->setBuddy(nfe);
    QObject::connect(nfe, &LineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setNameFilter);
    if (!o.name.isEmpty()) {
      nfe->setText(o.name);
    }
    // material filter
    auto* const mfl = new QLabel(QObject::tr("Material filter"));
    auto* const mfe = new LineEdit(q);
    mfl->setBuddy(mfe);
    QObject::connect(mfe, &LineEdit::textChanged, bfpm,
                     &MFMFilterProxyModel::setMaterialFilter);
    if (!o.material.isEmpty()) {
      mfe->setText(o.material);
    }
    // interface filter
    auto* const isl = new QLabel(QObject::tr("Interface"));
    if (!o.minterface.isEmpty()) {
      bfpm->setInterfaceFilter(o.minterface);
      this->view->hideColumn(4);
    } else {
      isl->setBuddy(this->isb);
      this->isb->addItem(".+");
      this->isb->addItems(db.getBehavioursInterfaces());
      QObject::connect(this->isb, &QComboBox::currentTextChanged, bfpm,
                       &MFMFilterProxyModel::setInterfaceFilter);
    }
    // hypothesis
    auto* const hsl = new QLabel(QObject::tr("Hypothesis"));
    QObject::connect(
        this->view->selectionModel(), &QItemSelectionModel::currentChanged,
        this->hsb, [this](const QModelIndex& index, const QModelIndex&) {
          using tfel::system::ExternalLibraryManager;
          auto& elm = ExternalLibraryManager::getExternalLibraryManager();
          this->hsb->clear();
          if (!index.isValid()) {
            return;
          }
          const auto* const m = index.model();
          const auto il = m->index(index.row(), 3);
          const auto ib = m->index(index.row(), 1);
          const auto l = m->data(il).toString().toStdString();
          const auto b = m->data(ib).toString().toStdString();
          try {
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
    QObject::connect(this->view, &QTreeView::doubleClicked, this,
                     [this](const QModelIndex& c) {
                       const auto b = this->getSelectedBehaviour(c);
                       emit doubleClicked(b);
                     });
    fg->addWidget(nfl, 0, 0);
    fg->addWidget(nfe, 0, 1);
    fg->addWidget(mfl, 1, 0);
    fg->addWidget(mfe, 1, 1);
    if (o.minterface.isEmpty()) {
      fg->addWidget(isl, 2, 0);
      fg->addWidget(this->isb, 2, 1);
    }
    fg->addWidget(hsl, 3, 0);
    fg->addWidget(this->hsb, 3, 1);
    // advanced options
    auto* const sl = new QCheckBox(QObject::tr("Show library path"));
    sl->setChecked(false);
    QObject::connect(sl, &QCheckBox::stateChanged, this, [this](const int s) {
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
  }  // end of SelectMFMBehaviour

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
  }  // end of getSelectedBehaviour

  BehaviourDescription SelectMFMBehaviour::getSelectedBehaviour() const {
    const auto indexes = this->view->selectionModel()->selectedRows();
    if (indexes.size() != 1) {
      return {};
    }
    return this->getSelectedBehaviour(indexes[0]);
  }  // end of getSelectedBehaviour

  SelectMFMBehaviour::~SelectMFMBehaviour() = default;

}  // end of namespace tfel::gui
