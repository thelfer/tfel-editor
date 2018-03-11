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
#include "QEmacs/MaterialPropertyDescription.hxx"
#include "QEmacs/MaterialPropertySelectorWidget.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsTextEditBase;

  struct ImportBehaviour : public QWizard {
    ImportBehaviour(QEmacsTextEditBase &);
    //! \return the descrioption of the selected
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportBehaviour() override;

   protected:
    struct SelectBehaviourPage;
    struct MaterialPropertyPage;
    struct ConclusionPage;

    SelectBehaviourPage *sb;
    MaterialPropertyPage *mp;
    ConclusionPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportBehaviour

  struct ImportBehaviour::SelectBehaviourPage : public QWizardPage {
    SelectBehaviourPage(QEmacsWidget &, ImportBehaviour &);

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

    bool validatePage() override;
    //!
    int nextId() const override;

   signals:

    void behaviourDescriptionChanged();

   private:
    //! list of behaviours declared in the selected library
    QComboBox *bl;
    //! list of modelling hypothesis for the selected behaviour
    QComboBox *mh;
    //! the library location
    QEmacsLineEdit *le;
    //! select library button
    QPushButton *slb;
    //! the calling wizard
    ImportBehaviour &wizard;
    Q_OBJECT
  };

  struct ImportBehaviour::MaterialPropertyPage : public QWizardPage {
    MaterialPropertyPage(QEmacsWidget &, ImportBehaviour &);

    int nextId() const override;
    //! return the list of selected material properties
    std::vector<MaterialPropertyDescription>
    getMaterialPropertyDescriptions() const;
    //! destructor
    ~MaterialPropertyPage();

   public slots:
    void updateMaterialPropertiesList();

   private:
    QEmacsWidget &qemacs;
    //! the calling wizard
    ImportBehaviour &wizard;
    //! list of material property selectors
    std::vector<MaterialPropertySelectorWidget *> mpsws;
    Q_OBJECT
  };  // end of ImportBehaviour::MaterialPropertyPage

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
