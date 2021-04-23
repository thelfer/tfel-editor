/*!
 * \file  TextBrowser.cxx
 * \brief
 * \author Helfer Thomas
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
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/TextBrowser.hxx"

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif /* Q_OS_UNIX */

namespace tfel {

  namespace gui {

    TextBrowser::TextBrowser(EditorWidget& g, Buffer& b)
        : TextEditBase(g, b), e(new QTextBrowser(this)) {
      this->initialize(this->e);
      auto* hl = new QHBoxLayout;
      setQAbstractScrollAreaInLayout(hl, this->e);
      this->e->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      this->setLayout(hl);
    }  // end of TextBrowser::TextBrowser()

    TextBrowser::TextBrowser(const QString& f, EditorWidget& g, Buffer& b)
        : TextEditBase(g, b), e(new QTextBrowser(this)) {
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
    }  // end of TextBrowser::TextBrowser()

    QAbstractScrollArea* TextBrowser::widget() { return this->e; }

    void TextBrowser::undo() { this->e->undo(); }  // end of TextBrowser::undo

    void TextBrowser::redo() { this->e->redo(); }  // end of TextBrowser::redo

    void TextBrowser::cut() { this->e->cut(); }

    void TextBrowser::paste() { this->e->paste(); }

    void TextBrowser::selectAll() { this->e->selectAll(); }

    void TextBrowser::copy() { this->e->copy(); }

    void TextBrowser::clear() { this->e->clear(); }

    bool TextBrowser::isReadOnly() { return this->e->isReadOnly(); }

    void TextBrowser::appendPlainText(const QString& t) {
      QTextCursor tc = this->e->textCursor();
      tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
      tc.insertText(t);
    }

    void TextBrowser::insertPlainText(const QString& t) {
      return this->e->insertPlainText(t);
    }

    void TextBrowser::setPlainText(const QString& t) {
      return this->e->setPlainText(t);
    }

    void TextBrowser::appendHtml(const QString& t) {
      QTextCursor tc = this->e->textCursor();
      tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
      tc.insertHtml(t);
    }

    void TextBrowser::insertHtml(const QString& t) {
      return this->e->textCursor().insertHtml(t);
    }

    void TextBrowser::setHtml(const QString& t) {
      this->clear();
      this->insertHtml(t);
    }

    QTextCursor TextBrowser::cursorForPosition(const QPoint& p) const {
      return this->e->cursorForPosition(p);
    }

    QRect TextBrowser::cursorRect(const QTextCursor& tc) const {
      return this->e->cursorRect(tc);
    }

    void TextBrowser::setUndoRedoEnabled(bool b) {
      return this->e->setUndoRedoEnabled(b);
    }

    void TextBrowser::setReadOnly(bool b) { return this->e->setReadOnly(b); }

    void TextBrowser::moveCursor(QTextCursor::MoveOperation mo,
                                 QTextCursor::MoveMode mm) {
      return this->e->moveCursor(mo, mm);
    }

    QTextDocument* TextBrowser::document() const {
      return this->e->document();
    }  // end of TextBrowser::document

    void TextBrowser::setExtraSelections(
        const QList<QTextEdit::ExtraSelection>& l) {
      return this->e->setExtraSelections(l);
    }

    QTextCursor TextBrowser::textCursor() const {
      return this->e->textCursor();
    }

    QList<QTextEdit::ExtraSelection> TextBrowser::extraSelections() const {
      return this->e->extraSelections();
    }  // end of TextBrowser::extraSelections

    QRect TextBrowser::cursorRect() const { return this->e->cursorRect(); }

    bool TextBrowser::find(const QString& t, QTextDocument::FindFlags f) {
      return this->e->find(t, f);
    }

    bool TextBrowser::isUndoRedoEnabled() const {
      return this->e->isUndoRedoEnabled();
    }

    void TextBrowser::setTextCursor(const QTextCursor& tc) {
      return this->e->setTextCursor(tc);
    }

    void TextBrowser::setTextInteractionFlags(Qt::TextInteractionFlags f) {
      this->e->setTextInteractionFlags(f);
    }

    TextBrowser::~TextBrowser() = default;

  }  // end of namespace gui
}  // end of namespace tfel
