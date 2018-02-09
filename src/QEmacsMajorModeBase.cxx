/*!
 * \file  QEmacsMajorModeBase.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 04 juil. 2012
 */

#include <QtCore/QDebug>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QtGui/QTextBlock>

#include "QEmacs/QEmacsMajorModeBase.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/QEmacsWidget.hxx"

namespace qemacs {

  static int countNumberOfSpacesAtBeginningOfString(const QString& s) {
    // number of spaces at the beginning of the string
    int p;
    auto found = false;
    for(p = 0; (p != s.size()) && (!found);) {
      if(s[p].isSpace()) {
        ++p;
      } else {
        found = true;
      }
    }
    return p;
  }

  QEmacsMajorModeBase::QEmacsMajorModeBase(QEmacsWidget& w,
                                           QEmacsBuffer& b,
                                           QEmacsTextEditBase& t,
                                           QWidget* const p)
      : QEmacsMajorMode(p), qemacs(w), buffer(b), textEdit(t){}
  // end of QEmacsMajorModeBase::QEmacsMajorModeBase

  void QEmacsMajorModeBase::update() {
    emit updatedMenu();
  }  // end of QEmacsMajorModeBase::update

  QCompleter* QEmacsMajorModeBase::getCompleter() {
    return nullptr;
  } // end of QEmacsMajorModeBase::getCompleter

  QString QEmacsMajorModeBase::getCompletionPrefix() {
    return this->textEdit.getCurrentWord();
  }

  void QEmacsMajorModeBase::completeContextMenu(QMenu* const,
                                                const QTextCursor&) {
  } // end of QEmacsMajorModeBase::completeContextMenu()

  QMenu* QEmacsMajorModeBase::getSpecificMenu() {
    return nullptr;
  } // end of QEmacsMajorModeBase::getSpecificMenu

  QIcon QEmacsMajorModeBase::getIcon() const {
    return QIcon();
  } // end of QEmacsMajorModeBase::getIcon

  void QEmacsMajorModeBase::completeCurrentWord(QEmacsTextEditBase& t,
                                                const QString& w) {
    QCompleter* c = this->getCompleter();
    if(c == nullptr) {
      return;
    }
    if(c->widget() != &t) {
      return;
    }
    QTextCursor tc = t.textCursor();
    int extra      = w.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(w.right(extra));
    tc.insertText(" ");
    t.setTextCursor(tc);
  } // end of QEmacsMajorModeBase::completeCurrentWord

  bool QEmacsMajorModeBase::keyPressEvent(QKeyEvent* const) {
    return false;
  } // end of QEmacsMajorModeBase::keyPressEvent

  bool QEmacsMajorModeBase::mousePressEvent(QMouseEvent* const) {
    return false;
  } // end of QEmacsMajorModeBase::mousePressEvent

  bool QEmacsMajorModeBase::handleShortCut(const int,
                                           const Qt::KeyboardModifiers,
                                           const int) {
    return false;
  } // end of QEmacsMajorModeBase::handleShortCut

  int QEmacsMajorModeBase::getMinimalCompletionLength() {
    return 3;
  }

  void QEmacsMajorModeBase::indentLine(const QTextCursor& c) {
    QTextCursor tc(c);
    tc.beginEditBlock();
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    tc.insertText("\t");
    tc.endEditBlock();
  } // end of QEmacsMajorModeBase::indentLine

  void QEmacsMajorModeBase::indentRegion(const QTextCursor&) {
  } // end of QEmacsMajorModeBase::indentRegion

  void QEmacsMajorModeBase::setSpellCheckLanguage(const QString&) {
  } // end of QEmacsMajorModeBase::setSpellCheckLanguage

  void QEmacsMajorModeBase::beginAndEndOfSelection(
      QTextCursor& b, QTextCursor& e, const QTextCursor& tc) const {
    b = tc;
    e = tc;
    if(tc.selectionStart() > tc.selectionEnd()) {
      b.setPosition(tc.selectionEnd());
      e.setPosition(tc.selectionStart());
    } else {
      b.setPosition(tc.selectionStart());
      e.setPosition(tc.selectionEnd());
    }
    b.clearSelection();
    e.clearSelection();
  }

  void QEmacsMajorModeBase::findBeginningAndEndOfTheParagraph(
      QTextCursor& b, QTextCursor& e) const {
    b = this->textEdit.textCursor();
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    e          = b;
    bool found = b.block().text().trimmed().isEmpty();
    while((!b.atStart()) && (!found)) {
      b.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
      found = b.block().text().trimmed().isEmpty();
      if(found) {
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
    }
    e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    if(e.atEnd()) {
      return;
    }
    found = e.block().text().trimmed().isEmpty();
    while((!e.atEnd()) && (!found)) {
      found = e.block().text().trimmed().isEmpty();
      if(found) {
        e.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
        e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      } else {
        e.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      }
    }
  } // end of QEmacsMajorModeBase::findBeginningAndEndOfTheParagraph

  void QEmacsMajorModeBase::doIndentLine(const QTextCursor& tc,
                                         const int i) const {
    QTextCursor nc(tc);
    nc.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
    QString l = nc.block().text();
    int pos   = nc.position();
    int p     = countNumberOfSpacesAtBeginningOfString(l);
    if(i > 0) {
      QString indent = QString(i, ' ');
      l              = QString(i, ' ') + l.trimmed();
    } else {
      l = l.trimmed();
    }
    nc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pos -= nc.position();
    pos -= p;
    nc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    nc.removeSelectedText();
    nc.clearSelection();
    nc.insertText(l);
    nc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    if(i > 0) {
      nc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, i);
    }
    if(pos >= 0) {
      nc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos);
    }
    this->textEdit.setTextCursor(nc);
  } // end of QEmacsMajorModeBase::doIndentLine

  QVector<QString>
  QEmacsMajorModeBase::getSelectedLines(const QTextCursor& tc) const {
    QVector<QString> l;
    QTextCursor b;
    QTextCursor e;
    if(!tc.hasSelection()) {
      return l;
    }
    this->beginAndEndOfSelection(b, e, tc);
    if(b == e) {
      return l;
    }
    QTextCursor end(e);
    end.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    while((b < end) && (!b.atEnd())) {
      QTextCursor be(b);
      be.clearSelection();
      be.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      if(be >= e) {
        be = b;
        be.clearSelection();
        be.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                        e.position() - b.position());
      }
      l.push_back(be.selectedText());
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    if(b == e) {
      l.push_back(QString());
    } else if(b < e) {
      b.clearSelection();
      b.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                     e.position() - b.position());
      l.push_back(b.selectedText());
    }
    return l;
  } // end of QEmacsMajorModeBase::getSelectedLines

  void QEmacsMajorModeBase::comment() {
    QString cs = this->getCommentSyntax();
    if(cs.isEmpty()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("no comment syntax defined"));
      return;
    }
    QTextCursor tc = this->textEdit.textCursor();
    if(tc.hasSelection()) {
      QTextCursor b;
      QTextCursor e;
      QTextCursor tmpb;
      this->beginAndEndOfSelection(b, e, tc);
      // position of the end of the selection
      int pi = e.position();
      // looking if the selected area is commented
      b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      bool commented = true;
      tmpb           = b;
      while((!b.atEnd()) && (b < e) && (commented)) {
        if(!b.block().text().trimmed().startsWith(cs)) {
          commented = false;
        }
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      e.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      b = tmpb;
      b.beginEditBlock();
      int i  = 0;
      int pe = e.position();
      if(commented) {
        while((!b.atEnd()) && (b.position() < pe + i)) {
          // remove spaces
          QString l = b.block().text();
          int li    = 0;
          while((li < l.size()) && (l[li].isSpace())) {
            ++li;
          }
          if(l.mid(li).startsWith(cs)) {
            b.clearSelection();
            b.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                           li + cs.size());
            b.removeSelectedText();
            i -= li + cs.size();
          }
          b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
      } else {
        int p = pi - pe;
        while((!b.atEnd()) && (b.position() < pe + i)) {
          b.insertText(cs);
          i += cs.size();
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
        if(pe != pi) {
          b.setPosition(pe + i);
          QString l = b.block().text();
          if(!l.mid(0, p).trimmed().isEmpty()) {
            b.insertText(cs);
            i += cs.size();
            b.setPosition(pi + i);
            if(!b.atBlockEnd()) {
              b.insertText("\n");
            }
          }
        }
      }
      b.endEditBlock();
      this->textEdit.setTextCursor(b);
    } else {
      tc.beginEditBlock();
      tc.insertText(cs);
      this->indentLine(tc);
      tc.endEditBlock();
      this->textEdit.setTextCursor(tc);
    }
  } // end of QEmacsMajorModeBase::comment

  QString QEmacsMajorModeBase::getCommentSyntax() {
    return "";
  } // end of QEmacsMajorModeBase::getCommentSyntax

  int QEmacsMajorModeBase::positionInCurrentBlock(const QTextCursor& tc) const {
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    return tc.position() - b.position();
  } // end of QEmacsMajorModeBase::positionInCurrentBlock

  QStringList QEmacsMajorModeBase::findFiles(const QString& d,
                                             const QString& f,
                                             const int m,
                                             const int s) {
    QStringList r;
    if((m <= 0) || (s < 0) || (s > m)) {
      return QStringList();
    }
    QDir dir(d);
    if(!dir.exists()) {
      return QStringList();
    }
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.size(); ++i) {
      const auto& fi = list.at(i);
      if((fi.isFile()) && (fi.fileName() == f)) {
        r.append(d + QDir::separator() + f);
      } else if(fi.isDir()) {
        if((fi.fileName() != ".") && (fi.fileName() != "..")) {
          QString dn(d + QDir::separator() + fi.fileName());
          r.append(this->findFiles(dn, f, m, s + 1));
        }
      }
    }
    return r;
  }

  void QEmacsMajorModeBase::report(const QString& m) {
    this->qemacs.displayInformativeMessage(m);
  }

  QEmacsMajorModeBase::~QEmacsMajorModeBase() = default;

} // end of namespace qemacs
