/*!
 * \file  CMajorModeBase.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryFile>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocumentWriter>

#include "QEmacs/CMajorModeBase.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "TFEL/Utilities/CxxTokenizer.hxx"

namespace qemacs {

  CMajorModeBase::CMajorModeBase(QEmacsWidget& w,
                                 QEmacsBuffer& b,
                                 QEmacsTextEditBase& t)
      : CompiledLanguageMajorModeBase(w, b, t) {
  } // end of CMajorModeBase::CMajorModeBase

  void CMajorModeBase::format() {
    auto c = this->textEdit.textCursor();
    auto b = c;
    auto e = c;
    b.movePosition(QTextCursor::Start);
    e.movePosition(QTextCursor::End);
    this->indent(b, e, c);
  } // end of CMajorModeBase::format

  void CMajorModeBase::comment() {
    using tfel::utilities::Token;
    using tfel::utilities::CxxTokenizer;
    QTextCursor tc = this->textEdit.textCursor();
    if(tc.hasSelection()) {
      QTextCursor tcb, tce, btmp;
      this->beginAndEndOfSelection(tcb, tce, tc);
      auto b = tcb;
      auto e = tce;
      if(b == e) {
        return;
      }
      // check if we shall comment and or uncomment the given region
      bool uncomment = false;
      btmp           = b;
      b              = tc;
      QTextCursor bl(tc);
      b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      CxxTokenizer tokenizer;
      while(b < bl) {
        try {
          tokenizer.parseString(b.block().text().toStdString());
        } catch(...) {
          return;
        }
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      // here we have the state of the beginning of the selection
      CxxTokenizer::TokensContainer stokens; // selected tokens
      if(!tokenizer.isCStyleCommentOpened()) {
        QVector<QString> lines = this->getSelectedLines(tc);
        unsigned short lNumber = 0u;
        uncomment              = true;
        if(lines.isEmpty()) {
          uncomment = false;
        }
        for(const QString& line : lines) {
          try {
            tokenizer.parseString(line.toStdString());
          } catch(...) {
            return;
          }
          auto p        = tokenizer.begin();
          const auto pe = tokenizer.end();
          if(p == pe) {
            stokens.push_back(Token("/**/", lNumber, 0, Token::Comment));
          }
          while((p != pe) && (uncomment)) {
            if(p->flag != Token::Comment) {
              uncomment = false;
            } else {
              stokens.push_back(*p);
            }
            ++p;
          }
          ++lNumber;
        }
      }
      b = btmp;
      if(uncomment) {
        // uncomment
        auto p        = stokens.begin();
        const auto pe = stokens.end();
        tc.beginEditBlock();
        tc.removeSelectedText();
        while(p != pe) {
          if(p == pe) {
            return;
          }
          const auto& w = QString::fromStdString(p->value);
          if(p != stokens.begin()) {
            auto p2 = p;
            --p2;
            if(p2->line != p->line) {
              tc.insertText("\n");
            } else {
              int s = p->offset - p2->offset - p2->value.size();
              if(s > 0) {
                tc.insertText(QString(s, ' '));
              }
            }
          }
          if((w[0] == '/') && (w[1] == '/')) {
            // C++ comment
            if((w.size() >= 3) && (w[2] == ' ')) {
              tc.insertText(w.mid(3, w.size() - 3));
            } else {
              tc.insertText(w.mid(2, w.size() - 2));
            }
          } else if((w[0] == '/') && (w[1] == '*')) {
            // C comment
            tc.insertText(w.mid(2, w.size() - 4));
          }
          ++p;
        }
        tc.endEditBlock();
        this->textEdit.setTextCursor(tc);
      } else {
        // comment
        b.beginEditBlock();
        while((b < e) && (!b.atEnd())) {
          int d = e.position() - b.position();
          if((b.blockNumber() == e.blockNumber()) && (!e.atBlockStart())) {
            b.insertText("// ");
            b.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                           d);
            b.insertText("\n");
            break;
          } else {
            b.insertText("// ");
            e = b;
            e.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                           d);
          }
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
          b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        }
        b.endEditBlock();
        this->textEdit.setTextCursor(b);
      }
      this->indent(tcb, tce, tce);
    } else {
      QTextCursor b(tc);
      QTextCursor bl(tc);
      b.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      bl.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      CxxTokenizer tokenizer;
      while(b < bl) {
        try {
          tokenizer.parseString(b.block().text().toStdString());
        } catch(...) {
          return;
        }
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      // tokenizer has the state at the beginning of the line
      QString l = b.block().text();
      if(tc.position() - bl.position() > 0) {
        try {
          tokenizer.parseString(
              l.mid(0, tc.position() - bl.position()).toStdString());
        } catch(...) {
          return;
        }
      }
      if(tokenizer.isCStyleCommentOpened()) {
        return;
      }
      if(tokenizer.begin() != tokenizer.end()) {
        const auto& t = *(tokenizer.end());
        if(t.flag == Token::Comment) {
          return;
        }
      }
      b = tc;
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      if(tc != b) {
        try {
          tokenizer.parseString(
              l.mid(b.position(), tc.position() - b.position()).toStdString());
        } catch(...) {
          return;
        }
      }
      if(tokenizer.isCStyleCommentOpened()) {
        this->textEdit.setTextCursor(tc);
        return;
      }
      if(tokenizer.begin() != tokenizer.end()) {
        const Token& t = *(tokenizer.end());
        if(t.flag == Token::Comment) {
          this->textEdit.setTextCursor(tc);
          return;
        }
      }
      if(l.isEmpty()) {
        tc.insertText("// ");
      } else {
        if(l[l.size() - 1].isSpace()) {
          tc.insertText("// ");
        } else {
          tc.insertText(" // ");
        }
      }
      this->textEdit.setTextCursor(tc);
    }
  } // end of CMajorModeBase::comment

  static void do_indent(QEmacsWidget& qemacs,
                        QEmacsTextEditBase& textEdit,
                        QTextCursor& c,
                        const QTextCursor& b,
                        const QTextCursor& e) {
    QRegExp rcursor("(\\d+)");
    const auto cf = textEdit.getFileName();
    QTemporaryFile tmp(cf);
    if (!tmp.open()) {
      qemacs.displayInformativeMessage(
          QObject::tr("can't open temporary file"));
      return;
    }
    QTextDocumentWriter writer;
    writer.setFormat("plaintext");
    QFile tmp2(tmp.fileName());
    writer.setDevice(&tmp2);
    if(!writer.write(textEdit.document())) {
      qemacs.displayInformativeMessage(
          QObject::tr("can't write buffer in temporary file"));
      return;
    }
    tmp2.close();
    QProcess cformat;
    cformat.setWorkingDirectory(QFileInfo(cf).canonicalPath());
    const auto ln = b.blockNumber();
    const auto lb = QString::number(ln + 1);
    const auto le = QString::number(e.blockNumber() + 1);
    cformat.start("clang-format",
                  QStringList()
                      << "-assume-filename=cxx"
                      << ("-lines=" + lb + ':' + le) << ("-style=file")
                      << ("-cursor=" + QString::number(c.position()))
                      << tmp.fileName());
    if (!cformat.waitForStarted()) {
      qemacs.displayInformativeMessage(
          QObject::tr("call to 'clang-format' failed"));
      return;
    }
    cformat.closeWriteChannel();
    if(!cformat.waitForFinished()) {
      qemacs.displayInformativeMessage(
          QObject::tr("call to 'clang-format' failed"));
      return;
    }
    QTextStream in(cformat.readAll());
    auto pr = in.readLine();
    if(rcursor.indexIn(pr, 0) == -1) {
      qemacs.displayInformativeMessage(
          QObject::tr("analysis of 'clang-format' output failed"));
      return;
    }
    const auto cpos = rcursor.cap(1).toInt();
    c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    c.insertText(in.readAll());
    c.setPosition(cpos);
  }  // end of do_indent

  void CMajorModeBase::indent(const QTextCursor& b,
                              const QTextCursor& e,
                              const QTextCursor& c) {
    const auto keepEmptyLine = [&b, &e, &c] {
      if(!((c == b) && (c == e))) {
        return false;
      }
      auto tc = c;
      tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      return tc.selectedText().trimmed().isEmpty();
    }();
    if (keepEmptyLine) {
      auto tc = c;
      // current line is empty
      while (true) {
        tc.movePosition(QTextCursor::StartOfBlock,
                        QTextCursor::MoveAnchor);
        if (tc.atStart()) {
          break;
        }
        tc.movePosition(QTextCursor::EndOfBlock,
                        QTextCursor::KeepAnchor);
        if (!tc.selectedText().trimmed().isEmpty()) {
          break;
        }
        tc.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
      }
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
      tc.beginEditBlock();
      do_indent(this->qemacs, this->textEdit, tc, tc, tc);
      auto tc2 = tc;
      tc2.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::MoveAnchor);
      tc2.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      const auto l = tc2.selectedText();
      if (l.trimmed().isEmpty()) {
        tc.movePosition(QTextCursor::StartOfBlock,
                        QTextCursor::MoveAnchor);
      } else {
        auto nspace = int{};
        for (const auto& cl : l) {
          if(!cl.isSpace()){
            break;
          }
          ++nspace;
        }
        tc.insertText("\n");
        if (nspace != 0) {
          tc.insertText(QString(nspace,' '));
        }
      }
      tc.endEditBlock();
      textEdit.setTextCursor(tc);
      return;
    }
    if ((c == b) && (c == e)) {
      auto tc = c;
      tc.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
      tc.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::KeepAnchor);
      if (tc.selectedText().trimmed().isEmpty()) {
        tc.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
        tc.beginEditBlock();
        do_indent(this->qemacs, this->textEdit, tc, tc, tc);
        tc.endEditBlock();
        textEdit.setTextCursor(tc);
        return;
      } else {
        tc = c;
        tc.beginEditBlock();
        do_indent(this->qemacs, this->textEdit, tc, tc, tc);
        tc.endEditBlock();
        textEdit.setTextCursor(tc);
      }
    } else {
      auto tc = c;
      tc.beginEditBlock();
      do_indent(this->qemacs, this->textEdit, tc, b, e);
      tc.endEditBlock();
      textEdit.setTextCursor(tc);
    }
  }

  void CMajorModeBase::indentLine(const QTextCursor& c) {
    this->indent(c, c, c);
  }

  void CMajorModeBase::indentRegion(const QTextCursor& tc) {
    if(!tc.hasSelection()) {
      return;
    }
    QTextCursor b(tc);
    QTextCursor e(tc);
    QTextCursor c(tc);
    c.setPosition(tc.selectionEnd());
    if(tc.selectionStart() > tc.selectionEnd()) {
      b.setPosition(tc.selectionEnd());
      e.setPosition(tc.selectionStart());
    } else {
      b.setPosition(tc.selectionStart());
      e.setPosition(tc.selectionEnd());
    }
    this->indent(b, e, c);
  } // end of QEmacsMajorModeBase::indentRegion

  CMajorModeBase::~CMajorModeBase() = default;

} // end of namespace qemacs
