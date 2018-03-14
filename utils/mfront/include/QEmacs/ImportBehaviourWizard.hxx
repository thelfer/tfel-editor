/*!
 * \file   ImportBehaviourWizard.hxx
 * \brief
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_QEMACS_MTEST_IMPORTBEHAVIOURWIZARD_HXX
#define LIB_QEMACS_MTEST_IMPORTBEHAVIOURWIZARD_HXX

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
  struct SelectBehaviourPage;
  // forward declaration
  struct MaterialPropertiesSelectionPage;

  struct QEMACS_MIW_VISIBILITY_EXPORT ImportBehaviourWizard : public QWizard {
    ImportBehaviourWizard(QEmacsTextEditBase &);
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportBehaviourWizard() override;

   protected:

    struct ConclusionPage;

    SelectBehaviourPage *sb;
    MaterialPropertiesSelectionPage *mp;
    ConclusionPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportBehaviourWizard

  struct ImportBehaviourWizard::ConclusionPage : public QWizardPage {
    ConclusionPage(ImportBehaviourWizard &);

    int nextId() const override;

   private:
    //! the calling wizard
    ImportBehaviourWizard &wizard;
    Q_OBJECT
  };  // end of ImportBehaviourWizard::ConclusionPage

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MTEST_IMPORTBEHAVIOURWIZARD_HXX */
