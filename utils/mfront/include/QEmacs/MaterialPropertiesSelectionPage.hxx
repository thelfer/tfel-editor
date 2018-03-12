/*!
 * \file   MaterialPropertyPage.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_QEMACS_MATERIALPROPERTYPAGE_HXX
#define LIB_QEMACS_MATERIALPROPERTYPAGE_HXX

#include <QtWidgets/QWizardPage>
#include "QEmacs/MaterialPropertyDescription.hxx"
#include "QEmacs/QEmacsMFrontImportWidgetsConfig.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct BehaviourDescription;
  // forward declaration
  struct MaterialPropertySelectorWidget;

  /*!
   * \brief a wizard page used to define the material properties 
   */
  struct QEMACS_MIW_VISIBILITY_EXPORT MaterialPropertiesSelectionPage
      : public QWizardPage {
    MaterialPropertiesSelectionPage(QEmacsWidget &);

    int nextId() const override;
    //! return the list of selected material properties
    std::vector<MaterialPropertyDescription>
    getMaterialPropertyDescriptions() const;
    //! destructor
    ~MaterialPropertiesSelectionPage();

   public slots:
    void updateMaterialPropertiesList(const BehaviourDescription&);

   private:
    QEmacsWidget &qemacs;
    //! list of material property selectors
    std::vector<MaterialPropertySelectorWidget *> mpsws;
    Q_OBJECT
  };  // end of struct MaterialPropertiesSelectionPage

} // end of namespace qemacs

#endif /* LIB_QEMACS_MATERIALPROPERTYPAGE_HXX */
