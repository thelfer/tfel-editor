/*!
 * \file   MFMFilterProxyModel.cxx
 * \brief
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#include "TFEL/GUI/MFMFilterProxyModel.hxx"

namespace tfel::gui {

  MFMFilterProxyModel::MFMFilterProxyModel(QObject* const p)
      : QSortFilterProxyModel(p) {
  }  // end of MFMFilterProxyModel::MFMFilterProxyModel

  void MFMFilterProxyModel::setNameFilter(const QString& i) {
    this->name.setPattern(i);
    this->invalidateFilter();
  }  // end of MFMFilterProxyModel::setNameFilter

  void MFMFilterProxyModel::setInterfaceFilter(const QString& i) {
    this->mfront_interface.setPattern(i);
    this->invalidateFilter();
  }  // end of MFMFilterProxyModel::setInterfaceFilter

  void MFMFilterProxyModel::setMaterialFilter(const QString& m) {
    this->material.setPattern(m);
    this->invalidateFilter();
  }  // end of MFMFilterProxyModel::setMaterialFilter

  void MFMFilterProxyModel::setMaterialKnowledgeType(const QString& t) {
    if ((t == "MaterialProperty") || (t == "Behaviour") || (t == "Model")) {
      this->mkt = t;
      this->invalidateFilter();
    }
  }  // end of MFMFilterProxyModel::setMaterialKnowledgeType

  bool MFMFilterProxyModel::filterAcceptsRow(const int r,
                                             const QModelIndex& p) const {
    const auto* m = this->sourceModel();
    const auto it = m->index(r, 0, p);
    const auto in = m->index(r, 1, p);
    const auto im = m->index(r, 2, p);
    const auto ii = m->index(r, 4, p);
    bool b = true;
    if (!this->mkt.isEmpty()) {
      b = b && m->data(it).toString() == this->mkt;
    }
    if (!b) {
      return false;
    }
    if (this->name.isValid()) {
      const auto match = this->name.match(m->data(in).toString());
      b = b && match.hasMatch();
    }
    if (!b) {
      return false;
    }
    if (this->mfront_interface.isValid()) {
      const auto match = this->mfront_interface.match(m->data(ii).toString());
      b = b && match.hasMatch();
    }
    if (!b) {
      return false;
    }
    if (this->material.isValid()) {
      const auto match = this->material.match(m->data(im).toString());
      b = b && match.hasMatch();
    }
    return b;
  }  // end of MFMFilterProxyModel::filterAcceptsRow

}  // end of namespace tfel::gui
