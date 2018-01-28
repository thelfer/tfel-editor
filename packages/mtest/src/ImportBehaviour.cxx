/*!
 * \file   ImportBehaviour.cxx
 * \brief    
 * \author THOMAS HELFER
 * \date   02 mai 2017
 */

#include<QtCore/QDebug>
#include<QtCore/QStringListModel>
#include<QtWidgets/QListWidget>
#include<QtWidgets/QPushButton>
#include<QtWidgets/QFileDialog>
#include "TFEL/System/ExternalLibraryManager.hxx"
#include "TFEL/Material/ModellingHypothesis.hxx"

#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ImportBehaviour.hxx"

namespace qemacs
{

  ImportBehaviour::SelectHypothesisPage::SelectHypothesisPage(ImportBehaviour& w)
    : wizard(w)
  {
    using tfel::material::ModellingHypothesis;
    auto *vl = new QVBoxLayout;
    auto *l  = new QListWidget;
    auto r   = 0;
    auto r3D = 0;
    l->setSelectionMode(QAbstractItemView::SingleSelection);
    for(const auto& h : ModellingHypothesis::getModellingHypotheses()){
      const auto sh = QString::fromStdString(ModellingHypothesis::toString(h));
      if(sh=="Tridimensional"){
	r3D = r;
      }
      auto *i = new QListWidgetItem;
      i->setText(sh);
      l->insertItem(r,i);
      ++r;
    }
    vl->addWidget(l);
    this->registerField("Hypothesis*",l);
    qDebug() << this->field("Hypothesis").toString();    
    //    l->setCurrentRow(r3D);
    this->setLayout(vl);
  } // end of ImportBehaviour::SelectHypothesisPage::SelectHypothesisPage
  
  ImportBehaviour::SelectBehaviourPage::SelectBehaviourPage(ImportBehaviour& w)
    : bl(new QComboBox(this)),
      le(new QLineEdit(this)),
      slb(new QPushButton(QObject::tr("Open"),this)),
      wizard(w)
  {
    this->setTitle("Select a behaviour");
    auto* label = new QLabel("This wizard will help you "
			     "to define the behaviour");
    label->setWordWrap(true);
    auto* librayLabel  = new QLabel(QObject::tr("&Library:"));
    librayLabel->setToolTip(QObject::tr("The user shall select a library"));
    librayLabel->setBuddy(this->le);
    auto* blabel = new QLabel(QObject::tr("&Behaviour:"));
    blabel->setToolTip(QObject::tr("The user shall shall select a behaviour "
				   "in the selected library."));
    auto* be = new QLineEdit(this);

    blabel->setBuddy(be);
    auto *l = new QVBoxLayout;
    auto *g = new QGridLayout;

    l->addWidget(label);
    g->addWidget(librayLabel,0,0);
    g->addWidget(this->le,0,1);
    g->addWidget(this->slb,0,2);
    // parent
    g->addWidget(blabel,1,0);
    g->addWidget(be,1,1);
    g->addWidget(this->bl,1,2);
    // main layout
    l->addLayout(g);
    this->setLayout(l);

    QObject::connect(this->slb, SIGNAL(pressed()),
		     this, SLOT(selectLibrary()));
    QObject::connect(this->le,SIGNAL(textChanged(const QString&)),
		     this,SLOT(updateBehaviourList(const QString&)));
    QObject::connect(this->bl,SIGNAL(activated(const QString &)),
		     be,SLOT(setText(const QString &)));
    // baseClassLabel = new QLabel(tr("B&ase class:"));
    // baseClassLineEdit = new QLineEdit;
    // baseClassLabel->setBuddy(baseClassLineEdit);

    // qobjectMacroCheckBox = new QCheckBox(tr("Generate Q_OBJECT &macro"));

    this->registerField("Library*", this->le);
    this->registerField("Behaviour*", be);
    // registerField("baseClass", baseClassLineEdit);
    // registerField("qobjectMacro", qobjectMacroCheckBox);
  }

  void ImportBehaviour::SelectBehaviourPage::selectLibrary(){
    qDebug() << "h: " << this->wizard.getHypothesis();
    const auto& l =
      QFileDialog::getOpenFileName(this,tr("Select Library"));
    if(l.isEmpty()){
      return;
    }
    this->le->setText(l);
  }

  void ImportBehaviour::SelectBehaviourPage::updateBehaviourList(const QString& l)
  {
    using tfel::system::ExternalLibraryManager;
    auto& elm = ExternalLibraryManager::getExternalLibraryManager();
    const auto lib  = l.toStdString();
    auto b = QStringList{};
    auto epts = std::vector<std::string>{};
    this->bl->clear();
    try{
      epts = elm.getEntryPoints(lib);
    } catch(std::exception& ex){
      qDebug() << ex.what();
      return;
    }
    for(const auto& e:epts){
      try{
	if(elm.getMaterialKnowledgeType(lib,e)==1u){
	  this->bl->addItem(QString::fromStdString(e));
	}
      } catch(std::exception& ex){
	qDebug() << ex.what();
      }
    }
  } // end of ImportBehaviour::SelectBehaviourPage::updateBehaviourList

  bool ImportBehaviour::SelectBehaviourPage::validatePage()
  {
    return this->wizard.getBehaviour()!=nullptr;
  } // end of ImportBehaviour::SelectLibrary::validatePage
  
  //! 
  int ImportBehaviour::SelectBehaviourPage::nextId() const
  {
    auto b = this->wizard.getBehaviour();
    if(b){
      return b->getMaterialPropertiesNames().empty() ? 3 : 1;
    }
    return 3;
  }

  ImportBehaviour::MaterialPropertyPage::MaterialPropertyPage(ImportBehaviour& w)
    : wizard(w)
  {
    
  }  // end of ImportBehaviour::MaterialPropertyPage::MaterialPropertyPage

  int ImportBehaviour::MaterialPropertyPage::nextId() const{
    return -1;
  }
  
  ImportBehaviour::ImportBehaviour(QEmacsTextEditBase& t)
    : QWizard(&t),
      sh(new SelectHypothesisPage(*this)),
      sb(new SelectBehaviourPage(*this)),
      mp(new MaterialPropertyPage(*this))
  {
    this->setWindowTitle(QObject::tr("Import Behaviour"));
    this->setPage(0,this->sh);
    this->setPage(1,this->sb);
    this->setPage(2,this->mp);
    this->setStartId(0);
  } // end of ImportBehaviour::ImportBehaviour

  QString ImportBehaviour::getHypothesis() const
  {
    return this->field("Hypothesis").toString();
  } // end of ImportBehaviour::getHypothesis
  
  std::shared_ptr<mtest::Behaviour> ImportBehaviour::getBehaviour() const
  {
    using namespace mtest;
    using namespace tfel::material;
    try{
      return Behaviour::getBehaviour(this->getHypothesis().toStdString(),
				     this->field("Library").toString().toStdString(),
				     this->field("Behaviour").toString().toStdString(),
				     Behaviour::Parameters(),
				     ModellingHypothesis::TRIDIMENSIONAL);
    } catch(std::exception&){
    }
    return {};    
  } // end of ImportBehaviour::getBehaviour
  
  ImportBehaviour::~ImportBehaviour() = default;
  
} // end of namespace qemacs
