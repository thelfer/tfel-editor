/*!
 * \file  TextBrowser.hxx
 * \brief
 * \author Helfer Thomas
 * \date   19/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTBROWSEREDIT_HXX
#define LIB_TFEL_GUI_TEXTBROWSEREDIT_HXX

#include <QtWidgets/QTextBrowser>
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  struct TFEL_GUI_VISIBILITY_EXPORT TextBrowser : public TextEditBase {
    TextBrowser(EditorWidget&, Buffer&);

    TextBrowser(const QString&, EditorWidget&, Buffer&);

    ~TextBrowser() override;

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

    void moveCursor(QTextCursor::MoveOperation, QTextCursor::MoveMode) override;

    bool isReadOnly() override;

    bool find(const QString&, QTextDocument::FindFlags = nullptr) override;

    QTextCursor textCursor() const override;

    QTextCursor cursorForPosition(const QPoint&) const override;

    QRect cursorRect(const QTextCursor&) const override;

    QRect cursorRect() const override;

    void setExtraSelections(const QList<QTextEdit::ExtraSelection>&) override;

    QList<QTextEdit::ExtraSelection> extraSelections() const override;

    bool isUndoRedoEnabled() const override;

    QAbstractScrollArea* widget() override;

   protected:
    QTextBrowser* e;

   private:
    Q_OBJECT

  };  // end of struct TextBrowser

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTBROWSEREDIT_HXX */
