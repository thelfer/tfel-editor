/*! 
 * \file  LicosInsertMaterialPropertyDialog.cxx
 * \brief
 * \author Helfer Thomas
 * \date   17/12/2012
 */

#include<QtGui/QTextBlock>
#include<QtWidgets/QVBoxLayout>
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/MaterialPropertySelector.hxx"
#include"QEmacs/LicosInsertMaterialPropertyDialog.hxx"

namespace qemacs
{
  
  LicosInsertMaterialPropertyDialog::LicosInsertMaterialPropertyDialog(QEmacsTextEditBase& t)
    : QDialog(&t),
      textEdit(t)
  {
    auto *mlayout = new QVBoxLayout;
    auto* s = new MaterialPropertySelector(t.getQEmacsWidget(), this);
    mlayout->addWidget(s);  
    this->setLayout(mlayout);
    QObject::connect(s,&MaterialPropertySelector::materialPropertiesSelected,
		     this,&LicosInsertMaterialPropertyDialog::insertMaterialProperties);
  }
  
  void
  LicosInsertMaterialPropertyDialog::insertMaterialProperties(const QVector<MaterialProperty>& mps)
  {
    auto tc = this->textEdit.textCursor();
    tc.beginEditBlock();
    if(tc.block().text().trimmed().isEmpty()){
      tc.select(QTextCursor::BlockUnderCursor);
      tc.removeSelectedText();
    } else {
      tc.insertText("\n");
    }
    for(const auto& mp : mps){
      tc.insertText(QString("MaterialProperty<castem> '' '%1' '%2'\n")
		    .arg(mp.library).arg(mp.function));
    }
    tc.endEditBlock();
  } // end of LicosInsertMaterialPropertyDialog::insertMaterialProperties

} // end of namespace qemacs
