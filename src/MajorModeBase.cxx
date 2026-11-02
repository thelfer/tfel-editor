/*!
 * \file  MajorModeBase.cxx
 * \brief
 * \author Thomas Helfer
 * \date   04/08/2012
 */

#include <QtCore/QDebug>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QtGui/QTextBlock>

#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/EditorWidget.hxx"

namespace tfel::gui {

  static int countNumberOfSpacesAtBeginningOfString(const QString& s) {
    // number of spaces at the beginning of the string
    int p;
    auto found = false;
    for (p = 0; (p != s.size()) && (!found);) {
      if (s[p].isSpace()) {
        ++p;
      } else {
        found = true;
      }
    }
    return p;
  }

  MajorModeBase::MajorModeBase(EditorWidget& w,
                               Buffer& b,
                               TextEditBase& t,
                               QWidget* const p)
      : MajorMode(p), editor(w), buffer(b), textEdit(t) {}
  // end of MajorModeBase::MajorModeBase

  void MajorModeBase::update() {
    emit updatedMenu();
  }  // end of MajorModeBase::update

  QCompleter* MajorModeBase::getCompleter() {
    return nullptr;
  }  // end of MajorModeBase::getCompleter

  QString MajorModeBase::getCompletionPrefix() {
    return this->textEdit.getCurrentWord();
  }

  void MajorModeBase::completeContextMenu(QMenu* const, const QTextCursor&) {
  }  // end of MajorModeBase::completeContextMenu()

  QMenu* MajorModeBase::getSpecificMenu() {
    return nullptr;
  }  // end of MajorModeBase::getSpecificMenu

  QIcon MajorModeBase::getIcon() const {
    return QIcon();
  }  // end of MajorModeBase::getIcon

  void MajorModeBase::completeCurrentWord(TextEditBase& t, const QString& w) {
    QCompleter* c = this->getCompleter();
    if (c == nullptr) {
      return;
    }
    if (c->widget() != &t) {
      return;
    }
    QTextCursor tc = t.textCursor();
    int extra = w.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(w.right(extra));
    tc.insertText(" ");
    t.setTextCursor(tc);
  }  // end of MajorModeBase::completeCurrentWord

  bool MajorModeBase::keyPressEvent(QKeyEvent* const) {
    return false;
  }  // end of MajorModeBase::keyPressEvent

  bool MajorModeBase::mousePressEvent(QMouseEvent* const) {
    return false;
  }  // end of MajorModeBase::mousePressEvent

  bool MajorModeBase::handleShortCut(const int,
                                     const Qt::KeyboardModifiers,
                                     const int) {
    return false;
  }  // end of MajorModeBase::handleShortCut

  int MajorModeBase::getMinimalCompletionLength() { return 3; }

  void MajorModeBase::indentAll() {
    QTextCursor tc(this->textEdit.textCursor());
    tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    this->indentRegion(tc);
  }  // end of MajorModeBase::indentAll

  void MajorModeBase::indentLine(const QTextCursor& c) {
    QTextCursor tc(c);
    tc.beginEditBlock();
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    tc.insertText("\t");
    tc.endEditBlock();
  }  // end of MajorModeBase::indentLine

  void MajorModeBase::indentRegion(const QTextCursor&) {
  }  // end of MajorModeBase::indentRegion

  void MajorModeBase::setSpellCheckLanguage(const QString&) {
  }  // end of MajorModeBase::setSpellCheckLanguage

  void MajorModeBase::beginAndEndOfSelection(QTextCursor& b,
                                             QTextCursor& e,
                                             const QTextCursor& tc) const {
    b = tc;
    e = tc;
    if (tc.selectionStart() > tc.selectionEnd()) {
      b.setPosition(tc.selectionEnd());
      e.setPosition(tc.selectionStart());
    } else {
      b.setPosition(tc.selectionStart());
      e.setPosition(tc.selectionEnd());
    }
    b.clearSelection();
    e.clearSelection();
  }

  void MajorModeBase::findBeginningAndEndOfTheParagraph(QTextCursor& b,
                                                        QTextCursor& e) const {
    b = this->textEdit.textCursor();
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    e = b;
    bool found = b.block().text().trimmed().isEmpty();
    while ((!b.atStart()) && (!found)) {
      b.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
      found = b.block().text().trimmed().isEmpty();
      if (found) {
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
    }
    e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    if (e.atEnd()) {
      return;
    }
    found = e.block().text().trimmed().isEmpty();
    while ((!e.atEnd()) && (!found)) {
      found = e.block().text().trimmed().isEmpty();
      if (found) {
        e.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
        e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      } else {
        e.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      }
    }
    // when the paragraph reaches the end of the document, the loop above
    // can stop on the last block without testing it, as the atEnd check
    // triggers first. If this last block is empty (which is the case when
    // the file ends with a line break), the paragraph ends on the previous
    // block: stopping on the empty last block would make the callers
    // consume the last line break of the file, removing a blank line.
    if ((!b.block().text().trimmed().isEmpty()) &&
        (e.block().text().trimmed().isEmpty())) {
      e.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
      e.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }
  }  // end of MajorModeBase::findBeginningAndEndOfTheParagraph

  void MajorModeBase::doIndentLine(const QTextCursor& tc,
                                   const int i,
                                   const bool updateEditorCursor) const {
    QTextCursor nc(tc);
    nc.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
    QString l = nc.block().text();
    int pos = nc.position();
    int p = countNumberOfSpacesAtBeginningOfString(l);
    if (i > 0) {
      QString indent = QString(i, ' ');
      l = QString(i, ' ') + l.trimmed();
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
    if (i > 0) {
      nc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, i);
    }
    if (pos >= 0) {
      nc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos);
    }
    if (updateEditorCursor) {
      this->textEdit.setTextCursor(nc);
    }
  }  // end of MajorModeBase::doIndentLine

  MajorModeBase::CursorMark MajorModeBase::getCursorMark(
      const QTextCursor& tc) const {
    return {tc.blockNumber(), tc.positionInBlock(),
            countNumberOfSpacesAtBeginningOfString(tc.block().text())};
  }  // end of MajorModeBase::getCursorMark

  void MajorModeBase::restoreCursorMark(const CursorMark& m) const {
    auto tc = QTextCursor(this->textEdit.document());
    const auto block =
        this->textEdit.document()->findBlockByNumber(m.blockNumber);
    const auto l = block.text();
    auto column = m.column;
    if (!l.trimmed().isEmpty()) {
      // as in doIndentLine, the cursor is attached to the first non
      // blank character of the line if it was in the blanks at the
      // beginning of the line. Blank lines are not touched by the
      // indentation algorithms: their column is preserved.
      column = countNumberOfSpacesAtBeginningOfString(l) +
               qMax(m.column - m.firstNonBlankColumn, 0);
    }
    tc.setPosition(block.position() + qMin(column, l.size()));
    this->textEdit.setTextCursor(tc);
  }  // end of MajorModeBase::restoreCursorMark

  MajorModeBase::AnchorMark MajorModeBase::getAnchorMark(
      const QTextCursor& b, const QTextCursor& c) const {
    auto m = AnchorMark{};
    auto tc = QTextCursor(this->textEdit.document());
    tc.setPosition(b.position());
    tc.setPosition(c.position(), QTextCursor::KeepAnchor);
    // `QTextCursor::selectedText` represents the line breaks of the selection
    // by a paragraph separator, which is a blank character: counting the non
    // blank characters of the selection is all what is needed, no matter how
    // the region is spread over the lines.
    for (const auto& cl : tc.selectedText()) {
      if (!cl.isSpace()) {
        ++m.nonBlanks;
      }
    }
    // The character on the left of the cursor. A cursor standing at the end of
    // a line is on the right of the last character of this line and not on the
    // left of the first character of the next one: the line break, which is a
    // blank character, separates them. A cursor in the blanks at the beginning
    // of its line is attached to the first non blank character of this line, as
    // `doIndentLine` does.
    const auto l = tc.block().text();
    const auto p = tc.positionInBlock();
    m.leftOfWord = ((p < l.size()) && (!l[p].isSpace())) ||
                   (p < countNumberOfSpacesAtBeginningOfString(l));
    return m;
  }  // end of MajorModeBase::getAnchorMark

  void MajorModeBase::restoreAnchorMark(const AnchorMark& m,
                                        const int first_,
                                        const int last_) const {
    const auto first = qMin(first_, last_);
    const auto last = qMax(first_, last_);
    // `selectedText` maps its characters one to one to the positions of the
    // document: the index of a character in the selection is its offset from
    // the beginning of the region.
    auto tc = QTextCursor(this->textEdit.document());
    tc.setPosition(first);
    tc.setPosition(last, QTextCursor::KeepAnchor);
    const auto l = tc.selectedText();
    auto n = int{};    // number of non blank characters met so far
    auto k = int{-1};  // index of the last non blank character met so far
    auto p = int{0};   // index of the visited character
    for (; p != l.size(); ++p) {
      if (l[p].isSpace()) {
        continue;
      }
      ++n;
      k = p;
      // the cursor goes on the left of the non blank character which follows
      // the `nonBlanks` characters preceding it, or on its right when it did
      // not stand on the left of a non blank character. `nonBlanks` being
      // null, the two cases match: this is the cursor which was in the blanks
      // at the beginning of the region, and it is attached to the first non
      // blank character of this region, as `doIndentLine` does.
      if ((n > m.nonBlanks) || ((n == m.nonBlanks) && (!m.leftOfWord))) {
        break;
      }
    }
    if (p == l.size()) {
      // the reformatted region holds less non blank characters than the mark
      // expects: the cursor is placed on the right of the last non blank
      // character of the region, or at its beginning when it holds none.
      p = qMax(k + 1, 0);
    } else if (!m.leftOfWord && (m.nonBlanks != 0)) {
      // the cursor did not stand on the left of a non blank character: it goes
      // on the right of the last non blank character which precedes it
      p = k + 1;
    }
    auto nc = QTextCursor(this->textEdit.document());
    nc.setPosition(first + qMin(p, last - first));
    this->textEdit.setTextCursor(nc);
  }  // end of MajorModeBase::restoreAnchorMark

  QVector<QString> MajorModeBase::getSelectedLines(
      const QTextCursor& tc) const {
    QVector<QString> l;
    QTextCursor b;
    QTextCursor e;
    if (!tc.hasSelection()) {
      return l;
    }
    this->beginAndEndOfSelection(b, e, tc);
    if (b == e) {
      return l;
    }
    QTextCursor end(e);
    end.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    while ((b < end) && (!b.atEnd())) {
      QTextCursor be(b);
      be.clearSelection();
      be.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      if (be >= e) {
        be = b;
        be.clearSelection();
        be.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                        e.position() - b.position());
      }
      l.push_back(be.selectedText());
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    if (b == e) {
      l.push_back(QString());
    } else if (b < e) {
      b.clearSelection();
      b.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                     e.position() - b.position());
      l.push_back(b.selectedText());
    }
    return l;
  }  // end of MajorModeBase::getSelectedLines

  void MajorModeBase::comment() {
    QString cs = this->getCommentSyntax();
    if (cs.isEmpty()) {
      this->editor.displayInformativeMessage(
          QObject::tr("no comment syntax defined"));
      return;
    }
    QTextCursor tc = this->textEdit.textCursor();
    if (tc.hasSelection()) {
      QTextCursor b;
      QTextCursor e;
      QTextCursor tmpb;
      this->beginAndEndOfSelection(b, e, tc);
      // position of the end of the selection
      int pi = e.position();
      // looking if the selected area is commented
      b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      bool commented = true;
      tmpb = b;
      while ((!b.atEnd()) && (b < e) && (commented)) {
        if (!b.block().text().trimmed().startsWith(cs)) {
          commented = false;
        }
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      e.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      b = tmpb;
      b.beginEditBlock();
      int i = 0;
      int pe = e.position();
      if (commented) {
        while ((!b.atEnd()) && (b.position() < pe + i)) {
          // remove spaces
          QString l = b.block().text();
          int li = 0;
          while ((li < l.size()) && (l[li].isSpace())) {
            ++li;
          }
          if (l.right(l.size() - li).startsWith(cs)) {
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
        while ((!b.atEnd()) && (b.position() < pe + i)) {
          b.insertText(cs);
          i += cs.size();
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        }
        if (pe != pi) {
          b.setPosition(pe + i);
          QString l = b.block().text();
          if (!l.mid(0, p).trimmed().isEmpty()) {
            b.insertText(cs);
            i += cs.size();
            b.setPosition(pi + i);
            if (!b.atBlockEnd()) {
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
  }  // end of MajorModeBase::comment

  QString MajorModeBase::getCommentSyntax() {
    return "";
  }  // end of MajorModeBase::getCommentSyntax

  int MajorModeBase::positionInCurrentBlock(const QTextCursor& tc) const {
    QTextCursor b(tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    return tc.position() - b.position();
  }  // end of MajorModeBase::positionInCurrentBlock

  QStringList MajorModeBase::findFiles(const QString& d,
                                       const QString& f,
                                       const int m,
                                       const int s) {
    QStringList r;
    if ((m <= 0) || (s < 0) || (s > m)) {
      return QStringList();
    }
    QDir dir(d);
    if (!dir.exists()) {
      return QStringList();
    }
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
      const auto& fi = list.at(i);
      if ((fi.isFile()) && (fi.fileName() == f)) {
        r.append(d + QDir::separator() + f);
      } else if (fi.isDir()) {
        if ((fi.fileName() != ".") && (fi.fileName() != "..")) {
          QString dn(d + QDir::separator() + fi.fileName());
          r.append(this->findFiles(dn, f, m, s + 1));
        }
      }
    }
    return r;
  }

  void MajorModeBase::report(const QString& m) {
    this->editor.displayInformativeMessage(m);
  }

  MajorModeBase::~MajorModeBase() = default;

}  // end of namespace tfel::gui
