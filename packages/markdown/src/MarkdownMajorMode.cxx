/*!
 * \file  MarkdownMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   07/08/2012
 */

#include <stdexcept>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QStringListModel>
#include <QtCore/QTimer>
#include <QtWidgets/QAbstractItemView>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/MarkdownSyntaxHighlighter.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MarkdownMajorMode.hxx"

namespace tfel::gui {

  MarkdownMajorMode::MarkdownMajorMode(EditorWidget &w,
                                       Buffer &b,
                                       TextEditBase &t)
      : CompiledLanguageMajorModeBase(w, b, t) {
  }  // end of MarkdownMajorMode::MarkdownMajorMode

  QString MarkdownMajorMode::getName() const {
    return "Markdown";
  }  // end of MarkdownMajorMode::getName

  QString MarkdownMajorMode::getDescription() const {
    return "major mode dedicated to the Markdown language";
  }  // end of MarkdownMajorMode::getDescription

  void MarkdownMajorMode::setSpellCheckLanguage(const QString & /* l */) {
    // this->spellChecker.setSpellCheckLanguage(l);
    if (this->highlighter != nullptr) {
      this->highlighter->rehighlight();
    }
  }  // end of MarkdownMajorMode::setSpellCheckLanguage

  void MarkdownMajorMode::format() {}  // end of MarkdownMajorMode::format()

  QMenu *MarkdownMajorMode::getSpecificMenu() {
    auto *t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    //     QMenu *m(new QMenu(QObject::tr("Markdown"), t));
    //     this->rm = new QAction(QObject::tr("Run pandoc"), this);
    //     m->addAction(this->rm);
    return nullptr;
  }  // end of

  QCompleter *MarkdownMajorMode::getCompleter() {
    return nullptr;
  }  // end of MarkdownMajorMode::getCompleter

  QString MarkdownMajorMode::getCompletionPrefix() {
    return "";
  }  // end of MarkdownMajorMode::getCompletionPrefix

  QIcon MarkdownMajorMode::getIcon() const {
    return QIcon::fromTheme("x-office-document");
  }  // end of MarkdownMajorMode::getIcon()

  void MarkdownMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    this->highlighter = new MarkdownSyntaxHighlighter(*this, d);
  }  // end of MarkdownMajorMode::setSyntaxHighlighter

  void MarkdownMajorMode::completeContextMenu(QMenu *const,
                                              const QTextCursor &) {
  }  // end of MarkdownMajorMode::completeContextMenu

  void MarkdownMajorMode::runPandoc() {
    if (this->textEdit.isModified()) {
      auto *input = this->textEdit.getSaveInput();
      QObject::connect(input, &TextEditBase::SaveInput::finished, this,
                       &MarkdownMajorMode::startPandoc);
      this->editor.setUserInput(input);
      return;
    }
    this->startPandoc();
  }  // end of MarkdownMajorMode::runPandoc()

  QString MarkdownMajorMode::getLanguageName() const {
    return "markdown";
  }  // end of MarkdownMajorMode::getLanguageName

  QString MarkdownMajorMode::getDefaultCompilationCommand() const {
    return "pandoc ";
  }  // end of MarkdownMajorMode::getDefaultCompilationCommand

  void MarkdownMajorMode::startPandoc() {
    const auto n = this->textEdit.getCompleteFileName();
    if (n.isEmpty()) {
      this->report(QObject::tr("MarkdownMajorMode::startPandoc: no file name"));
      return;
    }
    //     PandocAnalysisOptions o;
    //     MarkdownAnalysisOptionsDialog od(o, &(this->textEdit));
    //     if (od.exec() == QDialog::Rejected) { return; }
    //     const auto &af = QFileInfo(n).absoluteFilePath();
    //     auto *s = new MarkdownOutputFrame(this->editor, this->buffer,
    //     af, o);
    //     QObject::connect(s, &MarkdownOutputFrame::finished, this,
    //                      &MarkdownMajorMode::AnalysisFinished);
    //     this->buffer.addSlave(QObject::tr("Markdown Output"), s);
  }  // end of MarkdownMajorMode::startPandoc

  bool MarkdownMajorMode::handleShortCut(const int k1,
                                         const Qt::KeyboardModifiers m,
                                         const int k2) {
    if ((k1 == Qt::Key_C) && (m == Qt::NoModifier) && (k2 == Qt::Key_Equal)) {
      auto toc = new PlainTextEdit(this->editor, this->buffer);
      toc->setMajorMode("Markdown");
      auto c = toc->textCursor();
      auto tc = this->textEdit.textCursor();
      tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
      while (!tc.atEnd()) {
        tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        const auto l = tc.selectedText();
        if (l.startsWith('#')) {
          c.insertText(l + '\n');
        }
        if (!tc.atEnd()) {
          tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
          tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        }
      }
      toc->setReadOnly(true);
      this->buffer.attachSecondaryTask("Table of contents", toc);
      return true;
    }
    return CompiledLanguageMajorModeBase::handleShortCut(k1, m, k2);
  }  // end of MarkdownMajorMode::handleShortCut

  bool MarkdownMajorMode::keyPressEvent(QKeyEvent *const ev) {
    const auto k = ev->key();
    const auto m = ev->modifiers();
    // returns if the line under cursor starts a paragraph
    // The first member of the pair value is:
    // - -1: if the line can't be the start of a paragraph
    //       (preamble, header, empty line)
    // -  0: the line is the start of a list or the start of a footnote.
    // -  1: the line can be the start of a paragraph
    // The second member gives the indentation of the line.
    auto startsParagraph = [](QTextCursor tc) -> std::pair<int, int> {
      static auto f = [] {  // detect footnotes
        QRegularExpression r("^\\s*\\[\\^\\d+\\]:");
        r.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
        return r;
      }();
      static auto el = [] {  // enumerated list
        QRegularExpression r("^\\s*\\d+\\.\\s+");
        r.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
        return r;
      }();
      tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      const auto l = tc.selectedText();
      if (l.startsWith('%') || l.startsWith('#')) {
        // header or preamble
        return {-1, 0};
      }
      int indent = 0;
      for (const auto &c : l) {
        if (c.isSpace()) {
          ++indent;
        } else if ((c == '-') || (c == '*') || (c == '+')) {
          return {0, indent};
        } else if (c.isDigit()) {
          const auto match = el.match(l);
          if (match.hasMatch()) {
            return {0, indent};
          } else {
            return {1, 0};
          }
        } else if (c == '[') {
          const auto match = f.match(l);
          if (match.hasMatch()) {
            return {0, indent};
          } else {
            return {1, 0};
          }
        } else {
          return {1, 0};
        }
      }
      return {-1, 0};
    };  // end of startsParagraph
    if (((m == Qt::NoModifier) && (k == Qt::Key_Tab)) ||
        ((m == Qt::AltModifier) && (k == Qt::Key_Q))) {
      // cut paragraph to 72 columns
      auto tc = this->textEdit.textCursor();
      // search the beginning of the text to highlight
      auto b = tc;
      const auto r = startsParagraph(b);
      if (r.first == -1) {
        return true;
      }
      auto inList = r.first == 0;
      auto indent = r.second;
      if (r.first == 1) {
        b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        while (!b.atStart()) {
          auto bp = b;
          bp.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
          bp.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          const auto rp = startsParagraph(bp);
          if (rp.first == -1) {
            break;
          } else if (rp.first == 0) {
            inList = true;
            indent = rp.second;
            b = bp;
            break;
          } else if (rp.first == 1) {
            b = bp;
          }
        }
      }
      // now b points to the beginning of the paragraph
      b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      auto e = tc;
      e.movePosition(QTextCursor::EndOfBlock);
      while (!e.atEnd()) {
        auto en = e;
        en.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        en.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        const auto rn = startsParagraph(en);
        if (rn.first != 1) {
          break;
        }
        e = en;
      }
      e.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      e.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      tc = b;
      // gather selected lines in one string
      auto p = QString();
      b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
      b.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      p += b.selectedText().trimmed();
      if (!b.atEnd()) {
        b.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        b.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        while (b <= e) {
          b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          b.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
          p += ' ' + b.selectedText().trimmed();
          if (b.atEnd()) {
            break;
          }
          b.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
          b.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        }
        //         if (e.atEnd()) {
        //           b.movePosition(QTextCursor::StartOfBlock,
        //                          QTextCursor::MoveAnchor);
        //           b.movePosition(QTextCursor::EndOfBlock,
        //                          QTextCursor::KeepAnchor);
        //           p += ' ' + b.selectedText().trimmed();
        //         }
      }
      // split the string
      const auto words =
          p.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      // remove the selected lines and insert the splitted string
      tc.beginEditBlock();
      tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                      e.position() - tc.position());
      tc.removeSelectedText();
      bool lineStart = true;
      auto count = indent;
      tc.insertText(QString(indent, ' '));
      for (const auto &w : words) {
        const auto s = w.size();
        if (lineStart) {
          // beginning of a new line
          if ((count + s > 72)) {
            tc.insertText(w);
            tc.insertText("\n");
            if (indent == 0) {
              count = 0;
            } else {
              tc.insertText(QString(indent + 2, ' '));
              count = indent + 2;
            }
            lineStart = true;
          } else {
            tc.insertText(w);
            count += s;
            lineStart = false;
          }
        } else {
          if ((count + s + 1 > 72)) {
            tc.insertText("\n");
            if (inList == 0) {
              count = 0;
            } else {
              tc.insertText(QString(indent + 2, ' '));
              count = indent + 2;
            }
            tc.insertText(w);
            count += s;
            lineStart = false;
          } else {
            tc.insertText(' ' + w);
            count += s + 1;
            lineStart = false;
          }
        }
      }
      tc.endEditBlock();
      this->textEdit.setTextCursor(tc);
      return true;
    } else if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      QSettings s;
      const auto ch = s.value("pandoc/compilation/history").toStringList();
      const auto d = [&ch]() -> QString {
        if (ch.isEmpty()) {
          return "pandoc -f markdown+tex_math_single_backslash ";
        }
        return ch.back();
      }();
      auto *l = new ShellProcessLineEdit("compilation command :", d,
                                         "markdown-output", this->editor);
      l->setInputHistorySettingAddress("pandoc/compilation/history");
      this->editor.setUserInput(l);
      return true;
    }
    return CompiledLanguageMajorModeBase::keyPressEvent(ev);
  }  // end of MarkdownMajorMode::keyPressEvent

  // SpellChecker &MarkdownMajorMode::getSpellChecker() {
  //   return this->spellChecker;
  // }  // end of MarkdownMajorMode::getSpellChecker

  MarkdownMajorMode::~MarkdownMajorMode() = default;

  static StandardMajorModeProxy<MarkdownMajorMode> proxy(
      "Markdown",
      QVector<QRegularExpression>()
          << QRegularExpression("^" + fileNameRegExp() + ".md"),
      "x-office-document");

}  // end of namespace tfel::gui
