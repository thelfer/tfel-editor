/*!
 * \file   MaterialPropertyPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_TFEL_GUI_MATERIALPROPERTYPAGE_HXX
#define LIB_TFEL_GUI_MATERIALPROPERTYPAGE_HXX

#include <QtWidgets/QWizardPage>
#include "TFEL/GUI/MaterialPropertyDescription.hxx"
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct BehaviourDescription;
  // forward declaration
  struct MaterialPropertySelectorWidget;

  /*!
   * \brief a wizard page used to define the material properties
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT MaterialPropertiesSelectionPage
      : public QWizardPage {
    MaterialPropertiesSelectionPage(EditorWidget &);

    int nextId() const override;
    //! return the list of selected material properties
    std::vector<MaterialPropertyDescription> getMaterialPropertyDescriptions()
        const;
    //! destructor
    ~MaterialPropertiesSelectionPage();

   public slots:
    void updateMaterialPropertiesList(const BehaviourDescription &);

   private:
    EditorWidget &editor;
    //! list of material property selectors
    std::vector<MaterialPropertySelectorWidget *> mpsws;
    Q_OBJECT
  };  // end of struct MaterialPropertiesSelectionPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MATERIALPROPERTYPAGE_HXX */
