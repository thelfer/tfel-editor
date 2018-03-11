/*!
 * \file   QEmacsLineEdit.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   10/03/2018
 */

#include <QtGui/QKeyEvent>
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsShortCutStyle.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"

namespace qemacs {

  QEmacsLineEdit::QEmacsLineEdit(QEmacsWidget& q, QWidget* const p)
      : QLineEdit(p),
        qemacs(q) {}  // end ofQEmacsLineEdit::QEmacsLineEdit

  void QEmacsLineEdit::keyPressEvent(QKeyEvent* ev) {
    if (QEmacsShortCutStyle::getQEmacsShortCutStyle().getStyle() ==
        QEmacsShortCutStyle::QT) {
      QLineEdit::keyPressEvent(ev);
      return;
    }
    const auto k = ev->key();
    const auto m = ev->modifiers();
    const auto ctrl = m == Qt::ControlModifier;
    const auto alt = m == Qt::AltModifier;
    if (!(((ctrl) && (k == Qt::Key_P)) ||
          ((ctrl) && (k == Qt::Key_N)) ||
          ((m == Qt::NoModifier) && (k == Qt::Key_Up)) ||
          ((m == Qt::NoModifier) && (k == Qt::Key_Down)))) {
      this->hMove = false;
    }
    if (!(((ctrl) && (k == Qt::Key_Y)) ||
          ((alt) && (k == Qt::Key_Y)))) {
      this->yank = false;
    }
    if (((ctrl) && (k == Qt::Key_X)) && (!this->ctrlx) &&
        (!this->ctrlc)) {
      this->ctrlx = true;
      return;
    }
    if (((ctrl) && (k == Qt::Key_C)) && (!this->ctrlx) &&
        (!this->ctrlc)) {
      this->ctrlc = true;
      return;
    }
    if (this->ctrlx) {
      this->handleShortCut(Qt::Key_X, ev->modifiers(), k);
      this->ctrlx = false;
      return;
    }
    if (this->ctrlc) {
      this->handleShortCut(Qt::Key_C, ev->modifiers(), k);
      this->ctrlc = false;
      return;
    }
    if (((ctrl) && (k == Qt::Key_P)) || ((ctrl) && (k == Qt::Key_N)) ||
        ((m == Qt::NoModifier) && (k == Qt::Key_Up)) ||
        ((m == Qt::NoModifier) && (k == Qt::Key_Down))) {
      const auto h = this->getInputHistory();
      if (h.isEmpty()) {
        this->qemacs.displayInformativeMessage(
            QObject::tr("history is empty"));
        return;
      }
      if (this->hMove) {
        if (((ctrl) && (k == Qt::Key_P)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Up))) {
          --(this->pHistory);
          if (this->pHistory < 0) {
            this->pHistory = h.length() - 1;
          }
        }
        if (((ctrl) && (k == Qt::Key_N)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Down))) {
          ++(this->pHistory);
          if (this->pHistory >= h.length()) {
            this->pHistory = 0;
          }
        }
      } else {
        if (((ctrl) && (k == Qt::Key_P)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Up))) {
          this->pHistory = h.length() - 1;
        }
        if (((ctrl) && (k == Qt::Key_N)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Down))) {
          this->pHistory = 0;
        }
      }
      this->setText(h[this->pHistory]);
      this->hMove = true;
      return;
    }
    if ((ctrl) && (k == Qt::Key_Y)) {
      const auto& ring = this->qemacs.getKillRing();
      if (!ring.isEmpty()) {
        this->yank = true;
        this->pring = ring.size() - 1;
        this->insert(ring[this->pring]);
      }
      return;
    }
    if ((ctrl) && (k == Qt::Key_H)) {
      this->selectAll();
      return;
    }
    if ((ctrl) && (k == Qt::Key_Space)) {
      this->setSelection(this->cursorPosition(), 0);
      return;
    }
    if ((ctrl) && (k == Qt::Key_A)) {
      this->home(this->selectionStart() != -1);
      return;
    }
    if ((ctrl) && (k == Qt::Key_E)) {
      this->end(this->selectionStart() != -1);
      return;
    }
    if ((ctrl) && (k == Qt::Key_B)) {
      this->cursorBackward(this->selectionStart() != -1);
      return;
    }
    if ((ctrl) && (k == Qt::Key_F)) {
      this->cursorForward(this->selectionStart() != -1);
      return;
    }
    if ((alt) && (k == Qt::Key_Y)) {
      const QStringList& ring = this->qemacs.getKillRing();
      if ((ring.isEmpty()) || (!this->yank)) {
        if (ring.isEmpty()) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("kill ring is empty"));
        }
        if (!this->yank) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("previous command was not a yank"));
        }
        return;
      }
      int s = ring[this->pring].size();
      --(this->pring);
      if (this->pring == -1) {
        this->pring = ring.size();
        --(this->pring);
      }
      this->cursorBackward(true, s);
      this->backspace();
      this->insert(ring[this->pring]);
      this->yank = true;
      return;
    }
    if ((alt) && (k == Qt::Key_B)) {
      this->cursorWordBackward(this->selectionStart() != -1);
      return;
    }
    if ((alt) && (k == Qt::Key_F)) {
      this->cursorWordForward(this->selectionStart() != -1);
      return;
    }
    QLineEdit::keyPressEvent(ev);
  }  // end of QEmacsLineEdit::keyPressEvent

  void QEmacsLineEdit::handleShortCut(const int,
                                      const Qt::KeyboardModifiers,
                                      const int) {
    this->qemacs.displayInformativeMessage(
        QObject::tr("unknown shortcut"));
  } // end of QEmacsLineEdit::handleShortCut

  void QEmacsLineEdit::setInputHistory(const QStringList& h) {
    this->inputHistory = h;
  } // end of QEmacsLineEdit::setInputHistory

  QStringList QEmacsLineEdit::getInputHistory() const {
    return this->inputHistory;
  }  // end of QEmacsCommandLine::CustomLineEdit::getInputHistory

  QEmacsLineEdit::~QEmacsLineEdit() = default;

}  // end of namespace qemacs