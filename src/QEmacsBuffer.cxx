/*! 
 * \file  QEmacsBuffer.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 juin 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QSettings>

#include<QtCore/QTime>
#include<QtCore/QTimer>
#include<QtCore/QFileInfo>
#include<QtWidgets/QTabBar>
#include<QtWidgets/QScrollBar>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QSplitter>
#include"QEmacs/Utilities.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"
#include"QEmacs/QEmacsMajorMode.hxx"
#include"QEmacs/QWebEngineViewWrapper.hxx"
#include"QEmacs/QAbstractScrollAreaWrapper.hxx"
#include"QEmacs/QEmacsBuffer.hxx"

namespace qemacs
{
  
  struct QEmacsBuffer::SlaveTabWidget
    : public QTabWidget
  {
    SlaveTabWidget(QWidget *const p)
      : QTabWidget(p)
    {
      setFontSizeAndContentsMargins(this->tabBar());
      this->tabBar()->setFocusPolicy(Qt::NoFocus);
      this->setTabPosition(QTabWidget::North);
    }

    void show()
    {
      // for(int i=0;i!=this->count();++i){
      // 	QWidget *w = this->widget(i);
      // 	if(w->isVisible()){
	  QTabWidget::show();
      // 	  return;
      // 	}
      // }
    }
    
    void focusInEvent(QFocusEvent *) override
    {
      QTabWidget::setFocus();
      this->currentWidget()->setFocus();
    }

    void removeTab(int i)
    {
      auto *w = QTabWidget::widget(i);
      if(w==nullptr){
	return;
      }
      if(w->close()){
	QTabWidget::removeTab(i);
	delete w;
      }
      if(this->count()==0){
	this->hide();
      }
      return;
    }

  };

  QEmacsBuffer::QEmacsBuffer(const int i,
			     QEmacsWidget& w)
    : QWidget(&w),
      qemacs(w),
      slaves(new SlaveTabWidget(this)),
      e(new QEmacsPlainTextEdit(w,*this)),
      id(i)
  {
    this->initialize();
  }

  QEmacsBuffer::QEmacsBuffer(const QString& f,
			     const int i,
			     QEmacsWidget& w)
    : QWidget(&w),
      qemacs(w),
      slaves(new SlaveTabWidget(this)),
      e(new QEmacsPlainTextEdit(f,w,*this)),
      id(i)
  {
    this->initialize();
    this->emitNewTreatedFile(this->e->getCompleteFileName());
  }

  int
  QEmacsBuffer::getId() const
  {
    return this->id;
  } // end of QEmacsBuffer::getId

  void
  QEmacsBuffer::emitNewTreatedFile(const QString& f)
  {
    emit newTreatedFile(f);
  } // end of QEmacsBuffer::emitNewTreatedFile

  QString
  QEmacsBuffer::getBufferRawName() const
  {
    return this->e->getFileName();
  } // end of QEmacsBuffer::setBufferName

  QString
  QEmacsBuffer::getBufferName() const
  {
    QString s = this->getBufferNameSuffix();
    QString f = QFileInfo(this->getBufferRawName()).fileName();
    if(!s.isEmpty()){
      return f+" <"+s+">";
    }
    return f;
  } // end of QEmacsBuffer::setBufferName

  QString
  QEmacsBuffer::getBufferNameSuffix() const
  {
    return this->bufferNameSuffix;
  } // end of QEmacsBuffer::setBufferName
    
  void
  QEmacsBuffer::focusInEvent(QFocusEvent *)
  {
    if(!this->e->hasFocus()){
      this->e->setFocus();
    }
  } // end of QEmacsBuffer::focusInEvent

  void
  QEmacsBuffer::initialize()
  {
    // setting 
    this->e->setMainFrame(true);
    this->e->setFocus();
    // without slaves, the slaves tab widget is hidden
    this->slaves->setTabPosition(QTabWidget::South);
    this->slaves->setDocumentMode(true);
    this->slaves->setTabsClosable(true);
      //setFontSizeAndContentsSize(this->slaves->tabBar());
    if(this->slaves->count()==0){
      this->slaves->hide();
    }
    this->info   = new QHBoxLayout;
    this->timer  = new QTimer(this);
    this->bni = new QLabel(this);
    this->rpi = new QLabel(this);
    this->api = new QLabel(this);
    this->ii  = new QLabel(this);
    this->mi  = new QLabel(this);
    this->ti  = new QLabel(this);
    setQLabelFontSizeAndMargins(this->bni);
    setQLabelFontSizeAndMargins(this->rpi);
    setQLabelFontSizeAndMargins(this->api);
    setQLabelFontSizeAndMargins(this->mi);
    setQLabelFontSizeAndMargins(this->ti);
    this->info->addStretch();
    this->info->addWidget(this->bni);
    this->info->addStretch();
    this->info->addWidget(this->rpi);
    this->info->addStretch();
    this->info->addWidget(this->api);
    this->info->addStretch();
    this->info->addWidget(this->ii);
    this->info->addWidget(this->mi);
    this->info->addStretch();
    this->info->addWidget(this->ti);
    this->info->addStretch();
    // main gui
    auto *vl  = new QVBoxLayout;
    auto *hl  = new QHBoxLayout;
    auto *sp  = new QSplitter;
    sp->setOrientation(Qt::Vertical);
    QSizePolicy qs;
    qs.setHorizontalPolicy(QSizePolicy::Minimum);
    qs.setVerticalPolicy(QSizePolicy::Minimum);
    this->e->setSizePolicy(qs);
    qs.setHorizontalPolicy(QSizePolicy::Maximum);
    qs.setVerticalPolicy(QSizePolicy::Maximum);
    this->bni->setSizePolicy(qs);
    this->rpi->setSizePolicy(qs);
    this->api->setSizePolicy(qs);
    this->mi->setSizePolicy(qs);
    this->ti->setSizePolicy(qs);
    hl->setMargin(0);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(0);
    sp->addWidget(this->e);
    sp->addWidget(this->slaves);
    hl->addWidget(sp);
    vl->setMargin(0);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);
    //    vl->addWidget(this->e);
    //    vl->addWidget(this->slaves);
    vl->addLayout(hl);
    vl->addLayout(this->info);
    this->updateBufferInformations();
    this->setLayout(vl);
    // timer
    QObject::connect(this->timer, SIGNAL(timeout()),
		  this, SLOT(updateDate()));
    // update every second
    this->timer->start(1000);
    // connecting signals
    QObject::connect(this->e,SIGNAL(fileNameChanged(const QString&)),
		     this,SLOT(emitNewTreatedFile(const QString&)));
    QObject::connect(this->e,SIGNAL(fileNameChanged(const QString&)),
		     this,SLOT(updateBufferName()));
    QObject::connect(this->e,SIGNAL(cursorPositionChanged()),
		     this,SLOT(updatePosition()));
    QObject::connect(this->e,SIGNAL(cursorPositionChanged()),
		     this,SLOT(updateDate()));
    QObject::connect(this->e,SIGNAL(majorModeChanged()),
		     this,SLOT(updateBufferInformations()));
    QObject::connect(this->slaves,SIGNAL(tabCloseRequested(int)),
		     this,SLOT(closeSlave(int)));
    this->updateBufferName();
  }

  void
  QEmacsBuffer::updateBufferName()
  {
    QString o = this->getBufferName();
    this->bufferNameSuffix = this->qemacs.chooseBufferNameSuffix(this,this->e->getFileName()); 
    this->updateBufferInformations();
    QString n = this->getBufferName();
    emit bufferNameChanged(this,o,n);
  }

  QVector<QMenu*>
  QEmacsBuffer::getSpecificMenus()
  {
    return this->e->getSpecificMenus();
  } // end of QEmacsBuffer::getSpecificMenu

  QIcon
  QEmacsBuffer::getIcon() const
  {
    return this->e->getIcon();
  } // end of QEmacsBuffer::getIcon


  void
  QEmacsBuffer::updateDate()
  {
    QTime t = QTime::currentTime();
    int   h = t.hour(); 
    int   m = t.minute(); 
    QString tl;
    if(h<9){
      tl += "0"+QString::number(h);
    } else {
      tl += QString::number(h);
    }
    tl += ":";
    if(m<9){
      tl += "0"+QString::number(m);
    } else {
      tl += QString::number(m);
    }
    this->ti->setText(tl);
  }

  void
  QEmacsBuffer::updatePosition()
  {
    const QTextCursor&   c = this->e->textCursor();
    const QTextDocument& d = *(this->e->document());
    const int cn = c.blockNumber()+1;
    const int bn = d.blockCount();
    if(bn>0){
      if(cn==bn){
	this->rpi->setText("100%");
      } else {
	QString p = QString::number(static_cast<int>(qreal(cn)/qreal(bn)*100.))+"%";
	this->rpi->setText(p);
      }
    }
    QTextCursor b(c);
    b.movePosition(QTextCursor::StartOfBlock,
		   QTextCursor::KeepAnchor);
    this->api->setText("("+QString::number(cn)+","+QString::number(c.position()-b.position())+")");
  }
  
  void
  QEmacsBuffer::updateBufferInformations()
  {
    QString i;
    const QString& s = this->getBufferNameSuffix();
    if(s.isEmpty()){
      this->bni->setText("<b>"+this->getBufferName()+"</b>");
    } else {
      this->bni->setText("<b>"+this->getBufferName()+" &lt;"+s+"&gt;</b>");
    }
    this->updatePosition();
    QIcon ic = this->e->getIcon();
    if(!ic.isNull()){
      this->ii->setPixmap(ic.pixmap(12,12));
      this->ii->show();
    } else {
      this->ii->hide();
    }
    if(this->e->hasMajorMode()){
      if(!ic.isNull()){
	this->mi->setText(" "+this->e->getMajorMode().getName());
      } else {
	this->mi->setText(this->e->getMajorMode().getName());
      }
      this->mi->show();
    } else {
      this->mi->hide();
    }
    this->updateDate();
  } // end QEmacsBuffer::updateBufferInformations

  QEmacsPlainTextEdit&
  QEmacsBuffer::getMainFrame()
  {
    return *(this->e);
  } // end of QEmacsBuffer::getQEmacsTextEdit

  bool
  QEmacsBuffer::hasSlaves() const
  {
    if(this->slaves==nullptr){
      return false;
    }
    return this->slaves->count()!=0;
  }

  QWidget *
  QEmacsBuffer::addSlave(const QString& t,
			 QWidget *const s)
  {
    if(s==nullptr){
      return nullptr;
    }
    auto *pw = qobject_cast<QWebEngineView *>(s);
    auto *p  = qobject_cast<QAbstractScrollArea *>(s);
    if(pw!=nullptr){
      auto *w = new QWebEngineViewWrapper(pw,this);
      this->slaves->addTab(w,t);
      this->slaves->setCurrentWidget(w);
      this->slaves->show();
      return w;
    } else if(p!=nullptr){
      auto *w = new QAbstractScrollAreaWrapper(p,this);
      this->slaves->addTab(w,t);
      this->slaves->setCurrentWidget(w);
      this->slaves->show();
      return w;
    }
    this->slaves->addTab(s,t);
    this->slaves->setCurrentWidget(s);
    this->slaves->show();
    return s;
  } // end of QEmacsBuffer::addSlave

  int
  QEmacsBuffer::getSlaveIndex(QWidget * const p) const
  {
    for(int i=0;i!=this->slaves->count();++i){
      QWidget *pi = this->slaves->widget(i);
      if(pi==p){
	return i;
      }
      QAbstractScrollAreaWrapper *w;
      w = qobject_cast<QAbstractScrollAreaWrapper *>(pi);
      if(w!=nullptr){
	if(p==w->getWrappedObject()){
	  return i;
	}
      }
    }
    return -1;
  } // end of QEmacsBuffer::getSlaveIndex

  QString
  QEmacsBuffer::getSlaveName(QWidget * const p) const
  {
    QString n;
    int i = this->getSlaveIndex(p);
    if(i!=-1){
      n = this->slaves->tabText(i);
    }
    return n;
  }

  void
  QEmacsBuffer::setSlaveIcon(QWidget *const p,
			     const QIcon& i)
  {
    int sid = this->getSlaveIndex(p);
    if(sid!=-1){
      this->slaves->setTabIcon(sid,i);
    }
  }

  void
  QEmacsBuffer::setSlaveName(QWidget *const p,
			     const QString & n)
  {
    int i = this->getSlaveIndex(p);
    if(i!=-1){
      this->slaves->setTabText(i,n);
    }
  } // end of QEmacsBuffer::setSlaveName

  void
  QEmacsBuffer::removeSlave(QWidget *const s)
  {
    for(int i=0;i!=this->slaves->count();++i){
      if(s==this->slaves->widget(i)){
	this->slaves->removeTab(i);
	return;
      }
    }
  }

  void
  QEmacsBuffer::hideSlave(QWidget *const s)
  {
    bool h = true;
    for(int i=0;i!=this->slaves->count();++i){
      QWidget *w = this->slaves->widget(i);
      if(s==w){
	s->hide();
      }
      h = h && (!w->isVisible());
    }
    if(h){
      this->slaves->hide();
    }
  }

  void
  QEmacsBuffer::focusCurrentSlave()
  {
    if(this->slaves!=nullptr){
      QWidget *s = this->slaves->currentWidget();
      if(s!=nullptr){
	s->setFocus();
      }
    }
  } // end of QEmacsBuffer::focusCurrentSlave

  bool
  QEmacsBuffer::areSlavesVisible() const
  {
    if(this->slaves==nullptr){
      return false;
    }
    return this->slaves->isVisible();
  }

  void
  QEmacsBuffer::focusMainFrame()
  {
    this->e->setFocus();
  } // end of QEmacsBuffer::focusMainFrame

  void
  QEmacsBuffer::showSlaves()
  {
    if(this->slaves->count()==0){
      this->qemacs.displayInformativeMessage(QObject::tr("no slave to be shown"));
      return;
    }
    this->slaves->show();
  } // end of QEmacsBuffer::showSlaves

  void
  QEmacsBuffer::hideSlaves()
  {
    this->slaves->hide();
  } // end of QEmacsBuffer::hideSlaves

  void
  QEmacsBuffer::closeSlave(int i)
  {
    this->slaves->removeTab(i);
    if(this->slaves->count()==0){
      this->slaves->hide();
    }
  }

  bool
  QEmacsBuffer::isOkToClose() const
  {
    return !this->e->document()->isModified();
  } // end of QEmacsBuffer::isOkToClose

  void
  QEmacsBuffer::closeCurrentSlave()
  {
    if(this->slaves->count()!=0){
      this->closeSlave(this->slaves->currentIndex());
    }
  }

} // end of namespace qemacs
