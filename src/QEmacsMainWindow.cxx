/*! 
 * \file  QEmacsMainWindow.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 juin 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QSettings>
#include<QtWidgets/QAction>
#include<QtWidgets/QMenuBar>
#include<QtWidgets/QFileDialog>
#include<QtWidgets/QMessageBox>
#include<QtWidgets/QFontDialog>
#include<QtWidgets/QApplication>
#include<QtPrintSupport/QPrinter>
#include<QtPrintSupport/QPrintDialog>
#include<QtGui/QCloseEvent>
#include"QEmacs/QEmacsHunspellDictionariesManager.hxx"
#include"QEmacs/QEmacsShortCutStyle.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"
#include"QEmacs/QEmacsMainWindow.hxx"

namespace qemacs
{
  
  QEmacsMainWindow::QEmacsMainWindow(const QStringList& f,
				     QWidget *const p)
    : QMainWindow(p)
  {
    auto& s = QEmacsShortCutStyle::getQEmacsShortCutStyle();
    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setStyleSheet("background: rgba(255,255,255,100%)");
    auto* e = new QEmacsWidget(this);
    this->setCentralWidget(e);
    this->createActions();
    this->createMainMenu();
    QObject::connect(&s,&QEmacsShortCutStyle::shortCutStyleChanged,
		     this,&QEmacsMainWindow::updateOptionsMenu);
    QObject::connect(e,&QEmacsWidget::closed,
		     this,&QEmacsMainWindow::close);
    QObject::connect(e,&QEmacsWidget::bufferNameChanged,
		     this,&QEmacsMainWindow::updateBuffersMenu);
    QObject::connect(e,&QEmacsWidget::bufferAdded,
		     this,&QEmacsMainWindow::updateBuffersMenu);
    QObject::connect(e,&QEmacsWidget::bufferRemoved,
		     this,&QEmacsMainWindow::updateBuffersMenu);
    QObject::connect(e,&QEmacsWidget::currentBufferChanged,
		     this,&QEmacsMainWindow::createMainMenu);
    QObject::connect(e,&QEmacsWidget::newTreatedFile,
		     this,&QEmacsMainWindow::addToRecentFiles);
    if(f.empty()){
      e->createEmptyBuffer();
    } else {
      QStringList::const_iterator pf;
      for(pf=f.begin();pf!=f.end();++pf){
	e->openFile(*pf);
      }
    }
  }

  void QEmacsMainWindow::addToRecentFiles(const QString& f)
  {
    QSettings s;
    QStringList rf = s.value("recent files").toStringList();
    int p = 0;
    // remove previous appearance of the file
    p = rf.indexOf(f);
    while(p!=-1){
      rf.removeAt(p);
      p=rf.indexOf(f);
    }
    rf.push_back(f);
    if(rf.size()>=50){
      const auto file = rf.front();
      if(s.contains("positions in files")){
	auto pl = s.value("positions in files").value<QMap<QString, QVariant>>();
	if(pl.contains(file)){
	  pl.remove(file);
	  s.setValue("positions in files",pl);
	}
      }
      rf.pop_front();
    }
    s.setValue("recent files",rf);
  }

  void QEmacsMainWindow::closeEvent(QCloseEvent *ev)
  {
    auto* e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QObject::disconnect(e,&QEmacsWidget::closed,
			  this,&QEmacsMainWindow::close);
      if(e->close()){
	ev->accept();
      } else {
	QObject::connect(e,&QEmacsWidget::closed,
			 this,&QEmacsMainWindow::close);
	ev->ignore();
      }
    }
  } // end of QEmacsMainWindow::close()

  void QEmacsMainWindow::openFile()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    const auto& f = QFileDialog::getOpenFileName(this,tr("Open File"));
    if(f.isEmpty()){
      return;
    }
    e->openFile(f);
  } // end of QEmacsMainWindow::openFile

  void QEmacsMainWindow::selectFont()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      bool ok;
      QFont f = QFontDialog::getFont(&ok, this);
      f.setStyleStrategy(QFont::PreferAntialias);
      if(ok){
	QSettings s;
	s.setValue("font/textedit",f);
	e->changeMainFramesFont(f);
      }
    }
  } // end of QEmacsMainWindow::selectFont

  void QEmacsMainWindow::createActions()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    this->na = new QAction(tr("N&ew"), this);
    this->na->setStatusTip(tr("New buffer"));
    this->na->setIcon(QIcon::fromTheme("document-new"));
    this->na->setIconVisibleInMenu(true);
    QObject::connect(this->na,&QAction::triggered,
		     e,&QEmacsWidget::createEmptyBuffer);
    this->oa = new QAction(tr("O&pen"), this);
    this->oa->setStatusTip(tr("Open a file"));
    this->oa->setIcon(QIcon::fromTheme("document-open"));
    this->oa->setIconVisibleInMenu(true);
    QObject::connect(this->oa,&QAction::triggered,
		     this,&QEmacsMainWindow::openFile);
    this->sa = new QAction(tr("S&ave current buffer"), this);
    this->sa->setStatusTip(tr("Save the current buffer"));
    this->sa->setIcon(QIcon::fromTheme("document-save"));
    this->sa->setIconVisibleInMenu(true);
    QObject::connect(this->sa,&QAction::triggered,
		     e,&QEmacsWidget::saveCurrentBuffer);
    this->ka = new QAction(tr("C&lose current buffer"), this);
    this->ka->setStatusTip(tr("Close the current buffer"));
    QObject::connect(this->ka,&QAction::triggered,
		     e,&QEmacsWidget::closeCurrentBuffer);
    this->pra = new QAction(tr("Print current buffer"), this);
    this->pra->setIcon(QIcon::fromTheme("document-print"));
    this->pra->setIconVisibleInMenu(true);
    this->pra->setStatusTip(tr("Print the current buffer"));
    QObject::connect(this->pra,&QAction::triggered,
		     this,&QEmacsMainWindow::print);
    this->saa = new QAction(tr("S&ave all buffers"), this);
    this->saa->setStatusTip(tr("Save all buffers"));
    QObject::connect(this->saa,&QAction::triggered,
		     e,&QEmacsWidget::saveAllBuffers);
    // edit actions
    this->ua  = new QAction(tr("U&ndo"), this);
    this->ua->setIcon(QIcon::fromTheme("edit-undo"));
    this->ua->setIconVisibleInMenu(true);
    QObject::connect(this->ua,&QAction::triggered,
		     this,&QEmacsMainWindow::undo);
    this->ra  = new QAction(tr("R&edo"), this);
    this->ra->setIcon(QIcon::fromTheme("edit-redo"));
    this->ra->setIconVisibleInMenu(true);
    QObject::connect(this->ra,&QAction::triggered,
		     this,&QEmacsMainWindow::redo);
    this->sea  = new QAction(tr("S&elect All"), this);
    this->sea->setIcon(QIcon::fromTheme("edit-select-all"));
    this->sea->setIconVisibleInMenu(true);
    QObject::connect(this->sea,&QAction::triggered,
		     this,&QEmacsMainWindow::selectAll);
    this->ca  = new QAction(tr("C&ut"), this);
    this->ca->setStatusTip(tr("Cut the selected text"));
    this->ca->setIcon(QIcon::fromTheme("edit-cut"));
    this->ca->setIconVisibleInMenu(true);
    QObject::connect(this->ca,&QAction::triggered,
		     this,&QEmacsMainWindow::cut);
    this->ca2  = new QAction(tr("C&opy"), this);
    this->ca2->setStatusTip(tr("Copy the selected text to clipboard"));
    this->ca2->setIcon(QIcon::fromTheme("edit-copy"));
    this->ca2->setIconVisibleInMenu(true);
    QObject::connect(this->ca2,&QAction::triggered,
		     this,&QEmacsMainWindow::copy);
    this->pa  = new QAction(tr("P&aste"), this);
    this->pa->setStatusTip(tr("Paste from clipboard"));
    this->pa->setIcon(QIcon::fromTheme("edit-paste"));
    this->pa->setIconVisibleInMenu(true);
    QObject::connect(this->pa,&QAction::triggered,
		     this,&QEmacsMainWindow::paste);
    // exit actions
    this->ea = new QAction(tr("E&xit"), this);
    this->ea->setStatusTip(tr("Exit qemacs"));
    this->ea->setIcon(QIcon::fromTheme("window-close"));
    this->ea->setIconVisibleInMenu(true);
    QObject::connect(this->ea,&QAction::triggered,
		     this,&QEmacsMainWindow::close);
    // font actions
    this->fa = new QAction(tr("Main Frames' F&ont"), this);
    this->fa->setStatusTip(tr("Select Font"));
    QObject::connect(this->fa,&QAction::triggered,
		     this,&QEmacsMainWindow::selectFont);
    this->esca = new QAction(tr("Change to emacs's style shortcuts"), this);
    this->esca->setStatusTip(tr("Use emacs's style shortcuts"));
    QObject::connect(this->esca,&QAction::triggered,
		     this,&QEmacsMainWindow::useEmacsShortCuts);
    this->qsca = new QAction(tr("Change to Qt's style shortcuts"), this);
    this->qsca->setStatusTip(tr("Use Qt's style shortcuts"));
    QObject::connect(this->qsca,&QAction::triggered,
		     this,&QEmacsMainWindow::useQtShortCuts);
    // help actions
    this->aa = new QAction(tr("&About"), this);
    this->aa->setStatusTip(tr("Show the application's About box"));
    this->aa->setIcon(QIcon::fromTheme("help-about"));
    this->aa->setIconVisibleInMenu(true);
    connect(this->aa,&QAction::triggered,
	    this,&QEmacsMainWindow::about);
    this->aa2 = new QAction(tr("About &Qt"), this);
    this->aa2->setStatusTip(tr("Show the Qt library's About box"));
    connect(this->aa2,&QAction::triggered,
	    qApp,&QApplication::aboutQt);
  } // end of QEmacsMainWindow::createActions

  void QEmacsMainWindow::updateOptionsMenu()
  {
    typedef QEmacsHunspellDictionariesManager HunspellDictionaries;
    auto& s = QEmacsShortCutStyle::getQEmacsShortCutStyle();
    this->om->clear();
    // font
    this->om->addAction(this->fa);
    // available dictionaries
    auto& dm = HunspellDictionaries::getQEmacsHunspellDictionariesManager();
    QStringList dicts = dm.getAvailableDictionnaries();
    this->changeSpellCheckLanguageActions.clear();
    if(!dicts.isEmpty()){
      QMenu *d = this->om->addMenu(QObject::tr("Dictionaries"));
      QStringList::const_iterator pd;
      for(pd=dicts.begin();pd!=dicts.end();++pd){
	QAction *a = d->addAction(*pd);
	a->setData(*pd);
	this->changeSpellCheckLanguageActions.push_back(a);
      }
      QObject::connect(d,&QMenu::triggered,
		       this,&QEmacsMainWindow::spellCheckLanguageActionTriggered);
    }
    // short cuts
    if(s.getStyle()!=QEmacsShortCutStyle::EMACS){
      this->om->addAction(this->esca);
    }
    if(s.getStyle()!=QEmacsShortCutStyle::QT){
      this->om->addAction(this->qsca);
    }
  } // end of QEmacsMainWindow::updateOptionsMenu

  void
  QEmacsMainWindow::spellCheckLanguageActionTriggered(QAction* a)
  {
    if(this->changeSpellCheckLanguageActions.indexOf(a)!=-1){
      QEmacsWidget *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
      if(e!=nullptr){
	QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
	t.setSpellCheckLanguage(a->data().toString());
      }
    }
  } // end of QEmacsMainWindow::spellCheckLanguageActionTriggered


  void QEmacsMainWindow::updateBuffersMenu()
  {
    this->bm->clear();
    QEmacsWidget *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      const auto& bnames = e->getBuffersNames();
      const auto& bicons = e->getBuffersIcons();
      const auto& bids   = e->getBuffersIds();
      if((bnames.size()!=bicons.size())||
	 (bnames.size()!=bids.size())){
	return;
      }
      for(int i=0;i!=bicons.size();++i){
	const QString& n = bnames[i];
	const QIcon&   ic = bicons[i];
	const int      id = bids[i];
	QAction * a = this->bm->addAction(n);
	if(!ic.isNull()){
	  a->setIcon(ic);
	  a->setIconVisibleInMenu(true);
	}
	a->setData(id);
      }
    }
  } // end of QEmacsMainWindow::updateBuffersMenu

  void QEmacsMainWindow::bufferMenuActionTriggered(QAction * a)
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      int id = a->data().toInt();
      e->changeBuffer(id);
    }
  }
  
  void QEmacsMainWindow::useEmacsShortCuts()
  {
    auto& s = QEmacsShortCutStyle::getQEmacsShortCutStyle();
    s.setStyle(QEmacsShortCutStyle::EMACS);
    this->updateOptionsMenu();
  } // end of QEmacsMainWindow::useEmacsShortCuts

  void QEmacsMainWindow::useQtShortCuts()
  {
    auto& s = QEmacsShortCutStyle::getQEmacsShortCutStyle();
    s.setStyle(QEmacsShortCutStyle::QT);
    this->updateOptionsMenu();
  } // end of QEmacsMainWindow::useQtShortCuts

  void QEmacsMainWindow::createMainMenu()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    QSettings settings;
    this->menuBar()->clear();
    // this->menuBar()->setStyleSheet("background: rgba(0,0,0,100%)");
    this->fm = this->menuBar()->addMenu(tr("File"));
    this->fm->addAction(this->na);
    this->fm->addAction(this->oa);
    if(settings.contains("recent files")){
      QStringList files = settings.value("recent files").toStringList();
      QMenu *rfm = this->fm->addMenu(QIcon::fromTheme("document-open-recent"),
				     QObject::tr("&Recent Files"));
      //      rfm->setIconVisibleInMenu(true);
      this->createRecentFilesMenu(rfm,files);
    }
    this->fm->addAction(this->sa);
    this->fm->addAction(this->ka);
    this->fm->addSeparator();
    this->fm->addAction(this->pra);
    this->fm->addSeparator();
    this->fm->addAction(this->saa);
    this->fm->addSeparator();
    this->fm->addAction(this->ea);
    // edit menu
    this->em = this->menuBar()->addMenu(tr("Edit"));
    this->em->addAction(this->ua);
    this->em->addAction(this->ra);
    this->em->addSeparator();
    this->em->addAction(this->ca);
    this->em->addAction(this->ca2);
    this->em->addAction(this->pa);
    this->em->addAction(this->sea);
    // buffer menu
    this->bm = this->menuBar()->addMenu(tr("Buffers"));
    this->updateBuffersMenu();
    QObject::connect(this->bm,&QMenu::triggered,
		     this,&QEmacsMainWindow::bufferMenuActionTriggered);
    QVector<QMenu *> m = e->getCurrentBufferSpecificMenus();
    QVector<QMenu *>::const_iterator p;
    for(p=m.begin();p!=m.end();++p){
      this->menuBar()->addMenu(*p);
    }
    // options
    this->om = this->menuBar()->addMenu(tr("Options"));
    this->updateOptionsMenu();
    // help menu
    this->hm = this->menuBar()->addMenu(tr("Help"));
    this->hm->addAction(this->aa);
    this->hm->addAction(this->aa2);    
  } // end of QEmacsMainWindow::createMainMenu

  void QEmacsMainWindow::createRecentFilesMenu(QMenu * const m,
					       const QStringList& files)
  {
    int n = 0;
    int s = files.size();
    if(s==0){
      return;
    }
    for(int i = s-1;(i>=0)&&(n!=5);--i,++n){
      const QString& f = files[i];
      QFileInfo fi(f);
      if((fi.exists())&&(fi.isFile())&&(fi.isReadable ())){
	QAction *rf = m->addAction(fi.fileName());
	rf->setData(fi.absoluteFilePath());
      }
    }
    QObject::connect(m,&QMenu::triggered,
		     this,&QEmacsMainWindow::openRecentFileActionTriggered);
  } // end of QEmacsMainWindow::createRecentFilesMenu
  
  void QEmacsMainWindow::openRecentFileActionTriggered(QAction *a)
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      e->openFile(a->data().toString());
    }
  } // end of QEmacsMainWindow::openRecentFileActionTriggered

  void QEmacsMainWindow::about()
  {
    QMessageBox::about(this, tr("About QEmacs"),
		       tr("The <b>QEmacs</b> is a simple "
			  "qt based text editor."));
  } // end of QEmacsMainWindow::about()

  void QEmacsMainWindow::cut()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
      t.cut();
    }
  } // end of QEmacsMainWindow::cut

  void QEmacsMainWindow::undo()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
      t.undo();
    }
  } // end of QEmacsMainWindow::undo

  void QEmacsMainWindow::redo()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
      t.redo();
    }
  } // end of QEmacsMainWindow::redo

  void QEmacsMainWindow::selectAll()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
      t.selectAll();
    }
  } // end of QEmacsMainWindow::selectAll

  void QEmacsMainWindow::copy()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
      t.copy();
    }
  } // end of QEmacsMainWindow::copy

  void QEmacsMainWindow::paste()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
      t.paste();
    }
  } // end of QEmacsMainWindow::paste

  void QEmacsMainWindow::print()
  {
    auto *e = qobject_cast<QEmacsWidget *>(this->centralWidget());
    if(e!=nullptr){
      QPrinter printer;
      QPrintDialog printDialog(&printer,this);
      if (printDialog.exec() == QDialog::Accepted) {
	QEmacsPlainTextEdit& t = e->getCurrentBuffer().getMainFrame();
	t.print(&printer);
      }
    }    
  }

  QSize QEmacsMainWindow::sizeHint() const
  {
    return QSize(800,600);
  } // end of QEmacsMainWindow::sizeHint()
  
} // end of namespace qemacs

