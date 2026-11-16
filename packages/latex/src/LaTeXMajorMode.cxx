/*!
 * \file  LaTeXMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   18/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtGui/QActionGroup>
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

  namespace {

    /*!
     * \brief one of the programs a document of LaTeX is compiled with
     */
    struct Interpreter {
      //! name of the program, as it is started, and as the entry is named
      const char *name;
      //! sentence which says what that program does with the file
      const char *tip;
    };  // end of struct Interpreter

    //! the address the choice of the interpreter is kept in
    const char interpreter_address[] = "latex/interpreter";

    /*!
     * \return the programs a document is compiled with, in the order they are
     * offered, which is the order both the `Interpreter` submenu of the mode
     * and the combo box of the configuration dialog offer them in
     *
     * \note `latex` stands first and is the default: it is the program the mode
     * ran before there was a choice to make, and the answer a configuration
     * file which says nothing gives. The three are the engines of every TeX
     * distribution; a fourth one, `lualatex`, is none of what was asked here
     * and would be one entry more in this list, in the submenu and in the
     * choices of the entry, with nothing else to change
     */
    QVector<Interpreter> interpreters() {
      return QVector<Interpreter>()
          << Interpreter{
                 "latex",
                 "The traditional engine. It writes a DVI file, which is no "
                 "document one reads as it is: a step outside the editor, "
                 "`dvips` or `dvipdfmx`, makes a PDF file of it. It takes no "
                 "PNG nor JPEG image and only the fonts its map files name."}
          << Interpreter{"pdflatex",
                         "The engine which writes a PDF file of the file it is "
                         "given: no DVI step, and the images of the document "
                         "included as they are. What a document which is to be "
                         "read on screen and asks for no font of the system is "
                         "usually compiled with."}
          << Interpreter{
                 "xelatex",
                 "The engine of Unicode and of the fonts of the system: it "
                 "takes any font the session names, through the `fontspec` "
                 "package, and writes a PDF file directly. Slower than "
                 "`pdflatex`, and what a document which loads `fontspec` "
                 "needs."};
    }  // end of interpreters()

    /*!
     * \return the program a document is compiled with, from the address
     * `latex/interpreter`
     *
     * \note `latex` is what nothing stored answers with, and what a name which
     * is none of the three answers with too: the address is read on every
     * compile, so what the `Interpreter` submenu or the configuration dialog
     * chose reaches the next compile, and a name no TeX distribution has would
     * otherwise be reported as a program which does not exist — which is a
     * mistake in the configuration file, not a question to ask of a person
     * working on a document
     */
    QString chosenInterpreter() {
      const auto v = QSettings()
                         .value(QLatin1String(interpreter_address),
                                QStringLiteral("latex"))
                         .toString();
      for (const auto& i : interpreters()) {
        if (v.compare(QLatin1String(i.name), Qt::CaseInsensitive) == 0) {
          return QString::fromLatin1(i.name);
        }
      }
      return QStringLiteral("latex");
    }  // end of chosenInterpreter()

    /*!
     * \brief choose the program the documents of the editor are compiled with
     * \param[in] editor : editor widget to report the choice in
     * \param[in] name   : name of one of the three engines
     *
     * \note the choice is one for the editor and not for the buffer it was
     * asked in: a document is compiled with the engine its person works with,
     * wherever they happen to be standing when they say so. What is chosen is
     * reported in the minibuffer and reaches the next compile, since the
     * address is read when a process is started
     */
    void setInterpreter(EditorWidget& editor, const QString& name) {
      QSettings().setValue(QLatin1String(interpreter_address), name);
      editor.displayInformativeMessage(
          QObject::tr("compiling with %1").arg(name));
    }  // end of setInterpreter()

    /*!
     * \brief put the tick of the entries of \c group on the entry naming the
     *        engine the settings hold
     * \param[in] group : the three entries of the `Interpreter` submenu
     *
     * \note the tick is drawn when the submenu is built and again every time it
     * is opened: the configuration dialog, or a configuration file edited by
     * hand, answers the question differently between two builds of the menu, and
     * a tick which stayed where the last click put it would name a program the
     * next compile does not run
     */
    void redrawTick(const QActionGroup& group) {
      const auto name = chosenInterpreter();
      for (auto* const a : group.actions()) {
        a->setChecked(a->text() == name);
      }  // end of for
    }  // end of redrawTick()

    /*!
     * \return the values the combo box of the configuration dialog offers, in
     * the order \c interpreters answers them, the first one being the default
     *
     * \note the label of a choice is the name of the program: a person chooses
     * the engine their distribution installs under that name, and no other
     * spelling of it says anything one of the three does not
     */
    QVector<SettingsKey::Choice> interpreterChoices() {
      auto cs = QVector<SettingsKey::Choice>();
      for (const auto& i : interpreters()) {
        cs << SettingsKey::Choice(QLatin1String(i.name),
                                  QString::fromLatin1(i.name));
      }
      return cs;
    }  // end of interpreterChoices()

    /*!
     * \return the range of the word of the given text the given position stands
     * in, as a pair of a first position and of a position past the last
     * character, and `{-1, -1}` when the position stands on no word
     * \param[in] t : text to scan, a line or a portion of a line
     * \param[in] p : position in \c t
     *
     * A word is a run of letters, the same split \c SpellChecker::spellLine
     * reports the misspelled words of a line with.
     *
     * \note a position of a document is a place between two characters, not a
     * character: the click on the first letter of a word is answered by the
     * place which precedes it and the click on its last letter by the place
     * which follows it as likely as not, so both bounds of the run belong to
     * the word. A position which stands between two words therefore names
     * neither of them
     */
    std::pair<int, int> wordAt(const QString& t, const int p) {
      auto pos = 0;
      while (pos != t.size()) {
        if (!t[pos].isLetter()) {
          ++pos;
          continue;
        }
        const auto first = pos;
        while ((pos != t.size()) && (t[pos].isLetter())) {
          ++pos;
        }
        if ((p >= first) && (p <= pos)) {
          return {first, pos};
        }
      }
      return {-1, -1};
    }  // end of wordAt()

  }  // end of anonymous namespace

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
    // the prose of a document is what a proofreader reads, and none of the
    // commands are: what of the text is markup is said by `grammarCheckText`, and
    // `true` here says that the whole of it is text to be read — which is what
    // makes a buffer of LaTeX the one kind of buffer LanguageTool reads on its own
    // account, dictionary off, see `SpellingAndGrammar::effectiveChecker`. What
    // reads this buffer is the choice held in the settings when the mode is built,
    // the same choice the buffers opened afterwards follow
    this->grammar =
        new GrammarCheckSession(w, t, QStringLiteral("latex"), true, this);
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

  MajorMode::Category LaTeXMajorMode::getCategory() const {
    return Category::TEXT_EDITING;
  }  // end of getCategory

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
    // The three engines are web2c programs: they take the file to read as a plain
    // argument, add the `.tex' extension when the name has none, and report the
    // places of the file in one shape, which is what the `latex-output' mode of
    // the frame reads. What the name is followed by used to be a quoted `\input'
    // command, which is a trick of the way `pdftex' scans its command line and
    // which `xelatex' does not know: handed the argument `"\input"', that program
    // looked for a file named `"' and stopped (`I can't find file '..."'`,
    // measured). The file name alone reaches the three of them, absolute or
    // relative, with or without an extension, with or without a space in it
    QStringList args;
    args << "-file-line-error"
         << "-interaction=nonstopmode"
         << m;
    p.start(chosenInterpreter(), args);
  }  // end of LaTeXMajorMode::startLaTeX

  QMenu* LaTeXMajorMode::getSpecificMenu() {
    auto* const t = qobject_cast<QWidget*>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    auto* const m = new QMenu(QObject::tr("LaTeX"), t);
    m->setIcon(this->getIcon());
    // which TeX reads the file is one exclusive question, and its answers are
    // programs, named as they are called: an engine is chosen by name, and no
    // sentence dressed around `pdflatex` says anything the name does not
    auto* const interpreter_menu = m->addMenu(QObject::tr("Interpreter"));
    interpreter_menu->setObjectName(QStringLiteral("latex-interpreter"));
    interpreter_menu->setToolTip(
        QObject::tr("Which program compiles this document (Alt+M). The entry "
                    "`Program which compiles a document` of the `latex` theme "
                    "of the configuration writes the same address"));
    auto* const group = new QActionGroup(interpreter_menu);
    group->setExclusive(true);
    for (const auto& i : interpreters()) {
      const auto name = QString::fromLatin1(i.name);
      auto* const a = interpreter_menu->addAction(name);
      a->setCheckable(true);
      a->setToolTip(QObject::tr(i.tip));
      group->addAction(a);
      QObject::connect(a, &QAction::triggered, this, [this, name] {
        setInterpreter(this->editor, name);
      });
    }
    // the tick is drawn when the submenu is built and again every time it is
    // opened: the configuration dialog, the menu of another buffer, or a
    // configuration file edited by hand can have changed the engine since, and a
    // menu ticking the program which is not the one that compiles is a menu which
    // lies about what the next `Alt+M` does
    redrawTick(*group);
    QObject::connect(interpreter_menu, &QMenu::aboutToShow, this,
                     [group] { redrawTick(*group); });
    if (GrammarCheckSession::isAvailable()) {
      // the four choices of the proofreader and, under them, the languages that
      // choice reads. The checker of harper says on its own name that it knows
      // no word of French: a document of French is read by hunspell, by the
      // spellings of the dictionary which is chosen for it, and by nothing else
      m->addSeparator();
      this->grammar->addToMenu(*m);
    }
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
    // a comment nor a LaTeX command: the very same split of the line is used
    // here and in the highlighter, so that the menu and the underlining always
    // agree on what a misspelled word may lie in
    for (const auto& p : LaTeXSyntaxHighlighter::parseLine(text).plainText) {
      if ((posl < p.first) || (posl > p.first + p.second)) {
        continue;
      }
      const auto portion = text.mid(p.first, p.second);
      const auto w = wordAt(portion, posl - p.first);
      if (w.first < 0) {
        continue;
      }
      // one position stands in one word: what follows cannot hold it too
      this->addSpellCheckSuggestions(
          m, this->spellChecker, portion.mid(w.first, w.second - w.first),
          bl.position() + p.first + w.first,
          bl.position() + p.first + w.second);
      return;
    }
  }  // end of LaTeXMajorMode::completeContextMenu

  void LaTeXMajorMode::setSpellCheckLanguage(const QString& l) {
    this->spellChecker.setSpellCheckLanguage(l);
    if (!this->highlighter.isNull()) {
      this->highlighter->rehighlight();
    }
  }  // end of LaTeXMajorMode::setSpellCheckLanguage

  QString LaTeXMajorMode::grammarCheckText(const QString &text) const {
    // The markup of a line is blanked out and not removed: the answer of a
    // proofreader is a place in the text it was handed, and a text shortened by
    // what was taken out of it would have every place behind that run land on a
    // word the buffer does not hold there. A blanked run is nothing to the
    // checker and no word of it is reported, which is what a `\command` is to a
    // person reading the text of the file: `\textbf{value}` handed over as it
    // stands reads as a typo in `textbf` and says nothing of `value`, and that is
    // the whole of what the mode has to keep a proofreader from saying.
    //
    // What is kept is what `LaTeXSyntaxHighlighter::parseLine` names the plain
    // text of a line — the portions which are neither a command nor a comment —
    // and the text of the comment of the line: a note to a reader is prose like
    // any other and only its `%` is markup.
    auto result = QString();
    const auto lines = text.split(QLatin1Char('\n'));
    for (auto i = 0; i != lines.size(); ++i) {
      const auto &line = lines.at(i);
      const auto parsed = LaTeXSyntaxHighlighter::parseLine(line);
      auto kept = QVector<bool>(line.size(), false);
      for (const auto &part : parsed.plainText) {
        // the portions are pairs of a position and of a *length*, as the
        // highlighter spells them for the spell checker it draws with
        const auto end = qMin(part.first + part.second, line.size());
        for (auto k = qMax(0, part.first); k != end; ++k) {
          kept[k] = true;
        }
      }
      if (parsed.comment >= 0) {
        for (auto k = parsed.comment + 1; k != line.size(); ++k) {
          kept[k] = true;
        }
      }
      auto blanked = line;
      for (auto k = 0; k != blanked.size(); ++k) {
        if (!kept[k]) {
          blanked[k] = QLatin1Char(' ');
        }
      }
      result += blanked;
      if (i + 1 != lines.size()) {
        result += QLatin1Char('\n');
      }
    }
    return result;
  }  // end of LaTeXMajorMode::grammarCheckText

  GrammarCheckSession *LaTeXMajorMode::grammarCheckSession() const {
    // the choice of the proofreader walks the buffers of the editor through this
    // hook, and a document of LaTeX is the one kind of buffer it reads whole: the
    // text is handed to LanguageTool with its markup blanked out, see \c
    // grammarCheckText, and no dictionary stands beside it
    return this->grammar;
  }  // end of LaTeXMajorMode::grammarCheckSession

  SpellChecker& LaTeXMajorMode::getSpellChecker() { return this->spellChecker; }

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

  QVector<SettingsGroup> LaTeXMajorMode::declareSettings() {
    // the one entry the mode reads is gathered in a theme named after the
    // programs it launches, as the C++ mode does for `clangd` and the `Cast3M`
    // mode for `castem`: a person who finds that the file which comes out of
    // `Alt+M` is a DVI where they expected a PDF looks for the engine, and the
    // family of programs which read a `.tex` file is named `latex`. The icon of
    // the theme is the one of the mode, which is what the proxy of the mode adds
    auto latex = SettingsGroup{};
    latex.name = QStringLiteral("latex");
    latex.description = QObject::tr(
        "The program a document of this editor is compiled with. The address "
        "is read when a document is compiled, so what is chosen here reaches "
        "the next compile and leaves a compile which is running alone");
    latex.keys =
        QVector<SettingsKey>()
        << SettingsKey::choice(
               interpreter_address,
               QObject::tr("Program which compiles a document"),
               QObject::tr("Which TeX reads the file. `latex` writes a DVI "
                           "file, which a step outside the editor turns into a "
                           "PDF file; `pdflatex` writes a PDF file directly and "
                           "takes the images of the document as they are; "
                           "`xelatex` is the engine of a document written in "
                           "Unicode and asking for the fonts of the system, "
                           "through `fontspec`. The `Interpreter` submenu of the "
                           "`LaTeX` menu writes this same address and the tick "
                           "of its entries says which one a compile would use. "
                           "Whichever ran, its report is shown in the "
                           "secondary task of the buffer and read the same way, "
                           "since the three are web2c programs writing one "
                           "shape of report"),
               interpreterChoices(), QStringLiteral("latex"));
    // nothing else: the file the master document of a buffer is, and the name of
    // the buffer being compiled, are facts of the document, not choices of the
    // person using the editor, and the mode remembers nothing else of its own
    return QVector<SettingsGroup>() << latex;
  }  // end of LaTeXMajorMode::declareSettings

  static StandardMajorModeProxy<LaTeXMajorMode> proxy(
      "LaTeX",
      QVector<QRegularExpression>()
          << suffixRegExp("tex") << suffixRegExp("sty") << suffixRegExp("cls")
          << suffixRegExp("dtx"),
      ":/tfel/editor/languages/latex.png");

}  // end of namespace tfel::gui
