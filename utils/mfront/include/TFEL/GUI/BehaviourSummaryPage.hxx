/*!
 * \file   BehaviourSummaryPage.hxx
 * \brief
 * \author th202608
 * \date   15/03/2018
 */

#ifndef LIB_BEHAVIOURSUMMARYPAGE_HXX
#define LIB_BEHAVIOURSUMMARYPAGE_HXX

#include <QtWidgets/QWizardPage>

namespace tfel::gui {

  //! Page summarizing the selected behaviour
  struct BehaviourSummaryPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    BehaviourSummaryPage(QWidget* const = nullptr);

    int nextId() const override;

    // destructor
    ~BehaviourSummaryPage() override;

   private:
    Q_OBJECT
  };  // end of BehaviourSummaryPage

}  // end of namespace tfel::gui

#endif /* LIB_BEHAVIOURSUMMARYPAGE_HXX */
