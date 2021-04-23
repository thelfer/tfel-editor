/*!
 * \file   BehaviourSummaryPage.cxx
 * \brief
 * \author Thomas Helfer
 * \date   15/03/2018
 */

#include "TFEL/GUI/BehaviourSummaryPage.hxx"

namespace tfel {

  namespace gui {

    BehaviourSummaryPage::BehaviourSummaryPage(QWidget *const p)
        : QWizardPage(p) {
    }  // end of BehaviourSummaryPage::BehaviourSummaryPage

    int BehaviourSummaryPage::nextId() const {
      return -1;
    }  // end of BehaviourSummaryPage::nextId()

    BehaviourSummaryPage::~BehaviourSummaryPage() = default;

  }  // end of namespace gui
}  // end of namespace tfel
