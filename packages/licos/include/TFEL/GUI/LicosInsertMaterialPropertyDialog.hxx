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

namespace tfel {

  namespace gui {

    struct MaterialProperty;

    struct TextEditBase;

    struct LicosInsertMaterialPropertyDialog : public QDialog {
      LicosInsertMaterialPropertyDialog(TextEditBase&);

     protected slots:

      void insertMaterialProperties(const QVector<MaterialProperty>&);

     private:
      TextEditBase& textEdit;

      Q_OBJECT
    };  // end of struct LicosInsertMaterialPropertyDialog

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_H */
