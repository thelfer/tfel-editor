/*! 
 * \file  QEmacsPlainTextEdit.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 août 2012
 */

#include<fstream>
#include<QtCore/QDebug>
#include<QtCore/QDir>
#include<QtCore/QDate>
#include<QtCore/QFile>
#include<QtCore/QFileInfo>
#include<QtCore/QSettings>
#include<QtCore/QTextCodec>
#include<QtCore/QTextStream>
#include<QtCore/QStringListModel>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QMessageBox>
#include<QtWidgets/QApplication>
#include<QtWidgets/QListView>
#include<QtWidgets/QDialogButtonBox>
#include"QEmacs/utf8/utf8.h"
#include"QEmacs/Utilities.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"

#ifdef Q_OS_UNIX
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#endif /* Q_OS_UNIX */

namespace qemacs
{

  struct CodecSelectionDialog
    : public QDialog
  {
    
    CodecSelectionDialog(const QString& file,
			 QWidget *const p)
      : QDialog(p),
	model(new QStringListModel(this)),
	view(new QListView(this))
    {
      auto *mlayout = new QVBoxLayout;
      auto *l = new QLabel(QObject::tr("Select encoding for file '%1'")
			   .arg(QFileInfo(file).fileName()));
      auto codecs_ = QTextCodec::availableCodecs();
      QStringList codecs;
      for(int i=0;i!=codecs_.size();++i){
	codecs.append(codecs_[i]);
      }
      codecs.sort();
      model->setStringList(codecs);
      QSettings s;
      view->setModel(model);
      if(s.contains("previously selected codec")){
	auto c = s.value("previously selected codec").toString();
	view->setCurrentIndex(model->index(codecs.indexOf(c)));
      }
      auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok|
				      QDialogButtonBox::Cancel);
      connect(bb,&QDialogButtonBox::accepted,
	      this,&CodecSelectionDialog::accept);
      connect(bb,&QDialogButtonBox::rejected,
	      this,&CodecSelectionDialog::reject);
      mlayout->addWidget(l);
      mlayout->addWidget(view);  
      mlayout->addWidget(bb);  
      this->setLayout(mlayout);
    } // end of CodecSelectionDialog

    QString selectedCodec() const
    {
      const auto index = this->view->currentIndex();
      if(!index.isValid()){
	return QString();
      }
      const auto c = this->model->stringList()[index.row()];
      QSettings s;
      s.setValue("previously selected codec",c);
      return c;
    } // end of selectedCodec

  private:

    QStringListModel *model;
    QListView *view;
  }; // end of 

  static bool valid_utf8_file(const QString& f)
  {
    std::ifstream ifs(f.toStdString().c_str());
    if (!ifs){
      return false; // even better, throw here
    }
    std::istreambuf_iterator<char> it(ifs.rdbuf());
    std::istreambuf_iterator<char> eos;
    return utf8::is_valid(it, eos);
  }

#ifdef Q_OS_UNIX

  static QString getLoginName()
  {
    struct passwd *pw;
    uid_t uid;
    uid = geteuid ();
    pw = getpwuid (uid);
    if (pw!=nullptr){
      return QString(pw->pw_name);
    }
    return QString();
  }

  static QString getUserName()
  {
    struct passwd *pw;
    uid_t uid;
    uid = geteuid ();
    pw = getpwuid (uid);
    if (pw!=nullptr){
      return QString(pw->pw_gecos);
    }
    return QString();
  }  
  
#endif /* Q_WS_X11 */

  QEmacsPlainTextEdit::QEmacsPlainTextEdit(QEmacsWidget& g,
					   QEmacsBuffer& b)
    : QEmacsTextEditBase(g,b),
      e(new QPlainTextEdit(this))
  {
    this->initialize(this->e);
#pragma message ("warning")
    auto *hl  = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl,this->e);
    this->e->setTabStopWidth(40);
    this->e->setWordWrapMode(QTextOption::WrapAnywhere);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setLayout(hl);
  } // end of QEmacsPlainTextEdit::QEmacsPlainTextEdit()

  QEmacsPlainTextEdit::QEmacsPlainTextEdit(const QString& f,
					   QEmacsWidget& g,
					   QEmacsBuffer& b)
    : QEmacsTextEditBase(g,b),
      e(new QPlainTextEdit(this))
  {
    this->initialize(this->e);
    auto *hl = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl,this->e);
    this->e->setTabStopWidth(40);
    this->e->setWordWrapMode(QTextOption::WrapAnywhere);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->e->setContextMenuPolicy(Qt::NoContextMenu);
    this->setLayout(hl);
    // reading the file
    QFile file(f);
    if(file.exists()){
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
	QMessageBox::critical(this,tr("QEmacs::openFile"),
			      tr("opening file '%1' failed").arg(f));
	return;
      }
      // check if utf8
      QTextStream in(&file);
      bool u = valid_utf8_file(f);
      if(!u){
	// select codec
	CodecSelectionDialog d(f,this);
	if(d.exec()==QDialog::Accepted){
	  QString codec = d.selectedCodec();
	  if(!codec.isEmpty()){
	    in.setCodec(codec.toStdString().c_str());
	  }
	}
      }
      QApplication::setOverrideCursor(Qt::WaitCursor);
      this->setPlainText(in.readAll());
      QApplication::restoreOverrideCursor();
      this->document()->setModified(!u);
      this->moveCursor(QTextCursor::Start,
    		       QTextCursor::MoveAnchor);
      // setting file name
      this->setFileName(f);
      // see if we saved the file position
      QSettings s;
      if(s.contains("positions in files")){
	QMap<QString,QVariant> pl = s.value("positions in files").value<QMap<QString,QVariant> >();
	QMap<QString,QVariant>::const_iterator i;
	if(pl.contains(this->getCompleteFileName())){
	  this->gotoLine(pl[this->getCompleteFileName()].toInt());
	}
      }
    } else {
      if(!f.isEmpty()){
	this->readTemplateFile(f);
      }
      this->setFileName(f);
    }
  } // end of QEmacsPlainTextEdit::QEmacsPlainTextEdit()

  QAbstractScrollArea* QEmacsPlainTextEdit::widget()
  {
    return this->e;
  }

  void QEmacsPlainTextEdit::print(QPrinter *const p)
  {
    this->e->print(p);
  }

  void QEmacsPlainTextEdit::undo()
  {
    this->e->undo();
  } // end of QEmacsPlainTextEdit::undo

  void QEmacsPlainTextEdit::redo()
  {
    this->e->redo();
  } // end of QEmacsPlainTextEdit::redo

  void QEmacsPlainTextEdit::cut()
  {
    this->e->cut();
  }

  void QEmacsPlainTextEdit::paste()
  {
    this->e->paste();
  }
  
  void QEmacsPlainTextEdit::selectAll()
  {
    this->e->selectAll();
  }
  
  void QEmacsPlainTextEdit::copy()
  {
    this->e->copy();
  }

  void QEmacsPlainTextEdit::clear()
  {
    this->e->clear();
  }

  bool
  QEmacsPlainTextEdit::isReadOnly()
  {
    return this->e->isReadOnly();
  }
  
  void QEmacsPlainTextEdit::appendPlainText(const QString& t)
  {
    return this->e->appendPlainText(t);
  }

  void QEmacsPlainTextEdit::insertPlainText(const QString& t)
  {
    return this->e->insertPlainText(t);
  }

  void QEmacsPlainTextEdit::setPlainText(const QString& t)
  {
    return this->e->setPlainText(t);
  }

  void QEmacsPlainTextEdit::appendHtml(const QString& t)
  {
    return this->e->appendHtml(t);
  }

  void QEmacsPlainTextEdit::insertHtml(const QString& t)
  {
    return this->e->textCursor().insertHtml(t);
  }

  void QEmacsPlainTextEdit::setHtml(const QString& t)
  {
    this->clear();
    this->insertHtml(t);
  }

  QTextCursor
  QEmacsPlainTextEdit::cursorForPosition(const QPoint& p) const
  {
    return this->e->cursorForPosition(p);
  }

  QRect
  QEmacsPlainTextEdit::cursorRect(const QTextCursor& tc) const
  {
    return this->e->cursorRect(tc);
  }
  
  void QEmacsPlainTextEdit::setUndoRedoEnabled(bool b)
  {
    return this->e->setUndoRedoEnabled(b);
  }

  void QEmacsPlainTextEdit::setReadOnly(bool b)
  {
    this->e->setReadOnly(b);
  }
  
  void QEmacsPlainTextEdit::moveCursor(QTextCursor::MoveOperation mo,
				  QTextCursor::MoveMode mm)
  {
    return this->e->moveCursor(mo,mm);
  }
  
  QTextDocument*
  QEmacsPlainTextEdit::document() const
  {
    return this->e->document();
  } // end of QEmacsPlainTextEdit::document

  void QEmacsPlainTextEdit::setExtraSelections(const QList<QTextEdit::ExtraSelection>& l)
  {
    return this->e->setExtraSelections(l);
  }
  
  QTextCursor
  QEmacsPlainTextEdit::textCursor() const
  {
    return this->e->textCursor();
  }

  QList<QTextEdit::ExtraSelection>
  QEmacsPlainTextEdit::extraSelections() const
  {
    return this->e->extraSelections();
  } // end of QEmacsPlainTextEdit::extraSelections

  QRect
  QEmacsPlainTextEdit::cursorRect() const
  {
    return this->e->cursorRect();
  }

  bool QEmacsPlainTextEdit::find(const QString& t,
				 QTextDocument::FindFlags f)
  {
    return this->e->find(t,f);
  }

  bool
  QEmacsPlainTextEdit::isUndoRedoEnabled() const
  {
    return this->e->isUndoRedoEnabled();
  }
  
  void QEmacsPlainTextEdit::setTextCursor(const QTextCursor& tc)
  {
    return this->e->setTextCursor(tc);
  }

  void QEmacsPlainTextEdit::setTextInteractionFlags(Qt::TextInteractionFlags f)
  {
    this->e->setTextInteractionFlags(f);
  }

  void QEmacsPlainTextEdit::readTemplateFile(const QString& f)
  {
    // reading the template file associated with the file
    QFileInfo i(f);
    QString ext = i.completeSuffix();
    QFile t_file("/home/th202608/.templates/TEMPLATE."+ext+".tpl");
    if (!t_file.open(QIODevice::ReadOnly | QIODevice::Text)){
      return;
    }
#ifdef Q_OS_UNIX
    QString user  = getUserName();
    QString login = getLoginName();
#endif
    QTime   t     = QTime::currentTime();
    QString time  = QString::number(t.hour())+":"+QString::number(t.minute())+QString::number(t.second());

    QDate   d        = QDate::currentDate();
    QString date     = d.toString("d ddd yyyy");
    QString iso_date = d.toString(Qt::ISODate);
    QString vc_date  = iso_date+" "+time;

    QString file        = i.fileName();
    QString file_sans   = i.baseName();
    QString file_upcase = file_sans.toUpper();
    QString dir = i.dir().absolutePath();

    QString file_raw;
    QString file_num;
    QRegExp expr("(.*)(\\d.*)");
    int p = 0;
    if(expr.indexIn(file_sans,p)!=-1){
      file_raw = expr.cap(1);
      file_num = expr.cap(2);
    } else {
      file_raw = file_sans;
    }
    QTextStream in(&t_file);
    while (!in.atEnd()) {
      QString l = in.readLine();
#ifdef Q_OS_UNIX
      l.replace("(>>>AUTHOR<<<)",user);
      l.replace("(>>>USER_NAME<<<)",login);
      l.replace("(>>>LOGIN_NAME<<<)",login);
#endif
      l.replace("(>>>FILE<<<)",file);
      l.replace("(>>>FILE_RAW<<<)",file_raw);
      l.replace("(>>>FILE_UPCASE<<<)",file_upcase);
      l.replace("(>>>FILE_EXT<<<)",ext);
      l.replace("(>>>FILE_SANS<<<)",file_sans);
      l.replace("(>>>DIR<<<)",dir);
      l.replace("(>>>TIME<<<)",time);
      l.replace("(>>>DATE<<<)",date);
      l.replace("(>>>ISO_DATE<<<)",iso_date);
      l.replace("(>>>VC_DATE<<<)",vc_date);
      l.replace("(>>>COMMENT<<<)","");
      this->insertPlainText(l+"\n");
    }
    this->moveCursor(QTextCursor::Start,
		     QTextCursor::MoveAnchor);
    if(this->find("(>>>POINT<<<)")){
      QTextCursor c = this->textCursor();
      c.removeSelectedText();
      while(this->find("(>>>POINT<<<)")){
	c = this->textCursor();
	c.movePosition(QTextCursor::EndOfWord,
		       QTextCursor::KeepAnchor);
	c.removeSelectedText();
      }
    } else {
      this->moveCursor(QTextCursor::Start,
		       QTextCursor::MoveAnchor);
    }
    this->document()->setModified(false);
  } // end of QEmacsPlainTextEdit::readTemplateFile

  QEmacsPlainTextEdit::~QEmacsPlainTextEdit()
  {
    const auto&f = this->getCompleteFileName();
    QSettings s;
    QMap<QString, QVariant> pl;
    if(s.contains("positions in files")){
      pl = s.value("positions in files").value<QMap<QString, QVariant> >();
    }
    const auto& c = this->textCursor();
    pl[f] = c.blockNumber()+1;
    s.setValue("positions in files",pl);
  } // end of QEmacsPlainTextEdit::~QEmacsPlainTextEdit()

} // end of namespace qemacs
