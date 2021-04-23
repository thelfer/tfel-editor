/*!
 * \file  TextEditReplaceFilter.cxx
 * \brief
 * \author Helfer Thomas
 * \date   06/08/2012
 */

#include <QtCore/QDebug>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/TextEditQueryReplace.hxx"
#include "TFEL/GUI/TextEditReplaceFilter.hxx"

namespace tfel {

  namespace gui {

    TextEditReplaceFilter::TextEditReplaceFilter(EditorWidget& w,
                                                 TextEditBase& t,
                                                 TextEditQueryReplace& qr_,
                                                 const QString& s1_,
                                                 const QString& s2_)
        : TextEditKeyPressEventFilter(t),
          editor(w),
          textEdit(t),
          qr(qr_),
          s1(s1_),
          s2(s2_) {
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
      }
      this->bOK = this->findNext();
      if (!this->bOK) {
        this->editor.displayInformativeMessage(
            QObject::tr("'%1' not found").arg(this->s1));
      }
    }

    bool TextEditReplaceFilter::isOk() const {
      return this->bOK;
    }  // end of TextEditReplaceFilter::isOk

    bool TextEditReplaceFilter::findNext() {
      const auto tc = this->textEdit.textCursor();
      const auto nc = this->textEdit.document()->find(this->s1, tc, this->opts);
      const auto c = [this, nc] {
        if (this->hasSelection) {
          return (!nc.isNull()) && (!nc.atEnd()) && (nc < this->ec);
        }
        return (!nc.isNull()) && (!nc.atEnd());
      }();
      if (c) {
        this->textEdit.setTextCursor(nc);
      }
      return c;
    }  // end of TextEditReplaceFilter::findNext

    bool TextEditReplaceFilter::filterKeyPressEvent(QKeyEvent* const ev) {
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_G)) {
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
            QTextCursor tc = this->textEdit.textCursor();
            tc.beginEditBlock();
            tc.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
            tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                            -s1.size());
            tc.removeSelectedText();
            tc.insertText(s2);
            tc.endEditBlock();
            tc.clearSelection();
            this->textEdit.setTextCursor(tc);
            ++nb;
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
          bool c = true;
          auto tc = this->textEdit.textCursor();
          auto* const document = this->textEdit.document();
          tc.beginEditBlock();
          while (c) {
            tc.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
            tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                            -s1.size());
            tc.removeSelectedText();
            tc.insertText(s2);
            auto nc = document->find(s1, tc, this->opts);
            if (this->hasSelection) {
              c = (!nc.isNull()) && (!nc.atEnd()) && (nc < this->ec);
            } else {
              c = (!nc.isNull()) && (!nc.atEnd());
            }
            if (c) {
              tc = nc;
            }
            ++nb;
          }
          tc.endEditBlock();
          tc.clearSelection();
          this->textEdit.setTextCursor(tc);
          this->textEdit.setMoveMode(QTextCursor::MoveAnchor);
          this->textEdit.removeKeyPressEventFilter();
        }
      } else {
        this->editor.displayInformativeMessage(
            QObject::tr("type either 'y', 'n' or '!'"));
      }
      return true;
    }

    TextEditReplaceFilter::~TextEditReplaceFilter() {
      this->editor.removeUserInput(&qr);
      if ((this->nb == 0) || (this->nb == 1)) {
        this->editor.displayInformativeMessage(
            QObject::tr("replaced %1 occurrence").arg(QString::number(nb)));
      } else {
        this->editor.displayInformativeMessage(
            QObject::tr("replaced %1 occurrences").arg(QString::number(nb)));
      }
    }

  }  // end of namespace gui
}  // end of namespace tfel
