/*!
 * \file  TextEditReplaceFilter.cxx
 * \brief
 * \author Thomas Helfer
 * \date   06/08/2012
 */

#include <QtCore/QObject>
#include <QtCore/QtGlobal>

#include <QtGui/QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/TextEditQueryReplace.hxx"
#include "TFEL/GUI/TextEditReplaceFilter.hxx"

namespace tfel::gui {

  TextEditReplaceFilter::TextEditReplaceFilter(EditorWidget& w,
                                               TextEditBase& t,
                                               TextEditQueryReplace& qr_,
                                               const SearchPattern& p)
      : TextEditKeyPressEventFilter(t),
        editor(w),
        textEdit(t),
        qr(qr_),
        pattern(p) {
    const auto tc = this->textEdit.textCursor();
    if (tc.hasSelection()) {
      this->hasSelection = true;
      this->bc = tc;
      this->ec = tc;
      if (tc.selectionStart() > tc.selectionEnd()) {
        this->bc.setPosition(tc.selectionEnd());
        this->ec.setPosition(tc.selectionStart());
      } else {
        this->bc.setPosition(tc.selectionStart());
        this->ec.setPosition(tc.selectionEnd());
      }
      this->bc.clearSelection();
      this->ec.clearSelection();
      this->textEdit.setTextCursor(this->bc);
    } else {
      this->hasSelection = false;
    }
    this->bOK = this->findNext();
    if (!this->bOK) {
      this->editor.displayInformativeMessage(
          QObject::tr("'%1' not found").arg(this->pattern.pattern()));
    }
  }  // end of TextEditReplaceFilter::TextEditReplaceFilter

  bool TextEditReplaceFilter::isOk() const {
    return this->bOK;
  }  // end of TextEditReplaceFilter::isOk

  bool TextEditReplaceFilter::isInsideWorkingRegion(
      const SearchMatch& m) const {
    if (m.isNull()) {
      return false;
    }
    if (this->hasSelection) {
      return m.start <= this->ec.position();
    }
    return true;
  }  // end of TextEditReplaceFilter::isInsideWorkingRegion

  int TextEditReplaceFilter::searchFrom() const {
    const auto tc = this->textEdit.textCursor();
    return qMax(tc.position(), tc.selectionEnd());
  }  // end of TextEditReplaceFilter::searchFrom

  bool TextEditReplaceFilter::findNext() {
    auto* const document = this->textEdit.document();
    if (document == nullptr) {
      return false;
    }
    const auto m = this->pattern.find(*document, this->searchFrom());
    if (!this->isInsideWorkingRegion(m)) {
      return false;
    }
    auto nc = QTextCursor(document);
    nc.setPosition(m.start);
    nc.setPosition(m.end, QTextCursor::KeepAnchor);
    this->textEdit.setTextCursor(nc);
    this->current = m;
    return true;
  }  // end of TextEditReplaceFilter::findNext

  QString TextEditReplaceFilter::replaceCurrentMatch() {
    auto* const document = this->textEdit.document();
    const auto written = this->pattern.replacementOf(this->current);
    auto tc = QTextCursor(document);
    tc.setPosition(this->current.start);
    tc.setPosition(this->current.end, QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    tc.insertText(written);
    this->textEdit.setTextCursor(tc);
    return written;
  }  // end of TextEditReplaceFilter::replaceCurrentMatch

  bool TextEditReplaceFilter::filterKeyPressEvent(QKeyEvent* const ev) {
    if ((ev->key() == Qt::Key_Shift) || (ev->key() == Qt::Key_Control) ||
        (ev->key() == Qt::Key_Alt) || (ev->key() == Qt::Key_Meta)) {
      /* a modifier pressed on its own is no answer to anything: it is what a
         chord reaches the editor as before the key of the chord does, and a walk
         which reported `type either 'y', 'n' or '!'` on the Control of a
         `Ctrl-G` would tell the reader they are wrong at the moment they ask to
         stop. The shortcut handler of the editor has the same rule. */
      return true;
    }
    if ((ev->modifiers() == Qt::ControlModifier) && (ev->key() == Qt::Key_G)) {
      QTextCursor tc = this->textEdit.textCursor();
      tc.clearSelection();
      this->textEdit.setTextCursor(tc);
      this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
      this->textEdit.removeKeyPressEventFilter();
    }
    if (((ev->modifiers() == Qt::NoModifier) && (ev->key() == Qt::Key_Y)) ||
        ((ev->modifiers() == Qt::NoModifier) && (ev->key() == Qt::Key_N)) ||
        ((ev->modifiers() == Qt::NoModifier) &&
         (ev->key() == Qt::Key_Exclam))) {
      if ((ev->key() == Qt::Key_Y) || (ev->key() == Qt::Key_N)) {
        if (ev->key() == Qt::Key_Y) {
          this->replaceCurrentMatch();
          ++this->nb;
        }
        if (!this->findNext()) {
          QTextCursor tc = this->textEdit.textCursor();
          tc.clearSelection();
          this->textEdit.setTextCursor(tc);
          this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
          this->textEdit.removeKeyPressEventFilter();
        }
      }
      if (ev->key() == Qt::Key_Exclam) {
        auto tc = this->textEdit.textCursor();
        tc.beginEditBlock();
        /* the match on the screen is replaced first: the 'y' and 'n' answers
           have been bringing the walk to it one key at a time and '!' takes it
           and the ones behind it at once, the one it replaced nothing on yet
           included. */
        if (!this->current.isNull()) {
          this->replaceCurrentMatch();
          ++this->nb;
        }
        while (this->findNext()) {
          this->replaceCurrentMatch();
          ++this->nb;
        }
        tc.endEditBlock();
        /* the cursor of the text edit stands behind the last text written, and
           only the selection of the match it no longer holds is dropped: a
           cursor built from the document would start at its first character */
        auto ended = this->textEdit.textCursor();
        ended.clearSelection();
        this->textEdit.setTextCursor(ended);
        this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
        // the destructor reports how many times the pattern was replaced
        this->textEdit.removeKeyPressEventFilter();
      }
    } else {
      this->editor.displayInformativeMessage(
          QObject::tr("type either 'y', 'n' or '!'"));
    }
    return true;
  }  // end of TextEditReplaceFilter::filterKeyPressEvent

  TextEditReplaceFilter::~TextEditReplaceFilter() {
    this->editor.removeUserInput(&qr);
    /* one occurrence is the singular, and no occurrence is the plural of
       English, which is not the reading `nb < 2` gives: the walk which was
       ended before the first `y` answers 'replaced 0 occurrences', and a
       reader who reads that as a success is reading the report wrongly. */
    this->editor.displayInformativeMessage(
        (this->nb == 1)
            ? QObject::tr("replaced %1 occurrence").arg(this->nb)
            : QObject::tr("replaced %1 occurrences").arg(this->nb));
  }  // end of TextEditReplaceFilter::~TextEditReplaceFilter

}  // end of namespace tfel::gui
