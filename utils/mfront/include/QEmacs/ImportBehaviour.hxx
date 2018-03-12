/*!
 * \file   ImportBehaviour.hxx
 * \brief
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_QEMACS_MTEST_IMPORTBEHAVIOUR_HXX
#define LIB_QEMACS_MTEST_IMPORTBEHAVIOUR_HXX

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

  struct QEMACS_MIW_VISIBILITY_EXPORT ImportBehaviour : public QWizard {
    ImportBehaviour(QEmacsTextEditBase &);
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportBehaviour() override;

   protected:

    struct ConclusionPage;

    SelectBehaviourPage *sb;
    MaterialPropertiesSelectionPage *mp;
    ConclusionPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportBehaviour

  struct ImportBehaviour::ConclusionPage : public QWizardPage {
    ConclusionPage(ImportBehaviour &);

    int nextId() const override;

   private:
    //! the calling wizard
    ImportBehaviour &wizard;
    Q_OBJECT
  };  // end of ImportBehaviour::ConclusionPage

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MTEST_IMPORTBEHAVIOUR_HXX */
