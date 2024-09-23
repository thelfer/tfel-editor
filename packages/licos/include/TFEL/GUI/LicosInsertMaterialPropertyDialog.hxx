/*!
 * \file  LicosInsertMaterialPropertiesDialog.hxx
 * \brief
 * \author Helfer Thomas
 * \date   17/12/2012
 */

#ifndef LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX
#define LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX

#include <QtCore/QVector>
#include <QtWidgets/QDialog>
#include "TFEL/GUI/MaterialPropertySelector.hxx"

namespace tfel::gui {

  struct TextEditBase;

  struct LicosInsertMaterialPropertyDialog : public QDialog {
    LicosInsertMaterialPropertyDialog(TextEditBase&);

   protected slots:

    void insertMaterialProperties(const QVector<MaterialProperty>&);

   private:
    TextEditBase& textEdit;

    Q_OBJECT
  };  // end of struct LicosInsertMaterialPropertyDialog

}  // end of namespace tfel::gui

#endif /* LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX */
