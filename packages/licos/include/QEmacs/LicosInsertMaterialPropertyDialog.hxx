/*! 
 * \file  LicosInsertMaterialPropertiesDialog.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 déc. 2012
 */

#ifndef _LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_H_
#define _LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_H_ 

#include<QtCore/QVector>
#ifdef QEMACS_QT4
#include<QtGui/QDialog>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QDialog>
#endif /* QEMACS_QT5 */

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

#endif /* _LIB_LICOSINSERTMATERIALPROPERTIESDIALOG_H */

