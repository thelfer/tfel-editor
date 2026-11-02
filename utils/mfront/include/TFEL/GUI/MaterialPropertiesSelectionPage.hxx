/*!
 * \file   MaterialPropertyPage.hxx
 * \brief wizard page selecting the material properties to use.
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
   *
   * One \c MaterialPropertySelectorWidget is created for each material property
   * the selected behaviour declares, in a scroll area.
   *
   * \note the page registers no field with the wizard and does not require the
   * properties to be defined: those the user leaves undefined are simply not
   * returned
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT MaterialPropertiesSelectionPage
      : public QWizardPage {
    /*!
     * \brief constructor: an empty page, the selectors are only created by
     * \c updateMaterialPropertiesList
     * \param[in] q: editor widget, given to the selectors
     */
    MaterialPropertiesSelectionPage(EditorWidget &);

    /*!
     * \return the identifier of the summary page, whatever the material
     * properties are
     */
    int nextId() const override;
    /*!
     * \return the list of selected material properties, in the order the
     * selectors are shown
     *
     * \note a property the user has not defined, for which neither a constant
     * value nor a `Cast3M` library and function are given, is skipped
     */
    std::vector<MaterialPropertyDescription> getMaterialPropertyDescriptions()
        const;
    /*!
     * \brief destructor
     *
     * The selectors are children of the scroll area of the page and are
     * released with it
     */
    ~MaterialPropertiesSelectionPage();

   public slots:
    /*!
     * \brief rebuild the list of selectors for the given behaviour
     * \param[in] bd : description of the behaviour whose material
     * properties are to be defined
     *
     * The slot is connected by \c ImportBehaviourWizard and
     * \c ImportMFMBehaviourWizard to \c behaviourDescriptionChanged of the page
     * which selects the behaviour, and is called once by the constructor of
     * those wizards.
     *
     * The behaviour is instantiated to read the names of its material
     * properties and the material it belongs to, which is used to pre-fill the
     * filter of the MFrontMaterials import.
     *
     * \note the previous selectors are destroyed with the scroll area which
     * holds them and the values typed in them are lost; the list of
     * selectors is cleared before that, so it never holds dangling pointers
     *
     * \note the page is left untouched when the behaviour can not be
     * instantiated, and the material is left empty when the library of that
     * behaviour can not be analysed
     */
    void updateMaterialPropertiesList(const BehaviourDescription &);

   private:
    //! editor widget given to each selector which is created
    EditorWidget &editor;
    //! list of material property selectors
    std::vector<MaterialPropertySelectorWidget *> mpsws;
    Q_OBJECT
  };  // end of struct MaterialPropertiesSelectionPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MATERIALPROPERTYPAGE_HXX */
