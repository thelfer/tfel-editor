/*!
 * \file   BehaviourSummaryPage.hxx
 * \brief last page of the import wizards, which only ends them.
 * \author Thomas Helfer
 * \date   15/03/2018
 */

#ifndef LIB_BEHAVIOURSUMMARYPAGE_HXX
#define LIB_BEHAVIOURSUMMARYPAGE_HXX

#include <QtWidgets/QWizardPage>

namespace tfel::gui {

  /*!
   * \brief Page summarizing the selected behaviour
   *
   * The page shows nothing: it has no field and no widget, its only role is to
   * be the page the wizard ends on, so that the user validates the whole
   * selection at once
   */
  struct BehaviourSummaryPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    BehaviourSummaryPage(QWidget* const = nullptr);

    /*!
     * \return -1, which tells the wizard there is no next page: the page ends
     * the wizard, whose result is then read through the pages themselves
     */
    int nextId() const override;

    //! destructor
    ~BehaviourSummaryPage() override;

   private:
    Q_OBJECT
  };  // end of BehaviourSummaryPage

}  // end of namespace tfel::gui

#endif /* LIB_BEHAVIOURSUMMARYPAGE_HXX */
