/*!
 * \file  LicosInsertMaterialPropertiesDialog.hxx
 * \brief dialog which inserts the material properties the user has chosen.
 * \author Thomas Helfer
 * \date   17/12/2012
 */

#ifndef LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX
#define LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX

#include <QtCore/QVector>
#include <QtWidgets/QDialog>
#include "TFEL/GUI/MaterialPropertySelector.hxx"

namespace tfel::gui {

  struct TextEditBase;

  /*!
   * \brief a modal dialog which lets the user choose material properties and
   * inserts them in the licos input file at the cursor of the text edit
   *
   * \note the properties are those the `mfm-laws` executable reports, either
   * for `MFrontMaterials` or for a library the user points to
   *
   * \note the licos mode opens this dialog from its "Add material properties"
   * action
   */
  struct LicosInsertMaterialPropertyDialog : public QDialog {
    /*!
     * \brief constructor: build the dialog around a
     * \c MaterialPropertySelector and connect the properties it selects to
     * \c insertMaterialProperties
     * \param[in] t : text edit the properties are inserted in, and the parent
     * of the dialog in the Qt sense
     *
     * \note the selector is a child of this dialog, which owns it
     */
    LicosInsertMaterialPropertyDialog(TextEditBase&);

   protected slots:

    /*!
     * \brief insert one `MaterialProperty` line per given property at the
     * cursor of the text edit
     * \param[in] mps : properties the user has selected
     *
     * \note each line is a `MaterialProperty` line whose first argument, the
     * name of the property, is left empty and whose two last arguments are the
     * library and the function of the property, between simple quotes; the
     * keyword is followed by the `castem` argument, in angle brackets
     *
     * \note the line the cursor stands on is replaced by the properties when it
     * holds nothing but blanks; otherwise a line break is inserted first
     *
     * \note the insertion is a single edit operation, which the user can undo
     * at once
     */
    void insertMaterialProperties(const QVector<MaterialProperty>&);

   private:
    //! text edit the material properties are inserted in
    TextEditBase& textEdit;

    Q_OBJECT
  };  // end of struct LicosInsertMaterialPropertyDialog

}  // end of namespace tfel::gui

#endif /* LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX */
