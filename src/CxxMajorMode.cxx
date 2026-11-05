/*!
 * \file  CxxMajorMode.cxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtGui/QTextDocument>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QListView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtCore/QStringListModel>
#include <QtCore/QPair>
#include <QtCore/QSize>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPalette>
#include <algorithm>
#include <memory>

#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspCxxSupport.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"
#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel::gui {

  namespace {

    // the colors of the diagnostic underlines, by the severity the protocol
    // defines: error, warning, information, hint
    QColor diagnosticColor(const int severity) {
      switch (severity) {
        case 1:
          return QColor(170, 0, 0);
        case 2:
          return QColor(180, 100, 0);
        case 3:
          return QColor(0, 80, 160);
        default:
          return QColor(120, 120, 120);
      }
    }  // end of diagnosticColor()

    // the name of a severity, for the messages the mode echoes: the underline
    // says where and its color how serious, the echo is where the text of the
    // message is finally read, and it has to say which kind it is too
    QString diagnosticLabel(const int severity) {
      switch (severity) {
        case 1:
          return QStringLiteral("error");
        case 2:
          return QStringLiteral("warning");
        case 3:
          return QStringLiteral("note");
        default:
          return QStringLiteral("hint");
      }
    }  // end of diagnosticLabel()

    // the varieties of English the grammar checker is asked to read a text as,
    // spelled as the entries of the configuration want them: the value stored
    // is the name the server reads in its settings, the label the one a person
    // chooses under. Both come from \c LspHarperSupport, which is what the
    // client answers and what the submenus of the modes show, so the three
    // cannot disagree about which varieties there are
    QVector<SettingsKey::Choice> dialectChoices() {
      auto choices = QVector<SettingsKey::Choice>();
      for (const auto &name : LspHarperSupport::dialects()) {
        choices << SettingsKey::Choice(name,
                                       LspHarperSupport::dialectLabel(name));
      }
      return choices;
    }  // end of dialectChoices()

    // the face a run of text the server says what it is is drawn with: a
    // restrained set, which colors what a regular expression can not tell
    // apart and leaves the rest of the text as the highlighter already drew it
    //
    // the names are those of the legend the server announced, not of the
    // protocol: `clangd` names eleven of its kinds, three of them `variable`,
    // and a name which is not in the list simply gets no face
    bool semanticFaceOf(const LspSemanticToken &token,
                        QTextCharFormat &format) {
      // `auto` is answered as the type it is deduced to be: coloring that word
      // would ink a keyword over, which the highlighter owns
      if (token.modifiers.contains(QStringLiteral("deduced"))) {
        return false;
      }
      if (token.modifiers.contains(QStringLiteral("deprecated"))) {
        // what the server says a name is not worth using anymore: the face of
        // a strike-out, whatever else the name is
        format.setFontStrikeOut(true);
        return true;
      }
      const auto &type = token.type;
      if (type == QLatin1String("function") ||
          type == QLatin1String("method")) {
        format.setForeground(QColor(Qt::darkCyan));
        return true;
      }
      if (type == QLatin1String("macro")) {
        // not bold: the bold dark magenta is the keyword face of the
        // highlighter, and the two sets of words which get it never overlap
        format.setForeground(QColor(Qt::darkMagenta));
        return true;
      }
      if (type == QLatin1String("class") || type == QLatin1String("struct") ||
          type == QLatin1String("enum") || type == QLatin1String("type") ||
          type == QLatin1String("typeParameter") ||
          type == QLatin1String("concept")) {
        format.setForeground(QColor(Qt::darkBlue));
        return true;
      }
      return false;
    }  // end of semanticFaceOf()

    // the parameter list of a call as the minibuffer shows it: the label the
    // server wrote, with the argument being typed picked out of it. The
    // minibuffer is a plain line of text, so the markers are characters, and
    // `int f(int a, <double b>)` reads as the second argument of a call of f
    QString signature_text(const LspSignature &signature, const int active) {
      if ((active < 0) || (active >= signature.parameters.size())) {
        return signature.label;
      }
      const auto &parameter = signature.parameters.at(active);
      auto start = parameter.start;
      auto end = parameter.end;
      if ((start < 0) || (end <= start) || (end > signature.label.size())) {
        // a server which named the parameter instead of placing it, or placed
        // it past a label it has since shortened: the name is the only thing
        // left to look for, and it is usually written in the label
        start = parameter.text.isEmpty()
                    ? -1
                    : signature.label.indexOf(parameter.text);
        end = (start < 0) ? -1 : start + parameter.text.size();
      }
      if (start < 0) {
        return signature.label;
      }
      return signature.label.left(start) + QLatin1Char('<') +
             signature.label.mid(start, end - start) + QLatin1Char('>') +
             signature.label.mid(end);
    }  // end of signature_text()

    // what the mode asks of the server about the words it tracks: whether the
    // symbol under them answers to `go-to-definition`, `go-to-declaration`,
    // or neither; the answers are or-ed together
    constexpr int HAS_DEFINITION = 1;
    constexpr int HAS_DECLARATION = 2;

    // the range of the identifier a position visits, empty when the position
    // visits none: a word is jumped with, not a character, and the range is
    // the whole identifier whether the position sits before its first letter,
    // within it, or just after its last one. The text of the block is enough,
    // an identifier does not span lines, and reading the whole document on
    // every pixel of a mouse move is not what a tracked word is worth
    QPoint identifier_range(const QTextDocument &doc, const int position) {
      const auto block = doc.findBlock(position);
      if (!block.isValid()) {
        return QPoint(-1, -1);
      }
      const auto text = block.text();
      const auto is_identifier = [](const QChar c) {
        return c.isLetterOrNumber() || (c == QLatin1Char('_'));
      };
      auto start = position - block.position();
      auto end = start;
      while ((start > 0) && is_identifier(text.at(start - 1))) {
        --start;
      }
      while ((end < text.size()) && is_identifier(text.at(end))) {
        ++end;
      }
      if (start == end) {
        return QPoint(-1, -1);
      }
      return QPoint(block.position() + start, block.position() + end);
    }  // end of identifier_range()

    // the mouse cursor of a word whose definition or declaration is known:
    // the Freedesktop icon of that jump, the generic jump icon of the theme
    // when it has no specific one, and the pointing hand when it has neither.
    // A cursor which cannot be drawn is worse than a generic one, and the
    // feature itself does not depend on the theme
    QCursor jump_cursor(const char *const name) {
      auto icon = QIcon::fromTheme(QLatin1String(name));
      if (icon.isNull()) {
        icon = QIcon::fromTheme(QStringLiteral("go-jump"));
      }
      const auto pixmap = icon.pixmap(QSize(22, 22));
      if (pixmap.isNull()) {
        return QCursor(Qt::PointingHandCursor);
      }
      return QCursor(pixmap, pixmap.width() / 2, pixmap.height() / 2);
    }  // end of jump_cursor()

    // the minibuffer question for the new name of a symbol
    //
    // the position of the symbol is held by the question, not read back from
    // the cursor once the answer comes: the answer is typed while the user
    // can look around, and the cursor of another buffer knows nothing of the
    // symbol which was asked about
    struct RenameSymbolInput : public CommandLine {
      RenameSymbolInput(CxxMajorMode &m,
                        EditorWidget &w,
                        const QString &word,
                        const int symbol_position)
          : CommandLine(QObject::tr("rename %1 to :").arg(word), w),
            mode(&m),
            position(symbol_position) {
        // the empty answer cancels, as if the question had never been asked;
        // pre-filling the old name would let a bare Return rewrite the
        // symbol with its own name and report a lie
        this->input->setText(QString());
        // the name is what tells this question apart from every other input
        // waiting in the minibuffer, for tests as for the completion code
        this->setObjectName(QStringLiteral("rename-symbol"));
      }  // end of RenameSymbolInput

      ~RenameSymbolInput() override = default;

     protected:
      void treatUserInput() override {
        const auto name = this->input->text().trimmed();
        if ((!name.isEmpty()) && (this->mode != nullptr)) {
          this->mode->renameToAt(name, this->position);
        }
      }  // end of treatUserInput

     private:
      //! the mode which asked; null when the buffer it belongs to is gone
      QPointer<CxxMajorMode> mode;
      //! where the symbol stood when the question was raised
      int position;
    };  // end of struct RenameSymbolInput

    // the dash which separates the name of a symbol from what the server says
    // of it in the overview: an hyphen would read as the end of a name, and
    // this is punctuation, not a character of the language
    const auto em_dash = QString(QChar(u'\u2014'));

    // the minibuffer question for a symbol of the file: the overview the
    // server gave is shown as the list of completions, it narrows as a name is
    // typed, and the answer is the place to go to
    //
    // the list is held by the question, not asked for again when the answer
    // comes: the file did not change between the two, and the answer is a
    // place among the entries which were displayed
    struct DocumentSymbolInput : public CommandLine {
      DocumentSymbolInput(CxxMajorMode &m,
                          EditorWidget &w,
                          const QString &file,
                          const QVector<LspDocumentSymbol> &ss)
          : CommandLine(QObject::tr("go to symbol :"), w),
            mode(&m),
            heading(file),
            symbols(ss) {
        QStringList names;
        names.reserve(ss.size());
        for (const auto &s : ss) {
          names << s.name;
        }
        auto *const c = new QCompleter(names, this);
        c->setWidget(this->input);
        c->setCompletionMode(QCompleter::InlineCompletion);
        // nobody types the case of a symbol they have not looked at, and the
        // question is which entry of the list is meant, not which bytes it is
        c->setCaseSensitivity(Qt::CaseInsensitive);
        this->input->setCompleter(c, false);
        // the name is what tells this question apart from every other input
        // waiting in the minibuffer, for tests as for the completion code
        this->setObjectName(QStringLiteral("go-to-symbol"));
        // the whole overview is shown from the start: the question is which of
        // these to go to, and a name is easier to pick from a list than to
        // remember
        this->showCompletions(heading, this->narrowed(QString()));
      }  // end of DocumentSymbolInput

      ~DocumentSymbolInput() override = default;

     protected:
      void treatUserInput() override {
        const auto answer = this->input->text().trimmed();
        // the empty answer cancels, as if the question had never been asked
        if ((answer.isEmpty()) || (this->mode == nullptr)) {
          return;
        }
        // an exact name wins over the names which merely contain it: the
        // answer `paint` means `paint` when the file holds a `paint`, not the
        // twelve methods it is the tail of
        auto exact = QVector<const LspDocumentSymbol *>();
        auto tail = QVector<const LspDocumentSymbol *>();
        for (const auto &s : symbols) {
          if (s.name.compare(answer, Qt::CaseInsensitive) == 0) {
            exact.append(&s);
          } else if (s.name.contains(answer, Qt::CaseInsensitive)) {
            tail.append(&s);
          }
        }
        const auto &matches = exact.isEmpty() ? tail : exact;
        if (matches.isEmpty()) {
          this->mode->report(
              QObject::tr("no symbol named '%1' in this file").arg(answer));
          return;
        }
        // the first of several: the list said which ones the name visits, the
        // message says which one was gone to, and pressing the key again with
        // a longer name is how the next one is picked
        this->mode->goToDocumentSymbol(*matches.at(0));
      }  // end of treatUserInput

      void inputTextChanged(const QString &t) override {
        // the base emits the signal the list is hidden on; showing the
        // narrowed list behind it is what makes the overview follow the name
        // being typed instead of vanishing under it
        CommandLine::inputTextChanged(t);
        this->showCompletions(this->heading, this->narrowed(t.trimmed()));
      }  // end of inputTextChanged

     private:
      //! the mode which asked; null when the buffer it belongs to is gone
      QPointer<CxxMajorMode> mode;
      //! the heading the list of entries is introduced with
      QString heading;
      //! the overview, in the order the file spells its symbols
      QVector<LspDocumentSymbol> symbols;

      /*!\return the entries of the overview the given name visits: a name in
       * the middle of a qualified name visits it, since `paint` is what one
       * types for `Widget::paint`
       */
      QStringList narrowed(const QString &name) const {
        auto entries = QStringList();
        for (const auto &s : symbols) {
          if ((!name.isEmpty()) &&
              (!s.name.contains(name, Qt::CaseInsensitive))) {
            continue;
          }
          // what the server says beside the name is the signature of a
          // function, the kind of a class, and nothing for a namespace: the
          // kind is what is left to say then
          const auto said =
              s.detail.isEmpty() ? symbolKindName(s.kind) : s.detail;
          entries << ((said.isEmpty())
                          ? s.name
                          : (s.name + QStringLiteral(" ") + em_dash +
                             QStringLiteral(" ") + said));
          if (entries.size() == 100) {
            // a header full of accessors is a header full of accessors; the
            // ellipsis is the same one a directory of a thousand files gets
            entries << QStringLiteral("...");
            break;
          }
        }
        return entries;
      }  // end of narrowed
    };   // end of struct DocumentSymbolInput

    // true when requests can be sent: a server is running and its handshake
    // is over, or is expected to be over shortly. No request path launches a
    // server: what a session is launched with, in particular the compilation
    // database, is a decision of the user, see the "C/C++" menu and the
    // `launch-clangd` commands
    bool serverIsReachable(LspClient &c, EditorWidget &w) {
      if (c.isIdle()) {
        w.displayInformativeMessage(QStringLiteral(
            "clangd is not launched: use the 'C/C++' menu or the "
            "'launch-clangd' command"));
        return false;
      }
      if (c.isReady()) {
        return true;
      }
      w.displayInformativeMessage(QStringLiteral("contacting clangd..."));
      return c.waitUntilReady(5000);
    }  // end of serverIsReachable()

    // launch a clangd session on the client of the given editor widget;
    // `arguments` are the extra arguments of the server and `description`
    // completes the message which tells the user about the new session
    //
    // a `report` which is not empty replaces that message whole: a caller
    // which knows what it ended up doing tells the user that instead of
    // letting them piece it together from the fact that a session runs now
    //
    // a session which is still running is replaced: what it was started with,
    // the compilation database above all, can not be changed under a running
    // server, so it can not be changed by asking it either
    void startClangd(EditorWidget &w,
                     const QStringList &arguments,
                     const QString &description,
                     const QString &report = QString()) {
      auto &client = LspClient::get(w);
      const auto restarting = (!client.isIdle());
      auto error = QString();
      if (!client.launch(arguments, error)) {
        w.displayInformativeMessage(error);
        return;
      }
      if (!report.isEmpty()) {
        w.displayInformativeMessage(report);
        return;
      }
      auto message = QStringLiteral("clangd %1")
                         .arg(restarting ? QStringLiteral("restarted")
                                         : QStringLiteral("launched"));
      if (!description.isEmpty()) {
        message += QStringLiteral(": %1").arg(description);
      }
      w.displayInformativeMessage(message);
    }  // end of startClangd()

    // the minibuffer question for the compilation database to use
    //
    // the answer is a path: either the database file, whose directory clangd
    // is told to look in, or a directory holding it, which it is told to look
    // in as it stands; an empty answer launches a plain session, leaving the
    // search to the server
    struct CompileCommandsInput : public FilePathUserInput {
      CompileCommandsInput(EditorWidget &w, const QString &path)
          : FilePathUserInput(QObject::tr("compile commands :"), w) {
        this->input->setText(path);
        this->setInputHistorySettingAddress(
            QStringLiteral("clangd/compile-commands/history"));
      }  // end of CompileCommandsInput

      ~CompileCommandsInput() override = default;

     protected:
      void treatUserInput() override {
        const auto asked = this->input->text().trimmed();
        auto error = QString();
        const auto arguments =
            LspCxxSupport::compileCommandsArguments(asked, error);
        if (!error.isEmpty()) {
          this->editor.displayInformativeMessage(error);
          return;
        }
        // the answer is echoed back as it was given: the user knows what they
        // typed, the directory the option holds is the resolved version of it
        const auto description =
            arguments.isEmpty()
                ? QStringLiteral("without a compilation database")
                : QStringLiteral("with the compilation database of %1")
                      .arg(asked);
        startClangd(this->editor, arguments, description);
      }  // end of treatUserInput
    };   // end of struct CompileCommandsInput

    // the directory the file of the current buffer lives in, which is where
    // the question for a compilation database starts; empty when the buffer
    // visits no file yet
    QString currentFileDirectory(EditorWidget &w) {
      const auto file =
          w.getCurrentBuffer().getMainFrame().getCompleteFileName();
      if (file.isEmpty()) {
        return QString();
      }
      return QFileInfo(file).absolutePath() + QDir::separator();
    }  // end of currentFileDirectory()

    // refuse, and say where it is refused, when the setting turns the whole
    // clangd support off
    bool supportIsEnabled(EditorWidget &w) {
      if (LspCxxSupport::isEnabled()) {
        return true;
      }
      w.displayInformativeMessage(QStringLiteral(
          "'clangd/enable' is false: the clangd support is off"));
      return false;
    }  // end of supportIsEnabled()

    // the extra arguments of a session and the message which tells the user
    // what the session was started with, in the end
    struct ImplicitCompileCommands {
      QStringList arguments;
      QString message;
    };  // end of struct ImplicitCompileCommands

    // the database a session launched without an explicit one uses: the file
    // `tfel-editor` was started with, when `--compile-commands-file` named one
    // which exists, else the one of the project of the current buffer, else
    // none, which is a plain session leaving the search to the server
    //
    // the miss of a default file is not reported as it happens: the message
    // says what the session ended up using, and mentions the default only
    // when it played no part in that
    //
    // the editor performs the search itself, rather than leaving it to the
    // server, because the option which hands a database to clangd is set when
    // the session starts and the user is told which database it was started
    // with, which requires knowing it before launching
    ImplicitCompileCommands implicitCompileCommands(EditorWidget &w) {
      const auto restricted_to_current_file = QStringLiteral(
          "no compile_commands.json found, clangd is restricted to current "
          "C++ file");
      auto missing_default = QString();
      const auto file = LspCxxSupport::defaultCompileCommandsFile();
      if (!file.isEmpty()) {
        const auto path = QFileInfo(file).absoluteFilePath();
        auto error = QString();
        const auto arguments =
            LspCxxSupport::compileCommandsArguments(file, error);
        if (error.isEmpty()) {
          return {arguments,
                  QStringLiteral("using %1 to start clangd").arg(path)};
        }
        // a default which does not exist is a mistake the user made once, at
        // the command line; it does not void the launch, the project is asked
        // instead and the mistake surfaces in the final message only
        missing_default = path;
      }
      const auto directory = currentFileDirectory(w);
      const auto found = LspCxxSupport::searchCompileCommandsFile(
          directory.isEmpty() ? QDir::currentPath() : directory);
      if (found.isEmpty()) {
        if (missing_default.isEmpty()) {
          return {{}, restricted_to_current_file};
        }
        return {{},
                QStringLiteral("no compile_commands.json found and default "
                               "file %1 is not found, clangd is restricted to "
                               "current C++ file")
                    .arg(missing_default)};
      }
      auto error = QString();
      const auto arguments =
          LspCxxSupport::compileCommandsArguments(found, error);
      if (missing_default.isEmpty()) {
        return {arguments,
                QStringLiteral("using %1 to start clangd").arg(found)};
      }
      return {arguments,
              QStringLiteral("using %1 to start clangd (default file %2 "
                             "not found)")
                  .arg(found, missing_default)};
    }  // end of implicitCompileCommands()

  }  // end of namespace

  // the command `launch-clangd`: a session of clangd with no other option
  // than those of the settings, using the compilation database the editor
  // was started with, or the one of the project of the current buffer, or
  // none, in which case clangd searches a database by itself, as it does
  // when it is run in a terminal
  void launchClangd(EditorWidget &editor) {
    if (!supportIsEnabled(editor)) {
      return;
    }
    const auto choice = implicitCompileCommands(editor);
    startClangd(editor, choice.arguments, QString(), choice.message);
  }  // end of launchClangd

  // the command `launch-clangd-with-compile-commands`: the same, after the
  // compilation database to use has been asked for in the minibuffer
  void launchClangdWithCompileCommands(EditorWidget &editor) {
    if (!supportIsEnabled(editor)) {
      return;
    }
    // the question is asked before the client is touched: an answer may name a
    // database, or none at all, and a session started in the meantime would
    // have to be replaced before it was even used
    const auto directory = currentFileDirectory(editor);
    editor.setUserInput(new CompileCommandsInput(editor, directory));
  }  // end of launchClangdWithCompileCommands

  // the command `stop-clangd`: end the session which is running, if any, and
  // report through the minibuffer what became of it
  void stopClangd(EditorWidget &editor) {
    // `findChild` and not `LspClient::get`: asking for the client of an editor
    // which never launched one would build it, which is nothing a stop wants.
    // And by name: the editor widget holds the clients of the grammar checkers
    // too, and one of those is not a session of clangd to stop
    auto *const client = editor.findChild<LspClient *>(QLatin1String("clangd"));
    if ((client == nullptr) || (!client->stop())) {
      editor.displayInformativeMessage(
          QStringLiteral("clangd is not launched"));
      return;
    }
    editor.displayInformativeMessage(QStringLiteral("clangd stopped"));
  }  // end of stopClangd

  CxxMajorMode::CxxMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
      : CMajorModeBase(w, b, t) {
    // what the comments of a C++ file say in English is a question of grammar
    // and none of `clangd`'s: the session of the checker therefore stands on
    // this side of the gate below, and a buffer whose C++ support is off is
    // checked like any other buffer of the editor
    this->grammar = new GrammarCheckSession(w, t, QStringLiteral("cpp"), this);
    if (GrammarCheckSession::isAvailable() &&
        LspHarperSupport::launchByDefault()) {
      // the session follows the setting, which is what the entry of the menu
      // already says by being checked: it announces nothing, and it writes
      // nothing back — the setting is what it is
      this->grammar->setActive(true, false, false);
    }
    if (!LspCxxSupport::isEnabled()) {
      return;
    }
    this->client = &LspClient::get(w);
    QObject::connect(this->client.data(), &LspClient::diagnosticsUpdated, this,
                     &CxxMajorMode::updateDiagnostics);
    // the buffer of a new file has no name when the mode is built, and gains
    // it before or after, depending on how the file was reached; the same
    // holds for a save under a new name
    QObject::connect(&t, &TextEditBase::fileNameChanged, this,
                     [this](const QString &) { this->openOrRenameDocument(); });
    // the word under the cursor is underlined and the word under the mouse
    // asks for the icon of the jump: both are questions of the moment, so one
    // settling timer is shared by the two, restarted by every event which may
    // have moved a position, and the questions below are only asked when it
    // goes off
    this->navigation_timer = new QTimer(this);
    this->navigation_timer->setSingleShot(true);
    this->navigation_timer->setInterval(150);
    QObject::connect(this->navigation_timer, &QTimer::timeout, this,
                     &CxxMajorMode::refreshTrackedWords);
    // typing `(` or `,` asks what call the cursor is inside of; the wait is
    // what keeps a fast typist from one request per character, and the
    // answer goes to the minibuffer, not to the text
    this->signature_timer = new QTimer(this);
    this->signature_timer->setSingleShot(true);
    this->signature_timer->setInterval(120);
    QObject::connect(this->signature_timer, &QTimer::timeout, this,
                     [this] { this->showSignatureHelp(false); });
    // what the identifiers of the file are is a question about the whole file,
    // which a server answers only once it has parsed it: the wait is therefore
    // much longer than the two above, and the questions are asked on their
    // timer alone, restarted by everything which may have changed the text or
    // woken the server up — the handshake, a file parsed, a keystroke
    this->tokens_timer = new QTimer(this);
    this->tokens_timer->setSingleShot(true);
    this->tokens_timer->setInterval(700);
    QObject::connect(this->tokens_timer, &QTimer::timeout, this,
                     &CxxMajorMode::refreshSemanticTokens);
    QObject::connect(this->client.data(), &LspClient::ready, this,
                     [this] { this->tokens_timer->start(); });
    QObject::connect(t.document(), &QTextDocument::contentsChanged, this,
                     [this] { this->tokens_timer->start(); });
    QObject::connect(&t, &TextEditBase::cursorPositionChanged, this, [this] {
      this->cursor_tracking = true;
      this->navigation_timer->start();
    });
    // a stopped session knows nothing anymore: the underlines and the mouse
    // cursor of the one which is gone go away with it
    QObject::connect(this->client.data(), &LspClient::stopped, this,
                     &CxxMajorMode::resetNavigation);
    // the moves of the mouse are the events of the viewport, which reports
    // them only when the tracking is on
    if (auto *const area = t.widget()) {
      area->setMouseTracking(true);
      area->viewport()->setMouseTracking(true);
      area->viewport()->installEventFilter(this);
    }
    this->openOrRenameDocument();
  }  // end of CxxMajorMode::CxxMajorMode

  QString CxxMajorMode::getName() const {
    return "C++";
  }  // end of CxxMajorMode::getName

  QString CxxMajorMode::getLanguageName() const {
    return this->getName();
  }  // end of getLanguageName

  QString CxxMajorMode::getDescription() const {
    return "major mode dedicated to the C++ language";
  }  // end of CxxMajorMode::getDescription

  QIcon CxxMajorMode::getIcon() const {
    return QIcon(":/tfel/editor/languages/cxx.png");
  }  // end of CxxMajorMode::getIcon

  QMenu *CxxMajorMode::getSpecificMenu() {
    auto *const t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    auto *const m = new QMenu(QObject::tr("C/C++"), t);
    m->setIcon(this->getIcon());
    auto *const lc = m->addAction(QObject::tr("Launch clangd"));
    lc->setToolTip(
        QObject::tr("Start clangd with the compilation database of the "
                    "project, or the one tfel-editor was started with; a "
                    "session which is running is restarted"));
    QObject::connect(lc, &QAction::triggered, this,
                     [this] { launchClangd(this->editor); });
    auto *const cd =
        m->addAction(QObject::tr("Launch clangd with a compile_commands.json"));
    cd->setToolTip(QObject::tr(
        "Ask for a compilation database and start clangd with it; a "
        "session which is running is restarted"));
    QObject::connect(cd, &QAction::triggered, this,
                     [this] { launchClangdWithCompileCommands(this->editor); });
    m->addSeparator();
    // the overview of the file is the one entry of this menu which asks a
    // question of the buffer rather than of the session; it is here because a
    // key one has never heard of is a key never pressed
    auto *const gs = m->addAction(QObject::tr("Go to symbol..."));
    gs->setToolTip(
        QObject::tr("Ask which symbol of the file to go to, among the ones "
                    "clangd listed, and show the list as an overview (Alt-O)"));
    QObject::connect(gs, &QAction::triggered, this,
                     [this] { this->showDocumentSymbols(); });
    // the colors a server gives the identifiers are the one thing it says about
    // the text which is nobody's answer to a question: the entry is here
    // because the colors are the first thing a person who did not ask for them
    // notices, and the switch is worth finding next to the entries which
    // started the session which drew them
    auto *const ci = m->addAction(QObject::tr("Color identifiers"));
    ci->setCheckable(true);
    ci->setChecked(LspCxxSupport::semanticTokensEnabled());
    ci->setToolTip(QObject::tr(
        "Ask clangd what the identifiers of the file are and color them with "
        "the answer: the types, the functions, the macros and the names it "
        "says "
        "are deprecated. The choice is kept in the setting "
        "'clangd/semantic-tokens'"));
    QObject::connect(ci, &QAction::toggled, this, [this](const bool on) {
      QSettings().setValue(QStringLiteral("clangd/semantic-tokens"), on);
      // the buffer on screen follows the choice at once; the other C++ buffers
      // follow it as they settle, since the setting is read on every question
      if (on) {
        this->tokens_timer->start();
      } else {
        this->clearSemanticTokens();
      }
    });
    // the grammar checker is a program of its own, and the entries which steer
    // it are none of `clangd`'s business; they are in this menu because this is
    // the mode which holds the setting to launch it by default, and they are
    // offered only when the program is there to be launched
    if (this->grammar->isAvailable()) {
      m->addSeparator();
      this->grammar->addToMenu(*m, QObject::tr("Grammar checking"));
    }
    m->addSeparator();
    auto *const sc = m->addAction(QObject::tr("Stop clangd"));
    sc->setToolTip(
        QObject::tr("End the session of clangd which is running; "
                    "the diagnostics it published go away with it"));
    QObject::connect(sc, &QAction::triggered, this,
                     [this] { stopClangd(this->editor); });
    return m;
  }  // end of getSpecificMenu

  bool CxxMajorMode::keyPressEvent(QKeyEvent *const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      this->runCompilation();
      return true;
    }
    if (this->client != nullptr) {
      if ((m == Qt::AltModifier) && (k == Qt::Key_Tab)) {
        this->completeWithClangd();
        return true;
      }
      if ((m == Qt::AltModifier) && (k == Qt::Key_Period)) {
        this->jumpToDefinition();
        return true;
      }
      // Alt-Comma mirrors Alt-Period: where the one goes, the other lists
      if ((m == Qt::AltModifier) && (k == Qt::Key_Comma)) {
        this->showReferences();
        return true;
      }
      // Alt-R renames the symbol under the cursor; plain Alt only, since the
      // rectangle mark of `TextEditBase` sits behind the Ctrl-x prefix chain
      // and Ctrl-R belongs to the search history
      if ((m == Qt::AltModifier) && (k == Qt::Key_R)) {
        this->renameSymbol();
        return true;
      }
      // '?' is a shifted key: the platform puts ShiftModifier alongside
      // AltModifier, as the Alt-< and Alt-> bindings of TextEditBase learned
      // the hard way
      if ((m & Qt::AltModifier) && !(m & Qt::ControlModifier) &&
          (k == Qt::Key_Question)) {
        this->showHover();
        return true;
      }
      // Alt-S asks the parameter list of the call the cursor stands in; plain
      // Alt, since the Ctrl-x chain writes the buffer with `s` and Ctrl-S is
      // the incremental search
      if ((m == Qt::AltModifier) && (k == Qt::Key_S)) {
        this->showSignatureHelp(true);
        return true;
      }
      // Alt-O opens the overview of the file: which symbol to go to is a
      // question of the whole buffer, not of the word under the cursor, so it
      // is the one of the clangd keys which asks nothing about a position;
      // plain Alt, since Ctrl-O is the history of the minibuffer and Alt-O
      // belongs to nobody
      if ((m == Qt::AltModifier) && (k == Qt::Key_O)) {
        this->showDocumentSymbols();
        return true;
      }
    }
    // the signature question is not asked here: the character typed reaches
    // the document only once this whole consult has returned false and the
    // underlying text edit has inserted it, so `watchSignatureHelp` hands the
    // question to a settling timer, by which time the call on screen is the
    // call the answer is about
    const auto handled = CMajorModeBase::keyPressEvent(e);
    if (!handled) {
      this->watchSignatureHelp(*e);
    }
    return handled;
  }  // end of keyPressEvent

  QCompleter *CxxMajorMode::getCompleter() {
    if ((this->lspCompleter != nullptr) && (this->lspCompletionActive)) {
      if (!this->lspCompleter->popup()->isVisible()) {
        // the popup was closed under our feet: by Escape, by a focus change,
        // by a click elsewhere. The next Tab must go back to being a Tab
        this->lspCompletionActive = false;
      } else {
        return this->lspCompleter;
      }
    }
    return CMajorModeBase::getCompleter();
  }  // end of getCompleter

  int CxxMajorMode::getMinimalCompletionLength() {
    return (this->lspCompletionActive) ? 1 : 3;
  }  // end of getMinimalCompletionLength

  void CxxMajorMode::completeCurrentWord(TextEditBase &t, const QString &w) {
    if ((this->lspCompleter == nullptr) || (!this->lspCompletionActive)) {
      MajorModeBase::completeCurrentWord(t, w);
      return;
    }
    // the whole word under the cursor goes, not only the missing tail: clangd
    // completes fuzzily, so an item may share nothing but a few letters with
    // what was typed; and no blank follows, as a completion often ends on `(`
    auto c = t.textCursor();
    c.clearSelection();
    if (!c.atBlockStart()) {
      c.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
    }
    c.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    c.insertText(w);
    this->lspCompletionActive = false;
    this->lspCompleter->popup()->hide();
    t.setTextCursor(c);
  }  // end of completeCurrentWord

  void CxxMajorMode::openOrRenameDocument() {
    if (this->client == nullptr) {
      return;
    }
    // the words tracked on the previous file, and what a server said of them,
    // are none of the renamed buffer's business: the tracking starts over
    this->resetNavigation();
    const auto f = this->textEdit.getCompleteFileName();
    if (f == this->documentFile) {
      if ((!f.isEmpty()) && (!this->client->isDocumentOpen(f))) {
        // the client is a fresh one since this mode last opened the file
        this->document = this->textEdit.document();
        this->client->openDocument(f, this->document);
        this->tokens_timer->start();
      }
      return;
    }
    if (!this->documentFile.isEmpty()) {
      this->client->closeDocument(this->documentFile,
                                  this->textEdit.document());
      this->documentFile = QString();
      this->applyDiagnostics();
    }
    if (f.isEmpty()) {
      return;
    }
    this->documentFile = f;
    this->document = this->textEdit.document();
    // the document is registered on the client, ready to be sent, whether a
    // server runs or not: opening a file is not asking for a session, which
    // the user launches from the "C++" menu or the `launch-clangd` command
    this->client->openDocument(f, this->document);
  }  // end of openOrRenameDocument

  void CxxMajorMode::updateDiagnostics(const QString &f) {
    if (f == this->documentFile) {
      // a republished diagnostic may differ from the one the cursor stands
      // on even where the cursor never moved: what was echoed is no longer
      // what the server says, so the memory of it goes
      this->echoed_diagnostic.clear();
      this->applyDiagnostics();
      // a file which has just been parsed is a file the server can now say
      // something about: this is the moment the question of what the
      // identifiers are is worth asking, whether or not the parse found errors
      this->tokens_timer->start();
    }
  }  // end of updateDiagnostics

  void CxxMajorMode::applyDiagnostics() {
    auto selections = QList<QTextEdit::ExtraSelection>();
    if ((this->client != nullptr) && (!this->documentFile.isEmpty())) {
      auto &doc = *this->textEdit.document();
      for (const auto &d : this->client->diagnosticsFor(this->documentFile)) {
        auto selection = QTextEdit::ExtraSelection();
        auto c = QTextCursor(&doc);
        c.setPosition(
            LspProtocol::positionOf(doc, d.startLine, d.startCharacter));
        // the range the server published is that of the text it last saw;
        // clamped to what the buffer holds now, it may have collapsed: an
        // empty underline says nothing, so a single character is marked
        auto end = LspProtocol::positionOf(doc, d.endLine, d.endCharacter);
        if (end <= c.position()) {
          c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
          if (c.selectionEnd() == c.selectionStart()) {
            c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
          }
        } else {
          c.setPosition(end, QTextCursor::KeepAnchor);
        }
        selection.cursor = c;
        // Qt's `QPlainTextEdit` painting path draws only the basic underline
        // styles — a wave or spell-check style is silently not inked there,
        // see `QTextDocumentLayout` — so severity lives in the color; and
        // `setFontUnderline` is not called, as it writes `SingleUnderline`
        // over the style property (and clears it when given false)
        selection.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        selection.format.setUnderlineColor(diagnosticColor(d.severity));
        selection.format.setFontUnderline(true);
        selections.append(selection);
      }
    }
    this->textEdit.setExtraSelections("clangd", selections);
  }  // end of applyDiagnostics

  void CxxMajorMode::refreshSemanticTokens() {
    if ((this->client == nullptr) ||
        (!LspCxxSupport::semanticTokensEnabled())) {
      // the setting is read on every question, so a person who turns it off
      // sees the text go back to what the highlighter draws on the next settle
      // of the buffer, without a new session or a restart
      this->clearSemanticTokens();
      return;
    }
    if ((!this->client->isReady()) || (!this->client->hasSemanticTokens()) ||
        (this->documentFile.isEmpty()) || (this->document == nullptr)) {
      // no session to ask, or a session which announced no legend to read an
      // answer by: the file keeps the faces the highlighter gave it
      return;
    }
    // the answer describes the text it was asked on, which is what the revision
    // says, and the sequence number says whether a later question has made this
    // answer history meanwhile
    const auto revision = this->document->revision();
    const auto sequence = ++(this->tokens_request);
    auto self = QPointer<CxxMajorMode>(this);
    this->client->requestSemanticTokens(
        this->documentFile,
        [self, sequence, revision](const QVector<LspSemanticToken> &tokens) {
          if (self.isNull()) {
            return;
          }
          self->applySemanticTokens(tokens, sequence, revision);
        });
  }  // end of refreshSemanticTokens

  void CxxMajorMode::applySemanticTokens(
      const QVector<LspSemanticToken> &tokens,
      const int sequence,
      const qint64 revision) {
    if ((this->document == nullptr) || (this->tokens_request != sequence) ||
        (this->document->revision() != revision) ||
        (!LspCxxSupport::semanticTokensEnabled())) {
      // the answer describes another text, or a later question has asked about
      // this one, or the colors are not wanted anymore: either way the buffer
      // is left alone, and the question which describes *it* is on its way
      return;
    }
    // the whole file at once: a file too big for the answer to be worth
    // drawing is a file whose colors stay as they are rather than half of
    // them being inked over
    if (tokens.size() > 40000) {
      return;
    }
    auto &doc = *this->textEdit.document();
    auto selections = QList<QTextEdit::ExtraSelection>();
    for (const auto &token : tokens) {
      auto format = QTextCharFormat();
      if (!semanticFaceOf(token, format)) {
        continue;
      }
      // a line the answer names which the document does not have is an answer
      // which describes another text: nothing is drawn for it, rather than
      // what `positionOf` would clamp to the last line
      if (!doc.findBlockByNumber(token.line).isValid()) {
        continue;
      }
      const auto start =
          LspProtocol::positionOf(doc, token.line, token.character);
      // both ends are clamped to the line they are asked on, so a token of a
      // text which has moved a little cannot paint past the end of its line
      const auto end = LspProtocol::positionOf(doc, token.line,
                                               token.character + token.length);
      if (end <= start) {
        continue;
      }
      auto selection = QTextEdit::ExtraSelection();
      auto c = QTextCursor(&doc);
      c.setPosition(start);
      c.setPosition(end, QTextCursor::KeepAnchor);
      selection.cursor = c;
      selection.format = format;
      selections.append(selection);
    }
    this->textEdit.setExtraSelections("clangd-tokens", selections);
  }  // end of applySemanticTokens

  void CxxMajorMode::clearSemanticTokens() {
    // ++, not a reset: a request which is on its way carries the number it was
    // issued with, and finds this one past it when its answer comes back
    ++(this->tokens_request);
    this->textEdit.setExtraSelections("clangd-tokens",
                                      QList<QTextEdit::ExtraSelection>());
  }  // end of clearSemanticTokens

  QString CxxMajorMode::diagnosticAt(const QTextDocument &doc,
                                     const int position) const {
    if ((this->client == nullptr) || (this->documentFile.isEmpty())) {
      return QString();
    }
    // the same conversion `applyDiagnostics` draws with: what is echoed has
    // to be what is underlined, down to the collapsed ranges, which are
    // drawn over a character and have to be pointed at over it too
    for (const auto &d : this->client->diagnosticsFor(this->documentFile)) {
      const auto start =
          LspProtocol::positionOf(doc, d.startLine, d.startCharacter);
      auto end = LspProtocol::positionOf(doc, d.endLine, d.endCharacter);
      if (end <= start) {
        end = start + 1;
      }
      if ((position >= start) && (position <= end)) {
        auto text = diagnosticLabel(d.severity);
        if (!d.source.isEmpty()) {
          text += QLatin1String(" [") + d.source + QLatin1Char(']');
        }
        return text + QLatin1String(": ") + d.message;
      }
    }
    return QString();
  }  // end of diagnosticAt

  void CxxMajorMode::echoDiagnosticAtCursor() {
    const auto text =
        (this->document != nullptr)
            ? this->diagnosticAt(*this->document,
                                 this->textEdit.textCursor().position())
            : QString();
    if (text == this->echoed_diagnostic) {
      // the same diagnostic as last the timer went off: an edit elsewhere on
      // the line, a mouse move, restarts it, and the message would come back
      // once per restart
      return;
    }
    this->echoed_diagnostic = text;
    if (!text.isEmpty()) {
      this->report(text);
    }
  }  // end of echoDiagnosticAtCursor

  void CxxMajorMode::resetNavigation() {
    this->cursor_word = TrackedWord();
    this->hovered_word = TrackedWord();
    this->cursor_tracking = false;
    this->hover_tracking = false;
    this->echoed_diagnostic.clear();
    // a signature answer of the session which is gone, or of the name the
    // buffer no longer has, describes a call nobody is typing anymore: the
    // sequence number voids the one which may still be on its way
    this->signature_timer->stop();
    ++(this->signature_request);
    // the colors the server gave name the identifiers of the text the buffer no
    // longer holds, or of a session which is gone; a request on its way is
    // voided with them
    this->tokens_timer->stop();
    this->clearSemanticTokens();
    this->applyTrackedCursor();
    this->applyTrackedHover();
  }  // end of resetNavigation

  bool CxxMajorMode::eventFilter(QObject *const o, QEvent *const e) {
    if ((this->client != nullptr) && (this->navigation_timer != nullptr)) {
      if (e->type() == QEvent::ToolTip) {
        // resting the mouse on an underlined range says what the cursor
        // standing on it would be told: the same message, as a tooltip.
        // `Qt 6.2` delivers the ToolTip event as a contentless `QShowEvent`
        // — the 6.4 `QToolTipEvent` is the future shape —, so the position
        // is the mouse's own, which has been resting where the event fires
        if (this->document != nullptr) {
          const auto global = QCursor::pos();
          const auto pos =
              this->textEdit
                  .cursorForPosition(
                      static_cast<QWidget *>(o)->mapFromGlobal(global))
                  .position();
          const auto text = this->diagnosticAt(*this->document, pos);
          if (!text.isEmpty()) {
            QToolTip::showText(global, text, &this->textEdit);
            return true;
          }
        }
      } else if (e->type() == QEvent::MouseMove) {
        this->hovered_point = static_cast<QMouseEvent *>(e)->pos();
        this->hover_tracking = true;
        this->navigation_timer->start();
      } else if (e->type() == QEvent::Leave) {
        // a word the mouse has left is none to give an icon to anymore, and
        // the answer about it may even be on its way: forgetting the word is
        // what makes that answer late, hence void
        if (this->hover_tracking || (!this->hovered_word.word.isEmpty())) {
          this->hover_tracking = false;
          this->hovered_word = TrackedWord();
          this->applyTrackedHover();
        }
      }
    }
    return CMajorModeBase::eventFilter(o, e);
  }  // end of eventFilter

  void CxxMajorMode::refreshTrackedWords() {
    const auto asking_cursor = this->cursor_tracking;
    const auto asking_hover = this->hover_tracking;
    this->cursor_tracking = false;
    this->hover_tracking = false;
    if ((!asking_cursor) && (!asking_hover)) {
      return;
    }
    // the diagnostics the server already sent are the server's own answer:
    // what stands under the cursor can be echoed whether a session is up to
    // answer questions about words or not
    if (asking_cursor) {
      this->echoDiagnosticAtCursor();
    }
    if ((this->client == nullptr) || (!this->client->isReady()) ||
        (this->document == nullptr) || (this->documentFile.isEmpty())) {
      // no session to ask, and no answer worth carrying around either; the
      // keys of the editor stay empty underlines
      if ((!this->cursor_word.word.isEmpty()) ||
          (!this->hovered_word.word.isEmpty())) {
        this->resetNavigation();
      }
      return;
    }
    auto &doc = *this->document;
    const auto revision = doc.revision();
    if (asking_cursor) {
      const auto range =
          identifier_range(doc, this->textEdit.textCursor().position());
      if ((range.x() != this->cursor_word.start) ||
          (revision != this->cursor_word.revision)) {
        this->cursor_word = TrackedWord();
        if (range.x() >= 0) {
          const auto block = doc.findBlock(range.x());
          this->cursor_word.word = block.text().mid(
              range.x() - block.position(), range.y() - range.x());
          this->cursor_word.start = range.x();
          this->cursor_word.revision = revision;
          this->askTrackedWord(true);
        }
        // the underline of the previous word is none for this one: what the
        // answer says of the word itself draws the next one
        this->applyTrackedCursor();
      } else if (this->cursor_word.capabilities >= 0) {
        this->applyTrackedCursor();
      }
    }
    if (asking_hover) {
      const auto range = identifier_range(
          doc,
          this->textEdit.cursorForPosition(this->hovered_point).position());
      if ((range.x() != this->hovered_word.start) ||
          (revision != this->hovered_word.revision)) {
        this->hovered_word = TrackedWord();
        if (range.x() >= 0) {
          const auto block = doc.findBlock(range.x());
          this->hovered_word.word = block.text().mid(
              range.x() - block.position(), range.y() - range.x());
          this->hovered_word.start = range.x();
          this->hovered_word.revision = revision;
          this->askTrackedWord(false);
        }
        this->applyTrackedHover();
      } else if (this->hovered_word.capabilities >= 0) {
        this->applyTrackedHover();
      }
    }
  }  // end of refreshTrackedWords

  void CxxMajorMode::askTrackedWord(const bool track_cursor) {
    auto &word = track_cursor ? this->cursor_word : this->hovered_word;
    // the word the mouse rests on is very often the one the cursor has been
    // sitting on: what the cursor's question learned is the mouse's answer
    auto &other = track_cursor ? this->hovered_word : this->cursor_word;
    if ((other.start == word.start) && (other.revision == word.revision) &&
        (other.capabilities >= 0)) {
      word.capabilities = other.capabilities;
      if (track_cursor) {
        this->applyTrackedCursor();
      } else {
        this->applyTrackedHover();
      }
      return;
    }
    // the two questions of one word are one answer: it is defined or
    // declared, or it is neither, and that is known once both are back
    auto answer = std::make_shared<QPair<int, int>>();
    answer->first = 0;
    answer->second = 2;
    auto self = QPointer<CxxMajorMode>(this);
    const auto start = word.start;
    const auto revision = word.revision;
    auto note = [self, answer, track_cursor, start, revision](
                    const int capability,
                    const QVector<LspLocation> &locations) {
      if (!locations.isEmpty()) {
        answer->first |= capability;
      }
      if ((--answer->second) != 0) {
        return;
      }
      if (self == nullptr) {
        return;
      }
      // by the time both answers are in, either position may have moved on:
      // an answer about a word nobody looks at anymore is a late one, and a
      // void one
      auto &w = track_cursor ? self->cursor_word : self->hovered_word;
      if ((w.start != start) || (w.revision != revision)) {
        return;
      }
      w.capabilities = answer->first;
      if (track_cursor) {
        self->applyTrackedCursor();
      } else {
        self->applyTrackedHover();
      }
    };
    // any position within the identifier names it to clangd, its start will
    // do; a client which is not answering calls the reply back at once, empty,
    // which settles the answer as much as a server would
    this->client->requestDefinition(
        this->documentFile, *this->document, start,
        [note](const QVector<LspLocation> &locations) {
          note(HAS_DEFINITION, locations);
        });
    this->client->requestDeclaration(
        this->documentFile, *this->document, start,
        [note](const QVector<LspLocation> &locations) {
          note(HAS_DECLARATION, locations);
        });
  }  // end of askTrackedWord

  void CxxMajorMode::applyTrackedCursor() {
    auto selections = QList<QTextEdit::ExtraSelection>();
    if ((this->cursor_word.capabilities > 0) &&
        (this->cursor_word.start >= 0) && (this->document != nullptr)) {
      auto selection = QTextEdit::ExtraSelection();
      auto c = QTextCursor(this->document.data());
      c.setPosition(this->cursor_word.start);
      c.setPosition(this->cursor_word.start + this->cursor_word.word.size(),
                    QTextCursor::KeepAnchor);
      selection.cursor = c;
      // the same reasoning as the diagnostics': Qt's plain text painting path
      // draws the basic underline styles only and `setFontUnderline` writes
      // over the style property, see `applyDiagnostics`
      selection.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      // a jumpable word is neither an error nor a warning, and what it offers
      // is a jump: the color of the links is the one which says as much
      selection.format.setUnderlineColor(
          this->textEdit.palette().color(QPalette::Link));
      selection.format.setFontUnderline(true);
      selections.append(selection);
    }
    this->textEdit.setExtraSelections("clangd-navigation", selections);
  }  // end of applyTrackedCursor

  void CxxMajorMode::applyTrackedHover() {
    auto *const area = this->textEdit.widget();
    if (area == nullptr) {
      return;
    }
    // -1 says the question is not answered yet, and -1 has every bit set:
    // only a resolved capability word may set a cursor, everything else
    // hands the editor's own cursor back
    const auto capabilities = this->hovered_word.capabilities;
    if (capabilities > 0) {
      if ((capabilities & HAS_DEFINITION) != 0) {
        area->viewport()->setCursor(jump_cursor("go-jump-definition"));
      } else {
        area->viewport()->setCursor(jump_cursor("go-jump-declaration"));
      }
    } else {
      // Qt gives the viewport of a text edit its beam cursor explicitly, so
      // the same has to be given back explicitly: `unsetCursor` would let an
      // arrow show over the text instead
      area->viewport()->setCursor(Qt::IBeamCursor);
    }
  }  // end of applyTrackedHover

  void CxxMajorMode::completeWithClangd() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->editor)) {
      this->report("clangd is not answering");
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->openOrRenameDocument();
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    if (!this->client->isDocumentOpen(this->documentFile)) {
      this->client->openDocument(this->documentFile, this->textEdit.document());
    }
    struct Answer {
      QStringList labels;
    };
    auto answer = std::make_shared<Answer>();
    // the answer is carried by shared pointers, none of which names this
    // mode: the request may be answered after a timeout, when the buffer it
    // was asked for is long gone
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->client->requestCompletion(
              this->documentFile, *this->textEdit.document(),
              this->textEdit.textCursor().position(),
              [answer, stop](const QStringList &labels, const QStringList &) {
                answer->labels = labels;
                stop();
              });
        },
        3000);
    if ((!in_time) || (answer->labels.isEmpty())) {
      this->report("no completion for this position");
      return;
    }
    if (this->lspCompleter == nullptr) {
      this->lspCompleter = new QCompleter(&this->textEdit);
      this->lspCompleter->setFilterMode(Qt::MatchContains);
      this->lspCompleter->setCaseSensitivity(Qt::CaseInsensitive);
      this->lspCompleter->setCompletionMode(QCompleter::PopupCompletion);
      // the popup of the base mechanism: TextEditBase forwards Tab and
      // Backtab to it while it is up, and typing narrows it through
      // getCompleter/getCompletionPrefix like any other completer
      QObject::connect(this->lspCompleter,
                       qOverload<const QString &>(&QCompleter::activated), this,
                       [this](const QString &w) {
                         this->completeCurrentWord(this->textEdit, w);
                       });
    }
    auto *const previous = this->lspCompleter->model();
    auto *const model =
        new QStringListModel(answer->labels, this->lspCompleter);
    this->lspCompleter->setModel(model);
    if (previous != nullptr) {
      previous->deleteLater();
    }
    this->lspCompletionActive = true;
    this->lspCompleter->setCompletionPrefix(this->textEdit.getCurrentWord());
    auto *const popup = this->lspCompleter->popup();
    popup->setCurrentIndex(this->lspCompleter->completionModel()->index(0, 0));
    auto cr = this->textEdit.cursorRect();
    cr.setWidth(popup->sizeHintForColumn(0) +
                popup->verticalScrollBar()->sizeHint().width());
    this->lspCompleter->complete(cr);
  }  // end of completeWithClangd

  void CxxMajorMode::jumpToDefinition() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->editor)) {
      this->report("clangd is not answering");
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    auto answer = std::make_shared<QVector<LspLocation>>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->client->requestDefinition(
              this->documentFile, *this->textEdit.document(),
              this->textEdit.textCursor().position(),
              [answer, stop](const QVector<LspLocation> &locations) {
                *answer = locations;
                stop();
              });
        },
        3000);
    if ((!in_time) || (answer->isEmpty())) {
      this->report("no definition found");
      return;
    }
    const auto &location = answer->at(0);
    this->jumpTo(location);
  }  // end of jumpToDefinition

  void CxxMajorMode::showReferences() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->editor)) {
      this->report("clangd is not answering");
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    auto answer = std::make_shared<QVector<LspLocation>>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->client->requestReferences(
              this->documentFile, *this->textEdit.document(),
              this->textEdit.textCursor().position(),
              [answer, stop](const QVector<LspLocation> &locations) {
                *answer = locations;
                stop();
              });
        },
        3000);
    // a server which keeps the question is reported exactly as one which
    // answers that there is nowhere to look: waiting longer is a worse answer
    if ((!in_time) || (answer->isEmpty())) {
      this->report("no references found");
      return;
    }
    // a menu popped at the cursor, not run: a nested loop here would sit on
    // top of the `boundedWait` which has already sat on the key press, and a
    // list to choose from asks nothing of the rest of the editor meanwhile.
    // The name is what tells it apart from the menus the text edit and the
    // mode build elsewhere, in tests as in completion code
    auto *const menu = new QMenu(&this->textEdit);
    menu->setObjectName(QStringLiteral("clangd-references"));
    // the file names say what the entries are only once the line is read:
    // thirty of them is a menu a thumb can reach, a hundred is not a menu
    constexpr auto shown = 30;
    const auto total = answer->size();
    const auto entries = qMin(total, shown);
    auto *const doc = this->textEdit.document();
    for (auto i = 0; i != entries; ++i) {
      const auto location = answer->at(i);
      auto text = QFileInfo(location.file).fileName() + QLatin1Char(':') +
                  QString::number(location.line + 1);
      if ((this->document != nullptr) &&
          (location.file == this->documentFile)) {
        // this buffer's own line, read where the jump would land, is what
        // tells two uses of one file apart without a trip to it
        const auto block = doc->findBlock(
            LspProtocol::positionOf(*doc, location.line, location.character));
        const auto line = block.text().trimmed();
        if (!line.isEmpty()) {
          text += QLatin1String(": ") +
                  (line.size() > 60 ? line.left(59) + QChar(0x2026) : line);
        }
      }
      auto *const a = menu->addAction(text);
      QObject::connect(a, &QAction::triggered, this,
                       [this, location] { this->jumpTo(location); });
    }
    if (total > entries) {
      menu->addAction(QObject::tr("(%1 of %2 references shown)")
                          .arg(entries)
                          .arg(total))
          ->setEnabled(false);
    }
    // the menu is on its own once popped: hidden by a click elsewhere or by
    // Escape, it has no reason to outlive its hiding
    QObject::connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(
        this->textEdit.mapToGlobal(this->textEdit.cursorRect().bottomLeft()));
  }  // end of showReferences

  void CxxMajorMode::showDocumentSymbols() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->editor)) {
      this->report("clangd is not answering");
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    // the overview is asked for before the question is even put: the list the
    // user reads is the list the answer names a place in, and a question which
    // turns out to hold nothing is worse than not having asked
    auto answer = std::make_shared<QVector<LspDocumentSymbol>>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->client->requestDocumentSymbols(
              this->documentFile,
              [answer, stop](const QVector<LspDocumentSymbol> &symbols) {
                *answer = symbols;
                stop();
              });
        },
        3000);
    if (answer->isEmpty()) {
      // a file clangd does not parse holds no symbol to go to: the answer is
      // empty whether clangd read the file or never got round to it
      this->report((in_time)
                       ? QStringLiteral("clangd found no symbol in this file")
                       : QStringLiteral("clangd is not answering"));
      return;
    }
    this->editor.setUserInput(new DocumentSymbolInput(
        *this, this->editor, this->documentFile, *answer));
  }  // end of showDocumentSymbols

  void CxxMajorMode::renameSymbol() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->editor)) {
      this->report("clangd is not answering");
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    // a rename wants a name, and a name is worth asking only over a symbol:
    // the same identifier range the underline tracks
    const auto range = identifier_range(*this->document,
                                        this->textEdit.textCursor().position());
    if (range.x() < 0) {
      this->report("no symbol under the cursor");
      return;
    }
    const auto block = this->document->findBlock(range.x());
    const auto word =
        block.text().mid(range.x() - block.position(), range.y() - range.x());
    this->editor.setUserInput(
        new RenameSymbolInput(*this, this->editor, word, range.x()));
  }  // end of renameSymbol

  void CxxMajorMode::renameToAt(const QString &name, const int position) {
    if (this->client == nullptr) {
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    struct Answer {
      QMap<QString, QVector<LspTextEdit>> edits;
      QString error;
    };
    auto answer = std::make_shared<Answer>();
    const auto in_time = LspProtocol::boundedWait(
        [this, name, position, answer](const std::function<void()> &stop) {
          this->client->requestRename(
              this->documentFile, *this->textEdit.document(), position, name,
              [answer, stop](const QMap<QString, QVector<LspTextEdit>> &edits,
                             const QString &error) {
                answer->edits = edits;
                answer->error = error;
                stop();
              });
        },
        3000);
    if (!in_time) {
      this->report("clangd did not answer the rename");
      return;
    }
    if (answer->edits.isEmpty()) {
      // clangd refuses a name which is not a valid identifier, points at no
      // symbol, or names the symbol it already has; its own reason is the
      // message worth showing, and an empty edit list with no reason says the
      // symbol is already named what was asked
      this->report(answer->error.isEmpty()
                       ? QStringLiteral("nothing to rename")
                       : QStringLiteral("clangd refused the rename: %1")
                             .arg(answer->error));
      return;
    }
    const auto count = this->applyWorkspaceEdit(answer->edits);
    this->report(QStringLiteral("%1 occurrences renamed in %2 file(s)")
                     .arg(count)
                     .arg(answer->edits.size()));
  }  // end of renameToAt

  int CxxMajorMode::applyWorkspaceEdit(
      const QMap<QString, QVector<LspTextEdit>> &edits) {
    // the buffer of this mode is remembered by id: opening a file makes its
    // buffer current, and the user shall not find themselves somewhere else
    // once the rename is over
    const auto origin = this->buffer.getId();
    auto count = 0;
    for (auto it = edits.begin(); it != edits.end(); ++it) {
      auto *const visited = this->editor.getBufferVisitingFile(it.key());
      if (visited != nullptr) {
        // the file is visited somewhere: `changeBuffer` by id brings its
        // buffer forward, wherever it lives among the buffers
        this->editor.changeBuffer(visited->getId());
      } else {
        // the file is not visited: `openFile` creates the buffer and reads
        // the file, which is what tells the client of the document too,
        // through the mode the new buffer installs
        this->editor.openFile(it.key());
      }
      auto &frame = this->editor.getCurrentBuffer().getMainFrame();
      if (QFileInfo(frame.getCompleteFileName()) != QFileInfo(it.key())) {
        // whatever the two above did, the buffer which is current now has to
        // visit the edited file for its edits to land; a buffer renamed away
        // from the file — which `getBufferVisitingFile` would not have found
        // under this name — is not it, and editing whatever came up is worse
        // than skipping the file
        continue;
      }
      auto *const doc = frame.document();
      if (doc == nullptr) {
        continue;
      }
      // the positions of the protocol are those of the text as the server
      // knows it, which is what the buffer holds: the edits are converted
      // once, and applied from the end of the document to its start so that
      // no edit moves the place the next one is to cut
      auto ordered = it.value();
      std::sort(
          ordered.begin(), ordered.end(),
          [doc](const LspTextEdit &a, const LspTextEdit &b) {
            const auto pa =
                LspProtocol::positionOf(*doc, a.startLine, a.startCharacter);
            const auto pb =
                LspProtocol::positionOf(*doc, b.startLine, b.startCharacter);
            if (pa != pb) {
              return pa > pb;
            }
            return LspProtocol::positionOf(*doc, a.endLine, a.endCharacter) >
                   LspProtocol::positionOf(*doc, b.endLine, b.endCharacter);
          });
      auto cursor = QTextCursor(doc);
      for (const auto &edit : std::as_const(ordered)) {
        cursor.setPosition(
            LspProtocol::positionOf(*doc, edit.startLine, edit.startCharacter));
        cursor.setPosition(
            LspProtocol::positionOf(*doc, edit.endLine, edit.endCharacter),
            QTextCursor::KeepAnchor);
        cursor.insertText(edit.text);
        ++count;
      }
    }
    this->editor.changeBuffer(origin);
    return count;
  }  // end of applyRename

  void CxxMajorMode::goToDocumentSymbol(const LspDocumentSymbol &symbol) {
    // the overview names a place of this very file, so no buffer is opened to
    // reach it: the place is handed to the code which jumps to a definition
    // because that code is the one which knows how to put a cursor on a line
    // of the current buffer and of any other
    auto location = LspLocation();
    location.file = this->documentFile;
    location.line = symbol.line;
    location.character = symbol.character;
    this->jumpTo(location);
    // where the cursor went is said in the minibuffer: the view may not show
    // the line it landed on, and the name is what tells the jump from the one
    // Alt-. just did to somewhere else
    this->report(symbol.name + QStringLiteral(" (line ") +
                 QString::number(symbol.line + 1) + QStringLiteral(")"));
  }  // end of goToDocumentSymbol

  void CxxMajorMode::jumpTo(const LspLocation &location) {
    if (QFileInfo(location.file) == QFileInfo(this->documentFile)) {
      auto c = QTextCursor(this->textEdit.document());
      c.setPosition(LspProtocol::positionOf(*this->textEdit.document(),
                                            location.line, location.character));
      this->textEdit.setTextCursor(c);
      this->textEdit.setFocus();
      return;
    }
    // openFile reuses the buffer of a file which is visited already, so the
    // definition of a symbol in a header one has open does not fork a second
    // buffer on it
    this->editor.openFile(location.file);
    auto &frame = this->editor.getCurrentBuffer().getMainFrame();
    if (frame.document() == nullptr) {
      return;
    }
    auto c = QTextCursor(frame.document());
    c.setPosition(LspProtocol::positionOf(*frame.document(), location.line,
                                          location.character));
    frame.setTextCursor(c);
    this->report(QStringLiteral("%1").arg(QFileInfo(location.file).fileName()));
  }  // end of jumpTo

  void CxxMajorMode::showHover() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->editor)) {
      this->report("clangd is not answering");
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    struct Answer {
      QString text;
      bool markdown = false;
    };
    auto answer = std::make_shared<Answer>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->client->requestHover(
              this->documentFile, *this->textEdit.document(),
              this->textEdit.textCursor().position(),
              [answer, stop](const QString &text, const bool markdown) {
                answer->text = text;
                answer->markdown = markdown;
                stop();
              });
        },
        3000);
    if ((!in_time) || (answer->text.trimmed().isEmpty())) {
      this->report("no information");
      return;
    }
    auto doc = QTextDocument();
    if (answer->markdown) {
      doc.setMarkdown(answer->text);
    } else {
      doc.setPlainText(answer->text);
    }
    QToolTip::showText(
        this->textEdit.mapToGlobal(this->textEdit.cursorRect().bottomLeft()),
        doc.toHtml(), &this->textEdit);
  }  // end of showHover

  void CxxMajorMode::showSignatureHelp(const bool asked_by_user) {
    if (this->client == nullptr) {
      return;
    }
    if (asked_by_user) {
      if (!serverIsReachable(*this->client, this->editor)) {
        this->report("clangd is not answering");
        return;
      }
      if (this->documentFile.isEmpty()) {
        this->report("this buffer visits no file yet");
        return;
      }
    } else if ((!this->client->isReady()) || (this->documentFile.isEmpty()) ||
               (this->document == nullptr)) {
      // typing `(` with no session to ask is not a question; the empty
      // document is the state of the buffer which has gained a name but no
      // text yet
      return;
    }
    if (this->document == nullptr) {
      if (asked_by_user) {
        this->report("this buffer holds no document yet");
      }
      return;
    }
    // the answer belongs to the text and the place it was asked on: a call
    // being typed moves both, and typing is exactly what started this
    const auto position = this->textEdit.textCursor().position();
    const auto revision = this->document->revision();
    const auto sequence = ++(this->signature_request);
    auto self = QPointer<CxxMajorMode>(this);
    this->client->requestSignatureHelp(
        this->documentFile, *this->document, position,
        [self, sequence, position, revision, asked_by_user](
            const QVector<LspSignature> &signatures, const int active_signature,
            const int active_parameter) {
          auto *const mode = self.data();
          if (mode == nullptr) {
            return;
          }
          if ((sequence != mode->signature_request) ||
              (mode->document == nullptr) ||
              (mode->document->revision() != revision) ||
              (mode->textEdit.textCursor().position() != position)) {
            return;
          }
          if (signatures.isEmpty()) {
            // a brace of a body, a template argument, a parenthesised
            // expression: nothing to say about a call which is not one, and a
            // message per keystroke would make the feature unreadable
            if (asked_by_user) {
              mode->report("no call around the cursor");
            }
            return;
          }
          // the server has ranked the candidates and named the one the
          // argument being typed fits; the rest are the overloads it did not
          // rule out, and worth counting, not reading
          const auto best = qBound(0, active_signature,
                                   static_cast<int>(signatures.size()) - 1);
          auto text = signature_text(signatures.at(best), active_parameter);
          if (signatures.size() > 1) {
            text += QStringLiteral(" (%1 candidates)").arg(signatures.size());
          }
          mode->report(text);
        });
  }  // end of showSignatureHelp

  void CxxMajorMode::watchSignatureHelp(const QKeyEvent &e) {
    if ((this->client == nullptr) || (this->signature_timer == nullptr) ||
        (!this->client->isReady()) || (this->documentFile.isEmpty())) {
      return;
    }
    const auto text = e.text();
    // the character the key puts in the document is what the server listed as
    // a trigger, not the key itself: `<` and `(` carry modifiers on this
    // keyboard and none on that one
    if (text.isEmpty() ||
        (!this->client->signatureHelpTriggers().contains(text.at(0)))) {
      return;
    }
    this->signature_timer->start();
  }  // end of watchSignatureHelp

  void CxxMajorMode::completeContextMenu(QMenu *const m,
                                         const QTextCursor &tc) {
    MajorModeBase::completeContextMenu(m, tc);
    if ((this->client == nullptr) || (this->documentFile.isEmpty()) ||
        (this->document == nullptr) || (tc.document() != this->document)) {
      return;
    }
    // a right click neither launches a server nor waits for a handshake: the
    // menu would show up late, and no question has been asked of the server
    // yet for that to be paid for
    if ((!this->client->isReady())) {
      return;
    }
    // asking is the only way to know whether there is something to offer, and
    // the answer is waited for a moment only: clangd may be building the
    // preamble of the file, and a plain menu is better than a delayed one. The
    // answer is kept, so triggering the entry jumps without asking again. A
    // symbol without a definition, a function without a body for instance, is
    // not a special case: `clangd` answers the definition question with the
    // declaration of such a symbol, so the entry is offered and jumps to the
    // prototype, which is what the hover icon of the mode promised too
    //
    // The two questions this menu asks — where the symbol is defined, and what
    // can be fixed here — are asked at once and waited for together: they are
    // independent, and the menu would otherwise pay the round trip of each in
    // series. Whichever is back when the moment expires is used; a menu which
    // shows the fixes it has while the definition is still on its way is a
    // better menu than an empty one.
    auto locations = std::make_shared<QVector<LspLocation>>();
    auto actions = std::make_shared<QVector<LspCodeAction>>();
    auto pending = std::make_shared<int>(2);
    auto waiter = std::make_shared<std::function<void()>>();
    LspProtocol::boundedWait(
        [this, &tc, locations, actions, pending,
         waiter](const std::function<void()> &stop) {
          // one countdown shared by the two answers: whoever is first does not
          // stop the wait for the other, and the last one to come back ends it
          *waiter = [pending, stop] {
            if ((--(*pending)) == 0) {
              stop();
            }
          };
          this->client->requestDefinition(
              this->documentFile, *this->document, tc.position(),
              [locations, waiter](const QVector<LspLocation> &l) {
                *locations = l;
                (*waiter)();
              });
          this->client->requestCodeActions(
              this->documentFile, *this->document, tc.position(), tc.position(),
              [actions, waiter](const QVector<LspCodeAction> &a) {
                *actions = a;
                (*waiter)();
              });
        },
        1000);
    if (locations->isEmpty() && actions->isEmpty()) {
      return;
    }
    // a symbol defined in several places, a virtual override for instance,
    // gets one entry each: the file and the line, in the numbering a buffer
    // shows, are what tells them apart. Five is where a menu which goes on
    // listing them stops being a menu
    const auto definitions = qMin(locations->size(), 5);
    // everything the mode adds goes above the entries of the text edit — undo,
    // cut, copy, paste —, since those are the entries the right click came for;
    // `nullptr` is the insertion point of a menu which has none of its own, and
    // one point for both groups is what keeps them in the order written here
    const auto old = m->actions();
    auto *const before = old.isEmpty() ? nullptr : *(old.begin());
    // the fixes the server offers for the diagnostics of the line the menu was
    // opened on. An action is offered only when it carries the text it writes:
    // the AST tweaks of `clangd` — extract a variable, add an override — come
    // as a command the server would have to be asked a second question to
    // apply, and an entry which does nothing, or which lets the server write
    // over the head of the editor, is worse than no entry at all. Six is where
    // the fixes of a file full of errors stop being a menu
    auto fixes = QVector<const LspCodeAction *>();
    for (const auto &action : std::as_const(*actions)) {
      if (!action.edits.isEmpty()) {
        fixes.append(&action);
        if (fixes.size() == 6) {
          break;
        }
      }
    }
    // the entries of a group are added in reverse, each right above the
    // entries of the text edit, which leaves the group in the order it is
    // written here once the menu is drawn
    for (auto i = definitions - 1; i >= 0; --i) {
      const auto location = locations->at(i);
      auto text = QObject::tr("Go to definition");
      if (definitions > 1) {
        text = QObject::tr("Go to definition (%1:%2)")
                   .arg(QFileInfo(location.file).fileName())
                   .arg(location.line + 1);
      }
      auto *const a = new QAction(text, m);
      a->setIcon(QIcon::fromTheme("go-jump"));
      a->setIconVisibleInMenu(true);
      a->setStatusTip(QObject::tr("Jump to the definition of the symbol"));
      QObject::connect(a, &QAction::triggered, this,
                       [this, location] { this->jumpTo(location); });
      m->insertAction(before, a);
    }
    // the topmost entry of the group of the fixes, which is the one its last
    // insertion creates: the separator of the two groups goes right above it
    QAction *first_fix = nullptr;
    for (auto i = fixes.size() - 1; i >= 0; --i) {
      const auto &action = *fixes.at(i);
      const auto quick_fix = action.kind.startsWith(QLatin1String("quickfix"));
      auto text =
          quick_fix ? QObject::tr("Fix: %1").arg(action.title) : action.title;
      if (action.is_preferred) {
        text += QStringLiteral(" (*)");
      }
      auto *const a = new QAction(text, m);
      a->setIcon(QIcon::fromTheme(quick_fix ? "dialog-warning"
                                            : "accessories-text-editor"));
      a->setIconVisibleInMenu(true);
      // the message of the diagnostic the fix answers to is the whole reason
      // the entry is here, so it is what the mode line reads while the pointer
      // rests on it
      a->setStatusTip(
          action.diagnostic.isEmpty()
              ? QObject::tr("Apply the change %1 suggests").arg(action.title)
              : action.diagnostic);
      const auto edits = action.edits;
      const auto title = action.title;
      QObject::connect(a, &QAction::triggered, this, [this, edits, title] {
        const auto count = this->applyWorkspaceEdit(edits);
        this->report(count > 0
                         ? QObject::tr("%1: %2 edit(s) in %3 file(s)")
                               .arg(title)
                               .arg(count)
                               .arg(edits.size())
                         : QObject::tr("%1: nothing to apply").arg(title));
      });
      m->insertAction(before, a);
      first_fix = a;
    }
    // the separators say where the groups of the mode end, and Qt leaves a
    // separator out of the drawing where it would be the first or the last
    // entry, so a leading one costs nothing here and hides a special case
    if ((first_fix != nullptr) && (definitions > 0)) {
      m->insertSeparator(first_fix);
    }
    if (((definitions > 0) || (first_fix != nullptr)) && (before != nullptr)) {
      m->insertSeparator(before);
    }
  }  // end of completeContextMenu

  void CxxMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    this->setHighlighter(new CxxSyntaxHighlighter(d));
  }  // end of setSyntaxHighlighter

  CxxMajorMode::~CxxMajorMode() {
    // `textEdit.document()` must not be called from here: the mode is
    // destroyed as a child of the text edit widget, whose own Qt widget is
    // gone by then, and the virtual call would read it back through a
    // dangling pointer, the lifetime trap of issue #20 in a new dress. The
    // `QPointer` says what is alive; a dead document has released its
    // reference on the client by itself, see `LspClient::openDocument`
    if ((this->client != nullptr) && (!this->documentFile.isEmpty()) &&
        (this->document != nullptr)) {
      this->client->closeDocument(this->documentFile, this->document.data());
    }
  }  // end of ~CxxMajorMode

  QVector<SettingsGroup> CxxMajorMode::declareSettings() {
    // the four addresses `LspCxxSupport` reads are gathered in a theme of their
    // own, named after the program they are about: they are what a person looks
    // for when the C/C++ support does not do what they expect, and a theme
    // named `clangd` says it without spelling the mode out. An entry which
    // names no theme belongs to the theme it is declared in, so none of them
    // has to say it twice
    auto clangd = SettingsGroup{};
    clangd.name = QStringLiteral("clangd");
    clangd.description = QObject::tr(
        "What the C++ mode asks the language server, and what it "
        "asks it with. A session reads the program, its arguments "
        "and the compilation database when it is launched: what "
        "changes here reaches the next `Launch clangd`");
    clangd.keys =
        QVector<SettingsKey>()
        << SettingsKey::boolean(
               "clangd/enable", QObject::tr("Ask clangd anything"),
               QObject::tr("When this is off, the C++ mode highlights and "
                           "indents its files and asks nothing to anyone: "
                           "no diagnostics, no completion, no jump to a "
                           "definition. The modes which are already "
                           "attached to a buffer were built with this "
                           "answer, and `Launch clangd` refuses a session "
                           "while it is off"),
               true)
        << SettingsKey::text(
               "clangd/executable",
               QObject::tr("Program of the language server"),
               QObject::tr("The `clangd` to start. Left empty, the editor "
                           "looks for `clangd` in the directories of the "
                           "session, then for the versioned names "
                           "`clangd-<version>` in `/usr/bin`, "
                           "`/usr/local/bin` and `/bin`, then for the "
                           "newest `/usr/lib/llvm-<version>/bin/clangd`"))
        << SettingsKey::stringList(
               "clangd/arguments", QObject::tr("Arguments passed to clangd"),
               QObject::tr("What is appended to the arguments the client "
                           "needs, such as `--background-index=false`. In "
                           "the configuration file the entries of the "
                           "list are separated by a comma"))
        << SettingsKey::boolean(
               "clangd/semantic-tokens",
               QObject::tr("Color the identifiers as clangd "
                           "says they are"),
               QObject::tr("Asks the server what the names of a buffer "
                           "are and inks a function, a macro, a type and "
                           "a deprecated name with the face of what they "
                           "are. The `Color identifiers` entry of the "
                           "`C/C++` menu writes this same address, and "
                           "the buffer on screen follows it at once"),
               true);
    // the grammar checker is a program of its own, and the C++ mode is the one
    // which holds the setting to launch it by default; the theme is named after
    // the program, as the theme above is, and the `LaTeX` and `Markdown` modes
    // offer the same two choices under their own menus, since a checker is one
    // decision for the whole editor and not one per mode
    auto harper = SettingsGroup{};
    harper.name = QStringLiteral("harper");
    harper.description = QObject::tr(
        "The English grammar checker the buffers of the editor can be read "
        "with: what it reads is underlined, and the entry of a menu which "
        "names it turns it on and off. The checker knows no word of French, "
        "and it reads a C++ file as prose, which is to say its comments");
    harper.keys =
        QVector<SettingsKey>()
        << SettingsKey::boolean(
               "harper/launch-by-default",
               QObject::tr("Launch the grammar checker by default"),
               QObject::tr("When this is on, a buffer takes a grammar session "
                           "with it as it is opened, and underlines what the "
                           "checker finds: a wrong agreement, an article which "
                           "does not fit the word it introduces, a spelling of "
                           "another variety of English. The buffers already "
                           "open keep what they were built with, and the "
                           "`Grammar checking` entry of a menu writes this "
                           "same address. While a buffer is checked, the spell "
                           "checker of hunspell is taken off it: the two say "
                           "what is wrong with a word in the same place"),
               false)
        << SettingsKey::text(
               "harper/executable",
               QObject::tr("Program of the grammar checker"),
               QObject::tr("The `harper-ls` to start. Left empty, the editor "
                           "looks for `harper-ls` in the directories of the "
                           "session; when there is none, no menu offers a "
                           "grammar session at all"))
        << SettingsKey::choice(
               "harper/dialect", QObject::tr("Variety of English to read as"),
               QObject::tr("Which English the text is read as: which spellings "
                           "are held to be their own — `colour` and `color` do "
                           "not both fit one variety — and which inflections "
                           "are accepted. The `English variant` submenu of a "
                           "menu writes this same address, and a session which "
                           "runs reads its buffers again under the new "
                           "answer"),
               dialectChoices(), QStringLiteral("American"));
    // nothing else: what the mode remembers of the commands it was asked to run
    // is a history, and a history is not an entry of the configuration, see
    // `CoreSettings.cxx`
    return QVector<SettingsGroup>() << clangd << harper;
  }  // end of CxxMajorMode::declareSettings

  static StandardFunctionCommandProxy<launchClangd> launchClangdProxy(
      "launch-clangd");

  static StandardFunctionCommandProxy<launchClangdWithCompileCommands>
      launchClangdWithCompileCommandsProxy(
          "launch-clangd-with-compile-commands");

  static StandardFunctionCommandProxy<stopClangd> stopClangdProxy(
      "stop-clangd");

  static StandardMajorModeProxy<CxxMajorMode> proxy(
      "C++",
      QVector<QRegularExpression>()
          << suffixRegExp("cxx") << suffixRegExp("hxx") << suffixRegExp("ixx")
          << suffixRegExp("cpp") << suffixRegExp("hpp") << suffixRegExp("ipp")
          << suffixRegExp("CC") << suffixRegExp("hh") << suffixRegExp("c")
          << suffixRegExp("h") << suffixRegExp("cu"),
      ":/tfel/editor/languages/cxx.png");

}  // end of namespace tfel::gui
