/*! 
 * \file  LicosMaterialWizard.cxx
 * \brief
 * \author Helfer Thomas
 * \date   27/09/2012
 */

#include<QtCore/QDebug>
#include<QtWidgets/QPushButton>
#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/LicosThermalBehaviourWizard.hxx"
#include"QEmacs/LicosMaterialWizard.hxx"

namespace qemacs {

  struct LicosMaterialWizard::MaterialPage
    : public QWizardPage
  {
    explicit MaterialPage(QEmacsWidget &q, LicosMaterialWizard &w)
        : tbb(new QPushButton(QObject::tr("Add"))),
          mbb(new QPushButton(QObject::tr("Add"))),
          wizard(w) {
      this->setTitle("Create a new material");
      auto *label = new QLabel("This wizard will help you "
				 "to define the behaviours and the properties "
				 "of a set of materials.");
      label->setWordWrap(true);
      auto * classNameLabel = new QLabel(QObject::tr("&Materials:"));
      classNameLabel->setToolTip(QObject::tr("The user shall provide a list of materials.\n"
					     "A comma shall seperate each material.\n"
					     "A regular expression can be used to select "
					     "many materials at once\n"
					     "Exemple : FUEL.+,CLADDING"));
      auto *classNameLineEdit = new QEmacsLineEdit(q);
      classNameLabel->setBuddy(classNameLineEdit);

      auto *parentNameLabel   = new QLabel(QObject::tr("Material description:"));
      parentNameLabel->setToolTip(QObject::tr("The user shall provide a list of material description.\n"
					      "The material will inherit from the properties and the "
					      "behaviours from thoses material descriptions.\n"
					      "A comma shall seperate each material description.\n"
					      "Exemple : UO2"));
      auto* parentNameLineEdit = new QEmacsLineEdit(q);
      parentNameLabel->setBuddy(parentNameLineEdit);
      auto *l    = new QVBoxLayout;
      auto *grid = new QGridLayout;
      l->addWidget(label);
      grid->addWidget(classNameLabel,0,0);
      grid->addWidget(classNameLineEdit,0,1);
      // parent
      grid->addWidget(parentNameLabel,1,0);
      grid->addWidget(parentNameLineEdit,1,1);
      // thermal behaviour
      auto* tbl = new QLabel(QObject::tr("Thermal behaviour:"));
      grid->addWidget(tbl,2,0);
      grid->addWidget(this->tbb,2,1);
      tbl->setBuddy(this->tbb);
      // mechanical behaviour
      auto* mbl = new QLabel(QObject::tr("Mechanical behaviour:"));
      grid->addWidget(mbl,3,0);
      grid->addWidget(this->mbb,3,1);
      mbl->setBuddy(this->mbb);
      l->addLayout(grid);
      this->setLayout(l);
      
     // baseClassLabel = new QLabel(tr("B&ase class:"));
     // baseClassLineEdit = new QEmacsLineEdit;
     // baseClassLabel->setBuddy(baseClassLineEdit);

     // qobjectMacroCheckBox = new QCheckBox(tr("Generate Q_OBJECT &macro"));

      this->registerField("MaterialName*", classNameLineEdit);
      this->registerField("MaterialDescription", parentNameLineEdit);
     // registerField("baseClass", baseClassLineEdit);
     // registerField("qobjectMacro", qobjectMacroCheckBox);
    }

  private:

    //! Launch the mechanical behaviour wizard
    QPushButton *tbb;

    //! Launch the mechanical behaviour wizard
    QPushButton *mbb;

    //! the calling wizard
    LicosMaterialWizard& wizard;
    
  };

  LicosMaterialWizard::LicosMaterialWizard(QEmacsTextEditBase &t)
      : QWizard(&t),
        textEdit(t),
        mpage(new MaterialPage(t.getQEmacsWidget(), *this)),
        tbw(nullptr),
        mbw(nullptr) {
    this->setWindowTitle(QObject::tr("Material definition wizard"));
    this->addPage(this->mpage);
  } // end of LicosMaterialWizard::LicosMaterialWizard

  bool LicosMaterialWizard::validateCurrentPage() 
  {
    const auto m = this->field("MaterialName").toString();
    return !m.split(',').empty();
  }

  void LicosMaterialWizard::writeMaterial()
  {
    const auto m  =  this->field("MaterialName").toString();
    const auto md =  this->field("MaterialDescription").toString();
    const auto lm = m.split(',');
    if(lm.empty()){
      return;
    }
    auto tc = this->textEdit.textCursor();
    int i;
    tc.insertText("Material");
    if(!md.isEmpty()){
      const auto lmd = md.split(',');
      if(!lmd.empty()){
	tc.insertText("<");
	for(i=0;i!=lmd.size();){
	  tc.insertText(lmd[i]);
	  if(++i!=lmd.size()){
	    tc.insertText(",");
	  }
	}
	tc.insertText(">");
      }
    }
    tc.insertText(" ");
    for(i=0;i!=lm.size();){
      tc.insertText("'"+lm[i]+"'");
      if(++i!=lm.size()){
	tc.insertText(",");
      }
    }
    tc.insertText("\n");
    tc.insertText("EndOfMaterial\n");
  }
  
} // end of namespace qemacs
