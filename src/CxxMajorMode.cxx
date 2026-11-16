/*!
 * \file  CxxMajorMode.cxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtGui/QTextDocument>
#include <QtWidgets/QMenu>
#include <QtGui/QMouseEvent>

#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspCxxSupport.hxx"
#include "TFEL/GUI/LspNavigationSession.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"
#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel::gui {

  namespace {

    // start a clangd session on the client of the given editor widget;
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
    void startSession(EditorWidget &w,
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
                                         : QStringLiteral("started"));
      if (!description.isEmpty()) {
        message += QStringLiteral(": %1").arg(description);
      }
      w.displayInformativeMessage(message);
    }  // end of startSession()

    // the minibuffer question for the compilation database to use
    //
    // the answer is a path: either the database file, whose directory clangd
    // is told to look in, or a directory holding it, which it is told to look
    // in as it stands; an empty answer starts a plain session, leaving the
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
        startSession(this->editor, arguments, description);
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
    // what the session was started with, in the end; `database` is the file
    // which the arguments name, empty when the session is left to guess the
    // compilation options of the files it is fed — which is exactly what an
    // empty `arguments` says too
    struct ImplicitCompileCommands {
      QStringList arguments;
      QString message;
      QString database;
    };  // end of struct ImplicitCompileCommands

    // the database a session started without an explicit one uses: the file
    // `tfel-editor` was started with, when `--compile-commands-file` named one
    // which exists, else the one of the project of `directory` — or of the file
    // the current buffer visits, when no directory is given —, else none, which
    // is a plain session leaving the search to the server
    //
    // the miss of a default file is not reported as it happens: the message
    // says what the session ended up using, and mentions the default only
    // when it played no part in that
    //
    // the editor performs the search itself, rather than leaving it to the
    // server, because the option which hands a database to clangd is set when
    // the session starts and the user is told which database it was started
    // with, which requires knowing it before starting
    ImplicitCompileCommands implicitCompileCommands(EditorWidget &w,
                                                    const QString &directory) {
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
                  QStringLiteral("using %1 to start clangd").arg(path), path};
        }
        // a default which does not exist is a mistake the user made once, at
        // the command line; it does not void the session, the project is asked
        // instead and the mistake surfaces in the final message only
        missing_default = path;
      }
      const auto start =
          directory.isEmpty() ? currentFileDirectory(w) : directory;
      const auto found = LspCxxSupport::searchCompileCommandsFile(
          start.isEmpty() ? QDir::currentPath() : start);
      if (found.isEmpty()) {
        if (missing_default.isEmpty()) {
          return {{}, restricted_to_current_file, QString()};
        }
        return {{},
                QStringLiteral("no compile_commands.json found and default "
                               "file %1 is not found, clangd is restricted to "
                               "current C++ file")
                    .arg(missing_default),
                QString()};
      }
      auto error = QString();
      const auto arguments =
          LspCxxSupport::compileCommandsArguments(found, error);
      if (missing_default.isEmpty()) {
        return {arguments,
                QStringLiteral("using %1 to start clangd").arg(found), found};
      }
      return {arguments,
              QStringLiteral("using %1 to start clangd (default file %2 "
                             "not found)")
                  .arg(found, missing_default),
              found};
    }  // end of implicitCompileCommands()

  }  // end of namespace

  // the command `start-clangd`: a session of clangd with no other option
  // than those of the settings, using the compilation database the editor
  // was started with, or the one of the project of the current buffer, or
  // none, in which case clangd searches a database by itself, as it does
  // when it is run in a terminal
  void startClangd(EditorWidget &editor) {
    if (!supportIsEnabled(editor)) {
      return;
    }
    const auto choice = implicitCompileCommands(editor, QString());
    startSession(editor, choice.arguments, QString(), choice.message);
  }  // end of startClangd

  // the command `start-clangd-with-compile-commands`: the same, after the
  // compilation database to use has been asked for in the minibuffer
  void startClangdWithCompileCommands(EditorWidget &editor) {
    if (!supportIsEnabled(editor)) {
      return;
    }
    // the question is asked before the client is touched: an answer may name a
    // database, or none at all, and a session started in the meantime would
    // have to be replaced before it was even used
    const auto directory = currentFileDirectory(editor);
    editor.setUserInput(new CompileCommandsInput(editor, directory));
  }  // end of startClangdWithCompileCommands

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
          QStringLiteral("clangd is not started"));
      return;
    }
    editor.displayInformativeMessage(QStringLiteral("clangd stopped"));
  }  // end of stopClangd

  namespace {

    /*!
     * \brief the policy of the C++ mode about its language server
     *
     * Everything which is a question of the protocol — what to ask, when to ask
     * it, what to draw with the answer — belongs to \c LspNavigationSession,
     * which the \c Python mode uses the same way. What is left is what \c clangd
     * and the buffers of C++ decide between themselves: where the entries which
     * start a session are, which file of which project is worth starting one
     * without being asked for, and whether the identifiers of a file are colored
     * with what the server says they are
     *
     * \note no \c Q_OBJECT: a derived \c QObject which declares no signal, slot
     * or property of its own needs no meta object of its own, and the class is
     * not a header of its name, which is what the build's \c qt_sources would
     * have asked for
     */
    struct ClangdNavigationSession : public LspNavigationSession {
      ClangdNavigationSession(EditorWidget &w,
                              Buffer &b,
                              TextEditBase &t,
                              QObject *const p)
          : LspNavigationSession(w, b, t, QStringLiteral("clangd"),
                                 QStringLiteral("cpp"), p) {}

     protected:
      QString menuName() const override {
        return QStringLiteral("C/C++");
      }  // end of menuName

      QString startCommandName() const override {
        return QStringLiteral("start-clangd");
      }  // end of startCommandName

      bool semanticTokensWanted() const override {
        return LspCxxSupport::semanticTokensEnabled();
      }  // end of semanticTokensWanted

      void startSessionIfNeeded() override {
        if ((this->visitedFile().isEmpty()) ||
            (!LspCxxSupport::startedByDefault())) {
          // the setting is read on every buffer, so a person who turns the
          // automatic session off sees the next file opened without a server,
          // without a restart of the editor
          return;
        }
        if (!this->client().isIdle()) {
          // a session runs: the database it reads was chosen when it started and
          // a buffer of another project is none to replace the one somebody is
          // working with
          return;
        }
        const auto choice = implicitCompileCommands(
            this->editorWidget(),
            QFileInfo(this->visitedFile()).absolutePath());
        if (choice.database.isEmpty()) {
          // nothing was found to start a session with: a session which guesses
          // the compilation options of a file is a decision, not a discovery,
          // and the entries of the "C/C++" menu are where it is taken
          return;
        }
        auto error = QString();
        if (!this->client().launch(choice.arguments, error)) {
          // the program the settings name did not start. It is said once: the
          // failure is in the settings, and every buffer of the project would
          // otherwise report the same line again
          if (!this->auto_start_failure_reported) {
            this->auto_start_failure_reported = true;
            this->report(error);
          }
          return;
        }
        this->report(QStringLiteral("clangd started using %1")
                         .arg(choice.database));
      }  // end of startSessionIfNeeded

     private:
      /*!\brief true once the failure of an automatic session has been reported
       *
       * A program which will not start is a mistake the settings hold, and every
       * new buffer of the project would report it again; once is enough to find
       * the address of the setting, and the entries of the menu say what the
       * automatic start left unsaid.
       */
      bool auto_start_failure_reported = false;
    };  // end of struct ClangdNavigationSession

  }  // end of namespace

  CxxMajorMode::CxxMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
      : CMajorModeBase(w, b, t) {
    // what the comments of a C++ file say in English is a question of grammar
    // and none of `clangd`'s: the session of the checker therefore stands on
    // this side of the gate below, and a buffer whose C++ support is off is
    // checked like any other buffer of the editor. The `false` says which kind
    // of buffer this is: the words of a source file are its identifiers and a
    // proofreader reading whole texts has no sense of them, so the choice of
    // LanguageTool reaches the comments of such a buffer as spelling only, see
    // `SpellingAndGrammar::effectiveChecker`. What reads this buffer is decided
    // by the session itself when it is built: the choice held in the settings is
    // one for the editor and the buffer follows it, in this mode or in another
    this->grammar =
        new GrammarCheckSession(w, t, QStringLiteral("cpp"), false, this);
    if (!LspCxxSupport::isEnabled()) {
      return;
    }
    // the conversation with `clangd`: the client, the settling timers, the
    // tracked words, the diagnostics and the faces of the identifiers all live
    // there, and the mode keeps for itself only the answers of the session above
    this->clangd_session = new ClangdNavigationSession(w, b, t, this);
    // the session listens to the name of the buffer — a new file has no name
    // when the mode is built and gains it before or after, depending on how the
    // file was reached, and a save under a new name is the same event — but the
    // first hand-over is asked here, once the derived part of the session
    // exists: it is what decides whether the project of the file starts a
    // session of its own
    this->clangd_session->openOrRenameDocument();
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
    auto *const lc = m->addAction(QObject::tr("Start clangd"));
    lc->setToolTip(
        QObject::tr("Start clangd with the compilation database of the "
                    "project, or the one tfel-editor was started with; a "
                    "session which is running is restarted"));
    QObject::connect(lc, &QAction::triggered, this,
                     [this] { startClangd(this->editor); });
    auto *const cd =
        m->addAction(QObject::tr("Start clangd with a compile_commands.json"));
    cd->setToolTip(QObject::tr(
        "Ask for a compilation database and start clangd with it; a "
        "session which is running is restarted"));
    QObject::connect(cd, &QAction::triggered, this,
                     [this] {
                       startClangdWithCompileCommands(this->editor);
                     });
    m->addSeparator();
    // the overview of the file is the one entry of this menu which asks a
    // question of the buffer rather than of the session; it is here because a
    // key one has never heard of is a key never pressed
    auto *const gs = m->addAction(QObject::tr("Go to symbol..."));
    gs->setToolTip(
        QObject::tr("Ask which symbol of the file to go to, among the ones "
                    "clangd listed, and show the list as an overview (Alt-O)"));
    QObject::connect(gs, &QAction::triggered, this, [this] {
      if (this->clangd_session != nullptr) {
        this->clangd_session->showDocumentSymbols();
      }
    });
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
      if (this->clangd_session != nullptr) {
        if (on) {
          this->clangd_session->askSemanticTokens();
        } else {
          this->clangd_session->clearSemanticTokens();
        }
      }
    });
    // the proofreaders are programs of their own and the entries which steer them
    // are none of `clangd`'s business; they are in this menu because the comments
    // of a C++ file are read by them, and they are offered only when at least one
    // of the programs is there to be asked
    if (GrammarCheckSession::isAvailable()) {
      // the four choices of the proofreader, in the menu of this mode as in the
      // menu of `Options`: one choice for the editor, and the submenu of each
      // choice the languages that choice reads
      this->grammar->addToMenu(*m);
      m->addSeparator();
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
    if ((this->clangd_session != nullptr) &&
        (this->clangd_session->keyPressEvent(e))) {
      return true;
    }
    // the signature question is not asked here: the character typed reaches
    // the document only once this whole consult has returned false and the
    // underlying text edit has inserted it, so `watchSignatureHelp` hands the
    // question to a settling timer, by which time the call on screen is the
    // call the answer is about
    const auto handled = CMajorModeBase::keyPressEvent(e);
    if ((!handled) && (this->clangd_session != nullptr)) {
      this->clangd_session->watchSignatureHelp(*e);
    }
    return handled;
  }  // end of keyPressEvent

  bool CxxMajorMode::mousePressEvent(QMouseEvent *const e) {
    if ((this->clangd_session != nullptr) &&
        (this->clangd_session->mousePressEvent(e))) {
      return true;
    }
    // any other click — and any click of a buffer whose C++ support is off — is
    // the plain click the text edit makes of it itself
    return CMajorModeBase::mousePressEvent(e);
  }  // end of mousePressEvent

  QCompleter *CxxMajorMode::getCompleter() {
    if (this->clangd_session != nullptr) {
      if (auto *const c = this->clangd_session->getCompleter()) {
        return c;
      }
    }
    return CMajorModeBase::getCompleter();
  }  // end of getCompleter

  int CxxMajorMode::getMinimalCompletionLength() {
    if (this->clangd_session != nullptr) {
      return this->clangd_session->getMinimalCompletionLength();
    }
    return CMajorModeBase::getMinimalCompletionLength();
  }  // end of getMinimalCompletionLength

  void CxxMajorMode::completeCurrentWord(TextEditBase &t, const QString &w) {
    if ((this->clangd_session != nullptr) &&
        (this->clangd_session->completeCurrentWord(t, w))) {
      return;
    }
    // the popup of the server was not up: the completion of the mode, which is
    // the one of the base classes, is the one to apply
    MajorModeBase::completeCurrentWord(t, w);
  }  // end of completeCurrentWord

  void CxxMajorMode::completeContextMenu(QMenu *const m,
                                         const QTextCursor &tc) {
    MajorModeBase::completeContextMenu(m, tc);
    if (this->clangd_session != nullptr) {
      this->clangd_session->completeContextMenu(m, tc);
    }
  }  // end of completeContextMenu

  void CxxMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    this->setHighlighter(new CxxSyntaxHighlighter(d));
  }  // end of setSyntaxHighlighter

  CxxMajorMode::~CxxMajorMode() = default;

  GrammarCheckSession *CxxMajorMode::grammarCheckSession() const {
    // the choice of the proofreader walks the buffers of the editor through this
    // hook: a buffer of a C++ file is read by the session this mode holds, which-
    // ever of the two programs that choice names, and in a source file the answer
    // of the session is that LanguageTool reads no code there
    return this->grammar;
  }  // end of CxxMajorMode::grammarCheckSession

  QVector<SettingsGroup> CxxMajorMode::declareSettings() {
    // the five addresses `LspCxxSupport` reads are gathered in a theme of their
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
        "and the compilation database when it is started: what "
        "changes here reaches the next `Start clangd`");
    clangd.keys =
        QVector<SettingsKey>()
        << SettingsKey::boolean(
               "clangd/enable", QObject::tr("Ask clangd anything"),
               QObject::tr("When this is off, the C++ mode highlights and "
                           "indents its files and asks nothing to anyone: "
                           "no diagnostics, no completion, no jump to a "
                           "definition. The modes which are already "
                           "attached to a buffer were built with this "
                           "answer, and `Start clangd` refuses a session "
                           "while it is off"),
               true)
        << SettingsKey::boolean(
               "clangd/start-by-default",
               QObject::tr("Start clangd when the project of a "
                           "file has a compile_commands.json"),
               QObject::tr("When this is on, a buffer of a project which holds "
                           "a compilation database starts a session which reads "
                           "it, and the minibuffer names the database it used: "
                           "the options the files of a project are compiled "
                           "with are then known, and a buffer opened in such a "
                           "project was opened to be worked on with them. Where "
                           "no database is found, nothing is started either "
                           "way: a session which guesses the compilation "
                           "options of a file is a decision, and the two "
                           "`Start clangd` entries of the `C/C++` menu are "
                           "where it is taken. The buffers which are already "
                           "open keep the session they were opened with"),
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
    // nothing else: what the mode remembers of the commands it was asked to run
    // is a history, and a history is not an entry of the configuration, see
    // `CoreSettings.cxx`
    return QVector<SettingsGroup>() << clangd;
  }  // end of CxxMajorMode::declareSettings

  static StandardFunctionCommandProxy<startClangd> startClangdProxy(
      "start-clangd");

  static StandardFunctionCommandProxy<startClangdWithCompileCommands>
      startClangdWithCompileCommandsProxy(
          "start-clangd-with-compile-commands");

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
