/*!
 * \file   SelectBehaviourPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_QEMACS_SELECTBEHAVIOURPAGE_HXX
#define LIB_QEMACS_SELECTBEHAVIOURPAGE_HXX

#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizardPage>
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/BehaviourDescription.hxx"

namespace qemacs {

  /*!
   * \brief a wizard page used to select a behaviour from a shared
   * library.
   */
  struct QEMACS_MIW_VISIBILITY_EXPORT SelectBehaviourPage : public QWizardPage {
    SelectBehaviourPage(QEmacsWidget &);

    int nextId() const override;

    bool validatePage() override;
    //! \return the current behaviour description
    BehaviourDescription getBehaviourDescription() const;
    //! destructor
    ~SelectBehaviourPage();

   private slots:

    /*!
     * \brief update the list of behaviours when a library
     * is selected.
     */
    virtual void updateBehaviourList();
    /*!
     * \brief update the list of modelling hypotheses when a behaviour
     * is selected.
     */
    virtual void updateModellingHypotheses();

    virtual void selectLibrary();

   signals:

    void behaviourDescriptionChanged(const BehaviourDescription &);

  protected:
    //! list of behaviours declared in the selected library
    QComboBox *bl;
    //! list of modelling hypothesis for the selected behaviour
    QComboBox *mh;
    //! the library location
    QEmacsLineEdit *le;
    //! select library button
    QPushButton *slb;
  private:
    Q_OBJECT
  };  // end of struct SelectBehaviourPage

}  // end of namespace qemacs

#endif /* LIB_QEMACS_SELECTBEHAVIOURPAGE_HXX */
