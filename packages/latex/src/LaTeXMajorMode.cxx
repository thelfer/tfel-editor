/*!
 * \file  LaTeXMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   18/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtWidgets/QMenu>
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/LaTeXSyntaxHighlighter.hxx"
#include "TFEL/GUI/LaTeXMajorMode.hxx"

namespace tfel::gui {

  static int countNumberOfCurlyBraces(const QString& s) {
    int n = 0;
    for (int i = 0; i != s.size(); ++i) {
      if ((s[i] == '{') || (s[i] == '}')) {
        // check if previous character was not a '\'
        int j = i;
        bool b(true);
        --j;
        while ((j >= 0) && (s[j] == '\\')) {
          b = !b;
          --j;
        }
        if (b) {
          if (s[i] == '{') {
            ++n;
          } else {
            --n;
          }
        }
      }
    }
    return n;
  }

  struct LaTeXMajorMode::LaTeXInsertBlock : public CommandLine {
    LaTeXInsertBlock(EditorWidget& p, TextEditBase& t)
        : CommandLine(QObject::tr("insert block :"), p), textEdit(t) {
      QStringList n;
      n << "figure"
        << "minipage"
        << "equation";
      //      = LaTeXSyntaxHighlighter::getPrimaryBlocks();
      // n <<  LaTeXSyntaxHighlighter::getSecondaryBlocks();
      auto* const co = new QCompleter(n, &p);
      co->setWidget(this->input);
      co->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(co, false);
    }

    ~LaTeXInsertBlock() override = default;

   protected:
    void treatUserInput() override {
      const auto& b = this->input->text();
      if (!b.isEmpty()) {
        QTextCursor tc = this->textEdit.textCursor();
        if (tc.hasSelection()) {
          QTextCursor bs(tc);
          QTextCursor es(tc);
          if (tc.selectionStart() > tc.selectionEnd()) {
            bs.setPosition(tc.selectionEnd());
            es.setPosition(tc.selectionStart());
          } else {
            bs.setPosition(tc.selectionStart());
            es.setPosition(tc.selectionEnd());
          }
          int pe = es.position();
          es.movePosition(QTextCursor::EndOfBlock);
          if (pe != es.position()) {
            es.insertText("\n");
          }
          es.insertText("\\end{" + b + "}");
          bs.insertText("\\begin{" + b + "}\n");
        } else {
          tc.insertText("\\begin{" + b + "}\n");
          int p = tc.position();
          tc.movePosition(QTextCursor::EndOfBlock);
          tc.insertText("\n\\end{" + b + "}");
          tc.movePosition(QTextCursor::PreviousBlock);
          tc.setPosition(p);
          this->textEdit.setTextCursor(tc);
        }
      }
    }

    TextEditBase& textEdit;

  };  // end of struct TextEdit::LaTeXInsertBlock

  struct LaTeXMajorModeRessourceLoader {
    LaTeXMajorModeRessourceLoader() : l(getLoader()) {}

   private:
    struct Loader {
      Loader() { Q_INIT_RESOURCE(LaTeXModeResources); }
    };
    static Loader& getLoader() {
      static Loader l;
      return l;
    }
    Loader l;
  };

  LaTeXMajorMode::LaTeXMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
      : MajorModeBase(w, b, t, &t),
        rb(R"(\\begin\{\w+\})"),
        re(R"(\\end\{\w+\})"),
        rb2(R"(^\s*\\begin\{\w+\})"),
        re2(R"(^\s*\\end\{\w+\})") {
    qDebug() << "TeX-master : " << this->getTeXMasterFile();
    // the prose of a document is what the grammar checker reads, and none of
    // the commands are: the checker reads the buffer as the variety of English
    // the setting names, and hunspell is taken off the text while it runs,
    // since the two underline one word in two colors at once
    this->grammar =
        new GrammarCheckSession(w, t, QStringLiteral("latex"), this);
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
  }  // end of LaTeXMajorMode::LaTeXMajorMode

  QString LaTeXMajorMode::getTeXMasterFile() {
    // `[\w0-9_...]`, not `[\w-0-9_...]`: PCRE (used by QRegularExpression,
    // unlike QRegExp) rejects a range starting on a class escape
    QRegularExpression r(R"(^\s*%%% TeX-master: \"([\w0-9_\./]+)\"\s*$)");
    if (!r.isValid()) {
      return {};
    }
    QTextCursor tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    while (!tc.atEnd()) {
      const auto m = r.match(tc.block().text());
      if (m.hasMatch()) {
        return m.captured(1);
      }
      tc.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }
    return {};
  }  // end of LaTeXMajorMode::getTeXMasterFile

  QString LaTeXMajorMode::getName() const {
    return "LaTeX";
  }  // end of LaTeXMajorMode::LaTeXMajorMode

  QString LaTeXMajorMode::getDescription() const {
    return "major mode dedicated to the LaTeX";
  }  // end of LaTeXMajorMode::LaTeXMajorMode

  void LaTeXMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    // MajorMode::setHighlighter releases the previous highlighter: a document
    // accepts several highlighters and they all apply, so overwriting the
    // pointer alone left the stale one active and accumulating.
    this->setHighlighter(new LaTeXSyntaxHighlighter(*this, d));
  }  // end of LaTeXMajorMode::setSyntaxHighlighter

  void LaTeXMajorMode::completeCurrentWord(TextEditBase& t, const QString& w) {
    MajorModeBase::completeCurrentWord(t, w);
  }  // end of LaTeXMajorModeBase::completeCurrentWord

  void LaTeXMajorMode::runLaTeX() {
    if (this->textEdit.isModified()) {
      TextEditBase::SaveInput* input = this->textEdit.getSaveInput();
      QObject::connect(input, &TextEditBase::SaveInput::finished, this,
                       &LaTeXMajorMode::startLaTeX);
      this->editor.setUserInput(input);
      return;
    }
    this->startLaTeX();
  }

  void LaTeXMajorMode::startLaTeX() {
    QString m = this->getTeXMasterFile();
    if (m.isEmpty()) {
      m = this->textEdit.getCompleteFileName();
    }
    if (m.isEmpty()) {
      this->editor.displayInformativeMessage(QObject::tr("no file name"));
      return;
    }
    if (this->po == nullptr) {
      this->po = new ProcessOutputFrame(this->editor, this->buffer);
      this->buffer.attachSecondaryTask("*LaTeX* ouput", po);
    } else {
      this->buffer.setSecondaryTaskTitle(po, "*LaTeX* ouput");
      this->buffer.setSecondaryTaskIcon(po, QIcon());
    }
    po->clear();
    QFileInfo fn(m);
    if (!fn.exists()) {
      this->editor.displayInformativeMessage(QObject::tr("file '%1' does "
                                                         "not exist")
                                                 .arg(m));
      return;
    }
    QDir d(fn.dir());
    QProcess& p = po->getProcess();
    if (d.exists()) {
      p.setWorkingDirectory(d.absolutePath());
    } else {
      p.setWorkingDirectory(QDir::current().absolutePath());
    }
    po->setMajorMode("latex-output");
    QStringList args;
    args << "-file-line-error"
         << "-interaction=nonstopmode"
         << "\"\\input\"" << m;
    p.start("latex", args);
  }  // end of LaTeXMajorMode::startLaTeX

  QMenu* LaTeXMajorMode::getSpecificMenu() {
    auto* const t = qobject_cast<QWidget*>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    // the menu of the mode is the grammar checker's, and nothing else: the mode
    // has no other entry of its own, and a menu which would be empty on a
    // machine which has no checker to launch is a menu nobody should see
    if (!this->grammar->isAvailable()) {
      return nullptr;
    }
    auto* const m = new QMenu(QObject::tr("LaTeX"), t);
    m->setIcon(this->getIcon());
    // the note spells what the entry does not: the checker knows no word of
    // French, and a document of French is underlined by hunspell and by nothing
    // else
    this->grammar->addToMenu(*m,
                             QObject::tr("Grammar checking (English only)"));
    return m;
  }  // end of LaTeXMajorMode::getSpecificMenu

  QIcon LaTeXMajorMode::getIcon() const {
    static QIcon i(":/tfel/editor/languages/latex.png");
    return i;
  }  // end of LaTeXMajorMode::getIcon()

  void LaTeXMajorMode::completeContextMenu(QMenu* const m,
                                           const QTextCursor& tc) {
    MajorModeBase::completeContextMenu(m, tc);
    if (this->highlighter.isNull()) {
      return;
    }
    // position of the cursor relative to the beginning of its own line
    auto bl = QTextCursor(tc);
    bl.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    const auto posl = tc.position() - bl.position();
    const auto text = tc.block().text();
    // a misspelled word can only be in a portion of the line which is neither
    // a comment nor a LaTeX command
    for (const auto& p : LaTeXSyntaxHighlighter::parseLine(text).plainText) {
      if ((posl < p.first) || (posl >= p.first + p.second)) {
        continue;
      }
      this->completeContextMenuForMispelledWord(m, text.mid(p.first, p.second),
                                                posl - p.first,
                                                bl.position() + p.first);
    }
  }  // end of LaTeXMajorMode::completeContextMenu

  void LaTeXMajorMode::setSpellCheckLanguage(const QString& l) {
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
  }  // end of LaTeXMajorMode::setSpellCheckLanguage

  void LaTeXMajorMode::completeContextMenuForMispelledWord(QMenu* m,
                                                           const QString& l,
                                                           const int p,
                                                           const int ap) {
    int pos = 0;
    while (pos != l.size()) {
      if (l[pos].isLetter()) {
        int npos = pos;
        ++pos;
        while ((pos != l.size()) && (l[pos].isLetter())) {
          ++pos;
        }
        if ((p > npos) && (p < pos)) {
          // found !
          this->bmwp = ap + npos;
          this->emwp = ap + pos;
          const QString w = l.mid(npos, pos - npos);
          if (!this->spellChecker.spell(w)) {
            for (auto& pa : this->suggestions) {
              delete pa;
            }
            this->suggestions.clear();
            const auto ss = this->spellChecker.suggest(w);
            for (const auto& s : ss) {
              auto* a = new QAction(s, this);
              a->setData(s);
              this->suggestions.push_back(a);
            }
            if (this->suggestions.size() != 0) {
              auto cactions = m->actions();
              if (!cactions.isEmpty()) {
                m->insertAction(*(cactions.begin()), m->addSeparator());
              }
              for (int i = this->suggestions.size(); i > 0; --i) {
                auto* a = this->suggestions[i - 1];
                cactions = m->actions();
                if (cactions.isEmpty()) {
                  m->addAction(a);
                } else {
                  m->insertAction(*(cactions.begin()), a);
                }
              }
              QObject::connect(
                  m, &QMenu::triggered, this,
                  &LaTeXMajorMode::replaceMispelledWordBySuggestion);
            }
          }
        }
      } else {
        ++pos;
      }
    }
  }  // end of LaTeXMajorMode::completeContextMenuForMispelledWord

  SpellChecker& LaTeXMajorMode::getSpellChecker() { return this->spellChecker; }

  void LaTeXMajorMode::replaceMispelledWordBySuggestion(QAction* a) {
    if (this->suggestions.indexOf(a) != -1) {
      QTextCursor tc(this->textEdit.textCursor());
      tc.beginEditBlock();
      tc.setPosition(this->bmwp);
      tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                      this->emwp - this->bmwp);
      tc.removeSelectedText();
      tc.insertText(a->data().toString());
      tc.endEditBlock();
    }
  }  // end of LaTeXMajorMode::replaceMispelledWordBySuggestion

  bool LaTeXMajorMode::handleShortCut(const int k1,
                                      const Qt::KeyboardModifiers m,
                                      const int k2) {
    if ((k1 == Qt::Key_C) && (m == Qt::ControlModifier) && (k2 == Qt::Key_E)) {
      this->editor.setUserInput(
          new LaTeXInsertBlock(this->editor, this->textEdit));
      return true;
    }
    return false;
  }  // end of LaTeXMajorMode::handleShortCut

  bool LaTeXMajorMode::mousePressEvent(QMouseEvent* const) {
    return false;
  }  // end of LaTeXMajorMode::mousePressEvent

  void LaTeXMajorMode::format() {
    QTextCursor bc;
    QTextCursor ec;
    QTextCursor cc;
    this->findBeginningAndEndOfTheParagraph(bc, ec);
    // the position of the cursor is memorized before the paragraph is
    // rewritten and restored once it has been: reflowing a paragraph moves its
    // words from one line to another and may change the number of lines, so the
    // cursor cannot be restored from a plain offset. Without it, removing the
    // paragraph and inserting it back leaves the editor's cursor at the end of
    // the inserted text.
    const auto fp = bc.position();
    const auto m = this->getAnchorMark(bc, this->textEdit.textCursor());
    cc = bc;
    cc.beginEditBlock();
    QVector<QStringList> lines;
    cc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    ec.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    while ((!cc.atEnd()) && (cc <= ec)) {
      QString l = cc.block().text();
      // a group is created for each line of the paragraph, so that an
      // empty group unambiguously denotes a blank line of the source,
      // which the reformatting must restore as such.
      lines.push_back(QStringList());
      if (l.trimmed().isEmpty()) {
        cc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        cc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        continue;
      }
      int cpos = LaTeXSyntaxHighlighter::startOfComment(l);
      if (cpos != -1) {
        lines.back() << l.mid(0, cpos).split(' ', Qt::SkipEmptyParts);
        lines.back() << l.mid(cpos, l.size() - cpos);
      } else {
        lines.back() << l.split(' ', Qt::SkipEmptyParts);
      }
      cc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      cc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }
    if (ec.atEnd()) {
      QString l = cc.block().text();
      int cpos = LaTeXSyntaxHighlighter::startOfComment(l);
      lines.push_back(QStringList());
      if (cpos != -1) {
        lines.back() << l.mid(0, cpos).split(' ', Qt::SkipEmptyParts);
        lines.back() << l.mid(cpos, l.size() - cpos);
      } else {
        lines.back() << l.split(' ', Qt::SkipEmptyParts);
      }
    }
    cc = bc;
    cc.clearSelection();
    cc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                    ec.position() - bc.position());
    cc.removeSelectedText();
    QVector<QStringList>::const_iterator pl;
    int size = 0;
    int i;
    int di;
    bool be;
    this->getLineIndentation(cc, i, di, be);
    // the paragraph is written back word by word. started tells whether
    // the first word has been emitted and breaks counts the line breaks
    // emitted since this word, so that insertLineBreaks only emits the
    // line breaks which are still missing: reformatting a paragraph
    // never introduces a blank line, while the blank lines of the source
    // paragraph (the empty groups) are emitted back and never removed.
    bool started = false;
    int breaks = 0;
    auto insertLineBreaks = [&cc, &started, &breaks, &size](const int n) {
      if (!started) {
        return;
      }
      if (n > breaks) {
        cc.insertText(QString(n - breaks, '\n'));
        breaks = n;
      }
      size = 0;
    };
    int blanks = 0;
    for (pl = lines.begin(); pl != lines.end(); ++pl) {
      if (pl->isEmpty()) {
        // a blank line of the source paragraph: restore it, but only
        // between two groups containing words, as emitting it before
        // the first or after the last word would introduce a blank line.
        if (started) {
          bool content_after = false;
          for (auto pe = pl; pe != lines.end(); ++pe) {
            if (!pe->isEmpty()) {
              content_after = true;
              break;
            }
          }
          if (content_after) {
            ++blanks;
            insertLineBreaks(blanks + 1);
          }
        }
        continue;
      }
      blanks = 0;
      QVector<QStringList>::const_iterator pl2 = pl;
      QStringList::const_iterator pw;
      ++pl2;
      for (pw = pl->begin(); pw != pl->end(); ++pw) {
        auto pw2 = pw;
        ++pw2;
        const auto& w = *pw;
        QString indent;
        bool sk = false;
        if (((w == "\\[") || (w == "\\]") || (w == "\\medskip") ||
             (this->rb.match(w).hasMatch()) ||
             (this->re.match(w).hasMatch()))) {
          insertLineBreaks(1);
          sk = true;
        }
        if ((w == "\\item") ||
            ((w.startsWith("\\item<") && (w.endsWith('>'))))) {
          insertLineBreaks(1);
        }
        if (w.size() >= 72) {
          insertLineBreaks(1);
          di = this->getIndentationIncrement(w);
          // be: the word does not close an environment; see
          // LaTeXMajorMode::getLineIndentation for the polarity
          be = !this->re.match(w).hasMatch();
          if (w[0] == '}') {
            be = false;
          }
          if (be) {
            if (i > 0) {
              indent = QString(2 * i, ' ');
            }
          } else {
            if (i + di > 0) {
              indent = QString(2 * (i + di), ' ');
            }
          }
          cc.insertText(indent + w + "\n");
          i += di;
          started = true;
          // the word stands on a line of its own: the current line is
          // empty. The previous implementation set the size to the
          // indentation level instead, so the next \begin{...}, \end{...}
          // or \item word added one more line break, introducing a
          // blank line.
          breaks = 1;
          size = 0;
        } else {
          if (w.size() + size >= 72) {
            di = this->getIndentationIncrement(w);
            be = !this->re.match(w).hasMatch();
            if (w[0] == '}') {
              be = false;
            }
            if (be) {
              if (i > 0) {
                indent = QString(2 * i, ' ');
              }
            } else {
              if (i + di > 0) {
                indent = QString(2 * (i + di), ' ');
              }
            }
            insertLineBreaks(1);
            cc.insertText(indent + w);
            i += di;
            started = true;
            breaks = 0;
            size = indent.size() + w.size();
          } else {
            di = this->getIndentationIncrement(w);
            if (size == 0) {
              be = !this->re2.match(w).hasMatch();
              if (w[0] == '}') {
                be = false;
              }
              if (be) {
                if (i > 0) {
                  indent = QString(2 * i, ' ');
                }
              } else {
                if (i + di > 0) {
                  indent = QString(2 * (i + di), ' ');
                }
              }
              cc.insertText(indent + w);
              i += di;
              started = true;
              breaks = 0;
              size = indent.size() + w.size();
            } else {
              cc.insertText(' ' + w);
              size += w.size() + 1;
              i += di;
              started = true;
              breaks = 0;
            }
          }
        }
        if (!((pl2 == lines.end()) && (pw2 == pl->end()))) {
          // a comment must be closed by a line break, otherwise the text
          // which follows is commented out
          if ((sk) || (w == "\\\\") || (w.startsWith('%'))) {
            insertLineBreaks(1);
          }
        }
      }
    }
    // the reformatted paragraph never ends with a blank line: remove a
    // last line break which left the cursor on an empty line, so that
    // format() never introduces a blank line at the end of the paragraph
    // it rewrites.
    if ((started) && (cc.atBlockStart())) {
      cc.deletePreviousChar();
    }
    cc.endEditBlock();
    if (started) {
      // `cc` is the end of the reformatted paragraph. `bc` cannot be used
      // anymore: rewriting the paragraph at its own position moved it to the
      // end of the inserted text.
      this->restoreAnchorMark(m, fp, cc.position());
    }
  }  // end of LaTeXMajorMode::format

  bool LaTeXMajorMode::keyPressEvent(QKeyEvent* const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      this->runLaTeX();
      return true;
    }
    return false;
  }  // end of LaTeXMajorMode::keyPressEvent

  void LaTeXMajorMode::indentLine(const QTextCursor& tc) {
    if (LaTeXSyntaxHighlighter::stripComment(tc.block().text())
            .trimmed()
            .isEmpty()) {
      // empty lines are left untouched: reindenting a buffer shall
      // never introduce blank lines, nor remove them, nor write trailing
      // blanks on blank lines.
      return;
    }
    int i;
    int di;
    bool b;
    this->getLineIndentation(tc, i, di, b);
    if (b) {
      MajorModeBase::doIndentLine(tc, 2 * i);
    } else {
      MajorModeBase::doIndentLine(tc, 2 * (i + di));
    }
  }  // end of LaTeXMajorMode::indentLine

  void LaTeXMajorMode::getLineIndentation(const QTextCursor& tc_,
                                          int& i,
                                          int& di,
                                          bool& b) const {
    // getting the line
    QTextCursor tc(tc_);
    QTextCursor cc(tc_);
    tc.clearSelection();
    tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    i = 0;
    b = true;
    while (cc < tc) {
      i += this->getIndentationIncrement(cc.block().text());
      cc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    QString l =
        LaTeXSyntaxHighlighter::stripComment(tc.block().text()).trimmed();
    // b: the line does not close an environment. This test was written
    // `re2.indexIn(l, 0) == -1` in the QRegExp era; QRegularExpression
    // match(...).hasMatch() has the opposite sense, so the negation must
    // be kept. Without it, \begin{...} lines are indented at 2 * (i + di)
    // and \end{...} lines at 2 * i, which shifts the whole file by one
    // indentation level.
    b = !this->re2.match(l, 0).hasMatch();
    if (l.size() > 0) {
      if (l[0] == '}') {
        b = false;
      }
    }
    di = this->getIndentationIncrement(l);
  }  // end of LaTeXMajorMode::getLineIndentation

  int LaTeXMajorMode::getIndentationIncrement(const QString& line) const {
    int di = 0;
    QString l = LaTeXSyntaxHighlighter::stripComment(line).trimmed();
    if (l.isEmpty()) {
      return di;
    }
    int pos = 0;
    int ppos = 0;
    while (pos != -1) {
      const auto m1 = rb.match(l, pos);
      const auto m2 = re.match(l, pos);
      // The starting position of a match is given by capturedStart().
      // lastCapturedIndex() must not be used for that: it returns the
      // index of the last capture group of the pattern (0 here, as
      // rb/re have no groups), or -1 when there is no match. Using it
      // as a position made this loop never advance (e.g. on any line
      // containing \end{...}), which hung format() and indentLine().
      const int pos1 = m1.hasMatch() ? m1.capturedStart() : -1;
      const int pos2 = m2.hasMatch() ? m2.capturedStart() : -1;
      bool b1 = false;
      bool b2 = false;
      if ((pos1 != -1) && (pos2 == -1)) {
        b1 = true;
      } else if ((pos1 == -1) && (pos2 != -1)) {
        b2 = true;
      } else if ((pos1 != -1) && (pos2 != -1)) {
        if (pos1 < pos2) {
          b1 = true;
        } else {
          b2 = true;
        }
      }
      if (b1) {
        // \begin{document} does not open an indentation level: the body of
        // the document, and the \begin{document} line itself, must remain
        // at the same indentation level as before the command.
        if (l.mid(pos1, m1.capturedLength()) != "\\begin{document}") {
          di += 1;
        }
        if (ppos != pos1) {
          di += countNumberOfCurlyBraces(l.mid(ppos, pos1 - ppos));
        }
        ppos = pos1 + m1.capturedLength();
        pos = ppos;
      } else if (b2) {
        if (l.mid(pos2, m2.capturedLength()) != "\\end{document}") {
          di -= 1;
        }
        if (ppos != pos2) {
          di += countNumberOfCurlyBraces(l.mid(ppos, pos2 - ppos));
        }
        ppos = pos2 + m2.capturedLength();
        pos = ppos;
      } else {
        if (ppos != l.size()) {
          di += countNumberOfCurlyBraces(l.mid(ppos, l.size() - ppos));
        }
        pos = -1;
      }
    }
    return di;
  }

  void LaTeXMajorMode::indentRegion(const QTextCursor& tc) {
    if (!tc.hasSelection()) {
      return;
    }
    QTextCursor b;
    QTextCursor e;
    this->beginAndEndOfSelection(b, e, tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    e.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    // this is a paranoïd check
    if (b.atEnd()) {
      return;
    }
    if (b == e) {
      this->indentLine(tc);
      return;
    }
    // the position of the cursor is memorized before the region is
    // indented and restored once it has been indented, so that the
    // cursor remains next to the character it was next to before the
    // indentation.
    const auto m = this->getCursorMark(tc);
    int i;
    int di;
    bool bi;
    this->getLineIndentation(b, i, di, bi);
    b.beginEditBlock();
    while ((b <= e) && (!b.atEnd())) {
      QString l =
          LaTeXSyntaxHighlighter::stripComment(b.block().text()).trimmed();
      di = this->getIndentationIncrement(l);
      if (l.isEmpty()) {
        // empty lines are left untouched: reindenting a region shall
        // never introduce blank lines, nor remove them, nor write
        // trailing blanks on blank lines.
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        continue;
      }
      // see LaTeXMajorMode::getLineIndentation for the polarity
      bi = !this->re2.match(l).hasMatch();
      if (l.size() > 0) {
        if (l[0] == '}') {
          bi = false;
        }
      }
      if (bi) {
        // no setTextCursor for each line, see MajorModeBase::doIndentLine
        MajorModeBase::doIndentLine(b, 2 * i, false);
      } else {
        MajorModeBase::doIndentLine(b, 2 * (i + di), false);
      }
      i += di;
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    b.endEditBlock();
    this->restoreCursorMark(m);
  }  // end of LaTeXMajorMode::indentRegion

  QString LaTeXMajorMode::getCommentSyntax() {
    return "%";
  }  // end of LaTeXMajorMode::getCommentSyntax

  // `delete this->c` used to live here. Since c was never assigned, it was a
  // delete of a null pointer: a no-op, and dead code.
  LaTeXMajorMode::~LaTeXMajorMode() = default;

  static StandardMajorModeProxy<LaTeXMajorMode> proxy(
      "LaTeX",
      QVector<QRegularExpression>()
          << suffixRegExp("tex") << suffixRegExp("sty") << suffixRegExp("cls")
          << suffixRegExp("dtx"),
      ":/tfel/editor/languages/latex.png");

}  // end of namespace tfel::gui
