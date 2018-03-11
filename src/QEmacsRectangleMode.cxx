/*!
 * \file  QEmacsRectangleMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/09/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <QtGui/QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextBlock>

#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/QEmacsRectangleMode.hxx"

namespace qemacs {

  struct InsertRectangle : public QEmacsCommandLine {
    InsertRectangle(QEmacsTextEditBase& t, QEmacsWidget& p)
        : QEmacsCommandLine(QObject::tr("String rectangle"), p),
          textEdit(t) {
      static QString e("/rectangle-mode/insert/histroy");
      QSettings s;
      this->setInputHistorySettingAddress(e);
      const auto ch = s.value(e).toStringList();
      if (!ch.isEmpty()) {
        this->defaultEntry = ch.back();
        this->setLabel(QObject::tr("String rectangle (default %1)")
                           .arg(this->defaultEntry));
      }
    }  // end of InsertRectangle

    void treatUserInput() override {
      QString t = this->input->text();
      QTextCursor tc = this->textEdit.textCursor();
      if (t.isEmpty()) {
        t = this->defaultEntry;
      }
      if (tc.hasSelection()) {
        QTextCursor b(tc);
        QTextCursor e(tc);
        QTextCursor lb;
        QTextCursor le;
        int pb = tc.selectionStart();
        int pe = tc.selectionEnd();
        b.setPosition(qMin(pb, pe));
        e.setPosition(qMax(pb, pe));
        lb = b;
        lb.movePosition(QTextCursor::StartOfBlock,
                        QTextCursor::MoveAnchor);
        le = e;
        le.movePosition(QTextCursor::StartOfBlock,
                        QTextCursor::MoveAnchor);
        int posb = b.position() - lb.position();
        int pose = e.position() - le.position();
        int posl = qMin(posb, pose);
        int nb = qAbs(posb - pose);
        b.beginEditBlock();
        while ((b.blockNumber() <= e.blockNumber()) && (!b.atEnd())) {
          b.movePosition(QTextCursor::StartOfBlock,
                         QTextCursor::MoveAnchor);
          const QString ct = b.block().text();
          QString start;
          QString end;
          if (ct.size() < posl) {
            start = ct.leftJustified(posl, ' ', false);
          } else {
            start = ct.mid(0, posl);
          }
          if (ct.size() > posl + nb) {
            end = ct.mid(posl + nb);
          }
          b.movePosition(QTextCursor::EndOfBlock,
                         QTextCursor::KeepAnchor);
          b.removeSelectedText();
          b.insertText(start + t + end);
          b.movePosition(QTextCursor::NextBlock,
                         QTextCursor::MoveAnchor);
        }
        b.clearSelection();
        b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor,
                       e.blockNumber());
        b.movePosition(QTextCursor::StartOfBlock,
                       QTextCursor::MoveAnchor);
        b.movePosition(QTextCursor::NextCharacter,
                       QTextCursor::MoveAnchor, posl + t.size());
        b.endEditBlock();
        this->textEdit.setTextCursor(b);
      } else {
        QTextCursor b = this->textEdit.textCursor();
        b.insertText(t);
        this->textEdit.setTextCursor(b);
      }
    }  // end of treatUserInput

   protected:
    QEmacsTextEditBase& textEdit;

    QString defaultEntry;

  };  // end of InsertRectangle

  struct QEmacsRectangleMode::RectangleModeLineEdit
      : QEmacsCommandLine::CustomLineEdit {
    RectangleModeLineEdit(QEmacsTextEditBase& t,
                          QEmacsWidget& q,
                          QEmacsCommandLine& p)
        : CustomLineEdit(q, p),
          textEdit(t) {}  // end of RectangleModeLineEdit

    void keyPressEvent(QKeyEvent* ev) override {
      auto* qle = qobject_cast<QEmacsCommandLine*>(this->parent());
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_G)) {
        if (qle != nullptr) {
          qle->cancel();
        }
        return;
      }
      if ((ev->modifiers() == Qt::NoModifier) &&
          (ev->key() == Qt::Key_T)) {
        this->setText("t");
      }
      if ((ev->modifiers() == Qt::NoModifier) &&
          (ev->key() == Qt::Key_K)) {
        this->setText("k");
      }
      if ((ev->modifiers() == Qt::NoModifier) &&
          (ev->key() == Qt::Key_Y)) {
        this->setText("y");
      }
      qle->userEditingFinished();
    }

   protected:
    QEmacsTextEditBase& textEdit;
  };

  QVector<QString> QEmacsRectangleMode::killedRectangle;

  QEmacsRectangleMode::QEmacsRectangleMode(QEmacsTextEditBase& t,
                                           QEmacsWidget& p)
      : QEmacsCommandLine(QObject::tr("Ctrl-X r "), p, false),
        textEdit(t) {
    this->setLineEdit(new RectangleModeLineEdit(t, p, *this));
  }  // end of QEmacsRectangleMode::QEmacsRectangleMode

  bool QEmacsRectangleMode::isBlocking() const {
    return true;
  }  // end of QEmacsRectangleMode::isBlocking

  void QEmacsRectangleMode::treatUserInput() {
    const QString m = this->input->text();
    QEmacsTextEditBase& t = this->textEdit;
    QEmacsWidget& q = this->qemacs;
    if (m == "t") {
      this->qemacs.removeUserInput(this);
      this->qemacs.setUserInput(new InsertRectangle(t, q));
      return;
    } else if (m == "k") {
      this->killRectangle();
    } else if (m == "y") {
      this->pasteRectangle();
    }
  }  // end of QEmacsRectangleMode::treatUserInput

  void QEmacsRectangleMode::pasteRectangle() {
    if (QEmacsRectangleMode::killedRectangle.isEmpty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("no rectangle killed"));
    }
    QTextCursor tc = this->textEdit.textCursor();
    QTextCursor b(tc);
    QTextCursor bl(tc);
    b.beginEditBlock();
    if (tc.hasSelection()) {
      int pb = tc.selectionStart();
      int pe = tc.selectionEnd();
      b.setPosition(qMax(pb, pe));
    }
    bl.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    int posl = b.position() - bl.position();
    for (auto p = QEmacsRectangleMode::killedRectangle.cbegin();
         p != QEmacsRectangleMode::killedRectangle.cend();) {
      bl.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::MoveAnchor);
      bl.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::KeepAnchor);
      QString ct = bl.selectedText();
      QString start;
      QString end;
      if (ct.size() < posl) {
        start = ct.leftJustified(posl, ' ', false);
      } else {
        start = ct.mid(0, posl);
      }
      if (ct.size() > posl) {
        end = ct.mid(posl);
      }
      bl.removeSelectedText();
      bl.insertText(start + *p + end);
      bl.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      ++p;
      if ((p != QEmacsRectangleMode::killedRectangle.end()) &&
          (bl.atEnd())) {
        bl.insertText("\n");
      }
    }
    b.endEditBlock();
    this->textEdit.setTextCursor(b);
  }

  void QEmacsRectangleMode::killRectangle() {
    QTextCursor tc = this->textEdit.textCursor();
    if (tc.hasSelection()) {
      QTextCursor b(tc);
      QTextCursor e(tc);
      QTextCursor lb;
      QTextCursor le;
      int pb = tc.selectionStart();
      int pe = tc.selectionEnd();
      b.setPosition(qMin(pb, pe));
      e.setPosition(qMax(pb, pe));
      lb = b;
      lb.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::MoveAnchor);
      le = e;
      le.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::MoveAnchor);
      int posb = b.position() - lb.position();
      int pose = e.position() - le.position();
      int posl = qMin(posb, pose);
      int nb = qAbs(posb - pose);
      if (nb == 0) {
        e.clearSelection();
        this->textEdit.setTextCursor(e);
        return;
      }
      QEmacsRectangleMode::killedRectangle.clear();
      b.beginEditBlock();
      while ((b.blockNumber() <= e.blockNumber()) && (!b.atEnd())) {
        b.movePosition(QTextCursor::StartOfBlock,
                       QTextCursor::MoveAnchor);
        const QString ct = b.block().text();
        QString start;
        QString end;
        if (ct.size() < posl) {
          start = ct.leftJustified(posl, ' ', false);
        } else {
          start = ct.mid(0, posl);
        }
        QEmacsRectangleMode::killedRectangle.push_back(
            ct.mid(posl, nb));
        if (ct.size() > posl + nb) {
          end = ct.mid(posl + nb);
        }
        b.movePosition(QTextCursor::EndOfBlock,
                       QTextCursor::KeepAnchor);
        b.removeSelectedText();
        b.insertText(start + end);
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      b.clearSelection();
      b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor,
                     e.blockNumber());
      b.movePosition(QTextCursor::StartOfBlock,
                     QTextCursor::MoveAnchor);
      b.movePosition(QTextCursor::NextCharacter,
                     QTextCursor::MoveAnchor, posl);
      b.endEditBlock();
      this->textEdit.setTextCursor(b);
    }
  }  // end of QEmacsRectangleMode::killRectange

}  // end of namespace qemacs
