/*!
 * \file   ImportMFMBehaviour.cxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include "QEmacs/SelectMFMBehaviour.hxx"
#include "QEmacs/ImportMFMBehaviour.hxx"

namespace qemacs {

  ImportMFMBehaviour::ImportMFMBehaviour(QEmacsWidget& q,
                                         QWidget* const p)
      : QDialog(p), b(new SelectMFMBehaviour(q)) {
    auto* const lv = new QVBoxLayout;
    /* buttons */
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this,
            &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // main layout
    lv->addWidget(this->b);
    lv->addWidget(bb);
    this->setLayout(lv);
  }  // end of ImportMFMBehaviour::ImportMFMBehaviour

  BehaviourDescription ImportMFMBehaviour::getSelectedBehaviour()
      const {
    return this->b->getSelectedBehaviour();
  }  // end of ImportMFMBehaviour::getSelectedBehaviour

  ImportMFMBehaviour::~ImportMFMBehaviour() = default;

}  // end of namespace qemacs
