/*! 
 * \file   QEmacsWidget.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 juin 2012
 */

#include<QtCore/QFile>
#include<QtCore/QTimer>
#include<QtCore/QDebug>
#include<QtCore/QTextStream>

#ifdef QEMACS_QT4
#include<QtGui/QCompleter>
#include<QtGui/QMessageBox>
#include<QtGui/QFileDialog>
#include<QtGui/QHBoxLayout>
#include<QtGui/QVBoxLayout>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QCompleter>
#include<QtWidgets/QMessageBox>
#include<QtWidgets/QFileDialog>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QVBoxLayout>
#endif /* QEMACS_QT5 */
#include<QtGui/QCloseEvent>

#include"QEmacs/QEmacsCommand.hxx"
#include"QEmacs/QEmacsCommandFactory.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/QEmacsWidget.hxx"

struct QEmacsModeRessourceLoader
{
  QEmacsModeRessourceLoader()
    : l(getLoader())
  {}
private:
  struct Loader
  {
    Loader()
    {
      Q_INIT_RESOURCE(QEmacsResources);
    }
  };
  static Loader&
  getLoader()
  {
    static Loader l;
    return l;
  }
  Loader l;
};

namespace qemacs
{

  static void
  setQLineEditFont(QLineEdit *l)
  {
    QFont f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0,0,0,0);
  }

  static void
  setQEmacsLineEditFont(QEmacsLineEdit *l)
  {
    QFont f = l->font();
    f.setPointSize(8);
    l->setFont(f);
    l->setContentsMargins(0,0,0,0);
  }

  struct QEmacsWidget::OpenFile
    : public QEmacsFilePathUserInput
  {

    OpenFile(QEmacsWidget& p,
	     const QString& path)
      : QEmacsFilePathUserInput(QObject::tr("open file :"),p),
	qemacs(p)
    {
      QString npath(path);
      if(!npath.isEmpty()){
	if(!npath.endsWith(QDir::separator())){
	  npath += QDir::separator();
	}
      }
      this->input->setText(npath);
    }

    ~OpenFile()
    {}

  protected:

    virtual void
    treatUserInput(void) override
    {
      this->qemacs.openFile(this->input->text());
    }

    QEmacsWidget& qemacs;

  }; // end of struct QEmacsTextEdit::OpenFile

  struct QEmacsWidget::ChangeBuffer
    : public QEmacsLineEdit
  {

    ChangeBuffer(QEmacsWidget& p,
		 const QStringList& buffers,
		 const QString& d)
      : QEmacsLineEdit(QObject::tr("switch to buffer")
		       + (!d.isEmpty() ? QObject::tr("(default '%1')").arg(d) : "")
		       +QObject::tr(" :"),p),
	qemacs(p),
	defaultBuffer(d)
    {
      QStringList b(buffers);
      if(!this->defaultBuffer.isEmpty()){
	b << ""; // add the emtpy string for default completion
      }
      QCompleter * c = new QCompleter(buffers,&p);
      c->setWidget(this->input);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(c,false);
    }

    ~ChangeBuffer()
    {}

  protected:

    virtual void
    treatUserInput(void) override
    {
      QString t = this->input->text();
      if(t.isEmpty()){
	this->qemacs.changeBuffer(this->defaultBuffer);
      } else {
	this->qemacs.changeBuffer(t);
      }
    }

    QEmacsWidget& qemacs;

    QString defaultBuffer;

  }; // end of struct QEmacsTextEdit::ChangeBuffer

  struct QEmacsWidget::Command
    : public QEmacsLineEdit
  {

    Command(QEmacsWidget& p)
      : QEmacsLineEdit(QObject::tr("qemacs command :"),p),
	qemacs(p)
    {
      QEmacsCommandFactory& f = QEmacsCommandFactory::getQEmacsCommandFactory();
      QCompleter * c = new QCompleter(f.getAvailableQEmacsCommandsNames(),&p);
      c->setWidget(this->input);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(c,false);
    }

    ~Command()
    {}

  protected:

    virtual void
    treatUserInput(void) override
    {
      // the input must be kept before calling the removeUserInput
      // method (this causes the qemacswdget to delete this).
      QString i =this->input->text();
      this->qemacs.removeUserInput(this);
      this->qemacs.launchCommand(i);
    }

    QEmacsWidget& qemacs;

  }; // end of struct QEmacsTextEdit::Command

  QStringList&
  QEmacsWidget::getRecentFiles()
  {
    static QStringList files;
    return files;
  } // end of QEmacsWidget::getRecentFiles

  void
  QEmacsWidget::addToRecentFiles(const QString& f)
  {
    if(f.isEmpty()){
      return;
    }
    QStringList& files = QEmacsWidget::getRecentFiles();
    files.append(f);
    if(files.size()>100){
      files.pop_front();
    }
  } // end of QEmacsWidget::addToRecentFiles

  QEmacsWidget::QEmacsWidget(QWidget *const p)
    : QWidget(p), 
      buffers(new QStackedWidget),
      minibuffer(new QStackedWidget),
      um(new QLineEdit),
      ui(0),
      eui(new QLineEdit),
      nid(0)
  {
    QEmacsModeRessourceLoader loader;
    QVBoxLayout * vl = new QVBoxLayout;
    this->buffers->setFocusPolicy(Qt::NoFocus);
    this->setFocusPolicy(Qt::NoFocus);
    this->um->setReadOnly(true);
    this->eui->setReadOnly(true);
    setQLineEditFont(this->um);
    setQLineEditFont(this->eui);
    this->minibuffer->addWidget(this->um);
    this->minibuffer->addWidget(this->eui);
    this->minibuffer->setContentsMargins(0,0,0,0);
    QSizePolicy s;
    s.setHorizontalPolicy(QSizePolicy::Minimum);
    s.setVerticalPolicy(QSizePolicy::Maximum);
    this->minibuffer->setSizePolicy(s);
    vl->setMargin(0);
    vl->setContentsMargins(0,0,0,0);
    vl->addWidget(this->buffers);
    vl->addWidget(this->minibuffer);
    vl->setSpacing(0);
    this->setLayout(vl);
  } // end of QEmacsWigdet::QEmacsWigdet

  void
  QEmacsWidget::changeMainFramesFont(const QFont& f)
  {
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b==nullptr){
	continue;
      }
      QEmacsPlainTextEdit& t = b->getMainFrame();
      t.setFont(f);
    }
  } // end of QEmacsWidget::changeMainFrameFont

  QEmacsBuffer *
  QEmacsWidget::getBufferVisitingFile(const QString& f)
  {
    QFileInfo fi(f);
    QString af;
    if(fi.isSymLink()){
      af = fi.symLinkTarget();
    } else {
      af = fi.absoluteFilePath();
    }
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b==nullptr){
	continue;
      }
      QFileInfo bfi(b->getBufferRawName());
      QString bf;
      if(bfi.isSymLink()){
	bf = bfi.symLinkTarget();
      } else {
	bf = bfi.absoluteFilePath();
      }
      if(bf==af){
	return b;
      }
    }
    return nullptr;
  } // end of QEmacsWidget::getBufferVisitingFile

  void
  QEmacsWidget::openFile(const QString& f)
  {
    if(f.isEmpty()){
      return;
    }
    QEmacsBuffer *b = this->createNewBuffer(f);
    if(this->buffers->indexOf(b)==-1){
      this->buffers->addWidget(b);
      emit bufferAdded();
    }
    this->setCurrentBuffer(b);
  } // end of QEmacs::openFile(const QString&)

  void
  QEmacsWidget::createEmptyBuffer(void)
  {
    QEmacsBuffer *b = this->createNewBuffer();
    this->buffers->addWidget(b);
    this->setCurrentBuffer(b);
    emit bufferAdded();
  } // end of QEmacs::createEmptyBuffer()

  void
  QEmacsWidget::openFile()
  {
    QEmacsBuffer&   b = this->getCurrentBuffer();
    QEmacsPlainTextEdit& t = b.getMainFrame();
    QFileInfo fn(t.getCompleteFileName());
    QDir d(fn.dir());
    QString path;
    if(d.exists()){
      path = d.absolutePath();
    } else {
      path = QDir::current().absolutePath();
    }
    QEmacsLineEdit *l = new QEmacsWidget::OpenFile(*this,path);
    l->setInputHistorySettingAddress("recent files");
    this->setUserInput(l);
  } // end of QEmacs::openFile()

  QEmacsBuffer *
  QEmacsWidget::createNewBuffer(const QString& f)
  {
    QEmacsBuffer *b;
    if(f.isEmpty()){
      b = new QEmacsBuffer(this->nid,*this);
      ++(this->nid);
    } else {
      b = this->getBufferVisitingFile(f);
      if(b!=nullptr){
	this->displayInformativeMessage(QObject::tr("A buffer is already visiting file '%1'").arg(f));
	return b;
      }
      b = new QEmacsBuffer(f,this->nid,*this);
      ++(this->nid);
      this->emitNewTreatedFile(b->getMainFrame().getCompleteFileName());
    }
    QObject::connect(b,SIGNAL(bufferNameChanged(QEmacsBuffer *,
						const QString&,
						const QString&)),
		     this,SLOT(updateBufferName(QEmacsBuffer *,
						const QString&,
						const QString&)));
    QObject::connect(b,SIGNAL(newTreatedFile(const QString&)),
		     this,SLOT(emitNewTreatedFile(const QString&)));
    return b;
  } // end of QEmacsWidget::createNewBuffer

  void
  QEmacsWidget::removeBuffer(QEmacsBuffer * b)
  {
    QString n = b->getBufferName();
    this->bHistory.removeAll(n);
    this->buffers->removeWidget(b);    
    if(this->buffers->count()==0){
      this->createEmptyBuffer();
    } else {
      this->buffers->currentWidget()->setFocus();
      if(!this->bHistory.isEmpty()){
	this->changeBuffer(this->bHistory.front());
      } else {
	QEmacsBuffer *nb = qobject_cast<QEmacsBuffer *>(this->buffers->currentWidget());
	if(nb!=nullptr){
	  this->setCurrentBuffer(nb);
	}
      }
    }
    emit bufferRemoved();
  } // end of QEmacsWidget::removeBuffer
    
  void
  QEmacsWidget::updateBufferName(QEmacsBuffer * b,
				 const QString& o,
				 const QString& n)
  {
    int p = this->buffers->indexOf(b);
    if(p==-1){
      return;
    }
    for (int i = 0; i < this->bHistory.size(); ++i) {
      if (this->bHistory.at(i) == o){
	this->bHistory[i] = n;
      }
    }
    emit bufferNameChanged();
  }

  void
  QEmacsWidget::saveAllBuffers(void)
  {
    int i;
    for(i=0;i!=this->buffers->count();++i){
      QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b==nullptr){
	continue;
      }
      QEmacsPlainTextEdit& t = b->getMainFrame();
      if(t.document()->isModified()){
	t.save();
      }
    }
  } // end of QEmacsWidget::saveAllBuffers(void)

  bool
  QEmacsWidget::isOkToClose(void)
  {
    int i;
    bool found = false;
    for(i=0;(i!=this->buffers->count())&&(!found);++i){
      QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b==nullptr){
	continue;
      }
      found = !b->isOkToClose();
    }
    if (found){
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this, tr("QEmacs"),
				 tr("At least one buffer has been modified.\n"
				    "Do you want to save your changes?"),
				 QMessageBox::SaveAll | QMessageBox::Discard
				 | QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel){
	return false;
      } else if (ret == QMessageBox::SaveAll){
	this->saveAllBuffers();
      }
    }
    return true;
  } // end of QEmacsWidget::isOkToClose

  void
  QEmacsWidget::closeEvent(QCloseEvent *e)
  {
    if(this->isOkToClose()){
      // for(int i=0;i!=this->buffers->count();++i){
      // 	QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      // 	QCloseEvent cev;
      // 	cev.setAccepted(true);
      // 	b->closeEvent(&cev);
      // 	if(!cev.isAccepted()){
      // 	  e->ignore();
      // 	}
      // }
      e->accept();
      emit closed();
    } else {
      e->ignore();
    }
  } // end of QEmacsWidget::close
  
  void
  QEmacsWidget::displayInformativeMessage(const QString& m)
  {
    this->um->setText(m);
    this->minibuffer->setCurrentWidget(this->um);
    QTimer::singleShot(1000, this,
		       SLOT(resetUserInput()));
  } // end of QEmacsWidget::displayInformativeMessage

  void
  QEmacsWidget::setUserInput(QEmacsLineEdit* const l)
  {
    if(l==nullptr){
      return;
    }
    if(this->ui.contains(l)){
      return;
    }
    if(!this->ui.isEmpty()){
      if(this->ui.back()->isBlocking()){
	this->displayInformativeMessage(QObject::tr("command attemted "
						    "to use minibuffer "
						    "while in minibuffer"));
	return;
      }
    }
    this->ui.append(l);
    setQEmacsLineEditFont(this->ui.back());
    this->minibuffer->addWidget(this->ui.back());
    this->minibuffer->setCurrentWidget(this->ui.back());
    this->ui.back()->setFocus();
    if(this->ui.back()->isBlocking()){
      this->buffers->setEnabled(false);
    }
    QObject::connect(this->ui.back(),SIGNAL(finished(QEmacsLineEdit *)),
		     this,SLOT(removeUserInput(QEmacsLineEdit *)));
    QObject::connect(this->ui.back(),SIGNAL(destroyed(QEmacsLineEdit *)),
		     this,SLOT(removeUserInput(QEmacsLineEdit *)));
  }

  void
  QEmacsWidget::removeUserInput(QEmacsLineEdit *p)
  {
    if(this->ui.isEmpty()){
      return;
    }
    if(!this->ui.contains(p)){
      return;
    }
    if(p->isBlocking()){
      Q_ASSERT(p==this->ui.back());
      this->buffers->setEnabled(true);
    }
    this->minibuffer->removeWidget(p);
    this->ui.remove(this->ui.indexOf(p));
    if(this->ui.isEmpty()){
      this->minibuffer->setCurrentWidget(this->eui);
    } else {
      this->minibuffer->setCurrentWidget(this->ui.back());
    }
    this->buffers->currentWidget()->setFocus();
    p->deleteLater();
  } // end of QEmacsWidget::removeUserInput

  void
  QEmacsWidget::removeUserInput(void)
  {
    if(this->ui.isEmpty()){
      return;
    }
    this->removeUserInput(this->ui.back());
  } // end of QEmacsWidget::removeUserInput

  void
  QEmacsWidget::removeUserInputs(void)
  {
    while(!this->ui.isEmpty()){
      this->removeUserInput(this->ui.back());
    }
  } // end of QEmacsWidget::removeUserInputs

  bool
  QEmacsWidget::hasUserInput(void) const
  {
    return !this->ui.isEmpty();
  }

  void
  QEmacsWidget::focusUserInput(void)
  {
    if(!this->ui.isEmpty()){
      this->ui.back()->setFocus();
    }
  }

  void
  QEmacsWidget::resetUserInput(void)
  {
    if(!this->ui.isEmpty()){
      this->minibuffer->setCurrentWidget(this->ui.back());
    } else {
      this->minibuffer->setCurrentWidget(this->eui);
    }
    this->um->clear();
  } // end of QEmacsWidget::resetPreviousUserInput

  const QStringList&
  QEmacsWidget::getKillRing(void) const
  {
    return this->killRing;
  } // end of QEmacsWidget::getKillRing

  void
  QEmacsWidget::addToKillRing(const QString& t)
  {
    if(t.isEmpty()){
      return;
    }
    if(this->killRing.size()==100){
      this->killRing.pop_front();
    }
    this->killRing << t;
  } // end of QEmacsWidget::addToKillRing

  QStringList
  QEmacsWidget::getBuffersNames(void) const
  {
    QStringList n;
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer const * b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b!=nullptr){
	QString bn = b->getBufferName();
	if(bn.isEmpty()){
	  n.append("* unnamed buffer *");
	} else {
	  n.append(bn);
	}
      }
    }
    return n;
  } // end of QEmacsWidget::getBuffersNames

  QVector<QIcon>
  QEmacsWidget::getBuffersIcons(void) const
  {
    QVector<QIcon> icons;
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer const * b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b!=nullptr){
	icons.append(b->getIcon());
      }
    }
    return icons;
  }

  QVector<int>
  QEmacsWidget::getBuffersIds(void) const
  {
    QVector<int> ids;
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer const * b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b!=nullptr){
	ids.append(b->getId());
      }
    }
    return ids;
  }

  void
  QEmacsWidget::changeBuffer(const QString& n)
  {
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer * b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b!=nullptr){
	if(b->getBufferName()==n){
	  this->setCurrentBuffer(b);
	  return;
	}
      }
    }
  } // end of QEmacsWidget::changeBuffer

  void
  QEmacsWidget::changeBuffer(const int id)
  {
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer * b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b!=nullptr){
	if(b->getId()==id){
	  this->setCurrentBuffer(b);
	  return;
	}
      }
    }
  } // end of QEmacsWidget::changeBuffer

  QString
  QEmacsWidget::chooseBufferNameSuffix(QEmacsBuffer * b,
				       const QString& f)
  {
    if(f.isEmpty()){
      return "";
    }
    QFileInfo info(f);
    QString n  = info.fileName();
    QString s;
    int m = 0;
    bool found = true;
    while(found){
      found = false;
      for(int i=0;(i!=this->buffers->count())&&(!found);++i){
	QEmacsBuffer * ob = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
	if(ob!=nullptr){
	  if(ob!=b){
	    found = ((n == ob->getBufferName()) &&
		     (s == ob->getBufferNameSuffix()));
	  }
	}
      }
      if(found){
	++m;
	s = QString::number(m);
      }
    }
    return s;
  } // end of QEmacsWidget::chooseBufferName

  void
  QEmacsWidget::changeBuffer()
  {
    QEmacsBuffer *cb = qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    if(cb==nullptr){
      return;
    }
    const QString& bn = cb->getBufferName();
    QStringList bnames;
    for(int i=0;i!=this->buffers->count();++i){
      QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->widget(i));
      if(b!=nullptr){
	const QString& n = b->getBufferName();
	if(bn!=n){
	  bnames.append(n);
	}
      }
    }
    if(bnames.size()==0){
      this->displayInformativeMessage(QObject::tr("no other buffer"));
      return;
    }
    if(bnames.size()==1){
      this->changeBuffer(bnames[0]);
      return;
    }
    if(this->bHistory.size()>=2){
      this->setUserInput(new QEmacsWidget::ChangeBuffer(*this,bnames,this->bHistory[1]));
    } else {
      this->setUserInput(new QEmacsWidget::ChangeBuffer(*this,bnames,""));
    }
  } // end of QEmacsWidget::changeBuffer

  void
  QEmacsWidget::setCurrentBuffer(QEmacsBuffer * const b)
  {
    this->buffers->setCurrentWidget(b);
    b->setFocus();
    QString n = b->getBufferName();
    this->bHistory.removeAll(n);
    this->bHistory.push_front(n);
    emit currentBufferChanged();
  } // end of QEmacsWidget::setCurrentBuffer

  struct QEmacsWidget::SaveCurrentBufferAndKill
    : public QEmacsYesOrNoUserInput
  {
    SaveCurrentBufferAndKill(QEmacsWidget& p)
      : QEmacsYesOrNoUserInput(QObject::tr("buffer '%1' has been modified; kill anyway ?").arg(p.getCurrentBuffer().getBufferName()),p),
	t(p)
    {}
    virtual bool
    isBlocking(void) const override
    {
      return true;
    } // end of isBlocking
  protected:
    virtual void
    treatUserInput() override
    {
      if(this->input->text()=="y"){
	this->t.removeBuffer(&(this->buffer));
      }
    }// end of treatUserInput
    QEmacsWidget& t;
  };

  QVector<QMenu*>
  QEmacsWidget::getCurrentBufferSpecificMenus()
  {
    if(this->buffers->count()==0){
      return QVector<QMenu*>();
    }
    QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    return b->getSpecificMenus();
  } // end of QEmacsWidget::getCurrentBufferMenu()

  void
  QEmacsWidget::emitNewTreatedFile(const QString& f)
  {
    emit newTreatedFile(f);
  } // end of QEmacsWidget::emitNewTreatedFile

  void
  QEmacsWidget::launchCommand(void)
  {
    if(!this->ui.isEmpty()){
      this->displayInformativeMessage(QObject::tr("command attemted "
						  "to use minibuffer "
						  "while in minibuffer"));
    }
    QEmacsLineEdit * l = new QEmacsWidget::Command(*this);
    l->setInputHistorySettingAddress("command/history");
    this->setUserInput(l);
  } // end of QEmacsWidget::launchCommand

  void
  QEmacsWidget::launchCommand(const QString& c)
  {
    QEmacsCommandFactory& f = QEmacsCommandFactory::getQEmacsCommandFactory();
    QEmacsCommand *qc = f.getQEmacsCommand(c,*this);
    if(qc==nullptr){
      this->displayInformativeMessage(QObject::tr("unknown command '%1'").arg(c));
      return;
    }
    qc->execute();
    delete qc;
  } // end of QEmacsWidget::launchCommand

  QEmacsBuffer&
  QEmacsWidget::getCurrentBuffer(void)
  {
    QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    return *b;    
  } // end of QEmacsWidget::getCurrentBuffer

  void
  QEmacsWidget::closeBuffer(QEmacsBuffer& buffer,
			    const bool b)
  {
    if(b){
#pragma message("unimplemented")
      // if(!buffer.isOkToClose()){
      // 	this->setUserInput(new QEmacsWidget::SaveBufferAndKill(buffer,*this));
      // } else {
      this->removeBuffer(&buffer);
      // }
    } else {
      this->removeBuffer(&buffer);
    }
  } // end of QEmacsWidget::closeCurrentBuffer
  
  void
  QEmacsWidget::closeCurrentBuffer(const bool b)
  {
    QEmacsBuffer *buffer = qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    if(buffer==nullptr){
      return;
    }
    if(b){
      if(!buffer->isOkToClose()){
	this->setUserInput(new QEmacsWidget::SaveCurrentBufferAndKill(*this));
      } else {
	this->removeBuffer(buffer);
      }
    } else {
      this->removeBuffer(buffer);
    }
  } // end of QEmacsWidget::closeCurrentBuffer

  void
  QEmacsWidget::saveCurrentBuffer()
  {
    QEmacsBuffer *b = qobject_cast<QEmacsBuffer*>(this->buffers->currentWidget());
    if(b==nullptr){
      return;
    }
    QEmacsPlainTextEdit& t = b->getMainFrame();
    if(t.document()->isModified()){
      t.save();
    }
  } // end of QEmacsWidget::closeCurrentBuffer

  QEmacsWidget::~QEmacsWidget()
  {
    this->removeUserInputs();
    delete this->buffers;
    delete this->minibuffer;
  }

} // end of namespace qemacs



