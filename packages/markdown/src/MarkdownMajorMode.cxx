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
#include <QtWidgets/QMenu>
#include "TFEL/Utilities/CxxTokenizer.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/MarkdownSyntaxHighlighter.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/MarkdownMajorMode.hxx"

namespace tfel::gui {

  MarkdownMajorMode::MarkdownMajorMode(EditorWidget &w,
                                       Buffer &b,
                                       TextEditBase &t)
      : CompiledLanguageMajorModeBase(w, b, t) {
    // a buffer of Markdown is prose, which is what the grammar checker reads;
    // hunspell is taken off the text while the checker runs, since the two
    // underline one word in two colors at once
    this->grammar =
        new GrammarCheckSession(w, t, QStringLiteral("markdown"), this);
    QObject::connect(
        this->grammar, &GrammarCheckSession::stateChanged, this, [this] {
          // the language of the frame is what the frame was asked to
          // check with: giving it back to the mode is what puts the
          // spell checker back on the text when the checker is gone,
          // and an empty one what takes it off
          this->setSpellCheckLanguage(this->textEdit.getSpellCheckLanguage());
        });
    if (GrammarCheckSession::isAvailable() &&
        LspHarperSupport::launchByDefault()) {
      // the session follows the setting: it writes nothing back, and the entry
      // of the menu says what the buffer does whatever brought the session
      // about
      this->grammar->setActive(true, false, false);
    }
  }  // end of MarkdownMajorMode

  QString MarkdownMajorMode::getName() const {
    return "Markdown";
  }  // end of getName

  QString MarkdownMajorMode::getDescription() const {
    return "major mode dedicated to the Markdown language";
  }  // end of getDescription

  void MarkdownMajorMode::setSpellCheckLanguage(const QString &l) {
    // a buffer the grammar checker reads is not read by hunspell at the same
    // time: the two underline one word in two colors, and the person who turned
    // the checker on is not asking to read both. The language the frame was
    // given lives on the frame itself, see
    // \c TextEditBase::setSpellCheckLanguage, which is where the mode takes it
    // back from when the checker starts or stops
    this->spellChecker.setSpellCheckLanguage(
        ((this->grammar != nullptr) && (this->grammar->isActive())) ? QString()
                                                                    : l);
    if (!this->highlighter.isNull()) {
      this->highlighter->rehighlight();
    }
  }  // end of setSpellCheckLanguage

  void MarkdownMajorMode::format() {}  // end of format()

  QMenu *MarkdownMajorMode::getSpecificMenu() {
    auto *const t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    // the menu of the mode is the grammar checker's, and nothing else: what the
    // mode does otherwise — cutting a paragraph, running pandoc — is bound to
    // keys, and a menu which would be empty on a machine which has no checker
    // to launch is a menu nobody should see
    if (!this->grammar->isAvailable()) {
      return nullptr;
    }
    auto *const m = new QMenu(QObject::tr("Markdown"), t);
    m->setIcon(this->getIcon());
    // the note spells what the entry does not: the checker knows no word of
    // French, and a buffer of French is underlined by hunspell and by nothing
    // else
    this->grammar->addToMenu(*m,
                             QObject::tr("Grammar checking (English only)"));
    return m;
  }  // end of getSpecificMenu

  QCompleter *MarkdownMajorMode::getCompleter() {
    return nullptr;
  }  // end of getCompleter

  QString MarkdownMajorMode::getCompletionPrefix() {
    return "";
  }  // end of getCompletionPrefix

  QIcon MarkdownMajorMode::getIcon() const {
    return QIcon::fromTheme("x-office-document");
  }  // end of getIcon()

  void MarkdownMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    // MajorMode::setHighlighter releases the previous highlighter: a document
    // accepts several highlighters and they all apply, so overwriting the
    // pointer alone left the stale one active and accumulating.
    this->setHighlighter(new MarkdownSyntaxHighlighter(*this, d));
  }  // end of setSyntaxHighlighter

  void MarkdownMajorMode::completeContextMenu(QMenu *const,
                                              const QTextCursor &) {
  }  // end of completeContextMenu

  void MarkdownMajorMode::runPandoc() {
    if (this->textEdit.isModified()) {
      auto *input = this->textEdit.getSaveInput();
      QObject::connect(input, &TextEditBase::SaveInput::finished, this,
                       &MarkdownMajorMode::startPandoc);
      this->editor.setUserInput(input);
      return;
    }
    this->startPandoc();
  }  // end of runPandoc()

  QString MarkdownMajorMode::getLanguageName() const {
    return "markdown";
  }  // end of getLanguageName

  QString MarkdownMajorMode::getDefaultCompilationCommand() const {
    return "pandoc ";
  }  // end of getDefaultCompilationCommand

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
  }  // end of startPandoc

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
          // iterate over the lines of the file (text blocks), not over
          // the displayed lines which depend on the wrapping policy
          tc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
          tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        }
      }
      toc->setReadOnly(true);
      this->buffer.attachSecondaryTask("Table of contents", toc);
      return true;
    }
    return CompiledLanguageMajorModeBase::handleShortCut(k1, m, k2);
  }  // end of handleShortCut

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
        // the cursor is not in a paragraph (empty line, '#' heading,
        // '%' preamble, ...): let the key reach the generic handling so
        // Tab indents (returning true here swallowed Tab outright and
        // made indentation impossible in a Markdown buffer) and Alt-q
        // falls through to its generic binding
        return false;
      }
      auto inList = r.first == 0;
      auto indent = r.second;
      if (r.first == 1) {
        b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        while (!b.atStart()) {
          auto bp = b;
          bp.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
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
        en.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
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
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        b.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        while (b <= e) {
          b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          b.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
          p += ' ' + b.selectedText().trimmed();
          if (b.atEnd()) {
            break;
          }
          b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
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
      // the cursor is anchored to the number of non blank characters
      // between the beginning of the paragraph and the cursor: the
      // reflow only moves blanks, the sequence of the non blank
      // characters is left untouched, so this number identifies the
      // same position in the paragraph before and after the reflow.
      const auto cursor = this->textEdit.textCursor().position();
      auto nonBlankCharactersBeforeCursor = 0;
      {
        const auto &doc = *this->textEdit.document();
        for (auto i = tc.position(); i < cursor; ++i) {
          if (!doc.characterAt(i).isSpace()) {
            ++nonBlankCharactersBeforeCursor;
          }
        }
      }
      const auto paragraphStart = tc.position();
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
      // restore the cursor right after the n-th non blank character of
      // the reformatted paragraph, n being the number of non blank
      // characters which were before the cursor in the original
      // paragraph.
      {
        const auto end = tc.position();
        auto pos = paragraphStart;
        if (nonBlankCharactersBeforeCursor == 0) {
          // the cursor was before the first non blank character of the
          // paragraph: place it on this character, after the inserted
          // indentation.
          pos += indent;
        } else {
          auto n = 0;
          for (; pos < end; ++pos) {
            if (!this->textEdit.document()->characterAt(pos).isSpace()) {
              ++n;
              if (n == nonBlankCharactersBeforeCursor) {
                ++pos;
                break;
              }
            }
          }
        }
        tc.setPosition(qMin(pos, end));
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
  }  // end of keyPressEvent

  SpellChecker &MarkdownMajorMode::getSpellChecker() {
    return this->spellChecker;
  }  // end of getSpellChecker

  MarkdownMajorMode::~MarkdownMajorMode() = default;

  static StandardMajorModeProxy<MarkdownMajorMode> proxy(
      "Markdown",
      QVector<QRegularExpression>()
          << suffixRegExp("md") << suffixRegExp("markdown")
          << suffixRegExp("mdown"),
      "x-office-document");

}  // end of namespace tfel::gui
