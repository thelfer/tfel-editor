/*! 
 * \file  MTestMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 août 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QFileInfo>
#include<QtGui/QTextCursor>
                
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"
#include"QEmacs/MTestStudyOptions.hxx"
#include"QEmacs/MTestSyntaxHighlighter.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

#include"QEmacs/ImportBehaviour.hxx"
#include"QEmacs/MTestMajorMode.hxx"

namespace qemacs
{
  
  MTestMajorMode::MTestMajorMode(QEmacsWidget& w,
				 QEmacsBuffer& b,
				 QEmacsTextEditBase& t)
    : CxxMajorMode(w,b,t)
  {
    QStringList keys;
    for(const auto& k : this->getKeyWordsList()){
      keys << QString::fromStdString(k);
    }
    this->c = new QCompleter(keys,&t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,static_cast<void (QCompleter:: *)(const QString&)>(&QCompleter::activated),
		     &t,&QEmacsTextEditBase::insertCompletion);
    // creating actions
    this->ra = new QAction(QObject::tr("Run mtest"),this);
    QObject::connect(this->ra,&QAction::triggered,
		     this,&MTestMajorMode::run);
    this->iba  = new QAction(QObject::tr("Import Behaviour"),this);
    QObject::connect(this->iba,&QAction::triggered,
		     this,&MTestMajorMode::showImportBehaviourWizard);
  } // end of MTestMajorMode::MTestMajorMode
  
  QString MTestMajorMode::getName() const{
    return "MTest";
  } // end of MTestMajorMode::getName
    
  QString MTestMajorMode::getDescription() const{
    return "major mode dedicated to the mtest code";
  } // end of CppMajorMode

  bool MTestMajorMode::keyPressEvent(QKeyEvent * const e)
  {
    const auto k = e->key();
    const auto m = e->modifiers(); 
    if((m==Qt::AltModifier)&&(k==Qt::Key_M)){
      this->run();
      return true;
    }
    return false;
  } // end of MTestMajorMode::keyPressEvent
  
  QMenu* MTestMajorMode::getSpecificMenu()
  {
    auto *t = qobject_cast<QWidget *>(this->parent());
    if(t==nullptr){
      return nullptr;
    }
    auto  *m = new QMenu(this->getName(),t);
    m->addAction(this->ra);
    m->addSeparator();
    m->addAction(this->iba);
    m->addSeparator();
    auto *km = m->addMenu(QObject::tr("Keywords"));
    auto keys = this->getKeyWordsList();
    std::sort(keys.begin(),keys.end());
    for(const auto& k : keys){
      const auto qk = QString::fromStdString(k);
      auto * ka = new QAction(qk,this);
      ka->setData(qk);
      km->addAction(ka);
    }
    QObject::connect(km,&QMenu::triggered,
		     this,&MTestMajorMode::insertKeyword);
    return m;
  }

  void MTestMajorMode::insertKeyword(QAction *a){
    const auto k = a->data().toString();
    if(k.isEmpty()){
      return;
    }
    auto tc = this->textEdit.textCursor();
    tc.insertText(k+' ');
    this->textEdit.setTextCursor(tc);
  }
  
  void MTestMajorMode::setSyntaxHighlighter(QTextDocument *const d)
  {
    new MTestSyntaxHighlighter(d);
  } // end of MTestMajorMode::setSyntaxHighlighter
    
  void MTestMajorMode::completeContextMenu(QMenu *const m,
					   const QTextCursor& tc)
  {
    const auto& keys = this->getKeyWordsList();
    QEmacsMajorModeBase::completeContextMenu(m,tc);
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock,
		   QTextCursor::MoveAnchor);
    b.select(QTextCursor::LineUnderCursor);
    const auto l = b.selectedText();
    QRegExp r("^(@\\w+)");
    if(r.indexIn(l)>=0){
      const auto k = r.cap(1);
      if(std::find(keys.begin(),keys.end(),k.toStdString())!=keys.end()){
	delete this->ha;
	this->ha=new QAction(QObject::tr("Help on %1").arg(k),this);
	this->ha->setData(k);
	const auto cactions = m->actions();	
	if(cactions.isEmpty()){
	  m->addAction(this->ha);
	} else {
	  m->insertSeparator(*(cactions.begin()));
	  m->insertAction(*(cactions.begin()),this->ha);
	}
	QObject::connect(m,&QMenu::triggered,
			 this,&MTestMajorMode::actionTriggered);
      }
    }
  }

  const std::vector<std::string>&
  MTestMajorMode::getKeyWordsList() const{
    return MTestSyntaxHighlighter::getMTestKeys();
  }

  QString MTestMajorMode::getScheme() const
  {
    return "mtest";
  }
  
  void MTestMajorMode::actionTriggered(QAction *a)
  {
    if(a==this->ha){
      const auto k = this->ha->data().toString();
      auto nf = new ProcessOutputFrame(this->qemacs,this->buffer);
      this->buffer.addSlave(QObject::tr("help on '%1'").arg(k),nf);
      auto& p = nf->getProcess();
      if(p.state()!=QProcess::Running){
	p.start("mtest",QStringList() << ("--scheme="+this->getScheme())
		<< ("--help-keyword="+k));
	p.waitForStarted();
	p.waitForFinished(1000);	
      }
      nf->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
    }
  }

  QCompleter* MTestMajorMode::getCompleter()
  {
    return this->c;
  } // end of getCompleter

  QString MTestMajorMode::getCompletionPrefix()
  {
    auto tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::StartOfWord,
		    QTextCursor::MoveAnchor);
    if(!tc.atBlockStart()){
      tc.movePosition(QTextCursor::PreviousCharacter,
		      QTextCursor::KeepAnchor);
      if(tc.selectedText()=="@"){
	return '@'+this->textEdit.getCurrentWord();
      }
    }
    return CxxMajorMode::getCompletionPrefix();
  }
  
  void MTestMajorMode::run()
  {
    if(this->textEdit.isModified()){
      QEmacsTextEditBase::SaveInput *input = this->textEdit.getSaveInput();
      QObject::connect(input,&QEmacsTextEditBase::SaveInput::finished,
		       this,&MTestMajorMode::start);
      this->qemacs.setUserInput(input);
      return;
    }
    this->start();
  }

  void MTestMajorMode::start()
  {
    QString n = this->textEdit.getCompleteFileName();
    if(n.isEmpty()){
      this->qemacs.displayInformativeMessage(QObject::tr("no file name"));
      return;
    }
    MTestStudyOptions o;
    MTestStudyOptionsDialog od(o,&(this->textEdit));
    if(od.exec() == QDialog::Rejected) {
      return;
    }
    const QString& af = QFileInfo(n).absoluteFilePath ();
    auto nf = new ProcessOutputFrame(this->qemacs,this->buffer);
    this->buffer.addSlave(QObject::tr("MTest output"),nf);
    auto& p = nf->getProcess();
    if(p.state()!=QProcess::Running){
      auto arg = QStringList{};
      arg << ("--scheme="+this->getScheme())
	  << ("--verbose="+o.vlvl);
      if(!o.res){
	arg << "--result-file-output=no";
      }
      if(o.xml){
	arg << "--xml-output=true";
      }
      arg << af;
      p.start("mtest",arg);
      p.waitForStarted();
      //      p.waitForFinished(1000);	
    }
  } // end of MTestMajorMode::start

  void MTestMajorMode::showImportBehaviourWizard()
  {
    ImportBehaviour w(this->textEdit);
    if(w.exec()==QDialog::Accepted){
      
    }
  } // end of MTestMajorMode::showImportBehaviourWizard
  
  MTestMajorMode::~MTestMajorMode() = default;
  
  static StandardQEmacsMajorModeProxy<MTestMajorMode> proxy("MTest",
							     QVector<QRegExp>() << QRegExp("^[\\w-]+\\.mtest"));

} // end of namespace qemacs

