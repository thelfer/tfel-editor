/*!
 * \file  TextEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   19/08/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/TextEdit.hxx"

namespace tfel::gui {

  TextEdit::TextEdit(EditorWidget& g, Buffer& b)
      : TextEditBase(g, b), e(new QTextEdit(this)) {
    this->initialize(this->e);
    auto* hl = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl, this->e);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->e->setContextMenuPolicy(Qt::NoContextMenu);
    this->setLayout(hl);
  }  // end of TextEdit::TextEdit()

  TextEdit::TextEdit(const QString& f, EditorWidget& g, Buffer& b)
      : TextEditBase(g, b), e(new QTextEdit(this)) {
    this->initialize(this->e);
    auto* hl = new QHBoxLayout;
    setQAbstractScrollAreaInLayout(hl, this->e);
    this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setLayout(hl);
    // reading the file
    QFile file(f);
    if (file.exists()) {
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("::openFile"),
                              tr("opening file '%1' failed").arg(f));
        return;
      }
      QTextStream in(&file);
      QApplication::setOverrideCursor(Qt::WaitCursor);
      this->setPlainText(in.readAll());
      QApplication::restoreOverrideCursor();
      this->document()->setModified(false);
      this->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    }
    this->setFileName(f);
  }  // end of TextEdit::TextEdit()

  QAbstractScrollArea* TextEdit::widget() { return this->e; }

  void TextEdit::undo() { this->e->undo(); }  // end of TextEdit::undo

  void TextEdit::redo() { this->e->redo(); }  // end of TextEdit::redo

  void TextEdit::cut() { this->e->cut(); }

  void TextEdit::paste() { this->e->paste(); }

  void TextEdit::selectAll() { this->e->selectAll(); }

  void TextEdit::copy() { this->e->copy(); }

  void TextEdit::clear() { this->e->clear(); }

  bool TextEdit::isReadOnly() { return this->e->isReadOnly(); }

  void TextEdit::appendPlainText(const QString& t) {
    QTextCursor tc = this->e->textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    tc.insertText(t);
  }

  void TextEdit::insertPlainText(const QString& t) {
    return this->e->insertPlainText(t);
  }

  void TextEdit::setPlainText(const QString& t) {
    return this->e->setPlainText(t);
  }

  void TextEdit::appendHtml(const QString& t) {
    QTextCursor tc = this->e->textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    tc.insertHtml(t);
  }

  void TextEdit::insertHtml(const QString& t) {
    return this->e->textCursor().insertHtml(t);
  }

  void TextEdit::setHtml(const QString& t) {
    this->clear();
    this->insertHtml(t);
  }

  QTextCursor TextEdit::cursorForPosition(const QPoint& p) const {
    return this->e->cursorForPosition(p);
  }

  QRect TextEdit::cursorRect(const QTextCursor& tc) const {
    return this->e->cursorRect(tc);
  }

  void TextEdit::setUndoRedoEnabled(bool b) {
    return this->e->setUndoRedoEnabled(b);
  }

  void TextEdit::setReadOnly(bool b) { return this->e->setReadOnly(b); }

  void TextEdit::moveCursor(QTextCursor::MoveOperation mo,
                            QTextCursor::MoveMode mm) {
    return this->e->moveCursor(mo, mm);
  }

  QTextDocument* TextEdit::document() const {
    return this->e->document();
  }  // end of TextEdit::document

  void TextEdit::setExtraSelections(const QList<QTextEdit::ExtraSelection>& l) {
    return this->e->setExtraSelections(l);
  }

  QTextCursor TextEdit::textCursor() const { return this->e->textCursor(); }

  QList<QTextEdit::ExtraSelection> TextEdit::extraSelections() const {
    return this->e->extraSelections();
  }  // end of TextEdit::extraSelections

  QRect TextEdit::cursorRect() const { return this->e->cursorRect(); }

  bool TextEdit::find(const QString& t, QTextDocument::FindFlags f) {
    return this->e->find(t, f);
  }

  bool TextEdit::isUndoRedoEnabled() const {
    return this->e->isUndoRedoEnabled();
  }

  void TextEdit::setTextCursor(const QTextCursor& tc) {
    return this->e->setTextCursor(tc);
  }

  void TextEdit::centerCursor() {}  // end of TextEdit::centerCursor

  void TextEdit::setTextInteractionFlags(Qt::TextInteractionFlags f) {
    this->e->setTextInteractionFlags(f);
  }

  void TextEdit::print() {
    QPrinter printer;
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
      this->e->print(&printer);
    }
  }  // end of TextEdit::print

  TextEdit::~TextEdit() = default;

}  // end of namespace tfel::gui
