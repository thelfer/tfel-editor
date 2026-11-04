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
#include <QtWidgets/QListView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtCore/QStringListModel>
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
    // a session which is still running is replaced: what it was started with,
    // the compilation database above all, can not be changed under a running
    // server, so it can not be changed by asking it either
    void startClangd(EditorWidget &w,
                     const QStringList &arguments,
                     const QString &description) {
      auto &client = LspClient::get(w);
      const auto restarting = (!client.isIdle());
      auto error = QString();
      if (!client.launch(arguments, error)) {
        w.displayInformativeMessage(error);
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

    // the extra arguments of a session and what to say about them in the
    // minibuffer once it is up
    struct ImplicitCompileCommands {
      QStringList arguments;
      QString description;
    };  // end of struct ImplicitCompileCommands

    // the database a session launched without an explicit one uses: the file
    // `tfel-editor` was started with, when `--compile-commands-file` named one
    // which exists, else the one of the project of the current buffer, else
    // none, which is a plain session leaving the search to the server
    //
    // the editor performs the search itself, rather than leaving it to the
    // server, because the option which hands a database to clangd is set when
    // the session starts and the user is told which database it was started
    // with, which requires knowing it before launching
    ImplicitCompileCommands implicitCompileCommands(EditorWidget &w) {
      const auto file = LspCxxSupport::defaultCompileCommandsFile();
      if (!file.isEmpty()) {
        auto error = QString();
        const auto arguments =
            LspCxxSupport::compileCommandsArguments(file, error);
        if (error.isEmpty()) {
          return {arguments, QStringLiteral("with the compilation database "
                                            "tfel-editor was started with: %1")
                                 .arg(file)};
        }
        // a default which does not exist is a mistake the user made once, at
        // the command line; it does not void the launch, the project is asked
        // instead and the mistake is only reported on the way
        w.displayInformativeMessage(
            QStringLiteral("%1: searching the project instead").arg(error));
      }
      const auto directory = currentFileDirectory(w);
      const auto found = LspCxxSupport::searchCompileCommandsFile(
          directory.isEmpty() ? QDir::currentPath() : directory);
      if (found.isEmpty()) {
        return {{}, QStringLiteral("without a compilation database")};
      }
      auto error = QString();
      return {LspCxxSupport::compileCommandsArguments(found, error),
              QStringLiteral("with the compilation database of the project: %1")
                  .arg(found)};
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
    startClangd(editor, choice.arguments, choice.description);
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

  CxxMajorMode::CxxMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
      : CMajorModeBase(w, b, t) {
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
      // '?' is a shifted key: the platform puts ShiftModifier alongside
      // AltModifier, as the Alt-< and Alt-> bindings of TextEditBase learned
      // the hard way
      if ((m & Qt::AltModifier) && !(m & Qt::ControlModifier) &&
          (k == Qt::Key_Question)) {
        this->showHover();
        return true;
      }
    }
    return CMajorModeBase::keyPressEvent(e);
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
    const auto f = this->textEdit.getCompleteFileName();
    if (f == this->documentFile) {
      if ((!f.isEmpty()) && (!this->client->isDocumentOpen(f))) {
        // the client is a fresh one since this mode last opened the file
        this->document = this->textEdit.document();
        this->client->openDocument(f, this->document);
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
      this->applyDiagnostics();
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
    // answer is kept, so triggering the entry jumps without asking again
    auto locations = std::make_shared<QVector<LspLocation>>();
    const auto in_time = LspProtocol::boundedWait(
        [this, &tc, locations](const std::function<void()> &stop) {
          this->client->requestDefinition(
              this->documentFile, *this->document, tc.position(),
              [locations, stop](const QVector<LspLocation> &l) {
                *locations = l;
                stop();
              });
        },
        1000);
    if ((!in_time) || (locations->isEmpty())) {
      return;
    }
    // a symbol defined in several places, a virtual override for instance,
    // gets one entry each: the file and the line, in the numbering a buffer
    // shows, are what tells them apart. Five is where a menu which goes on
    // listing them stops being a menu
    const auto entries = qMin(locations->size(), 5);
    const auto old = m->actions();
    for (auto i = entries - 1; i >= 0; --i) {
      const auto location = locations->at(i);
      auto text = QObject::tr("Go to definition");
      if (entries > 1) {
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
      if (old.isEmpty()) {
        m->addAction(a);
      } else {
        m->insertAction(*(old.begin()), a);
      }
    }
    if (!old.isEmpty()) {
      m->insertSeparator(*(old.begin()));
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

  static StandardFunctionCommandProxy<launchClangd> launchClangdProxy(
      "launch-clangd");

  static StandardFunctionCommandProxy<launchClangdWithCompileCommands>
      launchClangdWithCompileCommandsProxy(
          "launch-clangd-with-compile-commands");

  static StandardMajorModeProxy<CxxMajorMode> proxy(
      "C++",
      QVector<QRegularExpression>()
          << suffixRegExp("cxx") << suffixRegExp("hxx") << suffixRegExp("ixx")
          << suffixRegExp("cpp") << suffixRegExp("hpp") << suffixRegExp("ipp")
          << suffixRegExp("CC") << suffixRegExp("hh") << suffixRegExp("c")
          << suffixRegExp("h") << suffixRegExp("cu"),
      ":/tfel/editor/languages/cxx.png");

}  // end of namespace tfel::gui
