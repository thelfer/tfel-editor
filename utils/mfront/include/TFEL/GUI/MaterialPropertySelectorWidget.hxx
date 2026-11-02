/*!
 * \file   include/TFEL/GUI/MaterialPropertySelectorWidget.hxx
 * \brief group box setting the value or the function of a property.
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#ifndef LIB_TFEL_GUI_MATERIALPROPERTYSELECTORWIDGET_HXX
#define LIB_TFEL_GUI_MATERIALPROPERTYSELECTORWIDGET_HXX

#include <QtCore/QString>
#include <QtWidgets/QGroupBox>
#include "TFEL/Utilities/GenTypeBase.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/MaterialPropertyDescription.hxx"
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  /*!
   * \brief a widget aiming at selecting material property
   *
   * The group box is titled with the name of the material property and offers a
   * combo box which selects between a constant value and a `Cast3M` function;
   * the two pages of the stacked widget hold the corresponding fields.
   *
   * \note switching the kind of property clears the fields, so a value which
   * has been typed is not kept when the user changes his mind
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT MaterialPropertySelectorWidget
      : QGroupBox {
    /*!
     * \brief constructor
     * \param[in] q: editor widget, used by the line edits and by the dialog
     * which imports a material property from the MFrontMaterials database
     * \param[in] n: name, that is the name of the material property as the
     * behaviour sees it; it titles the group box and names the description
     * which \c getMaterialProperty returns
     * \param[in] m: material, used to initialise the material filter of that
     * dialog
     * \param[in] p: parent widget
     *
     * \note the constant value is constrained by a double validator which
     * accepts the scientific notation; the two fields of a `Cast3M` property
     * are read only as they are filled by an import
     */
    MaterialPropertySelectorWidget(EditorWidget &,
                                   const QString &,
                                   const QString &,
                                   QWidget *const = nullptr);
    /*!
     * \return the selected material property: a `Cast3M` description when a
     * library and a function have been imported, a constant one when the value
     * field is not empty, an empty description otherwise
     */
    MaterialPropertyDescription getMaterialProperty() const;
    //! destructor
    ~MaterialPropertySelectorWidget() override;

   public slots:

    /*!
     * \brief import from a user library: this slot is connected to the
     * "Browse" button but is not implemented, so clicking that button does
     * nothing
     */
    void import();
    /*!
     * \brief import from mfront materials: open \c ImportMFMMaterialProperty,
     * filtered on the name and the material of the property and restricted to
     * the `Castem` interface, and fill the library and function fields with the
     * selection
     *
     * \note a cancelled dialog, or a selection with an empty library or
     * function, leaves the fields untouched
     */
    void importFromMFM();

   protected:
    //! editor widget
    EditorWidget &editor;
    //! external name of the material property
    QString name;
    //! material name
    QString material;
    /*!
     * \brief value used for a constant material property.
     */
    LineEdit *cv;
    /*!
     * \brief a line edit to display the library path for a `Cast3M`
     * material property.
     */
    LineEdit *cle;
    /*!
     * \brief a line edit to display the function for a `Cast3M`
     * material property.
     */
    LineEdit *cfe;

   private:
    Q_OBJECT
  };  // end of struct MaterialPropertySelectorWidget

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MATERIALPROPERTYSELECTORWIDGET_HXX */
