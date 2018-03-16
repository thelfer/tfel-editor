/*!
 * \file   ImportMFMBehaviourWizard.hxx
 * \brief
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_QEMACS_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX
#define LIB_QEMACS_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX

#include <memory>
#include <vector>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizard>
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/BehaviourDescription.hxx"
#include "QEmacs/SelectMFMBehaviourPage.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsTextEditBase;
  // forward declaration
  struct MaterialPropertiesSelectionPage;
  // forward declaration
  struct BehaviourSummaryPage;

  struct QEMACS_MIW_VISIBILITY_EXPORT ImportMFMBehaviourWizard
      : public QWizard {
    //! a simple alias
    using Options = SelectMFMBehaviourPage::Options;

    ImportMFMBehaviourWizard(QEmacsTextEditBase &,
                             const Options & = Options());
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportMFMBehaviourWizard() override;

   protected:
    SelectMFMBehaviourPage *sb;
    MaterialPropertiesSelectionPage *mp;
    BehaviourSummaryPage*c;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMBehaviourWizard

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX */
