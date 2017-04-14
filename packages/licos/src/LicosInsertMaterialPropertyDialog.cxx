/*! 
 * \file  LicosInsertMaterialPropertyDialog.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 déc. 2012
 */

#include<QtGui/QTextBlock>
#ifdef QEMACS_QT4
#include<QtGui/QVBoxLayout>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QVBoxLayout>
#endif /* QEMACS_QT5 */

#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/MaterialPropertySelector.hxx"
#include"QEmacs/LicosInsertMaterialPropertyDialog.hxx"

namespace qemacs
{
  
  LicosInsertMaterialPropertyDialog::LicosInsertMaterialPropertyDialog(QEmacsTextEditBase& t)
    : QDialog(&t),
      textEdit(t)
  {
    QVBoxLayout *mlayout = new QVBoxLayout;
    MaterialPropertySelector *s = new MaterialPropertySelector(this);
    mlayout->addWidget(s);  
    this->setLayout(mlayout);
    QObject::connect(s,SIGNAL(materialPropertiesSelected(const QVector<MaterialProperty>&)),
		     this,SLOT(insertMaterialProperties(const QVector<MaterialProperty>&)));
  }
  
  void
  LicosInsertMaterialPropertyDialog::insertMaterialProperties(const QVector<MaterialProperty>& mps)
  {
    QVector<MaterialProperty>::const_iterator pmp;
    QTextCursor tc = this->textEdit.textCursor();
    tc.beginEditBlock();
    if(tc.block().text().trimmed().isEmpty()){
      tc.select(QTextCursor::BlockUnderCursor);
      tc.removeSelectedText();
    } else {
      tc.insertText("\n");
    }
    for(pmp=mps.begin();pmp!=mps.end();++pmp){
      tc.insertText(QString("MaterialProperty<castem> '' '%1' '%2'\n")
		    .arg(pmp->library).arg(pmp->function));
    }
    tc.endEditBlock();
  } // end of LicosInsertMaterialPropertyDialog::insertMaterialProperties

} // end of namespace qemacs
