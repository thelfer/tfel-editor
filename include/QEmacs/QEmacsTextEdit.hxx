/*! 
 * \file  QEmacsTextEdit.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSTEXTEDIT_H_
#define _LIB_QEMACS_QEMACSTEXTEDIT_H_ 

#ifdef QEMACS_QT4
#include<QtGui/QTextEdit>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QTextEdit>
#endif /* QEMACS_QT5 */

#include"QEmacs/QEmacsTextEditBase.hxx"

namespace qemacs
{

  class QEmacsTextEdit
    : public QEmacsTextEditBase
  {

    Q_OBJECT

  public:

    QEmacsTextEdit(QEmacsWidget&,
		   QEmacsBuffer&);

    QEmacsTextEdit(const QString&,
			QEmacsWidget&,
			QEmacsBuffer&);
      
    virtual ~QEmacsTextEdit();

  public slots:

    virtual void
    clear() override;

    virtual void
    undo() override;

    virtual void
    redo() override;

    virtual void
    cut() override;
      
    virtual void
    copy() override;
      
    virtual void
    paste() override;
      
    virtual void
    selectAll() override;
      
    virtual void
    setPlainText(const QString&) override;
      
    virtual void
    insertPlainText(const QString&) override;

    virtual void
    appendPlainText(const QString&) override;

    virtual void
    setHtml(const QString&) override;
      
    virtual void
    insertHtml(const QString&) override;

    virtual void
    appendHtml(const QString&) override;
      
    virtual void
    setTextInteractionFlags(Qt::TextInteractionFlags) override;
      
    virtual void
    setTextCursor(const QTextCursor&) override;

    virtual void
    setReadOnly(bool) override;
      
    virtual void
    setUndoRedoEnabled(bool) override;

  public:
      
    virtual QTextDocument*
    document() const override;
      
    virtual void
    moveCursor(QTextCursor::MoveOperation,
	       QTextCursor::MoveMode) override;
      
    virtual bool
    isReadOnly() override;

    virtual bool
    find(const QString&,
	 QTextDocument::FindFlags = nullptr) override;
      
    virtual QTextCursor
    textCursor() const override;
      
    virtual QTextCursor
    cursorForPosition(const QPoint&) const override;
      
    virtual QRect
    cursorRect(const QTextCursor&) const override;
      
    virtual QRect
    cursorRect() const override;
      
    virtual void
    setExtraSelections(const QList<QTextEdit::ExtraSelection>&) override;
      
    virtual QList<QTextEdit::ExtraSelection>
    extraSelections() const override;
      
    virtual bool
    isUndoRedoEnabled() const override;

    virtual QAbstractScrollArea *
    widget() override;

  protected:

    QTextEdit *e;

  }; // end of struct QEmacsTextEdit

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSTEXTEDIT_H */

