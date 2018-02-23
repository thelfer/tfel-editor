/*!
 * \file   MFMFilterProxyModel.cxx
 * \brief    
 * \author th202608
 * \date   21/02/2018
 */

#include "QEmacs/MFMFilterProxyModel.hxx"

namespace qemacs {

  MFMFilterProxyModel::MFMFilterProxyModel(QObject* const p)
      : QSortFilterProxyModel(p) {
  }  // end of MFMFilterProxyModel::MFMFilterProxyModel

  void MFMFilterProxyModel::setNameFilter(const QString& i) {
    this->name.setPattern(i);
    this->invalidateFilter();
  }  // end of MFMFilterProxyModel::setNameFilter

  void MFMFilterProxyModel::setInterfaceFilter(const QString& i) {
    this->interface.setPattern(i);
    this->invalidateFilter();
  }  // end of MFMFilterProxyModel::setInterfaceFilter

  void MFMFilterProxyModel::setMaterialFilter(const QString& m) {
    this->material.setPattern(m);
    this->invalidateFilter();
  }  // end of MFMFilterProxyModel::setMaterialFilter

  void MFMFilterProxyModel::setMaterialKnowledgeType(const QString& t) {
    if ((t == "MaterialProperty") || (t == "Behaviour") ||
        (t == "Model")) {
      this->mkt = t;
      this->invalidateFilter();
    }
  } // end of MFMFilterProxyModel::setMaterialKnowledgeType

  bool MFMFilterProxyModel::filterAcceptsRow(
      const int r, const QModelIndex& p) const {
    const auto* m = this->sourceModel();
    const auto it = m->index(r, 0, p);
    const auto in = m->index(r, 1, p);
    const auto im = m->index(r, 2, p);
    const auto ii = m->index(r, 4, p);
    bool b = true;
    if(!this->mkt.isEmpty()){
      b = b && m->data(it).toString() == this->mkt;
    }
    if (!b) {
      return false;
    }
    if ((!this->name.isEmpty()) && (this->name.isValid())) {
      b = b && this->name.indexIn(m->data(in).toString()) != -1;
    }
    if (!b) {
      return false;
    }
    if ((!this->interface.isEmpty()) && (this->interface.isValid())) {
      b = b && this->interface.indexIn(m->data(ii).toString())!=-1;
    }
    if (!b) {
      return false;
    }
    if ((!this->material.isEmpty()) && (this->material.isValid())) {
      b = b && this->material.indexIn(m->data(im).toString())!=-1;
    }
    return b;
  } // end of MFMFilterProxyModel::filterAcceptsRow

} // end of namespace qemacs
