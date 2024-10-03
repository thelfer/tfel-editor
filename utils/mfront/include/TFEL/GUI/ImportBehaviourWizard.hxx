/*!
 * \file   ImportBehaviourWizard.hxx
 * \brief
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_TFEL_GUI_MTEST_IMPORTBEHAVIOURWIZARD_HXX
#define LIB_TFEL_GUI_MTEST_IMPORTBEHAVIOURWIZARD_HXX

#include <memory>
#include <vector>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizard>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct SelectBehaviourPage;
  // forward declaration
  struct MaterialPropertiesSelectionPage;
  // forward declaration
  struct BehaviourSummaryPage;

  struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportBehaviourWizard : public QWizard {
    ImportBehaviourWizard(TextEditBase &);
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportBehaviourWizard() override;

    SelectBehaviourPage *sb;
    MaterialPropertiesSelectionPage *mp;
    BehaviourSummaryPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportBehaviourWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTEST_IMPORTBEHAVIOURWIZARD_HXX */
