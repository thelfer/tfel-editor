/*! 
 * \file  LicosInsertMaterialPropertiesDialog.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 déc. 2012
 */

#ifndef LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX
#define LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_HXX 

#include<QtCore/QVector>
#include<QtWidgets/QDialog>

namespace qemacs
{

  struct MaterialProperty;

  struct QEmacsTextEditBase;

  struct LicosInsertMaterialPropertyDialog : public QDialog {

    LicosInsertMaterialPropertyDialog(QEmacsTextEditBase&);

  protected slots:

    void
    insertMaterialProperties(const QVector<MaterialProperty>&);

  private:

    QEmacsTextEditBase& textEdit;

    Q_OBJECT
  }; // end of struct LicosInsertMaterialPropertyDialog

} // end of namespace qemacs

#endif /* LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_H */

