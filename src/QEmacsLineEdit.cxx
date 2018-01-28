/*! 
 * \file  QEmacsLineEdit.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 juil. 2012
 */

#include<QtCore/QDebug>
#include<QtCore/QSettings>
#include<QtGui/QKeyEvent>
#include<QtWidgets/QDirModel>
#include<QtWidgets/QCompleter>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QAbstractItemView>

#include"QEmacs/Utilities.hxx"
#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"
#include"QEmacs/QEmacsTextEdit.hxx"
#include"QEmacs/QEmacsLineEdit.hxx"

namespace qemacs
{

  QEmacsLineEdit::CustomLineEdit::CustomLineEdit(QEmacsWidget& q,
						 QEmacsLineEdit& p)
    : QLineEdit(&p),
      qemacs(q),
      lineEdit(p),
      c_(nullptr),
      completerHandledByQLineEdit(false),
      pring(0),
      ctrlx(false),
      ctrlc(false),
      yank(false),
      pHistory(0),
      hMove(false)
  {}

  QEmacsLineEdit::CustomLineEdit::~CustomLineEdit()
  {
    if(!this->completerHandledByQLineEdit){
      delete this->c_;
    }
  } // end of QEmacsLineEdit::CustomLineEdit::~CustomLineEdit()

  void
  QEmacsLineEdit::CustomLineEdit::setCompleter(QCompleter * const c,
					       const bool b)
  {
    if(c==nullptr){
      return;
    }
    if(!this->completerHandledByQLineEdit){
      delete this->c_;
    }
    this->c_ = c;
    if(b){
      QLineEdit::setCompleter(c);
      this->completerHandledByQLineEdit = true;
    } else {
      this->completerHandledByQLineEdit = false;
    }
  } // end of QEmacsLineEdit::CustomLineEdit::setCompleter

  QCompleter*
  QEmacsLineEdit::CustomLineEdit::completer() const
  {
    return this->c_;
  } // end of QEmacsLineEdit::CustomLineEdit::completer
  
  void
  QEmacsLineEdit::setInputHistory(const QStringList& h)
  {
    if(this->input!=nullptr){
      this->input->setInputHistory(h);
    }
  }

  void
  QEmacsLineEdit::setInputHistorySettingAddress(const QString& a)
  {
    this->inputHistorySettingAddress = a;
    QSettings s;    
    this->setInputHistory(s.value(a).toStringList());
  }

  bool QEmacsLineEdit::CustomLineEdit::event(QEvent *ev)
  {
    if(ev->type()==QEvent::KeyPress){
      auto *kev = static_cast<QKeyEvent *>(ev);
      if(kev->key()==Qt::Key_Tab){
	auto *c = this->completer();
	if(c!=nullptr){
	  this->complete();
	}
	ev->accept();
	return true;
      }
    }
    return QLineEdit::event(ev);
  } // end of QEmacsLineEdit::CustomLineEdit::event

  void
  QEmacsLineEdit::CustomLineEdit::complete()
  {
    bool b;
    QString c = this->findCompletion(b);
    if(!c.isEmpty()){
      this->setText(c);
    }
  }

  QString
  QEmacsLineEdit::CustomLineEdit::findCompletion(bool& b)
  {
    QCompleter *c = this->completer();
    QString r;
    b = true;
    if(c!=nullptr){
      QStringList cc;
      c->setCompletionPrefix(this->text());
      b = c->completionCount() == 1;
      for (int i = 0; c->setCurrentRow(i); i++){
	cc.append(c->currentCompletion());
      }
      r = commonPart(cc);
      QString base = this->extractBaseForCompletion(r);
      if((r==this->text())&&(cc.size()!=1)&&(!cc.empty())){
	// show completions
	int s = base.size();
	for(int i = 0;i!=cc.size();++i){
	  cc[i] = cc[i].mid(s);
	}
	this->lineEdit.showCompletions(base,cc);
      }
    }
    return r;
  } // end of QEmacsLineEdit::CustomLineEdit::findCompletion

  QString
  QEmacsLineEdit::CustomLineEdit::extractBaseForCompletion(const QString& r)
  {
    return r;//    return "";
  } // end of QEmacsLineEdit::CustomLineEdit::extractBaseForCompletion

  void
  QEmacsLineEdit::CustomLineEdit::handleShortCut(const int k1,
						 const Qt::KeyboardModifiers m,
						 const int k2)
  {
    if((k1==Qt::Key_X)&&(m==Qt::NoModifier)&&(k2==Qt::Key_O)){
      auto qle = qobject_cast<QEmacsLineEdit *>(this->parent());
      if(qle!=nullptr){
	if(!qle->isBlocking()){
	  this->qemacs.getCurrentBuffer().focusMainFrame();
	}
      }
      return;
    }
    this->qemacs.displayInformativeMessage(QObject::tr("unknown shortcut"));
  }


  void
  QEmacsLineEdit::CustomLineEdit::keyPressEvent(QKeyEvent *ev)
  {
    const int k                   = ev->key();
    const Qt::KeyboardModifiers m = ev->modifiers(); 
    const bool ctrl = m == Qt::ControlModifier;
    const bool alt  = m == Qt::AltModifier;
    if(!(((ctrl)&&(k==Qt::Key_P))||
	 ((ctrl)&&(k==Qt::Key_N))||
	 ((m==Qt::NoModifier)&&(k==Qt::Key_Up))||
	 ((m==Qt::NoModifier)&&(k==Qt::Key_Down)))){
      this->hMove = false;
    }
    if(!(((ctrl)&&(k==Qt::Key_Y))||
	 ((alt) &&(k==Qt::Key_Y)))){
      this->yank = false;
    }
    if(((ctrl)&&(k==Qt::Key_X))&&(!this->ctrlx)&&(!this->ctrlc)){
      this->ctrlx = true;
      return;
    }
    if(((ctrl)&&(k==Qt::Key_C))&&(!this->ctrlx)&&(!this->ctrlc)){
      this->ctrlc = true;
      return;
    }
    if(this->ctrlx){
      this->handleShortCut(Qt::Key_X,ev->modifiers(),k);
      this->ctrlx = false;
      return;
    }
    if(this->ctrlc){
      this->handleShortCut(Qt::Key_C,ev->modifiers(),k);
      this->ctrlc = false;
      return;
    }
    if((ctrl)&&(k==Qt::Key_G)){
      auto qle = qobject_cast<QEmacsLineEdit *>(this->parent());
      if(qle!=nullptr){
	qle->cancel();
      }
      return;
    }
    if(((ctrl)&&(k==Qt::Key_P))||
       ((ctrl)&&(k==Qt::Key_N))||
       ((m==Qt::NoModifier)&&(k==Qt::Key_Up))||
       ((m==Qt::NoModifier)&&(k==Qt::Key_Down))){
      const QStringList h = this->getInputHistory();
      if(h.isEmpty()){
	this->qemacs.displayInformativeMessage(QObject::tr("history is empty"));
	return;
      }
      if(this->hMove){
	if(((ctrl)&&(k==Qt::Key_P))||
	   ((m==Qt::NoModifier)&&(k==Qt::Key_Up))){
	  --(this->pHistory);
	  if(this->pHistory<0){
	    this->pHistory = h.length()-1;
	  }
	}
	if(((ctrl)&&(k==Qt::Key_N))||
	   ((m==Qt::NoModifier)&&(k==Qt::Key_Down))){
	  ++(this->pHistory);
	  if(this->pHistory>=h.length()){
	    this->pHistory = 0;
	  }
	}
      } else {
	if(((ctrl)&&(k==Qt::Key_P))||
	   ((m==Qt::NoModifier)&&(k==Qt::Key_Up))){
	  this->pHistory = h.length()-1;
	}
	if(((ctrl)&&(k==Qt::Key_N))||
	   ((m==Qt::NoModifier)&&(k==Qt::Key_Down))){
	  this->pHistory = 0;
	}
      }
      this->setText(h[this->pHistory]);
      this->hMove = true;
      return;
    }
    if((ctrl)&&(k==Qt::Key_Y)){
      const QStringList& ring = this->qemacs.getKillRing();
      if(!ring.isEmpty()){
	this->yank = true;
	this->pring = ring.size()-1;
	this->insert(ring[this->pring]);
      }
      return;
    }
    if((ctrl)&&(k==Qt::Key_M)){
      auto qle = qobject_cast<QEmacsLineEdit *>(this->parent());
      if(qle!=nullptr){
	qle->userEditingFinished();
      }
      return;
    }
    if((ctrl)&&(k==Qt::Key_H)){
      this->selectAll();
      return;
    }
    if((ctrl)&&(k==Qt::Key_Space)){
      this->setSelection(this->cursorPosition(),0);
      return;
    }
    if((ctrl)&&(k==Qt::Key_A)){
      this->home(this->selectionStart ()!=-1);
      return;
    }
    if((ctrl)&&(k==Qt::Key_E)){
      this->end(this->selectionStart ()!=-1);
      return;
    }      
    if((ctrl)&&(k==Qt::Key_B)){
      this->cursorBackward(this->selectionStart ()!=-1);
      return;
    }
    if((ctrl)&&(k==Qt::Key_F)){
      this->cursorForward(this->selectionStart()!=-1);
      return;
    }
    if((alt)&&(k==Qt::Key_Y)){
      const QStringList& ring = this->qemacs.getKillRing();
      if((ring.isEmpty())||(!this->yank)){
      	if(ring.isEmpty()){
      	  this->qemacs.displayInformativeMessage(QObject::tr("kill ring is empty"));
      	}
      	if(!this->yank){
      	  this->qemacs.displayInformativeMessage(QObject::tr("previous command was not a yank"));
      	}
      	return;
      }
      int s = ring[this->pring].size();
      --(this->pring);
      if(this->pring==-1){
      	this->pring = ring.size();
      	--(this->pring);
      }
      this->cursorBackward(true,s);
      this->backspace();
      this->insert(ring[this->pring]);
      this->yank = true;
      return;
    }
    if((alt)&&(k==Qt::Key_B)){
      this->cursorWordBackward(this->selectionStart()!=-1);
      return;
    }      
    if((alt)&&(k==Qt::Key_F)){
      this->cursorWordForward(this->selectionStart ()!=-1);
      return;
    }
    QLineEdit::keyPressEvent(ev);
  } // end of QEmacsLineEdit::CustomLineEdit::keyPressEvent

  void
  QEmacsLineEdit::CustomLineEdit::setInputHistory(const QStringList& h)
  {
    this->inputHistory = h;
  }

  QStringList
  QEmacsLineEdit::CustomLineEdit::getInputHistory() const
  {
    return this->inputHistory;
  } // end of QEmacsLineEdit::CustomLineEdit::getInputHistory

  QEmacsLineEdit::QEmacsLineEdit(const QString& l,
				 QEmacsWidget& p,
				 const bool b)
    : QWidget(&p),
      qemacs(p),
      buffer(p.getCurrentBuffer()),
      // vl(new QVBoxLayout),
      hl(new QHBoxLayout),
      label(new QLabel(this)),
      input(nullptr),
      completions(nullptr),
      scompletions(nullptr),
      isUserEditingFinished(false)
  {
    this->setBackgroundRole(QPalette::Foreground);
    this->setContentsMargins(0,0,0,0);
    QSizePolicy s;
    s.setHorizontalPolicy(QSizePolicy::Fixed);
    s.setVerticalPolicy(QSizePolicy::Fixed);
    this->label->setSizePolicy(s);
    this->label->setAlignment(Qt::AlignHCenter);
    this->label->setFocusPolicy(Qt::NoFocus);
    this->label->setContentsMargins(0,0,0,0);
    this->label->setLineWidth(0);
    this->setLabel(l);
    this->hl->setMargin(0);
    this->hl->setSpacing(0);
    this->hl->setContentsMargins(0,0,0,0);
    this->setStyleSheet("background: rgba(255,255,255,100%)");
    this->label->setStyleSheet("background: rgba(255,255,255,100%)");
    this->hl->addWidget(this->label);
    // this->vl->setMargin(0); 
    // this->vl->setSpacing(0);
    // this->vl->setContentsMargins(0,0,0,0);
    // this->vl->addLayout(this->hl);
    // this->completions->hide();
    // this->completions->setFocusPolicy(Qt::NoFocus);
    // this->completions->setContentsMargins(0,0,0,0);
    // this->vl->addWidget(this->completions);
    if(b){
      this->setLineEdit(new CustomLineEdit(p,*this));
    }
    this->setLayout(this->hl);
  } // end of QEmacsLineEdit::QEmacsLineEdit
  
  bool
  QEmacsLineEdit::isBlocking() const
  {
    return false;
  } // end of QEmacsLineEdit::isBlocking

  void
  QEmacsLineEdit::hideCompletions()
  {
    this->buffer.hideSlave(this->scompletions);
  } // end of QEmacsLineEdit::hideCompletions

  void
  QEmacsLineEdit::showCompletions(const QString&,
				  const QStringList& cl)
  {
    if(this->completions==nullptr){
      this->completions = new QEmacsTextEdit(this->qemacs,this->buffer);
      this->completions->setReadOnly(true);
      this->scompletions = this->buffer.addSlave("*completions*",this->completions);
      QObject::connect(this,SIGNAL(textChanged(const QString&)),
		       this,SLOT(hideCompletions()));
    }
    this->completions->clear();
    this->completions->insertHtml("<font color=\"blue\">Possible completions :</font>");
    const auto d = cl.size()/3;
    const auto m = cl.size()%3;
    QString table;
    table += "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
    for(int i=0;i!=d;++i){
      QString l;
      l  = "<tr>";     
      l += "<td width=\"30%\">"+cl[i*3]+"</td>";
      l += "<td width=\"30%\">"+cl[i*3+1]+"</td>";
      l += "<td width=\"30%\">"+cl[i*3+2]+"</td>";
      l += "</tr>";
      table += l;
    }
    QString ll;
    if(m!=0){
      ll  = "<tr>";
      if(m==1){
	ll += "<td width=\"30%\">"+cl[d*3]+"</td>"
	  "<td width=\"30%\"></td>"
	  "<td width=\"30%\"></td>";
      }
      if(m==2){
	ll += "<td width=\"30%\">"+cl[d*3]+"</td>"
	  "<td width=\"30%\">"+cl[d*3+1]+"</td>"
	  "<td width=\"30%\"></td>";
      }
      ll += "</tr>";
      table += ll;
    }
    table += "</table>";
    this->completions->insertHtml(table);
    this->scompletions->show();
    this->buffer.showSlaves();
  } // end of QEmacsLineEdit::showCompletions

  void QEmacsLineEdit::setLabel(const QString& l)
  {
    this->label->setText("<font color=blue> "+l+" </font>");
  }

  void QEmacsLineEdit::keyPressEvent(QKeyEvent * e)
  {
    if(this->input==nullptr){
      return;
    }
    this->input->event(e);
  }

  void
  QEmacsLineEdit::setLineEdit(QEmacsLineEdit::CustomLineEdit *const i)
  {
    if(i==nullptr){
      return;
    }
    if(this->input!=nullptr){
      this->hl->removeWidget(this->input);
      QObject::disconnect(this->input,SIGNAL(returnPressed()),
			  this,SLOT(userEditingFinished()));
      QObject::disconnect(this->input,SIGNAL(textChanged(const QString&)),
			  this,SLOT(inputTextChanged(const QString&)));
      QObject::disconnect(this->input,SIGNAL(textEdited(const QString&)),
			  this,SLOT(inputTextEdited(const QString&)));
      this->input->deleteLater();
    }
    this->input = i;
    this->input->setContentsMargins(0,0,0,0);
    this->input->setFont(this->label->font());
    this->input->setFrame(false);
    this->input->setStyleSheet("background: rgba(255,255,255,100%)");
    this->hl->addWidget(this->input);
    QObject::connect(this->input,SIGNAL(returnPressed()),
		     this,SLOT(userEditingFinished()));
    QObject::connect(this->input,SIGNAL(textChanged(const QString&)),
		     this,SLOT(inputTextChanged(const QString&)));
    QObject::connect(this->input,SIGNAL(textEdited(const QString&)),
		     this,SLOT(inputTextEdited(const QString&)));
  }

  void QEmacsLineEdit::userEditingFinished()
  {
    if(!this->isUserEditingFinished){
      this->isUserEditingFinished = true;
      auto h = this->input->getInputHistory();
      h.append(this->input->text());
      this->input->setInputHistory(h);
      if(!this->inputHistorySettingAddress.isEmpty()){
	QSettings s;
	s.setValue(this->inputHistorySettingAddress,h);
      }
      this->treatUserInput();
      emit finished(this);
    }
  } // end of QEmacsLineEdit::userEditingFinished()

  void QEmacsLineEdit::cancel()
  {
    QObject::disconnect(this->input,SIGNAL(returnPressed()),
			this,SLOT(userEditingFinished()));
    this->isUserEditingFinished = true;
    emit finished(this);
  } // end of QEmacsLineEdit::cancel()

  void QEmacsLineEdit::setFocus()
  {
    this->input->setFocus();
  } // end of QEmacsLineEdit::setFocus()

  void
  QEmacsLineEdit::inputTextChanged(const QString & t)
  {
    emit textChanged(t);
  }

  void
  QEmacsLineEdit::inputTextEdited(const QString & t)
  {
    emit textEdited(t);
  }

  QEmacsLineEdit::~QEmacsLineEdit()
  {
    if((this->completions !=nullptr)&&
       (this->scompletions!=nullptr)){
      this->buffer.removeSlave(this->scompletions);
    }
    emit destroyed(this);
  } // end QEmacsLineEdit::~QEmacsLineEdit()

  struct QEmacsYesOrNoUserInput::YesOrNoLineEdit
    : QEmacsLineEdit::CustomLineEdit
  {
    YesOrNoLineEdit(QEmacsWidget& q,
		    QEmacsLineEdit& p)
      : CustomLineEdit(q,p)
    {} // end of YesOrNoLineEdit

    void keyPressEvent(QKeyEvent *ev) override
    {
      if((ev->modifiers() == Qt::ControlModifier)&&(ev->key()==Qt::Key_G)){
	auto qle = qobject_cast<QEmacsLineEdit *>(this->parent());
	if(qle!=nullptr){
	  qle->cancel();
	}
	return;
      }
      if((ev->modifiers() == Qt::ControlModifier)&&(ev->key()==Qt::Key_O)){
	auto *qle = qobject_cast<QEmacsLineEdit *>(this->parent());
	if(qle!=nullptr){
	  if(!qle->isBlocking()){
	    this->qemacs.getCurrentBuffer().setFocus();
	    this->qemacs.getCurrentBuffer().focusMainFrame();
	  }
	}
      }
      if(((ev->modifiers() == Qt::NoModifier)&&(ev->key()==Qt::Key_Y))||
	 ((ev->modifiers() == Qt::NoModifier)&&(ev->key()==Qt::Key_N))){
	if(ev->key()==Qt::Key_Y){
	  this->setText("y");
	}
	if(ev->key()==Qt::Key_N){
	  this->setText("n");
	}
	auto *qle = qobject_cast<QEmacsLineEdit *>(this->parent());
	qle->userEditingFinished();
      }
    }
  };

  QEmacsYesOrNoUserInput::QEmacsYesOrNoUserInput(const QString& i,
						 QEmacsWidget& p)
    : QEmacsLineEdit(QObject::tr("%1 (y or n)").arg(i),p,false)
  {
    this->setLineEdit(new YesOrNoLineEdit(p,*this));
  } // end of QEmacsYesOrNoUserInput::QEmacsYesOrNoUserInput
  
  struct QEmacsFilePathUserInput::FilePathLineEdit
    : QEmacsLineEdit::CustomLineEdit
  {
    FilePathLineEdit(QEmacsWidget& q,
		     QEmacsLineEdit& p)
      : CustomLineEdit(q,p)
    {
      auto *c  = new QCompleter(&p);
      auto *dm = new QDirModel(c);
      c->setModel(dm);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->setCompleter(c,false);
    } // end of FilePathLineEdit

  protected:

    void complete() override
    {
      auto p = this->text();
#ifdef Q_OS_UNIX
      if(p.startsWith("~/")){
	QString homePath = QDir::home().absolutePath();
	if(!homePath.endsWith(QDir::separator())){
	  homePath += QDir::separator();
	}
	p.replace(0,2,homePath);
	QLineEdit::setText(p);
      }
#endif
      bool b;
      QString c = this->findCompletion(b);
      if(c.isEmpty()){
	return;
      }
      this->setText(c,b);
    } // end of complete
    
    QString extractBaseForCompletion(const QString& c) override
    {
      QFileInfo f(c);
      QString d = f.dir().absolutePath();
      if(!d.endsWith(QDir::separator())){
	d += QDir::separator();
      }
      return d;
    }

    virtual void
    setText(const QString& c,
	    const bool b)
    {
      if(b){
	QFileInfo f(c);
	if(f.exists()){
	  if(f.isDir()){
	    if(!c.endsWith(QDir::separator())){
	      QLineEdit::setText(c+QDir::separator());
	    } else {
	      QLineEdit::setText(c);
	    }
	  } else {
	    QLineEdit::setText(c);
	  }
	} else {
	  QLineEdit::setText(c);
	}
      } else {
	QLineEdit::setText(c);
      }
    }

  };

  QEmacsFilePathUserInput::QEmacsFilePathUserInput(const QString& i,
						   QEmacsWidget& p)
    : QEmacsLineEdit(i,p)
  {
    this->setLineEdit(new FilePathLineEdit(p,*this));
  } // end of QEmacsFilePathUserInput::QEmacsFilePathUserInput

  QEmacsProcessLineEdit::QEmacsProcessLineEdit(const QString& l,
					       const QString& c,
					       const QString& m,
					       QEmacsWidget& p)
    : QEmacsLineEdit(l,p),
      qemacs(p),
      mode(m)
  {
    if(!c.isEmpty()){
      this->input->setText(c);
    }
  } // end of QEmacsProcessLineEdit::QEmacsProcessLineEdit
  
  void
  QEmacsProcessLineEdit::treatUserInput()
  {
    const QString c = this->input->text();
    QStringList args = c.split(" ",QString::SkipEmptyParts);
    if(args.empty()){
      this->qemacs.displayInformativeMessage(QObject::tr("empty command"));
      return;
    }
    const QString c0 = args.front();
    args.pop_front();
    this->run(c0,c0,args);
    return;
  }

  void
  QEmacsProcessLineEdit::run(const QString& on,
			     const QString& c,
			     const QStringList& args)
  {
    if(c.isEmpty()){
      this->qemacs.displayInformativeMessage(QObject::tr("empty command"));
      return;
    }
    auto&   b = this->qemacs.getCurrentBuffer();
    auto& t = b.getMainFrame();
    auto *po = new ProcessOutputFrame(this->qemacs,b);
    QFileInfo fn(t.getCompleteFileName());
    QDir d(fn.dir());
    auto& p = po->getProcess();
    if(d.exists()){
      p.setWorkingDirectory(d.absolutePath());
    } else {
      p.setWorkingDirectory(QDir::current().absolutePath());
    }
    if(!this->mode.isEmpty()){
      po->setMajorMode(this->mode);
    }
    p.start(c,args);
    b.addSlave("*"+on+"* ouput",po);
  } // end of QEmacsProcessLineEdit::run

  QEmacsProcessLineEdit::~QEmacsProcessLineEdit() = default;

  QEmacsShellProcessLineEdit::QEmacsShellProcessLineEdit(const QString& l,
							 const QString& c,
							 const QString& m,
							 QEmacsWidget& p)
    : QEmacsProcessLineEdit(l,c,m,p)
  {} // end of QEmacsShellProcessLineEdit::QEmacsShellProcessLineEdit
  
  void
  QEmacsShellProcessLineEdit::treatUserInput()
  {
    const QString c = this->input->text();
    if(c.isEmpty()){
      this->qemacs.displayInformativeMessage(QObject::tr("empty command"));
      return;
    }
    const char * s = ::getenv("SHELL");
    if(s==nullptr){
      this->qemacs.displayInformativeMessage(QObject::tr("no shell defined"));
    }
    this->run(c,s,QStringList() << "-c" << c);
  } // end of QEmacsShellProcessLineEdit::treatUserInput

  QEmacsShellProcessLineEdit::~QEmacsShellProcessLineEdit() = default;

} // end of namespace qemacs

