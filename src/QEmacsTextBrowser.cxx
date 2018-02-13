/*! 
 * \file  QEmacsTextBrowser.cxx
 * \brief
 * \author Helfer Thomas
 * \date   19/08/2012
 */

#include<QtCore/QDir>
#include<QtCore/QDate>
#include<QtCore/QFile>
#include<QtCore/QFileInfo>
#include<QtCore/QTextStream>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QMessageBox>
#include<QtWidgets/QApplication>
#include"QEmacs/Utilities.hxx"
#include"QEmacs/QEmacsTextBrowser.hxx"

#ifdef Q_OS_UNIX
#include<unistd.h>
#include<sys/types.h>
#include<pwd.h>
#endif /* Q_OS_UNIX */

namespace qemacs
{

  QEmacsTextBrowser::QEmacsTextBrowser(QEmacsWidget& g,
				       QEmacsBuffer& b)
    : QEmacsTextEditBase(g,b),
      e(new QTextBrowser(this))
  {
    this->initialize(this->e);
    auto *hl  = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl,this->e);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setLayout(hl);
  } // end of QEmacsTextBrowser::QEmacsTextBrowser()

  QEmacsTextBrowser::QEmacsTextBrowser(const QString& f,
				       QEmacsWidget& g,
				       QEmacsBuffer& b)
    : QEmacsTextEditBase(g,b),
      e(new QTextBrowser(this))
  {
    this->initialize(this->e);
    auto *hl = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl,this->e);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setLayout(hl);
    // reading the file
    QFile file(f);
    if(file.exists()){
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
	QMessageBox::critical(this,tr("QEmacs::openFile"),
			      tr("opening file '%1' failed").arg(f));
	return;
      }
      QTextStream in(&file);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      this->setPlainText(in.readAll());
      QApplication::restoreOverrideCursor();
      this->document()->setModified(false);
      this->moveCursor(QTextCursor::Start,
    		       QTextCursor::MoveAnchor);
    } 
    this->setFileName(f);
  } // end of QEmacsTextBrowser::QEmacsTextBrowser()

  QAbstractScrollArea* QEmacsTextBrowser::widget()
  {
    return this->e;
  }

  void QEmacsTextBrowser::undo()
  {
    this->e->undo();
  } // end of QEmacsTextBrowser::undo

  void QEmacsTextBrowser::redo()
  {
    this->e->redo();
  } // end of QEmacsTextBrowser::redo

  void QEmacsTextBrowser::cut()
  {
    this->e->cut();
  }

  void QEmacsTextBrowser::paste()
  {
    this->e->paste();
  }
  
  void QEmacsTextBrowser::selectAll()
  {
    this->e->selectAll();
  }
  
  void QEmacsTextBrowser::copy()
  {
    this->e->copy();
  }

  void QEmacsTextBrowser::clear()
  {
    this->e->clear();
  }

  bool
  QEmacsTextBrowser::isReadOnly()
  {
    return this->e->isReadOnly();
  }
  
  void QEmacsTextBrowser::appendPlainText(const QString& t)
  {
    QTextCursor tc = this->e->textCursor();
    tc.movePosition(QTextCursor::End,
		    QTextCursor::MoveAnchor);
    tc.insertText(t);
  }

  void QEmacsTextBrowser::insertPlainText(const QString& t)
  {
    return this->e->insertPlainText(t);
  }

  void QEmacsTextBrowser::setPlainText(const QString& t)
  {
    return this->e->setPlainText(t);
  }

  void QEmacsTextBrowser::appendHtml(const QString& t)
  {
    QTextCursor tc = this->e->textCursor();
    tc.movePosition(QTextCursor::End,
		    QTextCursor::MoveAnchor);
    tc.insertHtml(t);
  }

  void QEmacsTextBrowser::insertHtml(const QString& t)
  {
    return this->e->textCursor().insertHtml(t);
  }

  void QEmacsTextBrowser::setHtml(const QString& t)
  {
    this->clear();
    this->insertHtml(t);
  }

  QTextCursor
  QEmacsTextBrowser::cursorForPosition(const QPoint& p) const
  {
    return this->e->cursorForPosition(p);
  }

  QRect
  QEmacsTextBrowser::cursorRect(const QTextCursor& tc) const
  {
    return this->e->cursorRect(tc);
  }
  
  void QEmacsTextBrowser::setUndoRedoEnabled(bool b)
  {
    return this->e->setUndoRedoEnabled(b);
  }

  void QEmacsTextBrowser::setReadOnly(bool b)
  {
    return this->e->setReadOnly(b);
  }
  
  void QEmacsTextBrowser::moveCursor(QTextCursor::MoveOperation mo,
				  QTextCursor::MoveMode mm)
  {
    return this->e->moveCursor(mo,mm);
  }
  
  QTextDocument*
  QEmacsTextBrowser::document() const
  {
    return this->e->document();
  } // end of QEmacsTextBrowser::document

  void QEmacsTextBrowser::setExtraSelections(const QList<QTextEdit::ExtraSelection>& l)
  {
    return this->e->setExtraSelections(l);
  }
  
  QTextCursor
  QEmacsTextBrowser::textCursor() const
  {
    return this->e->textCursor();
  }

  QList<QTextEdit::ExtraSelection>
  QEmacsTextBrowser::extraSelections() const
  {
    return this->e->extraSelections();
  } // end of QEmacsTextBrowser::extraSelections

  QRect
  QEmacsTextBrowser::cursorRect() const
  {
    return this->e->cursorRect();
  }

  bool
  QEmacsTextBrowser::find(const QString& t,
			    QTextDocument::FindFlags f)
  {
    return this->e->find(t,f);
  }

  bool
  QEmacsTextBrowser::isUndoRedoEnabled() const
  {
    return this->e->isUndoRedoEnabled();
  }
  
  void QEmacsTextBrowser::setTextCursor(const QTextCursor& tc)
  {
    return this->e->setTextCursor(tc);
  }

  void QEmacsTextBrowser::setTextInteractionFlags(Qt::TextInteractionFlags f)
  {
    this->e->setTextInteractionFlags(f);
  }

  QEmacsTextBrowser::~QEmacsTextBrowser() = default;

} // end of namespace qemacs
