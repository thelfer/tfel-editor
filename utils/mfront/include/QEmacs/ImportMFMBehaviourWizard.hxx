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

namespace qemacs {

  // forward declaration
  struct QEmacsTextEditBase;
  // forward declaration
  struct SelectMFMBehaviourPage;
  // forward declaration
  struct MaterialPropertiesSelectionPage;

  struct QEMACS_MIW_VISIBILITY_EXPORT ImportMFMBehaviourWizard : public QWizard {
    ImportMFMBehaviourWizard(QEmacsTextEditBase &);
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportMFMBehaviourWizard() override;

   protected:

    struct ConclusionPage;

    SelectMFMBehaviourPage *sb;
    MaterialPropertiesSelectionPage *mp;
    ConclusionPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMBehaviourWizard

  struct ImportMFMBehaviourWizard::ConclusionPage : public QWizardPage {
    ConclusionPage(ImportMFMBehaviourWizard &);

    int nextId() const override;

   private:
    //! the calling wizard
    ImportMFMBehaviourWizard &wizard;
    Q_OBJECT
  };  // end of ImportMFMBehaviourWizard::ConclusionPage

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX */
