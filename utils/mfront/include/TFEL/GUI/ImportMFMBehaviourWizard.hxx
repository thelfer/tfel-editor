/*!
 * \file   ImportMFMBehaviourWizard.hxx
 * \brief
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_TFEL_GUI_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX
#define LIB_TFEL_GUI_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX

#include <memory>
#include <vector>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizard>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/SelectMFMBehaviourPage.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct MaterialPropertiesSelectionPage;
  // forward declaration
  struct BehaviourSummaryPage;

  struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportMFMBehaviourWizard
      : public QWizard {
    //! a simple alias
    using Options = SelectMFMBehaviourPage::Options;

    ImportMFMBehaviourWizard(TextEditBase &, const Options & = Options());
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportMFMBehaviourWizard() override;

   protected:
    SelectMFMBehaviourPage *sb;
    MaterialPropertiesSelectionPage *mp;
    BehaviourSummaryPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMBehaviourWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX */
