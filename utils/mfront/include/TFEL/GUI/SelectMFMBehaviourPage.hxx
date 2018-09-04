/*!
 * \file   SelectMFMBehaviourPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_TFEL_GUI_SELECTMFMBEHAVIOURPAGE_HXX
#define LIB_TFEL_GUI_SELECTMFMBEHAVIOURPAGE_HXX

#include <QtWidgets/QWizardPage>
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/SelectMFMBehaviour.hxx"

namespace tfel{

  namespace gui{

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief a wizard page used to select a behaviour from a shared
   * library.
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT SelectMFMBehaviourPage
      : public QWizardPage {
    //! a simple alias
    using Options = SelectMFMBehaviour::Options;

    SelectMFMBehaviourPage(EditorWidget &, const Options &);

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

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_SELECTMFMBEHAVIOURPAGE_HXX */
