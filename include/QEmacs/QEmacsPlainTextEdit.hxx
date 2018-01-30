/*! 
 * \file  QEmacsPlainTextEdit.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 août 2012
 */

#ifndef LIB_QEMACS_QEMACSPLAINTEXTEDIT_HXX
#define LIB_QEMACS_QEMACSPLAINTEXTEDIT_HXX 

#include<QtWidgets/QPlainTextEdit>
#include<QtPrintSupport/QPrinter>
#include"QEmacs/Config.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"

namespace qemacs
{

  class QEMACS_VISIBILITY_EXPORT QEmacsPlainTextEdit
    : public QEmacsTextEditBase
  {

    Q_OBJECT

  public:

    QEmacsPlainTextEdit(QEmacsWidget&,
			QEmacsBuffer&);

    QEmacsPlainTextEdit(const QString&,
			QEmacsWidget&,
			QEmacsBuffer&);

    virtual void
    print(QPrinter *const);
      
    ~QEmacsPlainTextEdit() override;

  public slots:

    void clear() override;

    void undo() override;

    void redo() override;

    void cut() override;
      
    void copy() override;
      
    void paste() override;
      
    void selectAll() override;
      
    void setPlainText(const QString&) override;
      
    void insertPlainText(const QString&) override;

    void appendPlainText(const QString&) override;

    void setHtml(const QString&) override;
    
    void insertHtml(const QString&) override;

    void appendHtml(const QString&) override;
      
    void setTextInteractionFlags(Qt::TextInteractionFlags) override;
      
    void setTextCursor(const QTextCursor&) override;
      
    void setReadOnly(bool) override;
      
    void setUndoRedoEnabled(bool) override;

  public:
      
    QTextDocument* document() const override;
      
    void moveCursor(QTextCursor::MoveOperation,
		    QTextCursor::MoveMode) override;
      
    bool isReadOnly() override;

    bool find(const QString&,
	      QTextDocument::FindFlags = nullptr) override;
      
    QTextCursor textCursor() const override;
      
    QTextCursor cursorForPosition(const QPoint&) const override;
    
    QRect cursorRect(const QTextCursor&) const override;
      
    QRect cursorRect() const override;
      
    void setExtraSelections(const QList<QTextEdit::ExtraSelection>&) override;
      
    QList<QTextEdit::ExtraSelection> extraSelections() const override;
      
    bool isUndoRedoEnabled() const override;

    QAbstractScrollArea* widget() override;

  protected:

    virtual void
    readTemplateFile(const QString&);

    QPlainTextEdit *e;

  }; // end of struct QEmacsPlainTextEdit

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSPLAINTEXTEDIT_HXX */
