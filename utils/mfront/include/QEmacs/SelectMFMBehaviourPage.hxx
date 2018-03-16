/*!
 * \file   SelectMFMBehaviourPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_QEMACS_SELECTMFMBEHAVIOURPAGE_HXX
#define LIB_QEMACS_SELECTMFMBEHAVIOURPAGE_HXX

#include <QtWidgets/QWizardPage>
#include "QEmacs/BehaviourDescription.hxx"
#include "QEmacs/SelectMFMBehaviour.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;

  /*!
   * \brief a wizard page used to select a behaviour from a shared
   * library.
   */
  struct QEMACS_MIW_VISIBILITY_EXPORT SelectMFMBehaviourPage
      : public QWizardPage {
    //! a simple alias
    using Options = SelectMFMBehaviour::Options;

    SelectMFMBehaviourPage(QEmacsWidget &, const Options &);

    int nextId() const override;

    bool validatePage() override;

    BehaviourDescription getBehaviourDescription() const;
    
    //! destructor
    ~SelectMFMBehaviourPage() override;

   signals:

    void behaviourDescriptionChanged(const BehaviourDescription &);

    void doubleClicked(const BehaviourDescription &);

   private:
    SelectMFMBehaviour *sb;
    Q_OBJECT
  };  // end of struct SelectMFMBehaviourPage

}  // end of namespace qemacs

#endif /* LIB_QEMACS_SELECTMFMBEHAVIOURPAGE_HXX */
