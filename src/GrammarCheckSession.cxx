/*!
 * \file   GrammarCheckSession.cxx
 * \brief  the proofreader of a text edit, whichever of them that is, the lints
 *         it draws under the text, and the choice which decides between them.
 * \author Thomas Helfer
 * \date   5 nov. 2026
 */

#include <QtCore/QEvent>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCursor>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QMenu>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolTip>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/GrammarCheckSession.hxx"
#include "TFEL/GUI/LanguageToolClient.hxx"
#include "TFEL/GUI/LanguageToolSupport.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {

    // the face of a lint: a plain underline in red, the spelling checker's own
    // look, and one face for every lint. The severity of the protocol lives in
    // the color of the diagnostics of `clangd` because that server sorts what it
    // finds by it; both proofreaders answer all of their lints with a severity
    // which says nothing — `harper-ls` calls every one of them a hint, Language
    // Tool names the category of its rules and no severity — so their whole
    // output is to be seen, and only the words they say something about are
    // inked. A wave underline would be the look of the hunspell underlines:
    // `QPlainTextEdit` does not ink one through the extra selections, see
    // `CxxMajorMode::applyDiagnostics`
    QTextCharFormat lintFace() {
      auto format = QTextCharFormat();
      format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
      format.setUnderlineColor(QColor(200, 0, 0));
      format.setFontUnderline(true);
      return format;
    }  // end of lintFace()

    // the underline of a lint is a range of the text the checker saw, which the
    // buffer may not hold anymore line for line; `positionOf` clamps a line the
    // document has not to the last block of the file, which would draw a lint
    // of a paragraph long gone over the last line of the buffer, so both ends
    // are asked for blocks which exist
    bool blockExists(const QTextDocument &doc, const int line) {
      return doc.findBlockByNumber(line).isValid();
    }  // end of blockExists()

  }  // end of anonymous namespace

  GrammarCheckSession::GrammarCheckSession(EditorWidget &w,
                                           TextEditBase &t,
                                           const QString &language_id,
                                           const bool whole_text_is_prose,
                                           QObject *const p)
      : QObject(p),
        editor(&w),
        text_edit(&t),
        language_id(language_id),
        whole_text_is_prose(whole_text_is_prose) {
    // `checker` is left at the value it is declared with, which names no grammar
    // checker: the choice held in the settings is asked of at the end of this
    // constructor, and that answer decides both which backend reads the buffer
    // and that the text of the buffer is handed to it — which is why the
    // document is registered before, while no backend holds it yet
    //
    // the buffer of a new file has no name when the mode is built, and gains it
    // before or after, depending on how the file was reached; the same holds for
    // a save under a new name
    QObject::connect(&t,
                     &TextEditBase::fileNameChanged,
                     this,
                     [this](const QString &) { this->openOrRenameDocument(); });
    if (auto *const scroll_area = t.widget()) {
      this->area = scroll_area;
      scroll_area->viewport()->installEventFilter(this);
    }
    // the document is registered whether a checker runs or not: a buffer gains
    // its file name after the mode was built, and a checker chosen afterwards
    // asks for the text of the buffer it is built on
    this->openOrRenameDocument();
    // the choice held in the settings decides now: a buffer opened while the
    // editor is set to check its prose is checked from the moment it is on the
    // screen, and says nothing about it, the way the buffers opened before it
    // were. The language of the spell checker is asked of the text edit, which
    // is a value stored at this point — the mode under which this session is
    // built is handed that value as soon as its constructor is over, see
    // `TextEditBase::setMajorMode`
    this->followChecker(false);
  }  // end of GrammarCheckSession::GrammarCheckSession

  GrammarCheckSession::~GrammarCheckSession() {
    // the backends report through signals: nobody may answer them by drawing on
    // a text edit which is dying with the mode, see the `area` member. The
    // shared client of LanguageTool outlives this session, so what it is told is
    // that this buffer reads it no longer
    if (this->harper != nullptr) {
      QObject::disconnect(this->harper, nullptr, this, nullptr);
    }
    if (this->language_tool != nullptr) {
      QObject::disconnect(this->language_tool, nullptr, this, nullptr);
    }
    if (!this->document_file.isEmpty()) {
      if (this->harper != nullptr) {
        this->harper->closeDocument(this->document_file, this->document);
      }
      if (this->language_tool != nullptr) {
        this->language_tool->closeDocument(this->document_file,
                                           this->document);
      }
    }
    if ((this->area != nullptr) && (this->text_edit != nullptr)) {
      // a mode which is replaced leaves its text edit behind, and the lints of
      // the checker it had with it, which are the keyed contribution this
      // session wrote and nobody else will clear: the backend of `harper-ls` is
      // a child of this session, so it is gone only after this body ran and
      // still holds the answer it published. Asking `applyDiagnostics` to redraw
      // it would put the underlines of a dead session back on the screen; handing
      // the key an empty list whole is what takes them off. A text edit which is
      // destroyed along with its mode needs none of this, and `area` is null by
      // then
      this->text_edit->setExtraSelections("grammar", {});
    }
  }  // end of GrammarCheckSession::~GrammarCheckSession

  bool GrammarCheckSession::isAvailable() {
    return (!LspHarperSupport::serverExecutable().isEmpty()) ||
           LanguageToolClient::isAvailable();
  }  // end of GrammarCheckSession::isAvailable

  bool GrammarCheckSession::isActive() const {
    return SpellingAndGrammar::usesHarper(this->checker_in_use) ||
           SpellingAndGrammar::usesLanguageTool(this->checker_in_use);
  }  // end of GrammarCheckSession::isActive

  SpellingAndGrammar::Checker GrammarCheckSession::checker() const {
    return this->checker_in_use;
  }  // end of GrammarCheckSession::checker

  bool GrammarCheckSession::readsWholeTextAsProse() const {
    return this->whole_text_is_prose;
  }  // end of GrammarCheckSession::readsWholeTextAsProse

  bool GrammarCheckSession::usesHarper() const {
    return SpellingAndGrammar::usesHarper(this->checker_in_use);
  }

  bool GrammarCheckSession::usesLanguageTool() const {
    return SpellingAndGrammar::usesLanguageTool(this->checker_in_use);
  }

  void GrammarCheckSession::followChecker(const bool announced) {
    const auto wanted =
        SpellingAndGrammar::effectiveChecker(this->whole_text_is_prose);
    const auto changed = wanted != this->checker_in_use;
    if (changed) {
      // the underlines of the checker which is left are none of the one which
      // comes: they go, and the answers of the new one take their place
      this->releaseDocument();
      // and so does the process of a checker which has nothing left to read: a
      // person who chooses another proofreader in a menu means it, and a
      // `harper-ls` which holds no document has no reason to stay started. The
      // shared LanguageTool server is ended by the same release above, which is
      // what spends the last use of it
      this->stopChecker();
      this->checker_in_use = wanted;
    }
    auto error = QString();
    auto started = true;
    if (this->usesHarper()) {
      started = this->startHarper(error);
    } else if (this->usesLanguageTool()) {
      started = this->startLanguageTool(error);
    }
    if (changed) {
      // the text of the buffer goes to the checker which is in use now, and the
      // answers of the one which is gone are spent with its underlines
      this->openDocumentOnChecker();
    } else if (started && this->isActive()) {
      // the same checker under a new language or a new set of rules: the request
      // reads its settings when it is built, so the texts have to go over again
      // for the answer to be of any use to a person looking at the old one
      this->recheckChecker();
    }
    // what becomes of the spell checker is decided by the same choice: a
    // proofreader which reads the whole text wants no dictionary beside it, and
    // an empty language is the one way a mode has of taking its underlines off
    this->text_edit->setSpellCheckLanguage(
        SpellingAndGrammar::spellCheckLanguage(this->checker_in_use));
    if (announced) {
      this->editor->displayInformativeMessage(started ? this->reportOf()
                                                      : error);
    }
    emit this->stateChanged();
  }  // end of GrammarCheckSession::followChecker

  void GrammarCheckSession::addToMenu(QMenu &menu) {
    // the four choices are one question asked of every buffer of the editor, so
    // the menu of a mode offers them as the menu of `Options` does; what the
    // mode says is for which kind of buffer that menu stands, which is the one
    // thing the choice itself cannot tell: in a source file, LanguageTool reads
    // no grammar and the entry has to say so
    SpellingAndGrammar::addToMenu(menu, *this->editor, this->whole_text_is_prose);
  }  // end of GrammarCheckSession::addToMenu

  bool GrammarCheckSession::eventFilter(QObject *const watched,
                                        QEvent *const event) {
    if ((event->type() == QEvent::ToolTip) && (!this->document.isNull()) &&
        (!this->document_file.isEmpty())) {
      // resting the mouse on an underlined run says what the checker said about
      // it: the event comes contentless on this Qt, so the position is the
      // mouse's own, which has been resting where the event fires, see
      // `CxxMajorMode::eventFilter`
      const auto global = QCursor::pos();
      const auto pos =
          this->text_edit
              ->cursorForPosition(
                  static_cast<QWidget *>(watched)->mapFromGlobal(global))
              .position();
      const auto text = this->diagnosticAt(*this->document, pos);
      if (!text.isEmpty()) {
        QToolTip::showText(global, text, this->text_edit);
        return true;
      }
    }
    return QObject::eventFilter(watched, event);
  }  // end of GrammarCheckSession::eventFilter

  bool GrammarCheckSession::startHarper(QString &error) {
    const auto program = LspHarperSupport::serverExecutable();
    if (program.isEmpty()) {
      error = QObject::tr("harper-ls was not found: install it or set the "
                          "'harper/executable' option");
      return false;
    }
    if (this->harper == nullptr) {
      // a client of its own, and not the one of the editor widget: that one
      // speaks with `clangd`, which a buffer of Markdown has no use for, and the
      // documents of the two kinds of server are not to be sent to each other
      this->harper = new LspClient(this->editor, this);
      this->harper->setServerName(QStringLiteral("harper-ls"));
      this->harper->setLanguageId(this->language_id);
      // the checker is silent about how it wants its documents and reads the text
      // it is given as the text of the document; the range of a change would
      // corrupt the copy it works on
      this->harper->setFullTextSync(true);
      this->harper->setConfiguration(LspHarperSupport::configuration());
      QObject::connect(this->harper,
                       &LspClient::diagnosticsUpdated,
                       this,
                       &GrammarCheckSession::updateDiagnostics);
      QObject::connect(this->harper, &LspClient::stopped, this, [this] {
        // a checker which exited of its own accord reads nothing anymore, and
        // the underlines of a dead server are none
        this->applyDiagnostics();
      });
    }
    if (this->harper->isReady()) {
      return true;
    }
    if (!this->harper->launch(program, LspHarperSupport::serverArguments(),
                              error)) {
      return false;
    }
    // what the process was started with is what `recheckChecker` compares the
    // setting to: a program which changed in the dialog is none of what the
    // server which runs reads
    this->harper_program = program;
    return true;
  }  // end of GrammarCheckSession::startHarper

  bool GrammarCheckSession::startLanguageTool(QString &error) {
    // the client is the one of the editor widget, and the reason it is shared is
    // the size of what it starts: a Java virtual machine with the rules of every
    // language the program knows, which costs a second to boot and a hundred
    // megabytes to sit there, and two buffers of one editor reading two files of
    // one project have nothing to gain from two of them
    this->language_tool = &LanguageToolClient::get(*this->editor);
    QObject::connect(this->language_tool,
                     &LanguageToolClient::diagnosticsUpdated,
                     this,
                     &GrammarCheckSession::updateDiagnostics,
                     Qt::UniqueConnection);
    QObject::connect(this->language_tool,
                     &LanguageToolClient::stopped,
                     this,
                     [this] { this->applyDiagnostics(); },
                     Qt::UniqueConnection);
    // the server is started by `openDocument`, which is where a text to read is
    // known: a checker chosen for a buffer which visits no file has nothing to
    // hand over and starts nothing
    auto unused = QString();
    if (this->document_file.isEmpty()) {
      return LanguageToolClient::isAvailable();
    }
    if (!LanguageToolSupport::isAvailable()) {
      error = QObject::tr("LanguageTool is not configured: set the installation "
                          "directory in Options > Configuration > LanguageTool");
      return false;
    }
    return this->language_tool->startSession(unused) ||
           LanguageToolClient::isAvailable();
  }  // end of GrammarCheckSession::startLanguageTool

  void GrammarCheckSession::stopChecker() {
    // the documents are released by `releaseDocument`, which is what ends the
    // shared server when this buffer was its last user
    if (this->harper != nullptr) {
      this->harper->stop();
    }
  }  // end of GrammarCheckSession::stopChecker

  void GrammarCheckSession::recheckChecker() {
    if (this->harper != nullptr) {
      if (this->harper_program != LspHarperSupport::serverExecutable()) {
        // the program of the checker is not the one which runs: the server which
        // answers is the one the person left behind, so it is ended and the one
        // which is named now is started. The dialect and the documents go with
        // it, since `launch` ends the session, spends the versions of the
        // documents and reads them again with its handshake
        this->harper->stop();
        auto error = QString();
        if (!this->startHarper(error) && !error.isEmpty()) {
          this->editor->displayInformativeMessage(error);
        }
        return;
      }
      // the settings the client answers with are what the next question of the
      // server reads; the documents have to be handed over again for the new
      // dialect to be of any use to the buffer on screen
      this->harper->setConfiguration(LspHarperSupport::configuration());
      this->harper->recheckDocuments();
    }
    if (this->language_tool != nullptr) {
      this->language_tool->recheckDocuments();
    }
  }  // end of GrammarCheckSession::recheckChecker

  void GrammarCheckSession::releaseDocument() {
    if (this->document_file.isEmpty()) {
      return;
    }
    // both backends are told, and not the one in use only: the client of
    // `harper-ls` is kept because the choice may come back to it, and a document
    // it still reads is a process it keeps running and underlines it keeps
    // drawing on a text edit which shows the answer of another checker
    if (this->harper != nullptr) {
      this->harper->closeDocument(this->document_file, this->document);
    }
    if (this->language_tool != nullptr) {
      this->language_tool->closeDocument(this->document_file, this->document);
    }
    this->applyDiagnostics();
  }  // end of GrammarCheckSession::releaseDocument

  void GrammarCheckSession::openDocumentOnChecker() {
    if (this->document_file.isEmpty()) {
      // a buffer which visits no file has no name to be read under; the checker
      // says nothing of it until the buffer is saved, which is also what the
      // diagnostics of `clangd` wait for
      return;
    }
    this->document = this->text_edit->document();
    if (this->harper != nullptr) {
      this->harper->openDocument(this->document_file, this->document);
    }
    if (this->language_tool != nullptr) {
      // the frame is given with the document: the mode of that frame is who says
      // what of its text a proofreader has any business reading
      this->language_tool->openDocument(this->document_file, this->document,
                                        this->text_edit);
    }
  }  // end of GrammarCheckSession::openDocumentOnChecker

  void GrammarCheckSession::updateDiagnostics(const QString &file) {
    if (file == this->document_file) {
      this->applyDiagnostics();
    }
  }  // end of GrammarCheckSession::updateDiagnostics

  void GrammarCheckSession::openOrRenameDocument() {
    const auto f = this->text_edit->getCompleteFileName();
    if (f == this->document_file) {
      if ((!f.isEmpty()) && (!this->openedOnChecker())) {
        // the buffer had its name before a checker was there to read it: the mode
        // of a new file is built before the file is visited, and a backend is
        // created when a choice names one, which may be after
        this->openDocumentOnChecker();
      }
      return;
    }
    this->releaseDocument();
    this->document_file = f;
    if (f.isEmpty()) {
      return;
    }
    this->openDocumentOnChecker();
  }  // end of GrammarCheckSession::openOrRenameDocument

  bool GrammarCheckSession::openedOnChecker() const {
    if (this->document_file.isEmpty()) {
      return false;
    }
    if (this->usesHarper()) {
      return (this->harper != nullptr) &&
             this->harper->isDocumentOpen(this->document_file);
    }
    if (this->usesLanguageTool()) {
      return (this->language_tool != nullptr) &&
             this->language_tool->isDocumentOpen(this->document_file);
    }
    return false;
  }  // end of GrammarCheckSession::openedOnChecker

  QVector<LspDiagnostic> GrammarCheckSession::diagnostics() const {
    if (this->document_file.isEmpty()) {
      return {};
    }
    if ((this->harper != nullptr) && this->usesHarper()) {
      return this->harper->diagnosticsFor(this->document_file);
    }
    if ((this->language_tool != nullptr) && this->usesLanguageTool()) {
      return this->language_tool->diagnosticsFor(this->document_file);
    }
    return {};
  }  // end of GrammarCheckSession::diagnostics

  QString GrammarCheckSession::reportOf() const {
    if (this->usesHarper()) {
      // the dialect is named because it is the one thing the answer changes, and
      // a person who reads a buffer of British English wants to see which
      // English the underlines were drawn against
      return QObject::tr("grammar checking with %1, reading the text as %2")
          .arg(LspHarperSupport::serverExecutable(),
               LspHarperSupport::dialectLabel(LspHarperSupport::dialect()));
    }
    if (this->usesLanguageTool()) {
      if (!this->whole_text_is_prose) {
        // the choice was made for the prose of the editor and this buffer is
        // none: what actually reads it is the one thing the entry of a menu does
        // not say, so the minibuffer does
        return QObject::tr(
            "LanguageTool reads no code: this buffer is checked for "
            "spelling only");
      }
      return QObject::tr("checking with LanguageTool, reading the text as %1")
          .arg(LanguageToolSupport::languageLabel(
              LanguageToolSupport::language()));
    }
    return QObject::tr("no spell or grammar checking");
  }  // end of GrammarCheckSession::reportOf

  void GrammarCheckSession::applyDiagnostics() {
    auto selections = QList<QTextEdit::ExtraSelection>();
    if ((this->area != nullptr) && (!this->document_file.isEmpty()) &&
        (this->document != nullptr)) {
      auto &doc = *this->document;
      for (const auto &d : this->diagnostics()) {
        if ((!blockExists(doc, d.startLine)) ||
            (!blockExists(doc, d.endLine))) {
          // the answer is late, or the text it described is shorter now
          continue;
        }
        auto selection = QTextEdit::ExtraSelection();
        auto c = QTextCursor(&doc);
        c.setPosition(
            LspProtocol::positionOf(doc, d.startLine, d.startCharacter));
        // a lint of one word is a range which may have collapsed on a text
        // which moved: an empty underline says nothing, so a single character
        // is marked, as the diagnostics of `clangd` do
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
        selection.format = lintFace();
        selections.append(selection);
      }
    }
    this->text_edit->setExtraSelections("grammar", selections);
  }  // end of GrammarCheckSession::applyDiagnostics

  QString GrammarCheckSession::diagnosticAt(const QTextDocument &doc,
                                            const int position) const {
    // several lints of one run are said together: the checker has a lot to say
    // about a single word sometimes, and the tooltip is the place where it is
    // read
    auto parts = QStringList();
    for (const auto &d : this->diagnostics()) {
      if ((!blockExists(doc, d.startLine)) || (!blockExists(doc, d.endLine))) {
        continue;
      }
      const auto start =
          LspProtocol::positionOf(doc, d.startLine, d.startCharacter);
      auto end = LspProtocol::positionOf(doc, d.endLine, d.endCharacter);
      if (end <= start) {
        end = start + 1;
      }
      if ((position >= start) && (position <= end)) {
        // the code of the lint, `SpellCheck` or `EN_A_VS_AN`, names the rule the
        // checker applied; it is what a person who thinks the rule is wrong looks
        // up in the documentation of the checker, so it goes with the message,
        // and the name of the checker only when neither named a rule
        auto text = d.message;
        const auto name = (!d.code.isEmpty()) ? d.code : d.source;
        if (!name.isEmpty()) {
          text += QStringLiteral(" [%1]").arg(name);
        }
        parts.append(text);
      }
    }
    return parts.join(QStringLiteral("\n"));
  }  // end of GrammarCheckSession::diagnosticAt

}  // end of namespace tfel::gui
