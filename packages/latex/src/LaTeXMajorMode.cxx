/*!
 * \file  LaTeXMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   18/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsPlainTextEdit.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/LaTeXSyntaxHighlighter.hxx"
#include "QEmacs/LaTeXMajorMode.hxx"

namespace qemacs {

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

  struct LaTeXMajorMode::LaTeXInsertBlock : public QEmacsLineEdit {
    LaTeXInsertBlock(QEmacsWidget& p, QEmacsTextEditBase& t)
        : QEmacsLineEdit(QObject::tr("insert block :"), p),
          textEdit(t) {
      QStringList n;
      n << "figure"
        << "minipage"
        << "equation";
      //      = LaTeXSyntaxHighlighter::getPrimaryBlocks();
      // n <<  LaTeXSyntaxHighlighter::getSecondaryBlocks();
      auto* c = new QCompleter(n, &p);
      c->setWidget(this->input);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->input->setCompleter(c, false);
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

    QEmacsTextEditBase& textEdit;

  };  // end of struct QEmacsTextEdit::LaTeXInsertBlock

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

  LaTeXMajorMode::LaTeXMajorMode(QEmacsWidget& w,
                                 QEmacsBuffer& b,
                                 QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w, b, t, &t),
        rb("\\\\begin\\{\\w+\\}"),
        re("\\\\end\\{\\w+\\}"),
        rb2("^\\s*\\\\begin\\{\\w+\\}"),
        re2("^\\s*\\\\end\\{\\w+\\}"),
        c(0),
        highlighter(nullptr) {
    qDebug() << "TeX-master : " << this->getTeXMasterFile();
  }  // end of LaTeXMajorMode::LaTeXMajorMode

  QString LaTeXMajorMode::getTeXMasterFile() {
    QRegExp r("^\\s*%%% TeX-master: \"([\\w-0-9_\\./]+)\"\\s*$");
    QTextCursor tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    while (!tc.atEnd()) {
      QString line = tc.block().text();
      if (r.indexIn(line) != -1) {
        return r.cap(1);
      }
      tc.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }
    return QString();
  }  // end of LaTeXMajorMode::getTeXMasterFile

  QString LaTeXMajorMode::getName() const {
    return "LaTeX";
  }  // end of LaTeXMajorMode::LaTeXMajorMode

  QString LaTeXMajorMode::getDescription() const {
    return "major mode dedicated to the LaTeX";
  }  // end of LaTeXMajorMode::LaTeXMajorMode

  void LaTeXMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    this->highlighter = new LaTeXSyntaxHighlighter(*this, d);
  }  // end of LaTeXMajorMode::setSyntaxHighlighter

  QCompleter* LaTeXMajorMode::getCompleter() {
    return this->c;
  }  // end of LaTeXMajorMode::getCompleter

  void LaTeXMajorMode::completeCurrentWord(QEmacsTextEditBase& t,
                                           const QString& w) {
    QEmacsMajorModeBase::completeCurrentWord(t, w);
  }  // end of LaTeXMajorModeBase::completeCurrentWord

  void LaTeXMajorMode::runLaTeX() {
    if (this->textEdit.isModified()) {
      QEmacsTextEditBase::SaveInput* input =
          this->textEdit.getSaveInput();
      QObject::connect(input, &QEmacsTextEditBase::SaveInput::finished,
                       this, &LaTeXMajorMode::startLaTeX);
      this->qemacs.setUserInput(input);
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
      this->qemacs.displayInformativeMessage(
          QObject::tr("no file name"));
      return;
    }
    if (this->po == nullptr) {
      this->po = new ProcessOutputFrame(this->qemacs, this->buffer);
      this->buffer.attachSecondaryTask("*LaTeX* ouput", po);
    } else {
      this->buffer.setSecondaryTaskTitle(po, "*LaTeX* ouput");
      this->buffer.setSecondaryTaskIcon(po, QIcon());
    }
    po->clear();
    QFileInfo fn(m);
    if (!fn.exists()) {
      this->qemacs.displayInformativeMessage(
          QObject::tr("file '%1' does "
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
    return nullptr;
  }  // end of CppMajorMode::getSpecificMenu

  QIcon LaTeXMajorMode::getIcon() const {
    static QIcon i(":/qemacs/languages/latex.png");
    return i;
  }  // end of LaTeXMajorMode::getIcon()

  void LaTeXMajorMode::completeContextMenu(QMenu* const m,
                                           const QTextCursor& tc) {
    typedef LaTeXSyntaxHighlighter::HighlightingRule Rule;
    typedef QVector<Rule> Rules;
    QEmacsMajorModeBase::completeContextMenu(m, tc);
    if (this->highlighter != nullptr) {
      QTextCursor bl(tc);
      bl.movePosition(QTextCursor::StartOfBlock,
                      QTextCursor::KeepAnchor);
      int posl = tc.position() - bl.position();
      const auto text = tc.block().text();
      const auto& rules =
          LaTeXSyntaxHighlighter::getHighlightingRules();
      // remove comments
      int pc = LaTeXSyntaxHighlighter::startOfComment(text);
      QString l;
      if (pc != -1) {
        if (posl >= pc) {
          return;
        }
        l = text.mid(0, pc);
      } else {
        l = text.mid(0, pc);
      }
      int pos = 0;
      while (pos != l.size()) {
        int cpos = -1;
        Rules::const_iterator mp;
        Rules::const_iterator p;
        mp = rules.end();
        for (p = rules.begin(); p != rules.end(); ++p) {
          const Rule& rule = *p;
          QRegExp e(rule.pattern);
          int rp = e.indexIn(text, pos);
          if (rp != -1) {
            if ((cpos == -1) || (rp <= cpos)) {
              mp = p;
              cpos = rp;
            }
          }
        }
        if (cpos != -1) {
          if (cpos != pos) {
            if ((posl >= pos) && (posl <= cpos)) {
              this->completeContextMenuForMispelledWord(
                  m, text.mid(pos, cpos - pos), posl - pos,
                  bl.position() + pos);
            }
          }
          // treating the LaTeX command
          const Rule& rule = *mp;
          QRegExp e(rule.pattern);
          e.indexIn(text, pos);
          int length = e.matchedLength();
          pos = cpos + length;
        } else {
          if (posl >= pos) {
            this->completeContextMenuForMispelledWord(
                m, text.mid(pos, text.size() - pos), posl - pos,
                bl.position() + pos);
          }
          pos = l.size();
        }
      }
    }
  }

  void LaTeXMajorMode::setSpellCheckLanguage(const QString& l) {
    this->spellChecker.setSpellCheckLanguage(l);
    if (this->highlighter != nullptr) {
      this->highlighter->rehighlight();
    }
  }  // end of LaTeXMajorMode::setSpellCheckLanguage

  void LaTeXMajorMode::completeContextMenuForMispelledWord(QMenu *m,
                                                           const QString &l,
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
            for (auto &pa : this->suggestions) { delete pa; }
            this->suggestions.clear();
            const auto ss = this->spellChecker.suggest(w);
              for (const auto& s : ss) {
              auto *a = new QAction(s, this);
              a->setData(s);
              this->suggestions.push_back(a);
            }
            if (this->suggestions.size() != 0) {
              auto cactions = m->actions();
              if (!cactions.isEmpty()) {
                m->insertAction(*(cactions.begin()), m->addSeparator());
              }
              for (int i = this->suggestions.size(); i > 0; --i) {
                auto *a = this->suggestions[i - 1];
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
  } // end of LaTeXMajorMode::completeContextMenuForMispelledWord

  SpellChecker& LaTeXMajorMode::getSpellChecker() {
    return this->spellChecker;
  }

  void LaTeXMajorMode::replaceMispelledWordBySuggestion(QAction *a) {
    if (this->suggestions.indexOf(a) != -1) {
      QTextCursor tc(this->textEdit.textCursor());
      tc.beginEditBlock();
      tc.setPosition(this->bmwp);
      tc.movePosition(QTextCursor::NextCharacter,
                      QTextCursor::KeepAnchor, this->emwp - this->bmwp);
      tc.removeSelectedText();
      tc.insertText(a->data().toString());
      tc.endEditBlock();
    }
  }  // end of LaTeXMajorMode::replaceMispelledWordBySuggestion

  bool LaTeXMajorMode::handleShortCut(const int k1,
                                      const Qt::KeyboardModifiers m,
                                      const int k2) {
    if ((k1 == Qt::Key_C) && (m == Qt::ControlModifier) &&
        (k2 == Qt::Key_E)) {
      this->qemacs.setUserInput(
          new LaTeXInsertBlock(this->qemacs, this->textEdit));
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
    cc = bc;
    cc.beginEditBlock();
    QVector<QStringList> lines;
    cc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    ec.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    while ((!cc.atEnd()) && (cc <= ec)) {
      QString l = cc.block().text();
      if (l.trimmed().isEmpty()) {
        lines.push_back(QStringList());
        lines.push_back(QStringList());
        cc.movePosition(QTextCursor::NextBlock,
                        QTextCursor::MoveAnchor);
        cc.movePosition(QTextCursor::EndOfBlock,
                        QTextCursor::MoveAnchor);
        continue;
      }
      int cpos = LaTeXSyntaxHighlighter::startOfComment(l);
      if (lines.isEmpty()) {
        lines.push_back(QStringList());
      }
      if (cpos != -1) {
        lines.back() << l.mid(0, cpos).split(' ',
                                             QString::SkipEmptyParts);
        lines.back() << l.mid(cpos, l.size() - cpos);
        lines.push_back(QStringList());
      } else {
        lines.back() << l.split(' ', QString::SkipEmptyParts);
      }
      cc.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      cc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }
    if (ec.atEnd()) {
      QString l = cc.block().text();
      int cpos = LaTeXSyntaxHighlighter::startOfComment(l);
      if (lines.isEmpty()) {
        lines.push_back(QStringList());
      }
      if (cpos != -1) {
        lines.back() << l.mid(0, cpos).split(' ',
                                             QString::SkipEmptyParts);
        lines.back() << l.mid(cpos, l.size() - cpos);
        lines.push_back(QStringList());
      } else {
        lines.back() << l.split(' ', QString::SkipEmptyParts);
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
    for (pl = lines.begin(); pl != lines.end(); ++pl) {
      QVector<QStringList>::const_iterator pl2 = pl;
      QStringList::const_iterator pw;
      ++pl2;
      for (pw = pl->begin(); pw != pl->end(); ++pw) {
        QStringList::const_iterator pw2 = pw;
        ++pw2;
        const QString& w = *pw;
        QString indent;
        bool sk = false;
        if (((w == "\\[") || (w == "\\]") || (w == "\\medskip") ||
             (this->rb.indexIn(w, 0) != -1) ||
             (this->re.indexIn(w, 0) != -1))) {
          if (size != 0) {
            cc.insertText("\n");
            size = 0;
          }
          sk = true;
        }
        if ((w == "\\item") && (size != 0)) {
          cc.insertText("\n");
          size = 0;
        }
        if (w.size() >= 72) {
          if (size != 0) {
            cc.insertText("\n");
          }
          di = this->getIndentationIncrement(w);
          be = this->re.indexIn(w, 0) == -1;
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
          size = qMax(i, 0);
        } else {
          if (w.size() + size >= 72) {
            di = this->getIndentationIncrement(w);
            be = this->re.indexIn(w, 0) == -1;
            if (w[0] == '}') {
              be = false;
            }
            if (be) {
              if (i > 0) {
                indent = QString(2 * i, ' ');
              }
              size = qMax(w.size() + 2 * i, 0);
            } else {
              if (i + di > 0) {
                indent = QString(2 * (i + di), ' ');
              }
              size = qMax(w.size() + 2 * (i + di), 0);
            }
            cc.insertText("\n" + indent + w);
            i += di;
          } else {
            di = this->getIndentationIncrement(w);
            if (size == 0) {
              be = this->re2.indexIn(w, 0) == -1;
              if (w[0] == '}') {
                be = false;
              }
              if (be) {
                if (i > 0) {
                  indent = QString(2 * i, ' ');
                }
                size = qMax(2 * i + w.size(), 0);
              } else {
                if (i + di > 0) {
                  indent = QString(2 * (i + di), ' ');
                }
                size = qMax(2 * (i + di) + w.size(), 0);
              }
              cc.insertText(indent + w);
              i += di;
              size += w.size() + indent.size();
            } else {
              cc.insertText(' ' + w);
              size += w.size() + 1u;
              i += di;
            }
          }
        }
        if (!((pl2 == lines.end()) && (pw2 == pl->end()))) {
          if ((sk) || (w == "\\\\")) {
            cc.insertText("\n");
            size = 0;
          }
        }
      }
    }
    cc.endEditBlock();
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
    int i;
    int di;
    bool b;
    this->getLineIndentation(tc, i, di, b);
    if (b) {
      QEmacsMajorModeBase::doIndentLine(tc, 2 * i);
    } else {
      QEmacsMajorModeBase::doIndentLine(tc, 2 * (i + di));
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
    QString l = LaTeXSyntaxHighlighter::stripComment(tc.block().text())
                    .trimmed();
    b = this->re2.indexIn(l, 0) == -1;
    if (l.size() > 0) {
      if (l[0] == '}') {
        b = false;
      }
    }
    di = this->getIndentationIncrement(l);
  }  // end of LaTeXMajorMode::getLineIndentation

  int LaTeXMajorMode::getIndentationIncrement(
      const QString& line) const {
    int di = 0;
    QString l = LaTeXSyntaxHighlighter::stripComment(line).trimmed();
    if (l.isEmpty()) {
      return di;
    }
    int pos = 0;
    int ppos = 0;
    while (pos != -1) {
      int pos1 = rb.indexIn(l, pos);
      int pos2 = re.indexIn(l, pos);
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
        pos = pos1;
        if (l.mid(pos, rb.matchedLength()) != "\\begin{document}") {
          di += 1;
        }
        if (pos == -1) {
          di += countNumberOfCurlyBraces(l.mid(ppos, l.size() - ppos));
        } else if (ppos != pos) {
          di += countNumberOfCurlyBraces(l.mid(ppos, pos - ppos));
        }
        pos += rb.matchedLength();
      } else if (b2) {
        pos = pos2;
        if (l.mid(pos, rb.matchedLength()) != "\\end{document}") {
          di -= 1;
        }
        if (pos == -1) {
          if (ppos != l.size()) {
            di +=
                countNumberOfCurlyBraces(l.mid(ppos, l.size() - ppos));
          }
        } else if (ppos != pos) {
          di += countNumberOfCurlyBraces(l.mid(ppos, pos - ppos));
        }
        pos += re.matchedLength();
      } else {
        if (ppos != l.size()) {
          di += countNumberOfCurlyBraces(l.mid(ppos, l.size() - ppos));
        }
        pos = -1;
      }
      ppos = pos;
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
    int i;
    int di;
    bool bi;
    this->getLineIndentation(b, i, di, bi);
    b.beginEditBlock();
    while ((b <= e) && (!b.atEnd())) {
      QString l = LaTeXSyntaxHighlighter::stripComment(b.block().text())
                      .trimmed();
      di = this->getIndentationIncrement(l);
      bi = this->re2.indexIn(l, 0) == -1;
      if (l.size() > 0) {
        if (l[0] == '}') {
          bi = false;
        }
      }
      if (bi) {
        QEmacsMajorModeBase::doIndentLine(b, 2 * i);
      } else {
        QEmacsMajorModeBase::doIndentLine(b, 2 * (i + di));
      }
      i += di;
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    b = e;
    b.clearSelection();
    b.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    b.endEditBlock();
    this->textEdit.setTextCursor(b);
  }  // end of LaTeXMajorMode::indentRegion

  QString LaTeXMajorMode::getCommentSyntax() {
    return "%";
  }  // end of LaTeXMajorMode::getCommentSyntax

  LaTeXMajorMode::~LaTeXMajorMode() {
    delete this->c;
  }  // end of LaTeXMajorMode::~LaTeXMajorMode()

  static StandardQEmacsMajorModeProxy<LaTeXMajorMode> proxy(
      "LaTeX",
      QVector<QRegExp>() << QRegExp("^[\\w-0-9_\\.]+\\.tex")
                         << QRegExp("^[\\w-0-9_\\.]+\\.sty"));

}  // end of namespace qemacs
