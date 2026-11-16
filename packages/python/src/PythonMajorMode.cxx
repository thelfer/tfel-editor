/*!
 * \file  PythonMajorMode.cxx
 * \brief major mode dedicated to the Python language.
 * \author Thomas Helfer
 * \date   26/09/2012
 */

#include <algorithm>
#include <functional>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>
#include <QtCore/QTemporaryFile>
#include <QtGui/QActionGroup>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QMenu>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspNavigationSession.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/ProcessInteractionFrame.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/PythonMajorMode.hxx"
#include "TFEL/GUI/PythonSyntaxHighlighter.hxx"

namespace tfel::gui {

  namespace {

    // what the minibuffer calls the server, the object name its client is found
    // by, and the key of the faces the session of a buffer draws
    const QString pylsp_server_name{QLatin1String("pylsp")};

    // what `pylsp` is told the files it is handed are, in the sense of the
    // `languageId` of the protocol
    const QString pylsp_language_id{QLatin1String("python")};

    // the program of the server: the setting first, since a person who works in
    // a virtual environment knows which `pylsp` holds the plugins they enabled
    // there; the name a terminal would find otherwise. An empty answer means
    // there is nothing to start, and who asks gets that said to them
    QString pylspProgram() {
      const auto configured =
          QSettings().value(QStringLiteral("pylsp/executable")).toString();
      if (!configured.trimmed().isEmpty()) {
        return configured.trimmed();
      }
      return QStandardPaths::findExecutable(QStringLiteral("pylsp"));
    }  // end of pylspProgram()

    //! the arguments the settings say to start the server with
    QStringList pylspArguments() {
      return QSettings()
          .value(QStringLiteral("pylsp/arguments"))
          .toStringList();
    }  // end of pylspArguments()

    //! \return true when a buffer of the language is to start a server of its own
    bool startedByDefault() {
      return QSettings()
          .value(QStringLiteral("pylsp/start-by-default"), true)
          .toBool();
    }  // end of startedByDefault()

    // the directory the server takes as the root of the project it reads: the
    // one of the file which asked, or of the file the current buffer visits,
    // empty when no file is in sight, which leaves the server the working
    // directory of the editor
    QString projectRoot(const QString &file) {
      if (file.isEmpty()) {
        return QString();
      }
      return QFileInfo(file).absolutePath();
    }  // end of projectRoot()

    // what the mode offers a server to run over the text of a buffer, under the
    // name its plugin answers to. `pylsp` decides what it runs out of the
    // settings it is handed, keyed by that name, so the tables below are the one
    // place the names are spelled — and the name of the plugin is not always the
    // name of the program: the mypy plugin is `pylsp_mypy`, the isort one
    // `pyls_isort`, and both are what a person would call `mypy` and `isort`
    struct Tool {
      //! name the plugin of the tool is enabled by
      const char *plugin;
      //! name the menu and the minibuffer call the tool
      const char *label;
      //! whether the tool runs when nothing was ever chosen, of a linter only
      bool enabled_by_default;
      //! the sentence the entry of the menu carries as its tooltip
      const char *tip;
    };

    /*!
     * \return the linters, in the order the menu of the mode lists them
     *
     * \note `ruff` stands first and alone enabled: it does the work of Pyflakes
     * and pycodestyle in one program which is faster than either, and this is
     * what the plugin of the same name does by default, so that a buffer does
     * not read the same mistake under three underlines
     *
     * \note `pylint` and `mypy` are off because they are slow, the first runs a
     * program over the file on disk and reports only what a saved file says, the
     * second needs the types of the names it reads to say anything and is noisy
     * on a file which does not type its arguments
     */
    const QVector<Tool> &linters() {
      static const auto tools =
          QVector<Tool>()
          << Tool{"ruff", "Ruff", true,
                  "An all-in-one program which checks and formats: what "
                  "Pyflakes and pycodestyle report, with a fix to write for "
                  "much of it"}
          << Tool{
                 "pyflakes", "Pyflakes", false,
                 "The names which are used before they are set, the imports "
                 "nobody uses, and the mistakes which stop a file from running"}
          << Tool{"pycodestyle", "Pycodestyle", false,
                  "What the style guide of the language says and a person "
                  "breaks daily: the line too long, the space missing after a "
                  "comma"}
          << Tool{"flake8", "Flake8", false,
                  "Pycodestyle and Pyflakes in one program, with the codes of "
                  "both, and slower than either"}
          << Tool{"pylint", "Pylint", false,
                  "The widest analysis of them all and the slowest: it reads "
                  "the file on disk, so it reports about a saved buffer only"}
          << Tool{"mccabe", "McCabe", false,
                  "The functions whose branches are too many for one person to "
                  "follow, counted"}
          << Tool{"pydocstyle", "Pydocstyle", false,
                  "The docstrings which are missing, and how the ones which "
                  "are there are written"}
          << Tool{"pylsp_mypy", "Mypy", false,
                  "Asks mypy what the types of the names are: on a file which "
                  "types nothing, more often a warning than a mistake"};
      return tools;
    }  // end of linters()

    /*!
     * \return the programs which reformat a buffer, in the order the menu lists
     * them, `none` last since it is the choice of a person who formats with
     * something outside the editor
     *
     * \note `ruff` is the default, and it is the same program as the linter of
     * the same name: the formatter of `ruff` is what `black` writes, in a program
     * which is already running to lint the file
     */
    const QVector<Tool> &formatters() {
      static const auto tools =
          QVector<Tool>()
          << Tool{"ruff", "Ruff", true,
                  "What Black writes, by the program which already reads this "
                  "file for what is wrong with it"}
          << Tool{"black", "Black", false,
                  "The `black` program: the whole file written in one style, "
                  "the quotes of its strings included"}
          << Tool{"autopep8", "Autopep8", false,
                  "The `autopep8` program: the smallest change which makes the "
                  "style right, and the quotes of the file left where they are"}
          << Tool{"yapf", "Yapf", false,
                  "The `yapf` program, which formats line by line and answers "
                  "one change per line it touches"}
          << Tool{"none", "None", false,
                  "Nothing: a request to format a buffer is answered with "
                  "nothing, and a buffer keeps what it says"};
      return tools;
    }  // end of formatters()

    /*!
     * \return the programs which sort the imports of a buffer
     *
     * \note `isort` is named by its own name, not by the `pyls_isort` of its
     * plugin: that name is what the settings of the server call it and this is
     * what a person calls it
     */
    const QVector<Tool> &import_sorters() {
      static const auto tools =
          QVector<Tool>()
          << Tool{"ruff", "Ruff", true,
                  "The `I` rules of Ruff, which is what the change it offers "
                  "for sorting the imports of a file is made of"}
          << Tool{"isort", "Isort", false,
                  "The `isort` program, which sorts the imports as part of "
                  "formatting the buffer and so follows the formatter chosen "
                  "above it"}
          << Tool{"none", "None", false,
                  "Nothing: the entry which sorts the imports says that no "
                  "program is chosen"};
      return tools;
    }  // end of import_sorters()

    //! the address a linter is switched at
    QString linterAddress(const char *const plugin) {
      return QStringLiteral("pylsp/linters/") + QLatin1String(plugin);
    }  // end of linterAddress

    //! the address a tool of a list of tools is chosen at
    QString toolsAddress(const char *const kind) {
      return QStringLiteral("pylsp/") + QLatin1String(kind);
    }  // end of toolsAddress

    // the name a tool is called by in the menu and the minibuffer. The tool
    // which names no program is the one name of these lists which is a word of
    // the language and not a program, so it is the one translated
    QString labelOf(const QString &plugin) {
      if (plugin == QLatin1String("none")) {
        return QObject::tr("None");
      }
      for (const auto &t : linters()) {
        if (plugin == QLatin1String(t.plugin)) {
          return QString::fromUtf8(t.label);
        }
      }
      for (const auto &t : formatters()) {
        if (plugin == QLatin1String(t.plugin)) {
          return QString::fromUtf8(t.label);
        }
      }
      for (const auto &t : import_sorters()) {
        if (plugin == QLatin1String(t.plugin)) {
          return QString::fromUtf8(t.label);
        }
      }
      return plugin;
    }  // end of labelOf

    //! the names of a list of tools, as the settings hold them and as one types
    QStringList namesOf(const QVector<Tool> &tools) {
      auto names = QStringList{};
      for (const auto &t : tools) {
        names << QLatin1String(t.plugin);
      }
      return names;
    }  // end of namesOf

    // the same list of tools as the entries of a choice of the configuration
    // dialog, which is where a person who prefers another program picks it
    // without remembering the name the commands are made of
    QVector<SettingsKey::Choice> choicesOf(const QVector<Tool> &tools) {
      auto choices = QVector<SettingsKey::Choice>();
      for (const auto &t : tools) {
        choices << SettingsKey::Choice(QString::fromLatin1(t.plugin),
                                       labelOf(QLatin1String(t.plugin)));
      }
      return choices;
    }  // end of choicesOf()

    /*!
     * \return true when the settings say a linter reads the buffers
     *
     * \note the address is read on every call, as every other address of this
     * mode, so what the menu clicks or the dialog writes reaches the next
     * question without a session being restarted
     */
    bool linterIsEnabled(const char *const plugin) {
      for (const auto &t : linters()) {
        if (QLatin1String(plugin) == QLatin1String(t.plugin)) {
          return QSettings()
              .value(linterAddress(plugin), t.enabled_by_default)
              .toBool();
        }
      }
      return false;
    }  // end of linterIsEnabled

    //! the program the settings say rewrites the text of a buffer
    QString chosenFormatter() {
      const auto v = QSettings()
                         .value(toolsAddress("formatter"),
                                QStringLiteral("ruff"))
                         .toString();
      return v.isEmpty() ? QStringLiteral("ruff") : v;
    }  // end of chosenFormatter()

    //! the program the settings say sorts the imports of a buffer
    QString chosenImportSorter() {
      const auto v = QSettings()
                         .value(toolsAddress("import-sorter"),
                                QStringLiteral("ruff"))
                         .toString();
      return v.isEmpty() ? QStringLiteral("ruff") : v;
    }  // end of chosenImportSorter()

    /*!
     * \brief the settings `pylsp` is told to run by, in the shape it reads them
     *
     * This is what the handshake of a session carries and what the questions the
     * server asks of the client are answered with, and the two have to say the
     * same thing: the first reaches the analysis of the file which starts with
     * the handshake, the second every analysis after a change of a choice.
     *
     * \note every tool the mode manages is named on every write, the ones which
     * are off included: `pylsp` replaces the settings it holds with what a write
     * says, so a tool which is merely absent keeps whatever the last write said
     * of it. Naming a plugin which is not installed costs nothing: the name is
     * looked for among the plugins which were loaded, where it simply is not
     *
     * \note exactly one formatter is enabled, and the formatter of `ruff` is
     * turned off with the same care: it is a wrapper of the format question
     * which answers whatever it computed itself when it is on, whatever any
     * other plugin of the same question had to say (measured), so two formatters
     * on is one formatter answering and the other one silent
     */
    QJsonValue pylspConfiguration() {
      auto plugins = QJsonObject{};
      for (const auto &t : linters()) {
        if (QLatin1String(t.plugin) != QLatin1String("ruff")) {
          plugins.insert(
              QLatin1String(t.plugin),
              QJsonObject{{QStringLiteral("enabled"),
                          linterIsEnabled(t.plugin)}});
        }
      }
      const auto formatter = chosenFormatter();
      plugins.insert(QStringLiteral("black"),
                     QJsonObject{{QStringLiteral("enabled"),
                                  formatter == QLatin1String("black")}});
      plugins.insert(QStringLiteral("autopep8"),
                     QJsonObject{{QStringLiteral("enabled"),
                                  formatter == QLatin1String("autopep8")}});
      plugins.insert(QStringLiteral("yapf"),
                     QJsonObject{{QStringLiteral("enabled"),
                                  formatter == QLatin1String("yapf")}});
      const auto sorter = chosenImportSorter();
      plugins.insert(QStringLiteral("pyls_isort"),
                     QJsonObject{{QStringLiteral("enabled"),
                                  sorter == QLatin1String("isort")}});
      // `ruff` is the one tool of this mode which is asked three different
      // things: whether it lints, whether it formats — its own switch, since a
      // person who lints with it may format with another — and whether it sees
      // unsorted imports as a problem to report and fix. That last one is what
      // its change sorting the imports is made of: without the `I` rules
      // selected, the program says nothing about the order of the imports and
      // offers no such change at all (measured)
      auto ruff = QJsonObject{
          {QStringLiteral("enabled"), linterIsEnabled("ruff")},
          {QStringLiteral("formatEnabled"),
           formatter == QLatin1String("ruff")}};
      if (sorter == QLatin1String("ruff")) {
        ruff.insert(QStringLiteral("extendSelect"),
                    QJsonArray{QStringLiteral("I")});
      }
      plugins.insert(QStringLiteral("ruff"), ruff);
      return QJsonObject{{QStringLiteral("pylsp"),
                          QJsonObject{{QStringLiteral("plugins"), plugins}}}};
    }  // end of pylspConfiguration()

    // hand a client what the settings say it runs: the settings of its handshake,
    // which is where `pylsp` reads them before its very first analysis, and the
    // answer of the questions it asks instead, which is what every analysis after
    // the handshake is read under
    void handSettingsTo(LspClient &client) {
      const auto settings = pylspConfiguration();
      client.setInitializationOptions(settings);
      client.setConfiguration(settings);
    }  // end of handSettingsTo()

    // tell the session which is running — if any — what the choices now are, and
    // hand it its documents to read again, which is what makes the underlines of
    // the buffers follow the entry which was just clicked. With no session
    // running there is nothing to tell: the settings written there are those the
    // next `Start pylsp` carries
    void applyConfigurationTo(EditorWidget &editor) {
      auto *const client =
          editor.findChild<LspClient *>(QLatin1String("pylsp"));
      if (client == nullptr) {
        return;
      }
      handSettingsTo(*client);
      client->pushConfiguration();
    }  // end of applyConfigurationTo()

    //! how many of the linters the settings say are on
    int countEnabledLinters() {
      auto count = 0;
      for (const auto &t : linters()) {
        if (linterIsEnabled(t.plugin)) {
          ++count;
        }
      }
      return count;
    }  // end of countEnabledLinters()

    // the sentence a choice of one of the things `ruff` does has to carry when
    // the plugin of that name is switched off: the three questions asked of it —
    // what is wrong with the file, what it would look like reformatted, what its
    // sorted imports are — are all answered by one plugin, and the one flag of
    // that plugin decides all three, so a choice of it decides nothing until it
    // is turned on
    QString ruffIsSwitchedOff() {
      return QStringLiteral(" (") +
             QObject::tr("the Ruff plugin is switched off: turn it on under "
                         "`Linters` of the `Python` menu") +
             QStringLiteral(")");
    }  // end of ruffIsSwitchedOff()

    // switch a linter on or off and hand the choice to the session which is
    // running: the underlines of the buffers are what this decides, and waiting
    // for the next key typed to see what the hand just clicked is a long wait
    void setLinter(EditorWidget &editor, const char *const plugin,
                   const bool on) {
      QSettings().setValue(linterAddress(plugin), on);
      applyConfigurationTo(editor);
      auto message =
          QStringLiteral("%1 %2")
              .arg(labelOf(QLatin1String(plugin)),
                   on ? QObject::tr("enabled") : QObject::tr("disabled"));
      if (countEnabledLinters() == 0) {
        // the last linter off is a buffer with no underlines at all, which is a
        // thing a person means to do once in a while and usually does not
        message += QStringLiteral("; ") +
                   QObject::tr("no linter is left enabled");
      }
      editor.displayInformativeMessage(message);
    }  // end of setLinter()

    // choose which program rewrites the text of a buffer when the format key is
    // pressed, and hand the choice to the session which is running
    void setFormatter(EditorWidget &editor, const QString &name) {
      QSettings().setValue(toolsAddress("formatter"), name);
      applyConfigurationTo(editor);
      if (name == QLatin1String("none")) {
        editor.displayInformativeMessage(QObject::tr(
            "no formatter: pylsp answers a request to format a buffer with "
            "nothing, and no plugin of the language is started to do it"));
        return;
      }
      editor.displayInformativeMessage(
          QObject::tr("formatting with %1").arg(labelOf(name)) +
          ((name == QLatin1String("ruff") && (!linterIsEnabled("ruff")))
               ? ruffIsSwitchedOff()
               : QString()));
    }  // end of setFormatter()

    // choose which program sorts the imports of a buffer, and hand the choice to
    // the session which is running
    void setImportSorter(EditorWidget &editor, const QString &name) {
      QSettings().setValue(toolsAddress("import-sorter"), name);
      applyConfigurationTo(editor);
      if (name == QLatin1String("none")) {
        editor.displayInformativeMessage(
            QObject::tr("no import sorter: the entry which sorts the imports of "
                        "a buffer will ask nothing and say so"));
        return;
      }
      auto message =
          QObject::tr("sorting the imports with %1").arg(labelOf(name));
      if (name == QLatin1String("isort")) {
        // what the choice is worth to the person who made it: the plugin of
        // `isort` is a wrapper of the question about formatting, so it sorts the
        // imports of a buffer as part of formatting it (measured), which is both
        // why the entry asks for a reformat and why a formatter of another name
        // still gets its imports sorted
        message += QStringLiteral("; ") +
                   QObject::tr("Isort sorts as part of formatting the buffer");
      } else if (name == QLatin1String("ruff") && (!linterIsEnabled("ruff"))) {
        message += ruffIsSwitchedOff();
      }
      editor.displayInformativeMessage(message);
    }  // end of setImportSorter()

    /*!
     * \brief the question of a choice among a list of programs
     *
     * The answer is a name of the settings, since that is what is written, and
     * what the completion therefore lists: a name is what one types, the label of
     * the menu is what one reads
     */
    struct ToolChoiceInput : public CommandLine {
      ToolChoiceInput(EditorWidget &w,
                      const QString &question,
                      const QString &name,
                      const QStringList &names,
                      const QString &history,
                      const std::function<void(const QString &)> &applied)
          : CommandLine(question, w), candidates(names), on_answer(applied) {
        auto *const c = new QCompleter(names, this);
        c->setWidget(this->input);
        c->setCompletionMode(QCompleter::InlineCompletion);
        // nobody types the case of a program they have not looked up, and the
        // question is which of these is meant, not which bytes it is
        c->setCaseSensitivity(Qt::CaseInsensitive);
        this->input->setCompleter(c, false);
        // the name is what tells this question apart from every other input the
        // minibuffer has held, for tests as for the completion code
        this->setObjectName(name);
        this->setInputHistorySettingAddress(history);
      }  // end of ToolChoiceInput

      ~ToolChoiceInput() override = default;

     protected:
      void treatUserInput() override {
        const auto answer = this->input->text().trimmed();
        // the empty answer cancels, as if the question had never been asked
        if (answer.isEmpty()) {
          return;
        }
        // an exact name wins over the names which merely contain it: the answer
        // `ruff` means `ruff`, and `none` is the tail of no other name
        auto exact = QStringList();
        auto tail = QStringList();
        for (const auto &n : this->candidates) {
          if (n.compare(answer, Qt::CaseInsensitive) == 0) {
            exact << n;
          } else if (n.contains(answer, Qt::CaseInsensitive)) {
            tail << n;
          }
        }
        const auto &matches = exact.isEmpty() ? tail : exact;
        if (matches.isEmpty()) {
          this->editor.displayInformativeMessage(
              QObject::tr("no program named '%1' among: %2")
                  .arg(answer, this->candidates.join(QStringLiteral(", "))));
          return;
        }
        if (matches.size() > 1) {
          // two programs of one list which share the name being typed — `auto`
          // is `autopep8` alone, so this is a list of one — leaves the choice
          // open and says what the names to choose between are
          this->editor.displayInformativeMessage(
              QObject::tr("%1 are all named like that: type one of them")
                  .arg(matches.join(QStringLiteral(", "))));
          return;
        }
        if (this->on_answer) {
          this->on_answer(matches.at(0));
        }
      }  // end of treatUserInput

     private:
      //! the names the answer is read against
      const QStringList candidates;
      //! what is done with the name which was chosen
      const std::function<void(const QString &)> on_answer;
    };  // end of struct ToolChoiceInput

    /*!
     * \return the mode of the current buffer when it is the Python one, null and
     * a report in the minibuffer when it is not
     *
     * Every command of this mode is about a session of the language and the
     * buffers that session reads: run from a buffer of another language, it
     * would write an address the mode of that buffer never reads, or ask a
     * question no server has an answer to, so it says what it refuses instead of
     * doing something which looks like working and does nothing
     *
     * \param[in] editor : the editor widget the command was launched from
     * \param[in] what : what the command was asked to do, for that report
     */
    PythonMajorMode *currentPythonMode(EditorWidget &editor,
                                       const QString &what) {
      if (editor.getBuffersNames().isEmpty()) {
        // asking the current buffer of an editor which holds none creates one
        editor.displayInformativeMessage(
            QObject::tr("%1 is a Python command: no buffer is here to apply it "
                        "to")
                .arg(what));
        return nullptr;
      }
      auto &frame = editor.getCurrentBuffer().getMainFrame();
      // `dynamic_cast`, not `qobject_cast`: a major mode is a `QObject` which
      // declares no signal, slot or property of its own and so carries no `Q_OBJECT`
      // macro, which is what `qobject_cast` reads — the modes of the editor are
      // told apart by their virtual interface and their names, as elsewhere
      auto *const mode =
          dynamic_cast<PythonMajorMode *>(const_cast<MajorMode *>(&frame.getMajorMode()));
      if (mode == nullptr) {
        editor.displayInformativeMessage(
            QObject::tr("%1 is a Python command: the major mode of the current "
                        "buffer is %2")
                .arg(what, frame.getMajorMode().getName()));
      }
      return mode;
    }  // end of currentPythonMode()

    // start a session of `pylsp` on the client of the given editor widget, with
    // the program and the arguments the settings name and the project root given

    // a session which is still running is replaced: the root it reads is chosen
    // when it starts and can not be changed by asking it, and a second session of
    // one server on one editor is none's intention — `LspClient::launch` ends the
    // running one and re-sends every document after the new handshake
    void startSession(EditorWidget &w, const QString &root) {
      auto &client = LspClient::get(w, pylsp_server_name);
      const auto restarting = (!client.isIdle());
      const auto program = pylspProgram();
      if (program.isEmpty()) {
        w.displayInformativeMessage(
            QStringLiteral("pylsp was not found: install it or set the "
                           "'pylsp/executable' option"));
        return;
      }
      // the client of a server is built by whoever asks for it first, and a
      // client remembers what it was told: `start-pylsp` from a buffer of
      // another language would otherwise leave it saying `cpp` of python files
      client.setLanguageId(pylsp_language_id);
      client.setRootUri(root);
      // which linters run, which program formats and which sorts the imports is
      // decided by the server out of the settings of its handshake, and it
      // decides before its first analysis: a session which was not told would
      // lint with its own defaults and be answered according to them
      handSettingsTo(client);
      auto error = QString();
      if (!client.launch(program, pylspArguments(), error)) {
        w.displayInformativeMessage(error);
        return;
      }
      auto message = QStringLiteral("pylsp %1")
                         .arg(restarting ? QStringLiteral("restarted")
                                         : QStringLiteral("started"));
      if (!root.isEmpty()) {
        message += QStringLiteral(" with project root %1").arg(root);
      }
      w.displayInformativeMessage(message);
    }  // end of startSession()

    /*!
     * \brief what `pylsp` and the buffers of this mode decide between themselves

     * Everything which is a question of the protocol belongs to
     * \c LspNavigationSession, which the C++ mode uses the same way with
     * \c clangd. What is left is the few answers which are not: where the entries
     * which start a session are, that this server has no declaration question and
     * no semantic tokens, and that a file of this language is worth starting a
     * session for without being asked
     *
     * \note no \c Q_OBJECT: a derived \c QObject which declares no signal, slot
     * or property of its own needs no meta object of its own, and the class is
     * not a header of its name, which is what the build's \c qt_sources would
     * have asked for
     */
    struct PythonNavigationSession : public LspNavigationSession {
      PythonNavigationSession(EditorWidget &w,
                              Buffer &b,
                              TextEditBase &t,
                              QObject *const p)
          : LspNavigationSession(
                w, b, t, pylsp_server_name, pylsp_language_id, p) {}

     protected:
      QString menuName() const override {
        return QStringLiteral("Python");
      }  // end of menuName

      QString startCommandName() const override {
        return QStringLiteral("start-pylsp");
      }  // end of startCommandName

      bool declarationSupported() const override {
        // `pylsp` answers `-32601` to `textDocument/declaration`: a jump is
        // decided by the definition question alone, and asking a second question
        // which is refused would make every tracked word wait on nothing
        return false;
      }  // end of declarationSupported

      bool semanticTokensWanted() const override {
        // the server announces no semantic tokens at all, so the question is
        // never asked and the faces of the identifiers are those of the
        // highlighter of this mode and nothing else
        return false;
      }  // end of semanticTokensWanted

      bool codeActionOffered(const LspCodeAction &action) const override {
        // `ruff` answers the changes of a whole file — sorting its imports,
        // applying every safe fix — on every range it is asked about, whatever
        // the menu was opened on (measured). Those two rewrite the buffer the
        // menu was opened in, and the `Python` menu and the commands are where
        // a person asks for that, in the knowledge of what it does to the file;
        // a menu opened over a word is not
        return !action.kind.startsWith(QLatin1String("source."));
      }  // end of codeActionOffered

      void startSessionIfNeeded() override {
        // a buffer which visits no file has no text the server can point an
        // error at by name, and no project to be read in: `*scratch*` and the
        // buffers of a process output are none of `pylsp`'s business
        if (this->visitedFile().isEmpty()) {
          return;
        }
        if (!startedByDefault()) {
          // the setting is read on every buffer, so a person who turns the
          // automatic session off sees the next file opened without a server,
          // without a restart of the editor
          return;
        }
        if (!this->client().isIdle()) {
          // a session runs, on the root it was given when it started; a buffer of
          // another project is none to replace the one somebody is working with,
          // and the two menu entries are where that choice is taken
          return;
        }
        const auto program = pylspProgram();
        if (program.isEmpty()) {
          reportAutoStartFailure(QStringLiteral(
              "pylsp was not found: install it or set the 'pylsp/executable' "
              "option"));
          return;
        }
        const auto root = projectRoot(this->visitedFile());
        auto error = QString();
        this->client().setLanguageId(pylsp_language_id);
        this->client().setRootUri(root);
        // the tools the settings name, before the handshake which carries them
        handSettingsTo(this->client());
        if (!this->client().launch(program, pylspArguments(), error)) {
          // the program the settings name did not start. It is said once: the
          // failure is in the settings, and every buffer of the project would
          // otherwise report the same line again
          reportAutoStartFailure(error);
          return;
        }
        // a session nobody asked for says what it reads: the project the answers
        // about a name are computed from is not something a reader can guess
        this->report(root.isEmpty()
                         ? QStringLiteral("pylsp started")
                         : QStringLiteral("pylsp started with project root %1")
                               .arg(root));
      }  // end of startSessionIfNeeded

     private:
      //! say once that the automatic session did not come up
      void reportAutoStartFailure(const QString &m) {
        if (!this->auto_start_failure_reported) {
          this->auto_start_failure_reported = true;
          this->report(m);
        }
      }  // end of reportAutoStartFailure

      /*!\brief true once the failure of an automatic session has been reported
       *
       * A program which will not start is a mistake the settings hold, and every
       * new buffer of the same project would report it again; once is enough to
       * find the address of the setting, and the entries of the menu say what the
       * automatic start left unsaid.
       */
      bool auto_start_failure_reported = false;
    };  // end of struct PythonNavigationSession

    // ==== The indentation of the language ================================
    //
    // What follows is a model of `python-indent-line-function' of `python.el'
    // GNU Emacs 27.1, measured press by press against the real thing
    // (`emacs -Q --batch' on the extracted `python.el' of this machine, the
    // probes and their answers under `/tmp/opencode/pylsp2/'). The two questions
    // the model answers for a line are: which indentation does the file above it
    // allow it — `python-indent--calculate-indentation', `proper' below — and
    // which levels does a run of presses walk it through — the same function's
    // `python-indent--calculate-levels'.
    //
    // The levels, and so the cycle of the presses:
    //
    // - the presses are a walk through the levels of the line, never a ladder the
    //   presses climb: the first press puts the line at the highest of them, each
    //   further press takes it to the highest level strictly below the place it
    //   holds, and the press which sits at the lowest brings it back to the
    //   highest. The last line of `print / if True: / if False: / print' is
    //   therefore `8 4 0 8' from whatever indentation it was written with;
    // - for a line whose `proper' is a plain number, the levels are that number
    //   and the multiples of the level which are under it — `number-sequence' of
    //   0 to `proper - 1' stepped by 4, with `proper' itself last: a line whose
    //   `proper' is 6 is written at 6, then at 4, then at 0 (`ladderLevels');
    // - a dedenter is the exception, and the reason the levels are a list rather
    //   than a sequence: its levels are the indentations of the blocks it can
    //   close, which are nowhere near a ladder. Measured: the `else' of an `if'
    //   which is itself the body of another `if' cycles `4 4 4' (`else-after-return':
    //   its own body already holds a line at the block's indentation, which closes
    //   the block for the lines below it), an `else' of three nested `if's cycles
    //   `8 4 0 8', and an `else' with nothing above it to close is pinned at 0;
    // - `Shift-Tab' takes the line one level down and stops at the lowest level of
    //   its cycle: it is the key which means out of a block, it has no top to wrap
    //   back to. `python.el' binds no shift tab at all — it shifts a region with
    //   `C-c <' and `C-c >' — so the key is this mode's own, and a selected region
    //   is shifted a level right by `Tab' and left by `Shift-Tab' rather than
    //   reindented line by line, which is what `python-indent-region' does.
    //
    // `proper', by the order the real function tries its contexts:
    //
    // - the first line of a file, and a file with nothing above the line, is the
    //   margin (\c :no-indent');
    // - a line which starts inside a string stands at the indentation of the line
    //   the string was opened on (`:inside-string'), except inside a docstring,
    //   which is prose and keeps the greater of its own indentation and that base
    //   (`:inside-docstring'). A docstring is the statement: the string is the
    //   first token of the line, the line assigns nothing, and above it stands the
    //   beginning of the file, an assignment, or the header of a `def' or a
    //   `class';
    // - a line inside brackets: the line which closes the innermost open bracket
    //   goes back to the indentation of the line that bracket was opened on when
    //   nothing but the bracket follows it on its line, and to the place right
    //   behind it otherwise (`:inside-paren-at-closing-paren'); an item of a
    //   bracket alone on its line stands one level to the right of that line
    //   (`:inside-paren-newline-start'); any other line stands right behind that
    //   bracket (`:inside-paren'), which is 6 for `def f(a,' and 4 for `foo(a,';
    // - the first line under a line ending with a backslash stands one level past
    //   it — or right behind the block keyword, when the line is led by one — and
    //   the lines below it keep the place of the line before them
    //   (`:after-backslash-first-line', `:after-backslash');
    // - a line which opens a block and ends a statement — `try:' with its `except'
    //   above — stands one level past the statement it closes (`:after-block-start');
    // - a dedenter is placed by the blocks it closes, collected the way
    //   `python-info-dedenter-opening-block-positions' collects them: the lines
    //   above are walked backwards over the words which open a block, a block is
    //   kept when nothing has been kept at its indentation yet, when nothing has
    //   been kept at a lower one, and when no non-blank line written between it and
    //   the target stands at its own indentation or lower; the walk stops at the
    //   margin and keeps only the blocks the dedenter closes;
    // - a line whose first word ends the block it stands in — `break', `continue',
    //   `pass', `raise', `return' — stands one level to the left of that word
    //   (`:after-block-end'). `assert', `exit', `yield', `quit' and their call
    //   forms are *not* among those words, measured: they place the line below them
    //   like any other statement;
    // - a comment-only line is copied as it stands by the line under it
    //   (`:after-comment'): the answer to a comment at 2 is 2, measured;
    // - any other line hands the line below its own indentation
    //   (`:after-line'), and the statement that indentation is taken from is the
    //   first line of the statement the line above belongs to, which a continuation
    //   of a bracket or a backslash may have moved far above it.

    //! The width of one level, `python-indent-offset' of the language
    constexpr int indent_offset = 4;

    //! The words which end the block their own line stands in: a line whose
    //! first word is one of them hands the line below one level *less* than
    //! its own, which is the press that leaves the block (`python-rx
    //! block-ender'). Measured: `assert', `yield', `exit()' and `quit()' are
    //! not among them; they place the line below like any other statement.
    const QStringList block_end_words{QStringLiteral("break"),
                                      QStringLiteral("continue"),
                                      QStringLiteral("pass"),
                                      QStringLiteral("raise"),
                                      QStringLiteral("return")};

    /*!
     * \brief the blocks a dedenter can close and the words which open them
     * \return the words of the blocks the given dedenter closes
     */
    QStringList closableBy(const QString &dedenter) {
      if (dedenter == QLatin1String("elif")) {
        return {QStringLiteral("if"), QStringLiteral("elif")};
      }
      if (dedenter == QLatin1String("else")) {
        return {QStringLiteral("if"), QStringLiteral("elif"),
                QStringLiteral("except"), QStringLiteral("for"),
                QStringLiteral("while")};
      }
      if (dedenter == QLatin1String("except")) {
        return {QStringLiteral("try"), QStringLiteral("except")};
      }
      // `finally' closes the `try' above, through the `except's and the
      // `else' of it
      return {QStringLiteral("try"), QStringLiteral("except"),
              QStringLiteral("else")};
    }  // end of closableBy()

    //! The words which open a block of the language
    bool opensBlockWord(const QString &word) {
      static const QStringList words{QStringLiteral("if"),
                                     QStringLiteral("elif"),
                                     QStringLiteral("else"),
                                     QStringLiteral("for"),
                                     QStringLiteral("while"),
                                     QStringLiteral("try"),
                                     QStringLiteral("except"),
                                     QStringLiteral("finally"),
                                     QStringLiteral("with"),
                                     QStringLiteral("def"),
                                     QStringLiteral("class"),
                                     QStringLiteral("async")};
      return words.contains(word);
    }  // end of opensBlockWord()

    /*!
     * \brief an opening bracket which is not closed yet, and what the lines
     * below it are placed by
     *
     * What a line inside brackets stands at is decided by what the line of the
     * bracket itself holds behind it: the items of a bracket which is alone on
     * its line stand one level to the right of that line and the bracket which
     * closes it goes back to the line's own indentation, while the lines of a
     * bracket which already holds an item stand under the place right behind
     * that bracket. `python.el' calls the two contexts
     * \c :inside-paren-newline-start and \c :inside-paren, and the bracket
     * which answers is the innermost one the line stands in: the items of
     * `foo(bar(` are placed by `bar(', not by `foo('.
     */
    struct Opener {
      //! the indentation of the line the bracket was opened on
      int line_indent = 0;
      //! the place right behind the bracket itself
      int column_after = 0;
      //! nothing but blanks, or a comment, stands behind the bracket
      bool on_its_own_line = false;
    };  // end of struct Opener

    /*!
     * \brief the state of the scanner of a line: what the text read so far
     * leaves open behind it
     */
    struct ScanState {
      //! the brackets which are not closed yet, the innermost last
      QVector<Opener> openers;
      //! the quote of the triple-quoted string which is not closed, if any
      QChar open_quote;
      //! the indentation of the line the open string starts on
      int string_indent = 0;
      //! the open string is the docstring of the block which holds it
      bool string_doc = false;
    };  // end of struct ScanState

    /*!
     * \brief what one line of a file is made of, seen from its first
     * character and not from the end of the previous one
     *
     * The scanner walks the line the way the language reads it: the brackets
     * inside a string or a comment count for nothing, the colon which opens a
     * block is the last significant character of the line and not the last
     * one, and a line whose first significant character is a `#' is a comment
     * and holds no code at all.
     *
     * The last three fields are not the scanner's doing: the scan of the file
     * fills them, since a statement is a run of lines and one line cannot say
     * on its own where the statement it belongs to begins.
     */
    struct LineFacts {
      //! the line holds nothing but blanks
      bool blank = true;
      //! the line is blanks and a `#', and holds no code
      bool comment = false;
      //! the line starts inside a string the file left open above it
      bool in_string = false;
      //! the first word of the line, empty when it holds none
      QString first_word;
      //! the last word of the line which opens a block, empty when none
      QString block_word;
      //! the indentation of the line, blanks counted and a tab worth a level
      int indent = 0;
      //! the line opens a block: its last significant character is a `:'
      bool opens_block = false;
      //! the line ends with a backslash, which calls the line below a
      //! continuation of this one
      bool backslash = false;
      //! the first non-blank character of the line, null when it holds none
      QChar first_significant;
      //! the line assigns something: it holds an `=' which is no comparison
      bool assignment = false;
      //! the line leaves a triple-quoted string open for the lines below
      bool string_opened = false;
      //! the open string is the first token of the line, or a prefixed one
      bool string_is_first = false;
      //! the place right behind the block keyword, when the line is led by one
      int block_column = 0;
      //! the line is the first line of the statement it belongs to
      bool first_of_statement = false;
      //! the indentation of the first line of the statement the line is part of
      int statement_indent = 0;
      //! the state the line hands over to the line below
      ScanState state;
    };  // end of struct LineFacts

    //! the place of the first character of a line which is not a blank
    int indentationEnd(const QString &line) {
      auto i = 0;
      while ((i < line.size()) && (line.at(i).isSpace())) {
        ++i;
      }
      return i;
    }  // end of indentationEnd()

    //! the value of a line's indentation, a tab being worth what the level is
    int indentationOf(const QString &line) {
      auto column = 0;
      for (const auto &c : line) {
        if (c == QLatin1Char(' ')) {
          ++column;
        } else if (c == QLatin1Char('\t')) {
          column += indent_offset - (column % indent_offset);
        } else {
          break;
        }
      }
      return column;
    }  // end of indentationOf()

    //! true when the character closes a bracket of the language
    bool isClosingBracket(const QChar &c) {
      return (c == QLatin1Char(')')) || (c == QLatin1Char(']')) ||
             (c == QLatin1Char('}'));
    }  // end of isClosingBracket()

    //! true when the word is one of the four which dedent the block they close
    bool isDedenter(const QString &word) {
      return (word == QLatin1String("elif")) ||
             (word == QLatin1String("else")) ||
             (word == QLatin1String("except")) ||
             (word == QLatin1String("finally"));
    }  // end of isDedenter()

    /*!
     * \brief scan one line of the file with the state the lines above left
     * \param[in] text : the text of the line
     * \param[in] incoming : the state at its first character
     * \return what the line is made of
     */
    LineFacts scanLine(const QString &text, const ScanState &incoming) {
      auto facts = LineFacts();
      facts.indent = indentationOf(text);
      facts.state = incoming;
      const auto end = text.size();
      // the place of the first character which is not a blank, which is what
      // says whether the line is led by a comment or by code, and whether the
      // string it opens is its first token or the tail of a statement
      const auto start = indentationEnd(text);
      facts.first_significant = ((start < end) ? text.at(start) : QChar());
      if (!incoming.open_quote.isNull()) {
        // the line is a line of a multi-line string: nothing it holds says
        // anything about the blocks of the code, and the string it closes is
        // still a line of the string as far as the indentation is concerned
        facts.in_string = true;
        facts.blank = false;
        auto i = start;
        while (i < end) {
          const auto c = text.at(i);
          if ((c == incoming.open_quote) && ((i + 2) < end) &&
              (text.at(i + 1) == c) && (text.at(i + 2) == c)) {
            facts.state.open_quote = QChar();
            break;
          }
          ++i;
        }
        return facts;
      }
      auto i = start;
      auto last = QChar();
      auto previous_word = QString();
      // the word a quote may belong to: at most two letters of `rRbBuUfF', and
      // only a quote right behind them makes the letters a prefix
      auto word_start = -1;
      auto word_end = -1;
      while (i < end) {
        const auto c = text.at(i);
        if (c.isSpace()) {
          ++i;
          continue;
        }
        if (c == QLatin1Char('#')) {
          // a comment runs to the end of the line, and the line is a comment
          // line only when nothing at all stood before it
          facts.comment = (i == start);
          break;
        }
        facts.blank = false;
        if ((c == QLatin1Char('\'')) || (c == QLatin1Char('"'))) {
          const auto opens_here = ((i == start) ||
                                   ((word_start == start) && (word_end == i)));
          if (((i + 2) >= end) || ((text.at(i + 1) == c) &&
                                   (text.at(i + 2) == c))) {
            // a triple-quoted string: it closes somewhere below on this line,
            // or it hands its opening over to the lines below
            auto j = ((i + 2) < end) ? i + 3 : end;
            while (j < end) {
              if ((text.at(j) == c) && ((j + 2) < end) &&
                  (text.at(j + 1) == c) && (text.at(j + 2) == c)) {
                break;
              }
              ++j;
            }
            if (j >= end) {
              facts.state.open_quote = c;
              facts.state.string_indent = facts.indent;
              facts.string_opened = true;
              facts.string_is_first = opens_here;
              last = c;
              break;
            }
            i = j + 3;
            last = c;
            continue;
          }
          // a one-line string: walked over whole, the quote of the other kind
          // included, a backslash hiding the character behind it
          auto j = i + 1;
          while (j < end) {
            if (text.at(j) == QLatin1Char('\\')) {
              j += 2;
              continue;
            }
            if (text.at(j) == c) {
              break;
            }
            ++j;
          }
          i = j + 1;
          last = c;
          continue;
        }
        if ((c == QLatin1Char('(')) || (c == QLatin1Char('[')) ||
            (c == QLatin1Char('{'))) {
          auto opener = Opener();
          opener.line_indent = facts.indent;
          opener.column_after = i + 1;
          // the line of a bracket which is followed by nothing, or by a
          // comment only, is the line its items and its closer are measured
          // from; a bracket behind it is a bracket of its own, not the item of
          // the one being opened
          auto j = i + 1;
          while ((j < end) && (text.at(j) == QLatin1Char(' '))) {
            ++j;
          }
          opener.on_its_own_line = ((j >= end) ||
                                    (text.at(j) == QLatin1Char('#')));
          facts.state.openers.append(opener);
          last = c;
          ++i;
          continue;
        }
        if (isClosingBracket(c)) {
          if (!facts.state.openers.isEmpty()) {
            facts.state.openers.removeLast();
          }
          last = c;
          ++i;
          continue;
        }
        if (c.isLetterOrNumber() || (c == QLatin1Char('_'))) {
          const auto j0 = i;
          auto j = i;
          while ((j < end) && ((text.at(j).isLetterOrNumber()) ||
                               (text.at(j) == QLatin1Char('_')))) {
            ++j;
          }
          const auto word = text.mid(j0, j - j0);
          if (facts.first_word.isEmpty()) {
            facts.first_word = word;
            if (opensBlockWord(word)) {
              // the place right behind the keyword and the blanks which follow
              // it, which is where a continuation of the line stands: the line
              // of a block which runs over a backslash is held by its keyword,
              // not by the level of the line (`python-rx block-start (* space)'
              // of the \c :after-backslash-block-continuation context)
              auto k = j;
              while ((k < end) && (text.at(k) == QLatin1Char(' '))) {
                ++k;
              }
              facts.block_column = k;
            }
          }
          if (opensBlockWord(word)) {
            // `async' goes with the word it introduces, which is then not the
            // plain word a dedenter can close: `python.el' matches the two of
            // them as one, and its table of the blocks a dedenter closes names
            // no `async for'
            facts.block_word = ((previous_word == QLatin1String("async")) &&
                                ((word == QLatin1String("def")) ||
                                 (word == QLatin1String("for")) ||
                                 (word == QLatin1String("with"))))
                                   ? QStringLiteral("async ") + word
                                   : word;
          }
          previous_word = word;
          word_start = j0;
          word_end = j;
          last = QLatin1Char('a');
          i = j;
          continue;
        }
        if (c == QLatin1Char('=')) {
          // an assignment, and neither a comparison nor one of the operators
          // which write their own left operand back into it
          const auto before = ((i > 0) ? text.at(i - 1) : QChar());
          const auto after = (((i + 1) < end) ? text.at(i + 1) : QChar());
          const auto compound = (before == QLatin1Char('+')) ||
                                (before == QLatin1Char('-')) ||
                                (before == QLatin1Char('*')) ||
                                (before == QLatin1Char('/')) ||
                                (before == QLatin1Char('%')) ||
                                (before == QLatin1Char('&')) ||
                                (before == QLatin1Char('|')) ||
                                (before == QLatin1Char('^')) ||
                                (before == QLatin1Char('<')) ||
                                (before == QLatin1Char('>')) ||
                                (before == QLatin1Char('!'));
          if ((after != QLatin1Char('=')) && (!compound)) {
            facts.assignment = true;
          }
        }
        last = c;
        ++i;
      }
      // a line which ends with a colon opens a block, when nothing stands open
      // behind the colon: the `:' of a dictionary or of a slice leaves its
      // bracket behind, and a line of a string is no code at all
      facts.opens_block = (last == QLatin1Char(':')) &&
                          (facts.state.openers.isEmpty()) && (!facts.in_string);
      facts.backslash = (last == QLatin1Char('\\'));
      return facts;
    }  // end of scanLine()

    /*!
     * \brief the indentation the presses of a key walk a line through
     *
     * These are the levels `python.el' builds and no other places, so a run of
     * presses is a walk through a list the line itself decides, and not a
     * ladder the presses would climb. Two shapes of list:
     *
     * - the line has one proper indentation, and the levels are that place and
     *   the multiples of the level which are below it, the proper place
     *   included: a line whose proper place is 6 is written at 6, then at 4,
     *   then at the margin (`python-indent--calculate-levels');
     * - the line is a dedenter, and the levels are the places of the blocks it
     *   closes, which are nowhere near a ladder (`else' of an `if' at 4 stands
     *   at 4 and at nothing between).
     */
    struct Indentation {
      //! the levels a run of presses walks the line through, ascending
      QVector<int> levels;
    };  // end of struct Indentation

    /*!
     * \brief the levels of a line whose proper indentation is the given one
     * \param[in] proper : the place the code above the line writes it at
     * \return the levels, ascending
     */
    QVector<int> ladderLevels(const int proper) {
      auto levels = QVector<int>();
      const auto bottom = qMax(proper, 0);
      for (auto l = 0; l < bottom; l += indent_offset) {
        levels.append(l);
      }
      levels.append(bottom);
      return levels;
    }  // end of ladderLevels()

    //! true when the line leaves nothing open behind it
    bool statementClosed(const LineFacts &line) {
      return (!line.backslash) && (line.state.open_quote.isNull()) &&
             (line.state.openers.isEmpty());
    }  // end of statementClosed()

    /*!
     * \brief the file up to the line the key is pressed on
     *
     * The whole file above the line is scanned, which is the only way to know
     * what a line of Python is: the brackets and the strings which reach it,
     * and the statement each of the lines above is part of, are nothing a
     * window of a few lines around the cursor can tell.
     */
    struct Scan {
      //! the lines above the target, the first line of the file first
      QVector<LineFacts> lines;
      //! the state the text leaves open at the first character of the target
      ScanState state;
      //! the target line itself
      LineFacts target;
    };  // end of struct Scan

    //! the lines above the given block, its own line, and what reaches it
    Scan scanTo(const QTextBlock &block) {
      auto scan = Scan();
      if (!block.isValid()) {
        return scan;
      }
      auto previous = LineFacts();
      auto has_previous = false;
      for (auto b = block.document()->begin(); b != block; b = b.next()) {
        auto facts = scanLine(b.text(), scan.state);
        // a statement is a run of lines: the line below a bracket, a string or
        // a backslash which are not closed belongs to the line above
        facts.first_of_statement = ((!has_previous) ||
                                    statementClosed(previous));
        facts.statement_indent = (facts.first_of_statement)
                                     ? facts.indent
                                     : previous.statement_indent;
        if (facts.string_opened) {
          // `python-info-docstring-p': the string which is opened here is the
          // docstring of the block which holds it when the statement it stands
          // in is nothing but the string — no assignment in front of it — and
          // what stands above it is the beginning of the file, a statement
          // which assigns something, or the header of a function or of a
          // class. Two docstrings in a row are allowed by that predicate and
          // left out here: what the two readings decide for the lines of the
          // second one is the same place either way
          auto k = scan.lines.size() - 1;
          while ((k >= 0) && ((scan.lines.at(k).blank) ||
                              (scan.lines.at(k).comment) ||
                              (scan.lines.at(k).in_string))) {
            --k;
          }
          auto above = QString();
          auto assigns = false;
          if (k >= 0) {
            auto s = k;
            while ((s > 0) && (!scan.lines.at(s).first_of_statement)) {
              --s;
            }
            above = scan.lines.at(s).first_word;
            for (auto m = s; m <= k; ++m) {
              assigns = assigns || scan.lines.at(m).assignment;
            }
          }
          facts.state.string_doc =
              facts.string_is_first && (!facts.assignment) &&
              ((k < 0) || assigns || (above == QLatin1String("def")) ||
               (above == QLatin1String("class")) ||
               (above == QLatin1String("async")));
        }
        scan.lines.append(facts);
        previous = facts;
        has_previous = true;
        scan.state = facts.state;
      }
      scan.target = scanLine(block.text(), scan.state);
      return scan;
    }  // end of scanTo()

    /*!
     * \brief the places the line of the given block may stand at
     * \param[in] block : the line the key is pressed on
     * \return the levels, ascending, never empty
     *
     * The order of the questions below is the order of the contexts
     * `python-indent-context' tries them in: a line of a string is asked
     * nothing else, the brackets which are open decide before the code above
     * the line can, and the colon of a block header is stronger than the word
     * the line begins with.
     */
    Indentation indentationOf(const QTextBlock &block) {
      auto result = Indentation();
      result.levels = {0};
      if (!block.isValid()) {
        return result;
      }
      const auto scan = scanTo(block);
      const auto &above = scan.lines;
      const auto &t = scan.target;
      const auto &state = scan.state;
      if (t.in_string) {
        // a line of a multi-line string: prose keeps the place it was written
        // at, since the docstring of a block is written by its author and not
        // by the code around it, and any other string stands where the line it
        // was opened on stands (`:inside-docstring', \c :inside-string)
        result.levels = ladderLevels((state.string_doc)
                                         ? qMax(t.indent, state.string_indent)
                                         : state.string_indent);
        return result;
      }
      if (!state.openers.isEmpty()) {
        const auto &top = state.openers.last();
        if (isClosingBracket(t.first_significant)) {
          // the bracket which closes the open one goes back to the line that
          // bracket was opened on, unless that line already holds an item, in
          // which case the closer stands under the item
          // (`:inside-paren-at-closing-paren')
          result.levels = ladderLevels((top.on_its_own_line)
                                           ? top.line_indent
                                           : top.column_after);
          return result;
        }
        if (top.on_its_own_line) {
          // an item of a bracket which is alone on its line stands one level
          // to the right of that line (`:inside-paren-newline-start')
          result.levels = ladderLevels(top.line_indent + indent_offset);
          return result;
        }
        // and an item of a bracket which holds one already stands right behind
        // that bracket (`:inside-paren')
        result.levels = ladderLevels(top.column_after);
        return result;
      }
      if ((!above.isEmpty()) && (above.last().backslash) && (!t.comment)) {
        // a continuation of the line above: the first line of a run of them
        // goes one level past the line it continues — or right behind its
        // block keyword, which is what a header split over a backslash keeps —
        // and the lines which follow keep the place of the line before them
        // (`:after-backslash-first-line', \c :after-backslash)
        const auto &continued = above.last();
        const auto proper =
            (continued.first_of_statement)
                ? ((continued.block_column > 0) ? continued.block_column
                                                : continued.indent +
                                                      indent_offset)
                : continued.indent;
        result.levels = ladderLevels(proper);
        return result;
      }
      // the line of code above the target, the blanks and the comment lines
      // between them included, since a comment says nothing of the block it
      // stands in the middle of
      auto code = above.size() - 1;
      while ((code >= 0) && ((above.at(code).blank) ||
                             (above.at(code).comment))) {
        --code;
      }
      if (code >= 0) {
        const auto &line = above.at(code);
        if (line.opens_block) {
          // the line is the first line of the body of the block above it, and
          // a body stands one level to the right of the header which opens it,
          // however many lines that header is written over
          // (`:after-block-start')
          result.levels = ladderLevels(line.statement_indent + indent_offset);
          return result;
        }
      }
      if (isDedenter(t.first_word)) {
        // a dedenter stands at the indentation of one of the blocks it closes.
        // Those are collected as `python-info-dedenter-opening-block-positions'
        // collects them, walking the lines above backwards over the words which
        // open a block, with two rules which are what makes the answer of a
        // nested block right: a block whose body was already left by a line
        // written at the block's own indentation closes nothing below that
        // line, and the indentation of a block found stops the search for the
        // blocks of the lines above, since those are closed by the block which
        // was just found
        const auto closers = closableBy(t.first_word);
        auto collected = QVector<int>();
        auto levels = QVector<int>();
        for (auto j = above.size() - 1; j >= 0; --j) {
          const auto &candidate = above.at(j);
          if ((candidate.in_string) || (candidate.block_word.isEmpty())) {
            continue;
          }
          const auto indent = candidate.indent;
          if (collected.contains(indent)) {
            continue;
          }
          if ((!collected.isEmpty()) &&
              (indent >= *std::min_element(collected.constBegin(),
                                           collected.constEnd()))) {
            continue;
          }
          auto open = true;
          for (auto k = j + 1; k < above.size(); ++k) {
            if ((above.at(k).blank) || (above.at(k).indent > indent)) {
              continue;
            }
            open = false;
            break;
          }
          if (!open) {
            continue;
          }
          collected.append(indent);
          if (closers.contains(candidate.block_word)) {
            levels.append(indent);
          }
          if (indent == 0) {
            break;
          }
        }
        if (!levels.isEmpty()) {
          std::sort(levels.begin(), levels.end());
          result.levels = levels;
          return result;
        }
        // nothing above closes anything this dedenter returns from: the line is
        // pinned at the margin, which is what an `else' with no `if' is written
        // at
        return result;
      }
      // the line above the target, the blanks only skipped: a comment is the
      // line a comment answers to, and it is the statement the line above
      // belongs to — which may well begin further up — which decides the place
      auto near = above.size() - 1;
      while ((near >= 0) && (above.at(near).blank)) {
        --near;
      }
      if (near < 0) {
        // the first line of the file, or a file whose lines above hold nothing
        // at all, is written at the margin and stays there (`:no-indent')
        return result;
      }
      if (above.at(near).comment) {
        // a comment is copied by the line under it, as it stands: whoever wrote
        // it placed it there, and the code under it is what has to follow
        // (`:after-comment')
        result.levels = ladderLevels(above.at(near).indent);
        return result;
      }
      auto statement = near;
      while ((statement > 0) && (!above.at(statement).first_of_statement)) {
        --statement;
      }
      if (block_end_words.contains(above.at(statement).first_word)) {
        // the line ends the block it is written in, so the line under it is the
        // first line after that block and stands one level to the left of it
        // (`:after-block-end')
        result.levels = ladderLevels(above.at(statement).indent -
                                     indent_offset);
        return result;
      }
      // and any other line hands the line under it its own place
      // (`:after-line')
      result.levels = ladderLevels(above.at(statement).indent);
      return result;
    }  // end of indentationOf()

    /*!
     * \brief the indentation a fresh press of the key gives the line
     * \param[in] indentation : the levels of the line
     * \return the indentation the first press gives
     *
     * The highest of the levels, which is the place the code above the line
     * writes the line at: `python.el' takes it for the first press and calls it
     * the maximum of the cycle, and the presses which follow walk the line down
     * through the rest of the list.
     */
    int freshIndentation(const Indentation &indentation) {
      return *std::max_element(indentation.levels.constBegin(),
                               indentation.levels.constEnd());
    }  // end of freshIndentation()

    /*!
     * \brief the indentation the next press of the key gives a line already
     * walking through its cycle
     * \param[in] indentation : the levels of the line
     * \param[in] current : the indentation the line stands at
     * \return the indentation the press gives
     *
     * The highest level below the place the line stands at, and the top of the
     * cycle once the line is written at the lowest of them
     * (`python-indent--previous-level'): that is the run of presses which takes
     * a statement out of the blocks it was written in and back into the
     * deepest one.
     */
    int nextIndentation(const Indentation &indentation, const int current) {
      auto best = -1;
      for (const auto level : indentation.levels) {
        if ((level < current) && (level > best)) {
          best = level;
        }
      }
      return (best >= 0) ? best : freshIndentation(indentation);
    }  // end of nextIndentation()

    //! the text a send handed the interpreter, as a session shows it
    QString echoed(const QString &text) {
      auto t = text;
      while (t.endsWith(QLatin1Char('\n'))) {
        t.chop(1);
      }
      return t;
    }  // end of echoed()

    /*!
     * \return true when the last line of a session ends where the interpreter is
     * ready to read a line
     *
     * The primary prompt of Python is `>>> ' and the prompt of a continuation is
     * `... ', both with a trailing space and no line break, and both are written
     * *before* the line is read: a piece of the stream which ends with one of
     * them is the place the line the buffer was asked to send belongs to. A
     * prompt with nothing behind it is a prompt nobody has answered yet.
     *
     * \note these are the prompts of CPython, which is what `python/executable`
     * names. A program of another shape — an interactive shell prompting
     * `In [1]:` for instance — is not recognised here, and the echo of a line
     * sent to it then waits behind a prompt which never comes.
     */
    bool ends_at_prompt(const QString &line) {
      auto t = line;
      while (t.endsWith(QLatin1Char(' '))) {
        t.chop(1);
      }
      return t.endsWith(QLatin1String(">>>")) ||
             t.endsWith(QLatin1String("..."));
    }  // end of ends_at_prompt()

    /*!
     * \brief write an echo at the end of a document, and open the line the
     * answer of the interpreter will be written on
     *
     * \note the line breaks are made with `insertBlock' and never with a `\n'
     * handed to `insertText': no block of a document of the editor holds a LINE
     * SEPARATOR, see `normalizeLineSeparators'
     */
    void write_echoed_text(QTextCursor &c, const QString &text) {
      const auto lines = text.split(QLatin1Char('\n'));
      for (auto i = 0; i != lines.size(); ++i) {
        if (i != 0) {
          c.insertBlock();
        }
        c.insertText(lines.at(i));
      }
      c.insertBlock();
    }  // end of write_echoed_text()

  }  // end of namespace

  // the command `start-pylsp`: a session of `pylsp` with the program, the
  // arguments and the project root the settings and the current buffer give it
  void startPylsp(EditorWidget &editor) {
    startSession(editor,
                 projectRoot(editor.getCurrentBuffer()
                                 .getMainFrame()
                                 .getCompleteFileName()));
  }  // end of startPylsp

  // the command `stop-pylsp`: end the session which is running, if any, and
  // report through the minibuffer what became of it
  void stopPylsp(EditorWidget &editor) {
    // `findChild` and not `LspClient::get`: asking for the client of an editor
    // which never launched one would build it, which is nothing a stop wants.
    // And by name: the editor widget holds the clients of the grammar checkers
    // too, and one of those is not a session of `pylsp` to stop
    auto *const client =
        editor.findChild<LspClient *>(QLatin1String("pylsp"));
    if ((client == nullptr) || (!client->stop())) {
      editor.displayInformativeMessage(
          QStringLiteral("pylsp is not started"));
      return;
    }
    editor.displayInformativeMessage(QStringLiteral("pylsp stopped"));
  }  // end of stopPylsp

  // the commands `python-enable-<tool>` and `python-disable-<tool>`, one of each
  // per linter `pylsp` can run: what reads the text of a buffer is a choice, and
  // a choice one can only make in a dialog three levels deep is a choice nobody
  // tries
  void setPythonLinter(EditorWidget &editor,
                       const char *const plugin,
                       const bool on) {
    const auto what = on
                          ? QObject::tr("Enabling %1").arg(labelOf(QLatin1String(plugin)))
                          : QObject::tr("Disabling %1")
                                .arg(labelOf(QLatin1String(plugin)));
    if (currentPythonMode(editor, what) == nullptr) {
      return;
    }
    setLinter(editor, plugin, on);
  }  // end of setPythonLinter()

  // the command `python-enable-ruff`
  void enablePythonRuff(EditorWidget &editor) {
    setPythonLinter(editor, "ruff", true);
  }  // end of enablePythonRuff

  // the command `python-disable-ruff`
  void disablePythonRuff(EditorWidget &editor) {
    setPythonLinter(editor, "ruff", false);
  }  // end of disablePythonRuff

  // the command `python-enable-pyflakes`
  void enablePythonPyflakes(EditorWidget &editor) {
    setPythonLinter(editor, "pyflakes", true);
  }  // end of enablePythonPyflakes

  // the command `python-disable-pyflakes`
  void disablePythonPyflakes(EditorWidget &editor) {
    setPythonLinter(editor, "pyflakes", false);
  }  // end of disablePythonPyflakes

  // the command `python-enable-pycodestyle`
  void enablePythonPycodestyle(EditorWidget &editor) {
    setPythonLinter(editor, "pycodestyle", true);
  }  // end of enablePythonPycodestyle

  // the command `python-disable-pycodestyle`
  void disablePythonPycodestyle(EditorWidget &editor) {
    setPythonLinter(editor, "pycodestyle", false);
  }  // end of disablePythonPycodestyle

  // the command `python-enable-flake8`
  void enablePythonFlake8(EditorWidget &editor) {
    setPythonLinter(editor, "flake8", true);
  }  // end of enablePythonFlake8

  // the command `python-disable-flake8`
  void disablePythonFlake8(EditorWidget &editor) {
    setPythonLinter(editor, "flake8", false);
  }  // end of disablePythonFlake8

  // the command `python-enable-pylint`
  void enablePythonPylint(EditorWidget &editor) {
    setPythonLinter(editor, "pylint", true);
  }  // end of enablePythonPylint

  // the command `python-disable-pylint`
  void disablePythonPylint(EditorWidget &editor) {
    setPythonLinter(editor, "pylint", false);
  }  // end of disablePythonPylint

  // the command `python-enable-mccabe`
  void enablePythonMcCabe(EditorWidget &editor) {
    setPythonLinter(editor, "mccabe", true);
  }  // end of enablePythonMcCabe

  // the command `python-disable-mccabe`
  void disablePythonMcCabe(EditorWidget &editor) {
    setPythonLinter(editor, "mccabe", false);
  }  // end of disablePythonMcCabe

  // the command `python-enable-pydocstyle`
  void enablePythonPydocstyle(EditorWidget &editor) {
    setPythonLinter(editor, "pydocstyle", true);
  }  // end of enablePythonPydocstyle

  // the command `python-disable-pydocstyle`
  void disablePythonPydocstyle(EditorWidget &editor) {
    setPythonLinter(editor, "pydocstyle", false);
  }  // end of disablePythonPydocstyle

  // the command `python-enable-mypy`: the mypy plugin of `pylsp` answers to the
  // name of `pylsp_mypy`, which is what the settings of the server hold; the
  // command is named after the program a person asks for
  void enablePythonMypy(EditorWidget &editor) {
    setPythonLinter(editor, "pylsp_mypy", true);
  }  // end of enablePythonMypy

  // the command `python-disable-mypy`
  void disablePythonMypy(EditorWidget &editor) {
    setPythonLinter(editor, "pylsp_mypy", false);
  }  // end of disablePythonMypy

  // the command `python-set-formatter`: ask in the minibuffer which program a
  // request to format a buffer is answered with, and write the answer, which is
  // what makes the session which is running answer the next question under it
  void setPythonFormatter(EditorWidget &editor) {
    if (currentPythonMode(editor, QObject::tr("Choosing the formatter")) ==
        nullptr) {
      return;
    }
    editor.setUserInput(new ToolChoiceInput(
        editor, QObject::tr("format with :"), QStringLiteral("pylsp-formatter"),
        namesOf(formatters()), QStringLiteral("pylsp/formatter/history"),
        [e = &editor](const QString &name) { setFormatter(*e, name); }));
  }  // end of setPythonFormatter

  // the command `python-set-import-sorter`: the same question, about the program
  // which sorts the imports of a buffer
  void setPythonImportSorter(EditorWidget &editor) {
    if (currentPythonMode(editor,
                          QObject::tr("Choosing the import sorter")) ==
        nullptr) {
      return;
    }
    editor.setUserInput(new ToolChoiceInput(
        editor, QObject::tr("sort imports with :"),
        QStringLiteral("pylsp-import-sorter"), namesOf(import_sorters()),
        QStringLiteral("pylsp/import-sorter/history"),
        [e = &editor](const QString &name) { setImportSorter(*e, name); }));
  }  // end of setPythonImportSorter

  // the command `python-sort-import`: sort the imports of the buffer, with the
  // program the setting names, which is the question `ruff` answers with a
  // change of its own and the `isort` plugin with a sorted text on the question
  // about formatting
  void sortPythonImports(EditorWidget &editor) {
    if (auto *const mode =
            currentPythonMode(editor, QObject::tr("Sorting the imports"))) {
      mode->sortImports();
    }
  }  // end of sortPythonImports

  // the command `start-python': an interactive interpreter in a secondary task of
  // the buffer, which is what the three keys of the session have to talk to. The
  // program is the one the configuration names and its working directory is the
  // directory of the file, so a script imports what stands beside it
  void startPythonInterpreter(EditorWidget &editor) {
    if (auto *const mode = currentPythonMode(
            editor, QObject::tr("Starting the Python interpreter"))) {
      mode->startPythonInterpreter();
    }
  }  // end of startPythonInterpreter

  // the command `python-send-line': the line the cursor stands in, to the
  // interpreter of the buffer, which is started by the send when none runs
  void sendLineToPython(EditorWidget &editor) {
    if (auto *const mode =
            currentPythonMode(editor, QObject::tr("Sending the line"))) {
      mode->sendLineToPython();
    }
  }  // end of sendLineToPython

  // the command `python-send-region': the lines of the region, as one text
  void sendRegionToPython(EditorWidget &editor) {
    if (auto *const mode =
            currentPythonMode(editor, QObject::tr("Sending the region"))) {
      mode->sendRegionToPython();
    }
  }  // end of sendRegionToPython

  // the command `python-send-buffer': the whole buffer, as one text the
  // interpreter reads from a file
  void sendBufferToPython(EditorWidget &editor) {
    if (auto *const mode =
            currentPythonMode(editor, QObject::tr("Sending the buffer"))) {
      mode->sendBufferToPython();
    }
  }  // end of sendBufferToPython

  PythonMajorMode::PythonMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
      : CompiledLanguageMajorModeBase(w, b, t) {
    // the conversation with `pylsp`: the client, the settling timers, the tracked
    // words, the diagnostics and the completion all live there, and the mode
    // keeps for itself only the answers of the session above
    this->pylsp = new PythonNavigationSession(w, b, t, this);
    // the session listens to the name of the buffer — a new file has no name when
    // the mode is built and gains it before or after, depending on how the file
    // was reached, and a save under a new name is the same event — but the first
    // hand-over is asked here, once the derived part of the session exists: it is
    // what decides whether this file starts a session of its own
    this->pylsp->openOrRenameDocument();
  }  // end of PythonMajorMode::PythonMajorMode

  QString PythonMajorMode::getName() const {
    return QStringLiteral("Python");
  }  // end of getName

  MajorMode::Category PythonMajorMode::getCategory() const {
    return Category::PROGRAMMING;
  }  // end of getCategory

  QString PythonMajorMode::getDescription() const {
    return QStringLiteral("major mode dedicated to the Python language");
  }  // end of getDescription

  QIcon PythonMajorMode::getIcon() const {
    // the freedesktop name of the icon of a Python file, with the older name as
    // the fallback: the editor has no picture of its own for this language
    auto icon = QIcon::fromTheme(QStringLiteral("text-x-python"));
    if (icon.isNull()) {
      icon = QIcon::fromTheme(QStringLiteral("application-x-python"));
    }
    return icon;
  }  // end of getIcon

  QMenu *PythonMajorMode::getSpecificMenu() {
    auto *const t = qobject_cast<QWidget *>(this->parent());
    if (t == nullptr) {
      return nullptr;
    }
    auto *const m = new QMenu(QObject::tr("Python"), t);
    m->setIcon(this->getIcon());
    auto *const lp = m->addAction(QObject::tr("Start pylsp"));
    lp->setToolTip(
        QObject::tr("Start the Python language server named by the "
                    "'pylsp/executable' setting, with the project of the "
                    "current file; a session which is running is restarted"));
    QObject::connect(lp, &QAction::triggered, this,
                     [this] { startPylsp(this->editor); });
    m->addSeparator();
    // the overview of the file is the one entry of this menu which asks a
    // question about the buffer rather than of the session; it is here because a
    // key one has never heard of is a key never pressed
    auto *const gs = m->addAction(QObject::tr("Go to symbol..."));
    gs->setToolTip(QObject::tr(
        "Ask which symbol of the file to go to, among the ones pylsp "
        "listed, and show the list as an overview (Alt-O)"));
    QObject::connect(gs, &QAction::triggered, this, [this] {
      if (this->pylsp != nullptr) {
        this->pylsp->showDocumentSymbols();
      }
    });
    m->addSeparator();
    // the interpreter is the other program this mode talks to: the language server
    // reads the file, the interpreter runs it. The key of each entry is written in
    // its tip, and the entries are here because a key one has never heard of is a
    // key never pressed
    auto *const spk = m->addAction(QObject::tr("Start interpreter"));
    spk->setIcon(QIcon::fromTheme("system-run"));
    spk->setToolTip(QObject::tr(
        "Start an interactive interpreter of the program the "
        "'python/executable' setting names, in the directory of this file; "
        "the answers it prints are shown in a secondary task of the buffer"));
    QObject::connect(spk, &QAction::triggered, this,
                     [this] { this->startPythonInterpreter(); });
    auto *const sll = m->addAction(QObject::tr("Send line to interpreter"));
    sll->setIcon(QIcon::fromTheme("system-run"));
    sll->setToolTip(QObject::tr(
        "Send the line the cursor stands in, and start the interpreter when "
        "none is running (Ctrl-C Ctrl-L)"));
    QObject::connect(sll, &QAction::triggered, this,
                     &PythonMajorMode::sendLineToPython);
    auto *const slr = m->addAction(QObject::tr("Send region to interpreter"));
    slr->setIcon(QIcon::fromTheme("system-run"));
    slr->setToolTip(QObject::tr(
        "Send the lines of the selected region as one text, which is what "
        "keeps a block which spans them together (Ctrl-C Ctrl-R)"));
    QObject::connect(slr, &QAction::triggered, this,
                     &PythonMajorMode::sendRegionToPython);
    auto *const slb = m->addAction(QObject::tr("Send buffer to interpreter"));
    slb->setIcon(QIcon::fromTheme("system-run"));
    slb->setToolTip(QObject::tr(
        "Send the whole buffer as one text, which the interpreter reads from a "
        "file it is asked to execute (Ctrl-C Ctrl-B)"));
    QObject::connect(slb, &QAction::triggered, this,
                     &PythonMajorMode::sendBufferToPython);
    m->addSeparator();
    // sorting the imports: one entry asks for the change, the submenu below says
    // who is asked, since the two sorters of this language are asked in two
    // different ways — `ruff` offers a change of a kind of its own, the `isort`
    // plugin sorts on the question about formatting and offers no change at all
    auto *const si = m->addAction(QObject::tr("Sort imports"));
    si->setToolTip(QObject::tr(
        "Sort the block of the imports of this buffer with the program the "
        "'Sort imports with' submenu names (python-sort-import)"));
    QObject::connect(si, &QAction::triggered, this,
                     [this] { this->sortImports(); });
    // the three submenus of the tools are one shape: the programs of a kind, the
    // one the settings choose ticked, and a click writing that choice and handing
    // it to the session which is running, which is what makes the underlines of
    // the buffers change under the hand which clicked
    auto add_tools_menu =
        [m](const QString &title, const QString &tip, const QVector<Tool> &tools,
            const QString &current,
            const std::function<void(const QString &)> &chosen) {
          auto *const sub = m->addMenu(title);
          sub->setToolTip(tip);
          auto *const group = new QActionGroup(sub);
          group->setExclusive(true);
          for (const auto &tool : tools) {
            const auto name = QLatin1String(tool.plugin);
            auto *const a = sub->addAction(labelOf(name));
            a->setCheckable(true);
            a->setChecked(current == name);
            a->setToolTip(QObject::tr(tool.tip));
            group->addAction(a);
            QObject::connect(a, &QAction::triggered, sub,
                             [chosen, n = QString(name)] { chosen(n); });
          }
          return sub;
        };
    // the linters are no exclusive question: several programs reading a text for
    // what is wrong with it is the normal case, and each of them sees a mistake
    // the others do not. What is ticked is what the settings say runs. No
    // `QActionGroup` is used here on purpose: the default state of one is
    // exclusive, which is the opposite of the question
    auto *const lint = m->addMenu(QObject::tr("Linters"));
    lint->setToolTip(QObject::tr(
        "Which programs pylsp reads the text of the buffers with. Several at "
        "once is the norm; what is ticked reads the buffers now, and the "
        "underlines follow the click. Each entry writes a 'pylsp/linters' "
        "setting of the name of the tool"));
    for (const auto &tool : linters()) {
      auto *const a = lint->addAction(QString::fromUtf8(tool.label));
      a->setCheckable(true);
      a->setChecked(linterIsEnabled(tool.plugin));
      a->setToolTip(QObject::tr(tool.tip));
      QObject::connect(a, &QAction::triggered, lint,
                       [this, plugin = tool.plugin](const bool on) {
                         setLinter(this->editor, plugin, on);
                       });
    }
    add_tools_menu(
        QObject::tr("Format with"),
        QObject::tr("Which program rewrites the buffer when it is asked to be "
                    "formatted. Exactly one of these runs: two formatters on at "
                    "once is one answering and the other one silent, since the "
                    "server hands the text to the first which has something to "
                    "say"),
        formatters(), chosenFormatter(),
        [this](const QString &name) { setFormatter(this->editor, name); });
    add_tools_menu(
        QObject::tr("Sort imports with"),
        QObject::tr("Which program sorts the block of the imports of a buffer. "
                    "'Isort' sorts whatever the formatter above produced, since "
                    "its plugin is a wrapper of the question about formatting"),
        import_sorters(), chosenImportSorter(),
        [this](const QString &name) {
          setImportSorter(this->editor, name);
        });
    m->addSeparator();
    auto *const sp = m->addAction(QObject::tr("Stop pylsp"));
    sp->setToolTip(QObject::tr(
        "End the session of pylsp which is running; the diagnostics "
        "it published go away with it"));
    QObject::connect(sp, &QAction::triggered, this,
                     [this] { stopPylsp(this->editor); });
    return m;
  }  // end of getSpecificMenu

  bool PythonMajorMode::keyPressEvent(QKeyEvent *const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      // a script language has no build step: running the file is what the
      // compilation command of the buffer is for, and its output is the
      // compilation output of the buffer
      this->runCompilation();
      return true;
    }
    if ((k == Qt::Key_Backtab) && (m == Qt::ShiftModifier)) {
      // `Shift-Tab' is the key which takes a line, or a region, one level out of
      // the blocks it stands in. The text edit binds it to completing the previous
      // item, and the mode is asked before the text edit sees the key: completion
      // here is `Alt-Tab', the key of the language server
      auto c = this->textEdit.textCursor();
      if (c.hasSelection()) {
        shiftRegion(c, -indent_offset);
        return true;
      }
      indentLineBackward(c);
      return true;
    }
    if ((this->pylsp != nullptr) && (this->pylsp->keyPressEvent(e))) {
      return true;
    }
    // the signature question is not asked here: the character typed reaches the
    // document only once this whole consult has returned false and the underlying
    // text edit has inserted it, so `watchSignatureHelp` hands the question to a
    // settling timer, by which time the call on screen is the call the answer is
    // about
    const auto handled = MajorModeBase::keyPressEvent(e);
    if ((!handled) && (this->pylsp != nullptr)) {
      this->pylsp->watchSignatureHelp(*e);
    }
    return handled;
  }  // end of keyPressEvent

  bool PythonMajorMode::mousePressEvent(QMouseEvent *const e) {
    if ((this->pylsp != nullptr) && (this->pylsp->mousePressEvent(e))) {
      return true;
    }
    // any other click is the plain click the text edit makes of it itself
    return MajorModeBase::mousePressEvent(e);
  }  // end of mousePressEvent

  QCompleter *PythonMajorMode::getCompleter() {
    if (this->pylsp != nullptr) {
      if (auto *const c = this->pylsp->getCompleter()) {
        return c;
      }
    }
    return MajorModeBase::getCompleter();
  }  // end of getCompleter

  int PythonMajorMode::getMinimalCompletionLength() {
    if (this->pylsp != nullptr) {
      return this->pylsp->getMinimalCompletionLength();
    }
    return MajorModeBase::getMinimalCompletionLength();
  }  // end of getMinimalCompletionLength

  void PythonMajorMode::completeCurrentWord(TextEditBase &t, const QString &w) {
    if ((this->pylsp != nullptr) &&
        (this->pylsp->completeCurrentWord(t, w))) {
      return;
    }
    // the popup of the server was not up: the completion of the mode, which is
    // the one of the base class, is the one to apply
    MajorModeBase::completeCurrentWord(t, w);
  }  // end of completeCurrentWord

  void PythonMajorMode::completeContextMenu(QMenu *const m,
                                            const QTextCursor &tc) {
    MajorModeBase::completeContextMenu(m, tc);
    if (this->pylsp != nullptr) {
      this->pylsp->completeContextMenu(m, tc);
    }
  }  // end of completeContextMenu

  void PythonMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    // `MajorMode::setHighlighter` releases the previous highlighter and lets the
    // document own the new one
    this->setHighlighter(new PythonSyntaxHighlighter(d));
  }  // end of setSyntaxHighlighter

  void PythonMajorMode::format() {
    if ((this->pylsp == nullptr) || (this->pylsp->formatDocument())) {
      return;
    }
    // the session answers false both when it changed nothing and when there was
    // nothing to ask, and only the second case is worth saying: a buffer which is
    // already formatted is the common answer of this key and a message per press
    // would make it unusable. A key which formats is never what launches a
    // server: that is a decision, and the menu and the command are where it is
    // taken
    if (this->pylsp->client().isIdle()) {
      this->pylsp->report(
          QStringLiteral("pylsp is not started: use the 'Python' menu or the "
                         "'start-pylsp' command to format this buffer"));
    }
  }  // end of format

  void PythonMajorMode::sortImports() {
    if (this->pylsp == nullptr) {
      return;
    }
    const auto sorter = chosenImportSorter();
    if (sorter == QLatin1String("none")) {
      this->pylsp->report(
          QObject::tr("no import sorter is chosen: pick one with 'Sort imports "
                      "with' of the 'Python' menu, or with the "
                      "'python-set-import-sorter' command"));
      return;
    }
    if (this->pylsp->client().isIdle()) {
      // an entry which rewrites the file is none's business to launch a server
      // for: that is a decision, and the menu and the command are where it is
      // taken. A session which is up but busy is waited for by the question
      // itself, and its answer is what the sort is made of
      this->pylsp->report(
          QStringLiteral("pylsp is not started: use the 'Python' menu or the "
                         "'start-pylsp' command to sort the imports of this "
                         "buffer"));
      return;
    }
    auto why = QString();
    // the two sorters are asked in the two different ways their plugins answer:
    // the `isort` one is a wrapper of the question about formatting and offers no
    // change of its own (measured), `ruff` offers exactly one, of the
    // `source.organizeImports` kind, and answers it among the fixes of the
    // diagnostics of the file, which is why that question picks its answer out
    const auto sorted = (sorter == QLatin1String("isort"))
                            ? this->pylsp->formatDocument(why)
                            : this->pylsp->sortImports(why);
    if (sorted) {
      return;
    }
    if (sorter == QLatin1String("isort")) {
      // what the choice is worth here: the plugin of `isort` sorts as part of
      // formatting the buffer, so the reason nothing changed is the reason of a
      // request to format, and `pylsp changed nothing` would read as though the
      // sort had been asked and refused
      why = QObject::tr("%1; Isort sorts the imports as part of formatting the "
                        "buffer")
                .arg(why);
    }
    this->pylsp->report(why);
  }  // end of sortImports

  QString PythonMajorMode::getCommentSyntax() {
    return QStringLiteral("#");
  }  // end of getCommentSyntax

  QString PythonMajorMode::getLanguageName() const {
    return this->getName();
  }  // end of getLanguageName

  QString PythonMajorMode::getDefaultCompilationCommand() const {
    // the shell the command runs in is the directory of the buffer, so the bare
    // name of the file is what reaches the interpreter; a buffer which visits no
    // file is left an unfinished command, which is what an empty name says
    const auto name =
        QFileInfo(this->textEdit.getCompleteFileName()).fileName();
    if (name.isEmpty()) {
      return QStringLiteral("python3 ");
    }
    return QStringLiteral("python3 ") + name;
  }  // end of getDefaultCompilationCommand

  // ==== Indenting with the tab key ======================================

  void PythonMajorMode::indentLine(const QTextCursor &c_) {
    if (this->textEdit.isReadOnly()) {
      this->editor.displayInformativeMessage(tr("read only buffer"));
      return;
    }
    const auto tc = QTextCursor(c_);
    auto block = tc.block();
    // `python-indent-line' tells the two questions apart by `last-command': a
    // second press of the tab key, with nothing but the blanks of the line touched
    // in between, does not ask the line again what the lines above allow it — it
    // takes the line one place further down the cycle that first answer set. Qt has
    // no `last-command', so the mode remembers the line the last press indented and
    // the place it gave it: a press which finds that line standing there continues
    // the run, and a press which finds it elsewhere — because something was typed,
    // or the formatter reached the buffer, or the cursor is on another line or in
    // another buffer — asks the line afresh
    const auto indentation = indentationOf(block);
    const auto current = indentationOf(block.text());
    const auto repeats = (this->cycle_document == tc.document()) &&
                         (this->cycle_block == block.blockNumber()) &&
                         (current == this->cycle_target);
    const auto target =
        repeats ? nextIndentation(indentation, current)
                : freshIndentation(indentation);
    // `doIndentLine' is the one place which writes the indentation. It replaces the
    // contents of the line by their trimmed text at the asked place, which takes the
    // trailing blanks away as `python-indent-line' does, and it puts the cursor on
    // the first non blank character when it stood in the leading blanks
    MajorModeBase::doIndentLine(tc, target, true);
    this->cycle_document = tc.document();
    this->cycle_block = block.blockNumber();
    this->cycle_target = target;
  }  // end of indentLine

  void PythonMajorMode::indentLineBackward(const QTextCursor &c_) {
    if (this->textEdit.isReadOnly()) {
      this->editor.displayInformativeMessage(tr("read only buffer"));
      return;
    }
    const auto tc = QTextCursor(c_);
    auto block = tc.block();
    const auto indentation = indentationOf(block);
    const auto current = indentationOf(block.text());
    auto below = -1;
    for (const auto level : indentation.levels) {
      if ((level < current) && (level > below)) {
        below = level;
      }
    }
    // the line goes to the highest level under the place it holds, and stays at the
    // lowest one of its cycle when nothing stands under that place: `Shift-Tab' means
    // out of the blocks the line was written in, so it has no top to wrap back to
    const auto target =
        (below >= 0)
            ? below
            : *std::min_element(indentation.levels.constBegin(),
                                indentation.levels.constEnd());
    MajorModeBase::doIndentLine(tc, target, true);
    // the press is remembered as the one which indented is, so that a `Tab' following
    // a `Shift-Tab' goes on down the cycle instead of jumping back to the top of it
    this->cycle_document = tc.document();
    this->cycle_block = block.blockNumber();
    this->cycle_target = target;
  }  // end of indentLineBackward

  void PythonMajorMode::shiftRegion(const QTextCursor &tc, const int offset) {
    auto b = QTextCursor();
    auto e = QTextCursor();
    beginAndEndOfSelection(b, e, tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    e.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    if (b == e) {
      // the selection sits inside a single line: shifting a run of characters of
      // it is nobody's business, the key indents the line they stand on
      indentLine(tc);
      return;
    }
    // the position of the cursor is memorized before the region moves and restored
    // once it has, so that the cursor stays next to the character it was next to
    const auto m = getCursorMark(tc);
    b.beginEditBlock();
    while ((b <= e) && (!b.atEnd())) {
      // a blank line of the region is left where it is: there is nothing in it to
      // move, and a line which holds nothing but blanks is what separates two
      // functions from each other in this language
      if (!b.block().text().trimmed().isEmpty()) {
        const auto target = qMax(indentationOf(b.block().text()) + offset, 0);
        // no setTextCursor for each line, see MajorModeBase::doIndentLine
        MajorModeBase::doIndentLine(b, target, false);
      }
      b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    b.endEditBlock();
    restoreCursorMark(m);
  }  // end of shiftRegion

  void PythonMajorMode::indentRegion(const QTextCursor &tc) {
    if (this->textEdit.isReadOnly()) {
      this->editor.displayInformativeMessage(tr("read only buffer"));
      return;
    }
    // The region goes one level of four spaces the way the line does: `Tab' right,
    // `Shift-Tab' left. What `python-indent-region' does instead is to reindent
    // every line of the region with the same calculation a lone press of the tab key
    // makes, and the effect is that a region whose lines stand where their language
    // puts them — which is the common case, a person selects a block precisely
    // because it is a block — loses the inner levels nobody asked to move.
    // `Ctrl-TAB' of this editor is bound to cycling through the buffers, so nothing
    // here is expected to do what that Emacs key does
    shiftRegion(tc, indent_offset);
  }  // end of indentRegion

  // ==== Talking to the interpreter ========================================

  namespace {

    /*!
     * \brief the text of the blocks from the one the given cursor stands on to the
     * one \a e stands on, each of them as the file writes it
     *
     * The lines are handled as text blocks, that is as the lines of the file, and
     * not as displayed lines, which depend on the wrapping policy. The text of a
     * block is asked for and not the selection which covers the line: a selection of
     * a line reaches back to the character the preceding line ends with, and that
     * character is U+2029, which no encoding turns into anything but a `?' in front
     * of the line the interpreter is handed
     */
    QStringList blockTexts(QTextCursor b, const QTextCursor &e) {
      auto lines = QStringList();
      const auto last = e.block().blockNumber();
      while (true) {
        lines << b.block().text();
        if (b.block().blockNumber() >= last) {
          break;
        }
        b.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
      }
      return lines;
    }  // end of blockTexts()

  //! the program a session runs when `python/executable' holds nothing
  //!
  //! \note the default of the entry of the configuration and the fall back of
  //! `PythonMajorMode::pythonInterpreter' are this one name, so that the field
  //! shows the program the mode would run and an answer equal to it is removed
  //! from the file instead of being stored there, as `castem/executable' does
  constexpr auto default_python_interpreter = "python3";

  }  // end of namespace

  QString PythonMajorMode::pythonInterpreter() {
    // the name of the program is read at every launch, as the Cast3M mode reads its
    // own: a person who changes the field and starts an interpreter expects the new
    // program, with no restart of the editor in between
    const auto stored = QSettings()
                            .value(QStringLiteral("python/executable"))
                            .toString()
                            .trimmed();
    if (!stored.isEmpty()) {
      return stored;
    }
    return QString::fromLatin1(default_python_interpreter);
  }  // end of pythonInterpreter

  ProcessInteractionFrame *PythonMajorMode::startPythonInterpreter() {
    if (this->python != nullptr) {
      if (this->python->getProcess().state() != QProcess::NotRunning) {
        this->report(tr("the Python interpreter is already started"));
        return this->python;
      }
      // the process of the frame is gone: the person typed `exit()', or the
      // interpreter died on what it was handed. The frame is the page of a tab and
      // goes away through the detach machinery, the same door the failure of a start
      // takes it out of
      detachPythonFrame();
      return startPythonInterpreter();
    }
    // a frame whose program never came up must leave the secondary-task machinery
    // consistently, see `CastemMajorMode::startCastem': the current task goes back
    // through `closeCurrentSecondaryTask', any other one through
    // `detachSecondaryTask', which deletes the widget once no buffer refers to it
    // anymore. A bare `close' would leave the registry holding a dangling pointer
    auto cleanup = [this] { detachPythonFrame(); };
    auto *const frame = new ProcessInteractionFrame(this->editor, this->buffer);
    frame->setAttribute(Qt::WA_DeleteOnClose);
    // the frame is deleted through `deleteLater' (`WA_DeleteOnClose'), so its
    // destruction can land after a replacement is already up: the frame is only
    // forgotten when it is still this very one
    QObject::connect(frame, &QObject::destroyed, this, [this, f = frame] {
      if (this->python == f) {
        this->python = nullptr;
      }
    });
    this->python = frame;
    auto &p = frame->getProcess();
    // a send waits for a prompt, and a prompt is written in the same piece as
    // the answer of the line before it: the arrival of a piece of the stream is
    // therefore the moment a waiting send can go out. The connection is queued
    // because the order of direct slots of one signal is the order of their
    // connections and `displayProcessOutput' reconnects itself on every piece,
    // which moves its own connection behind anything added later: a directly
    // connected judge then sees the document before the piece was appended, on
    // every piece after the first, and no send is ever placed (measured: a
    // transcript stuck at the banner while the flush ran and found no prompt).
    // Queued is correct here and not merely enough: a send waits for a prompt,
    // so no answer of the interpreter can be on its way while a flush waits to
    // run — nothing reaches the interpreter but what a flush writes
    QObject::connect(&p, &QProcess::readyReadStandardOutput, frame,
                     [this] { this->flushPythonSends(); }, Qt::QueuedConnection);
    const auto d = QDir(this->textEdit.getDirectory());
    p.setWorkingDirectory(
        (d.exists()) ? d.absolutePath() : QDir::current().absolutePath());
    const auto program = pythonInterpreter();
    // a program which is not there is not reported inside `start': the call returns
    // while the process is still `Starting' and Qt reports the failure through this
    // signal a few milliseconds later. The connection is made before the frame is
    // attached because the handler takes the frame away, and a handler which ran
    // inside `start' would leave the very next statement attaching a frame it had
    // already removed. The message names the program which did not start, which is
    // the whole report of a wrong path now that the path is what the configuration
    // of the mode says
    QObject::connect(
        &p, &QProcess::errorOccurred, this,
        [this, cleanup, program](QProcess::ProcessError e) {
          if (e == QProcess::FailedToStart) {
            this->report(QObject::tr("%1 could not be started").arg(program));
            cleanup();
          }
        });
    // `-i' and `-u' are both needed, measured over a pipe:
    // - without `-i', the interpreter reads what is written to it and exits when
    //   that is all there is: the line which started the session would be the whole
    //   of the session, and a buffer which sends a second line would speak to
    //   nothing. An error inside what is sent ends the process as well, which is not
    //   what a traceback in a session nobody asked to close is worth;
    // - without `-u', the banner and the `>>> ' prompts stay in the pipe until the
    //   interpreter decides to flush them, and the frame of the session reads as
    //   empty while the interpreter has been answering for a while.
    // The working directory is the directory of the file, so the imports of a project
    // and the name of the module beside it are what the interpreter sees, the way
    // they are when the file is run instead
    p.start(program, QStringList() << QStringLiteral("-i")
                                  << QStringLiteral("-u"));
    this->buffer.attachSecondaryTask(tr("Python Output"), frame);
    if (p.state() == QProcess::NotRunning) {
      cleanup();
      return nullptr;
    }
    return frame;
  }  // end of startPythonInterpreter

  void PythonMajorMode::detachPythonFrame() {
    if (this->python == nullptr) {
      return;
    }
    if (this->buffer.getCurrentSecondaryTask() == this->python) {
      this->buffer.closeCurrentSecondaryTask();
    } else {
      this->editor.detachSecondaryTask(&(this->buffer), this->python);
    }
    this->python = nullptr;
    // sends waiting for a prompt of a session which is gone have no place left
    // where they could be written: the next session reads its own transcript
    this->python_sends.clear();
  }  // end of detachPythonFrame

  bool PythonMajorMode::sendToPython(const QString &text,
                                     const bool whole_block) {
    // the session is asked for and not merely read: the first line sent to the
    // interpreter is what starts it, which is what the key and the entry of the menu
    // do in one gesture
    if (startPythonInterpreter() == nullptr) {
      return false;
    }
    if (this->python->getProcess().state() == QProcess::NotRunning) {
      // the process died since the frame was made, or is gone: a session of `exit()'
      // is replaced on the next press, which reaches `startPythonInterpreter' again
      this->report(tr("the Python interpreter is not running"));
      return false;
    }
    // the text is not written here: it waits for a prompt of the session, where
    // its echo can stand with it, see `flushPythonSends'
    this->python_sends.append({text, whole_block});
    this->flushPythonSends();
    return true;
  }  // end of sendToPython

  void PythonMajorMode::flushPythonSends() {
    if ((this->python == nullptr) || (this->python_sends.isEmpty())) {
      return;
    }
    auto &p = this->python->getProcess();
    if (p.state() == QProcess::NotRunning) {
      return;
    }
    auto *const doc = this->python->document();
    if ((doc == nullptr) || (!ends_at_prompt(doc->lastBlock().text()))) {
      return;
    }
    // one send per prompt: the interpreter can not answer a line before the
    // prompt that line was written to, so a send which waits behind the send
    // before it is no more out of place than a press which waits behind the
    // press before it
    const auto send = this->python_sends.takeFirst();
    // the bytes first, which is the part that can still fail: an echo is not
    // written for a text that never reaches the interpreter
    QByteArray bytes;
    if (send.whole_block) {
      // What `python-shell-send-string' does with a text of more than one line: it
      // writes it to a temporary file and asks the interpreter to read and execute
      // that file. The second form is not a matter of taste. The interactive parser
      // ends an indented block at a blank line, so a function whose body holds an
      // empty line — which is what a buffer of Python is full of — is sent as a block
      // whose body stops at that line, and the rest of it comes back as
      // `IndentationError: unexpected indent' with the function left undefined
      // (measured: the same text through a file defines the function, as text it does
      // not)
      if (this->python_script == nullptr) {
        this->python_script = new QTemporaryFile(this);
        if (!this->python_script->open()) {
          this->python_script->deleteLater();
          this->python_script = nullptr;
          this->report(
              tr("no temporary file to send the text to the interpreter"));
          return;
        }
      }
      // the file is rewritten on the flush and not on the enqueue: its name is
      // what reaches the interpreter, so a text written at the enqueue of a
      // second send would be what the exec line of the first one reads
      this->python_script->resize(0);
      this->python_script->write(send.text.toUtf8());
      this->python_script->flush();
      // the name of the file is written as a *literal* of the line the interpreter
      // is handed: `open(/tmp/xxx.py, ...)' is not a call but a syntax error — the
      // division of a name by two names — which is what `python-shell-send-file'
      // quotes for too. A raw literal because a Windows temp directory is written
      // with backslashes, and a single-quoted one because the name a
      // `QTemporaryFile` makes holds no apostrophe
      const auto path = "r'" + this->python_script->fileName() + "'";
      // the two parentheses which close `exec' and `compile' are part of what is
      // written: an answer whose parenthesis is missing is not an error the
      // interpreter prints and forgets but a line it waits for the end of, so the
      // interactive parser goes on reading in continuation mode and the next thing
      // the buffer sends is swallowed as the tail of this one (measured: the frame
      // shows `...', and every line sent after it is never answered)
      bytes = QByteArray("exec(compile(open(") + path.toUtf8() +
                        ", encoding='utf-8').read(), " + path.toUtf8() +
                        ", 'exec'))\n";
    } else {
      bytes = send.text.toUtf8() + '\n';
    }
    // the echo of the send stands in the frame before its bytes reach the
    // interpreter, which is what keeps the answer of a line from ever arriving
    // before the line itself shows up
    auto c = this->python->textCursor();
    const auto follows = c.atEnd();
    c.movePosition(QTextCursor::End);
    write_echoed_text(c, echoed(send.text));
    if (follows) {
      // a reader who moved the cursor into the scrollback is not dragged to the
      // end by what the buffer sends
      this->python->setTextCursor(c);
    }
    // the guard is `NotRunning' and not `Running': a process which was asked to
    // start is still `Starting' for a few milliseconds, and QProcess keeps what
    // is written to it then and hands it to the child as soon as it is up
    // (measured). Refusing to write in that state is what made the first line of
    // a Cast3M session drop the line which started it
    p.write(bytes);
  }  // end of flushPythonSends

  bool PythonMajorMode::sendStatement(const QString &line) {
    // a line which opens a block is not a statement the interpreter can run: it
    // waits for the body of the block, and the empty line is what the interactive
    // parser needs to be told the block is over — which is what Emacs sends too
    const auto opens = scanLine(line, ScanState()).opens_block;
    return sendToPython((opens) ? line + QLatin1Char('\n') : line, false);
  }  // end of sendStatement

  bool PythonMajorMode::sendLineToPython() {
    // the text of the block the cursor stands in, whatever the place of the cursor
    // in it and whatever the wrapping policy gives it on the screen
    return sendStatement(this->textEdit.textCursor().block().text());
  }  // end of sendLineToPython

  bool PythonMajorMode::sendRegionToPython() {
    const auto tc = this->textEdit.textCursor();
    if (!tc.hasSelection()) {
      this->report(tr("no region to send: select the lines to send first"));
      return false;
    }
    auto b = QTextCursor();
    auto e = QTextCursor();
    beginAndEndOfSelection(b, e, tc);
    b.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    const auto lines = blockTexts(b, e);
    // a region of several lines goes as a whole, which is what keeps a block which
    // spans them together: a region whose lines were sent one by one would have
    // every indented line after the first refused
    if (lines.size() == 1) {
      return sendStatement(lines.first());
    }
    return sendToPython(lines.join(QLatin1Char('\n')), true);
  }  // end of sendRegionToPython

  bool PythonMajorMode::sendBufferToPython() {
    auto lines = QStringList();
    for (auto b = this->textEdit.document()->begin(); b.isValid(); b = b.next()) {
      lines << b.text();
    }
    // the empty lines at the end of the buffer are dropped: a buffer which ends with
    // a line break has an empty block at its end, and a text whose last line is
    // empty makes the interpreter read a line it has nothing to do with
    while ((!lines.isEmpty()) && (lines.last().isEmpty())) {
      lines.removeLast();
    }
    if (lines.isEmpty()) {
      this->report(tr("the buffer holds nothing to send"));
      return false;
    }
    // the whole buffer is one text, whatever the number of its lines: a buffer whose
    // first line is a block opener is a file, and a file is what the temporary route
    // of `sendToPython' is the answer to
    return sendToPython(lines.join(QLatin1Char('\n')), true);
  }  // end of sendBufferToPython

  bool PythonMajorMode::handleShortCut(const int k1,
                                       const Qt::KeyboardModifiers m,
                                       const int k2) {
    // The three keys of the interpreter are claimed whatever the state of the
    // session, and the slots say what went wrong themselves. Claiming them only when
    // the send succeeds is the mistake the Cast3M mode was made of: the key map asks
    // this function whether the shortcut is known, and answering false on a failure
    // made the editor report `unknown shortcut : Ctrl-C - L' for a key which is
    // bound and whose interpreter was simply not started
    if ((k1 == Qt::Key_C) && (m == Qt::ControlModifier)) {
      if (k2 == Qt::Key_L) {
        return sendLineToPython();
      }
      if (k2 == Qt::Key_R) {
        return sendRegionToPython();
      }
      if (k2 == Qt::Key_B) {
        return sendBufferToPython();
      }
    }
    return false;
  }  // end of handleShortCut

  PythonMajorMode::~PythonMajorMode() = default;

  QVector<SettingsGroup> PythonMajorMode::declareSettings() {
    // the three addresses this mode reads are gathered in a theme named after the
    // program they are about, as the C++ mode does for `clangd`: they are what a
    // person looks for when the Python support does not do what they expect, and
    // a theme called `pylsp` says it without spelling the mode out. An entry which
    // names no theme belongs to the theme it is declared in, so none of them has
    // to say it twice
    auto pylsp = SettingsGroup{};
    pylsp.name = QStringLiteral("pylsp");
    pylsp.description = QObject::tr(
        "What the Python mode asks the language server, what it asks it "
        "with, and which programs that server runs over the text of a "
        "buffer. A session reads the program, its arguments and the "
        "project of the file it is started for when it is started; the "
        "tools it runs are handed to it at that moment too, and the "
        "entries of the `Python` menu which choose them apply the choice "
        "to the session which is running at once");
    pylsp.keys =
        QVector<SettingsKey>()
        << SettingsKey::text(
               "pylsp/executable",
               QObject::tr("Program of the language server"),
               QObject::tr("The `pylsp` to start — a bare name is the one a "
                           "terminal would find in the directories of the "
                           "session, and a full path the program of a virtual "
                           "environment, where the plugins which make the "
                           "diagnostics of a file worth reading are "
                           "installed. Left empty, the editor looks for "
                           "`pylsp` itself"))
        << SettingsKey::stringList(
               "pylsp/arguments", QObject::tr("Arguments passed to pylsp"),
               QObject::tr("What is appended to the arguments the client "
                           "needs. In the configuration file the entries of "
                           "the list are separated by a comma"))
        << SettingsKey::boolean(
               "pylsp/start-by-default",
               QObject::tr("Start pylsp when a Python file is opened"),
               QObject::tr("When this is on, a buffer which visits a "
                           "Python file starts a session for it, and the "
                           "minibuffer names the project that session "
                           "reads: unlike a C++ server, `pylsp` needs "
                           "nothing beyond the text of the file to say "
                           "what a name is or where it is defined, so a "
                           "session costs the time it takes to boot and "
                           "buys the underlines of the file. Where "
                           "nothing is to be found to start, nothing is "
                           "started and it is said once. The `Start "
                           "pylsp` entry of the `Python` menu is where "
                           "a session is asked for when this is off, "
                           "and the buffers which are already open keep "
                           "the session they were opened with"),
               true);
    // The tools the server runs are entries of the same theme: which program
    // reads the text, which rewrites it and which puts its imports in order is
    // part of what a person looks for when the Python support does not do what
    // they expect, and it is the same decision as which server to start
    auto keys = pylsp.keys;
    for (const auto &t : linters()) {
      keys << SettingsKey::boolean(
          linterAddress(t.plugin),
          QObject::tr("%1 reads the buffers").arg(QString::fromUtf8(t.label)),
          QObject::tr("%1. This is what the server runs to draw the underlines "
                      "of a buffer, and several of these at once is the norm, "
                      "since each sees a mistake the others do not. The "
                      "`Linters` submenu of the `Python` menu writes this same "
                      "address and the buffers which are open follow it at once"),
          t.enabled_by_default);
    }
    keys << SettingsKey::choice(
               "pylsp/formatter",
               QObject::tr("Program which formats a buffer"),
               QObject::tr("Which program a request to format a buffer is "
                           "answered with. Exactly one of these runs: every "
                           "formatter of `pylsp` answers the same question and "
                           "the server keeps the first answer which holds "
                           "something, so two of these on at once is one "
                           "answering and the other silent. `None` answers a "
                           "request to format with nothing, which is what "
                           "leaves the formatting of the file to a program "
                           "outside the editor. The `Format with` submenu of "
                           "the `Python` menu writes this same address, and "
                           "`Autopep8` is the one of these which leaves the "
                           "quotes of a file where they are"),
               choicesOf(formatters()), QStringLiteral("ruff"))
         << SettingsKey::choice(
                "pylsp/import-sorter",
                QObject::tr("Program which sorts the imports of a buffer"),
                QObject::tr("Which program the `Sort imports` entry and the "
                            "`python-sort-import` command ask. `Ruff` needs "
                            "the `I` rules, which the editor selects for it "
                            "while this names it, since a Ruff which does not "
                            "see the order of the imports as a problem offers "
                            "no change sorting them; `Isort` is a wrapper of "
                            "the question about formatting, so it sorts "
                            "whatever the formatter above produced and the "
                            "entry asking for a sort asks for a reformat with "
                            "it. `None` leaves that entry saying that no "
                            "program is chosen. The `Sort imports with` submenu "
                            "of the `Python` menu writes this same address"),
                choicesOf(import_sorters()), QStringLiteral("ruff"));
    pylsp.keys = keys;
    // The interpreter is the other program of the mode and it gets its own theme:
    // the entries of the `pylsp` theme are about the server which reads the file,
    // this one is about the program which runs it, and a person looking for the
    // release their scripts must run against has that release's name in mind, not
    // the language server's
    auto python = SettingsGroup{};
    python.name = QStringLiteral("python");
    python.description = QObject::tr(
        "The program an interactive session of the Python mode runs. The line, "
        "the region and the buffer are handed to it, and everything it answers "
        "is shown in a secondary task of the buffer");
    python.keys = QVector<SettingsKey>() << SettingsKey::text(
                                                "python/executable",
                                                QObject::tr("Program of the "
                                                            "interpreter"),
                                                QObject::tr(
                                                    "The program a session of "
                                                    "the Python mode starts — a "
                                                    "bare name is the one a "
                                                    "terminal would find in the "
                                                    "directories of the session, "
                                                    "a full path the interpreter "
                                                    "of a virtual environment, "
                                                    "where the modules the "
                                                    "buffer imports are "
                                                    "installed. Left empty, "
                                                    "`python3` is run. The "
                                                    "address is read on every "
                                                    "launch, so a session asked "
                                                    "for after the field is "
                                                    "changed runs the program it "
                                                    "names with no restart of "
                                                    "the editor; the session "
                                                    "starts in the directory of "
                                                    "the file it was asked "
                                                    "from"),
                                                QString::fromLatin1(
                                                    default_python_interpreter));
    // nothing else: what the mode remembers of the commands it was asked to run
    // is a history, and a history is not an entry of the configuration, see
    // `CoreSettings.cxx`
    return QVector<SettingsGroup>() << pylsp << python;
  }  // end of declareSettings

  static StandardFunctionCommandProxy<startPythonInterpreter>
      startPythonInterpreterProxy("start-python");

  static StandardFunctionCommandProxy<sendLineToPython> sendLineToPythonProxy(
      "python-send-line");

  static StandardFunctionCommandProxy<sendRegionToPython>
      sendRegionToPythonProxy("python-send-region");

  static StandardFunctionCommandProxy<sendBufferToPython>
      sendBufferToPythonProxy("python-send-buffer");

  static StandardFunctionCommandProxy<startPylsp> startPylspProxy(
      "start-pylsp");

  static StandardFunctionCommandProxy<stopPylsp> stopPylspProxy("stop-pylsp");

  // The commands of the tools of the language: `python-sort-import` asks for a
  // sorting, `python-set-formatter` and `python-set-import-sorter` ask which
  // program does it, and the sixteen linter commands switch one program of the
  // list `pylsp` reads. They are written out, one line each, because the name a
  // person types in the minibuffer is what has to be found with a search, in the
  // documentation and in the tests, and not assembled from a table
  static StandardFunctionCommandProxy<sortPythonImports> sortImportsProxy(
      "python-sort-import");

  static StandardFunctionCommandProxy<setPythonFormatter> setFormatterProxy(
      "python-set-formatter");

  static StandardFunctionCommandProxy<setPythonImportSorter>
      setImportSorterProxy("python-set-import-sorter");

  static StandardFunctionCommandProxy<enablePythonRuff> enableRuffProxy(
      "python-enable-ruff");

  static StandardFunctionCommandProxy<disablePythonRuff> disableRuffProxy(
      "python-disable-ruff");

  static StandardFunctionCommandProxy<enablePythonPyflakes>
      enablePyflakesProxy("python-enable-pyflakes");

  static StandardFunctionCommandProxy<disablePythonPyflakes>
      disablePyflakesProxy("python-disable-pyflakes");

  static StandardFunctionCommandProxy<enablePythonPycodestyle>
      enablePycodestyleProxy("python-enable-pycodestyle");

  static StandardFunctionCommandProxy<disablePythonPycodestyle>
      disablePycodestyleProxy("python-disable-pycodestyle");

  static StandardFunctionCommandProxy<enablePythonFlake8> enableFlake8Proxy(
      "python-enable-flake8");

  static StandardFunctionCommandProxy<disablePythonFlake8> disableFlake8Proxy(
      "python-disable-flake8");

  static StandardFunctionCommandProxy<enablePythonPylint> enablePylintProxy(
      "python-enable-pylint");

  static StandardFunctionCommandProxy<disablePythonPylint> disablePylintProxy(
      "python-disable-pylint");

  static StandardFunctionCommandProxy<enablePythonMcCabe> enableMcCabeProxy(
      "python-enable-mccabe");

  static StandardFunctionCommandProxy<disablePythonMcCabe> disableMcCabeProxy(
      "python-disable-mccabe");

  static StandardFunctionCommandProxy<enablePythonPydocstyle>
      enablePydocstyleProxy("python-enable-pydocstyle");

  static StandardFunctionCommandProxy<disablePythonPydocstyle>
      disablePydocstyleProxy("python-disable-pydocstyle");

  static StandardFunctionCommandProxy<enablePythonMypy> enableMypyProxy(
      "python-enable-mypy");

  static StandardFunctionCommandProxy<disablePythonMypy> disableMypyProxy(
      "python-disable-mypy");

  static StandardMajorModeProxy<PythonMajorMode> proxy(
      "Python",
      QVector<QRegularExpression>()
          << suffixRegExp("py") << suffixRegExp("pyi") << suffixRegExp("pyx"),
      "text-x-python");

}  // end of namespace tfel::gui
