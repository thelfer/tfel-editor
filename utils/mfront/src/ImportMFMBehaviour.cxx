/*!
 * \file   ImportMFMBehaviour.cxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include "TFEL/GUI/SelectMFMBehaviour.hxx"
#include "TFEL/GUI/ImportMFMBehaviour.hxx"

namespace tfel::gui {

  ImportMFMBehaviour::ImportMFMBehaviour(EditorWidget& q,
                                         const Options& o,
                                         QWidget* const p)
      : QDialog(p), b(new SelectMFMBehaviour(q, o)) {
    auto* const lv = new QVBoxLayout;
    /* buttons */
    auto* bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // main layout
    lv->addWidget(this->b);
    lv->addWidget(bb);
    this->setLayout(lv);
  }  // end of ImportMFMBehaviour::ImportMFMBehaviour

  BehaviourDescription ImportMFMBehaviour::getSelectedBehaviour() const {
    return this->b->getSelectedBehaviour();
  }  // end of ImportMFMBehaviour::getSelectedBehaviour

  ImportMFMBehaviour::~ImportMFMBehaviour() = default;

}  // end of namespace tfel::gui
