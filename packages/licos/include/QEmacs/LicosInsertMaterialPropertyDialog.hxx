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

  class LicosInsertMaterialPropertyDialog
    : public QDialog
  {

    Q_OBJECT

  public:

    LicosInsertMaterialPropertyDialog(QEmacsTextEditBase&);

  protected slots:

    void
    insertMaterialProperties(const QVector<MaterialProperty>&);

  private:

    QEmacsTextEditBase& textEdit;

  }; // end of struct LicosInsertMaterialPropertyDialog

} // end of namespace qemacs

#endif /* LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_H */

