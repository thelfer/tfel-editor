/*! 
 * \file  QEmacsTextEdit.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 août 2012
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
#include"QEmacs/QEmacsTextEdit.hxx"

namespace qemacs
{

  QEmacsTextEdit::QEmacsTextEdit(QEmacsWidget& g,
					   QEmacsBuffer& b)
    : QEmacsTextEditBase(g,b),
      e(new QTextEdit(this))
  {
    this->initialize(this->e);
    auto *hl  = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl,this->e);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->e->setContextMenuPolicy(Qt::NoContextMenu);
    this->setLayout(hl);
  } // end of QEmacsTextEdit::QEmacsTextEdit()

  QEmacsTextEdit::QEmacsTextEdit(const QString& f,
					   QEmacsWidget& g,
					   QEmacsBuffer& b)
    : QEmacsTextEditBase(g,b),
      e(new QTextEdit(this))
  {
    this->initialize(this->e);
    auto *hl  = new QHBoxLayout;
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
  } // end of QEmacsTextEdit::QEmacsTextEdit()

  QAbstractScrollArea *
  QEmacsTextEdit::widget()
  {
    return this->e;
  }

  void
  QEmacsTextEdit::undo()
  {
    this->e->undo();
  } // end of QEmacsTextEdit::undo

  void
  QEmacsTextEdit::redo()
  {
    this->e->redo();
  } // end of QEmacsTextEdit::redo

  void
  QEmacsTextEdit::cut()
  {
    this->e->cut();
  }

  void
  QEmacsTextEdit::paste()
  {
    this->e->paste();
  }
  
  void
  QEmacsTextEdit::selectAll()
  {
    this->e->selectAll();
  }
  
  void
  QEmacsTextEdit::copy()
  {
    this->e->copy();
  }

  void
  QEmacsTextEdit::clear()
  {
    this->e->clear();
  }

  bool
  QEmacsTextEdit::isReadOnly()
  {
    return this->e->isReadOnly();
  }
  
  void
  QEmacsTextEdit::appendPlainText(const QString& t)
  {
    QTextCursor tc = this->e->textCursor();
    tc.movePosition(QTextCursor::End,
		    QTextCursor::MoveAnchor);
    tc.insertText(t);
  }

  void
  QEmacsTextEdit::insertPlainText(const QString& t)
  {
    return this->e->insertPlainText(t);
  }

  void
  QEmacsTextEdit::setPlainText(const QString& t)
  {
    return this->e->setPlainText(t);
  }

  void
  QEmacsTextEdit::appendHtml(const QString& t)
  {
    QTextCursor tc = this->e->textCursor();
    tc.movePosition(QTextCursor::End,
		    QTextCursor::MoveAnchor);
    tc.insertHtml(t);
  }

  void
  QEmacsTextEdit::insertHtml(const QString& t)
  {
    return this->e->textCursor().insertHtml(t);
  }

  void
  QEmacsTextEdit::setHtml(const QString& t)
  {
    this->clear();
    this->insertHtml(t);
  }

  QTextCursor
  QEmacsTextEdit::cursorForPosition(const QPoint& p) const
  {
    return this->e->cursorForPosition(p);
  }

  QRect
  QEmacsTextEdit::cursorRect(const QTextCursor& tc) const
  {
    return this->e->cursorRect(tc);
  }
  
  void
  QEmacsTextEdit::setUndoRedoEnabled(bool b)
  {
    return this->e->setUndoRedoEnabled(b);
  }

  void
  QEmacsTextEdit::setReadOnly(bool b)
  {
    return this->e->setReadOnly(b);
  }
  
  void
  QEmacsTextEdit::moveCursor(QTextCursor::MoveOperation mo,
				  QTextCursor::MoveMode mm)
  {
    return this->e->moveCursor(mo,mm);
  }
  
  QTextDocument*
  QEmacsTextEdit::document() const
  {
    return this->e->document();
  } // end of QEmacsTextEdit::document

  void
  QEmacsTextEdit::setExtraSelections(const QList<QTextEdit::ExtraSelection>& l)
  {
    return this->e->setExtraSelections(l);
  }
  
  QTextCursor
  QEmacsTextEdit::textCursor() const
  {
    return this->e->textCursor();
  }

  QList<QTextEdit::ExtraSelection>
  QEmacsTextEdit::extraSelections() const
  {
    return this->e->extraSelections();
  } // end of QEmacsTextEdit::extraSelections

  QRect
  QEmacsTextEdit::cursorRect() const
  {
    return this->e->cursorRect();
  }

  bool
  QEmacsTextEdit::find(const QString& t,
			    QTextDocument::FindFlags f)
  {
    return this->e->find(t,f);
  }

  bool
  QEmacsTextEdit::isUndoRedoEnabled() const
  {
    return this->e->isUndoRedoEnabled();
  }
  
  void
  QEmacsTextEdit::setTextCursor(const QTextCursor& tc)
  {
    return this->e->setTextCursor(tc);
  }

  void
  QEmacsTextEdit::setTextInteractionFlags(Qt::TextInteractionFlags f)
  {
    this->e->setTextInteractionFlags(f);
  }

  QEmacsTextEdit::~QEmacsTextEdit() = default;

} // end of namespace qemacs
