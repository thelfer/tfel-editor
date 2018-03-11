/*! 
 * \file  MaterialPropertySelector.cxx
 * \brief
 * \author Helfer Thomas
 * \date   14/12/2012
 */

#include<algorithm>

#include<QtCore/QSet>
#include<QtCore/QDebug>
#include<QtCore/QProcess>
#include<QtCore/QtAlgorithms>
#include<QtWidgets/QLabel>
#include<QtWidgets/QHeaderView>
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QPushButton>
#include<QtWidgets/QFileDialog>
#include<QtWidgets/QDialogButtonBox>
#include<QtWidgets/QDialog>

#include "QEmacs/Utilities.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/MaterialPropertySelector.hxx"

namespace qemacs
{

  MaterialPropertyModel::MaterialPropertyModel(QObject *const p)
    : QAbstractTableModel(p)
  {}
  
  int MaterialPropertyModel::rowCount(const QModelIndex&) const
  {
    return this->m.size();
  }
  
  int MaterialPropertyModel::columnCount(const QModelIndex&) const
  {
    return 2;
  }

  QVariant MaterialPropertyModel::data(const QModelIndex& i,
				       int r) const
  {
    if(!i.isValid()){
      return QVariant();
    }
    if(r==Qt::TextAlignmentRole){
      return static_cast<int>(Qt::AlignLeft|Qt::AlignVCenter);
    } else if(r==Qt::DisplayRole){
      const MaterialProperty& mp = this->m.at(i.row());
      if(i.column()==0){
	return mp.function;
      } else if(i.column()==1){
	return mp.library;
      }
    }
    return QVariant();
  }
    
  QVariant
  MaterialPropertyModel::headerData(int i,
				    Qt::Orientation o,
				    int r) const
  {
    if(r==Qt::DisplayRole){
      if((o==Qt::Horizontal)&&(i==0)){
	return QObject::tr("Name");
      } else if((o==Qt::Horizontal)&&(i==1)){
	return QObject::tr("Library");
      }
    }
    return QVariant();
  }
  
  bool
  MaterialPropertyModel::sort1(const MaterialProperty& m1, const MaterialProperty& m2)
  {
    return m1.function < m2.function;
  }

  bool
  MaterialPropertyModel::sort2(const MaterialProperty& m1, const MaterialProperty& m2)
  {
    return m1.function > m2.function;
  }

  bool
  MaterialPropertyModel::sort3(const MaterialProperty& m1, const MaterialProperty& m2)
  {
    return m1.library < m2.library;
  }

  bool
  MaterialPropertyModel::sort4(const MaterialProperty& m1, const MaterialProperty& m2)
  {
    return m1.library > m2.library;
  }

  void
  MaterialPropertyModel::sort(int c, Qt::SortOrder o)
  {
    if((c==0)&&(o==Qt::AscendingOrder)){
      std::sort(this->m.begin(),this->m.end(),MaterialPropertyModel::sort2);
    } else if((c==0)&&(o==Qt::DescendingOrder)){
      std::sort(this->m.begin(),this->m.end(),MaterialPropertyModel::sort1);
    } else if((c==1)&&(o==Qt::AscendingOrder)){
      std::sort(this->m.begin(),this->m.end(),MaterialPropertyModel::sort4);
    } else if((c==1)&&(o==Qt::DescendingOrder)){
      std::sort(this->m.begin(),this->m.end(),MaterialPropertyModel::sort3);
    }
    this->beginResetModel();
    this->endResetModel();
  }

  const QVector<MaterialProperty>&
  MaterialPropertyModel::materialProperties() const
  {
    return this->m;
  }

  void
  MaterialPropertyModel::setMaterialProperties(const QVector<MaterialProperty>& m_)
  {
    this->m = m_;
    this->beginResetModel();
    this->endResetModel();
  }

  MaterialPropertySelector::MaterialPropertySelector(QEmacsWidget& q,
                                                     QWidget* const p)
      : QWidget(p),
        model(new MaterialPropertyModel(this)),
        view(new QTableView(this)),
        l(new QLabel(QObject::tr("<b>Material properties available in "
                                 "MFrontMaterials:</b>"))),
        lf(new QEmacsLineEdit(q)) {
    auto *mlayout = new QVBoxLayout;
    auto *flayout = new QHBoxLayout;
    // MFrontMaterials
    this->l->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    auto *f = new QLabel(QObject::tr("Material filter or library:"));
    auto *openlib = new QPushButton(QIcon::fromTheme("document-open"),"");
    f->setBuddy(lf);
    this->view->setModel(model);
    this->view->setSortingEnabled(true);
    this->view->setShowGrid(false);
    this->view->verticalHeader()->setVisible(false);
    this->view->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->updateMFMModel(".\\+");
    auto *buttonBox = new QDialogButtonBox();
    auto *add       = new QPushButton(tr("&Add"));
    buttonBox->addButton(add,QDialogButtonBox::ActionRole);
    mlayout->addWidget(l);
    mlayout->addWidget(view);
    flayout->addWidget(f);
    flayout->addWidget(lf);
    flayout->addWidget(openlib);
    mlayout->addLayout(flayout);  
    mlayout->addWidget(buttonBox);  
    this->setLayout(mlayout);
    this->setMinimumWidth(600);
    QObject::connect(openlib,&QPushButton::pressed,
		     this,&MaterialPropertySelector::openLibrary);
    QObject::connect(add,&QPushButton::pressed,
		     this,&MaterialPropertySelector::addMaterialProperties);
    QObject::connect(lf,&QEmacsLineEdit::textChanged,
		     this,&MaterialPropertySelector::updateMFMModel);
  } // end of MaterialPropertySelector::MaterialPropertySelector

  void MaterialPropertySelector::addMaterialProperties()
  {
    const auto lmp = this->view->selectionModel()->selectedIndexes();
    const auto &m  = this->model->materialProperties();
    QVector<MaterialProperty> r;
    QSet<int> rows;
    QModelIndexList::const_iterator p;
    QSet<int>::const_iterator pr;
    for(p=lmp.begin();p!=lmp.end();++p){
      rows.insert(p->row());
    }
    for(pr=rows.begin();pr!=rows.end();++pr){
      r.append(m.at(*pr));
    }
    emit materialPropertiesSelected(r);
  }

  void MaterialPropertySelector::openLibrary()
  {
    const auto& f = QFileDialog::getOpenFileName(this,QObject::tr("Open File"),
						 "","*.so");
    if(f.isEmpty()){
      return;
    }
    this->lf->setText(f);
  } // end of MaterialPropertySelector::openLibrary

  void MaterialPropertySelector::updateMFMModel(const QString& m)
  {
    if(m.endsWith(".so")){
      QFileInfo f(m);
      this->l->setText(QObject::tr("<b>Material properties available in '%1':</b>")
		       .arg(f.fileName()));
    } else {
      this->l->setText(QObject::tr("<b>Material properties available in MFrontMaterials:</b>"));
    }
    this->model->setMaterialProperties(this->callMFMLaws(m));
    this->view->resizeColumnsToContents();
    this->view->horizontalHeader()->setStretchLastSection(true);
  } // end of MaterialPropertySelector::updateMFMModel

  QVector<MaterialProperty> MaterialPropertySelector::callMFMLaws(const QString& r) const
  {
    static QRegExp rout("("+fileNameRegExp()+")\\s+\\(("+fileNameRegExp()+")\\)");
    QVector<MaterialProperty> mprops;
    QProcess p;
    p.start("mfm-laws", QStringList() << r);
    if (!p.waitForStarted()){
      return mprops;
    }
    p.closeWriteChannel();
    if (!p.waitForFinished()){
      return mprops;
    }
    QTextStream stream(&p);
    while (!stream.atEnd()) {
      QString line = stream.readLine();
      if(rout.indexIn(line)>=0){
	MaterialProperty m;
	m.function = rout.cap(1);
	m.library  = rout.cap(2);
	mprops.append(m);
      }
    }
    return mprops;
  }

} // end of namespace qemacs

