/*!
 * \file   GrammarCheckSession.cxx
 * \brief  a `harper-ls` session, the lints it draws under a text edit, and the
 *         menu which steers it.
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
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspHarperSupport.hxx"
#include "TFEL/GUI/LspProtocol.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {

    // the face of a lint: a plain underline in red, the spelling checker's own
    // look, and one face for every lint. The severity of the protocol lives in
    // the color of the diagnostics of `clangd` because that server sorts what
    // it finds by it; the grammar checker answers all of its lints with the
    // same severity — a hint — so its whole output is to be seen, and only the
    // words it says something about are inked. A wave underline would be the
    // look of the hunspell underlines: `QPlainTextEdit` does not ink one
    // through the extra selections, see `CxxMajorMode::applyDiagnostics`
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
                                           QObject *const p)
      : QObject(p), editor(&w), text_edit(&t) {
    // a client of its own, and not the one of the editor widget: that one
    // speaks with `clangd`, which a buffer of Markdown has no use for, and the
    // documents of the two kinds of server are not to be sent to each other
    this->client = new LspClient(&w, this);
    this->client->setServerName(QStringLiteral("harper-ls"));
    this->client->setLanguageId(language_id);
    // the checker is silent about how it wants its documents and reads the text
    // it is given as the text of the document; the range of a change would
    // corrupt the copy it works on
    this->client->setFullTextSync(true);
    this->client->setConfiguration(LspHarperSupport::configuration());
    QObject::connect(this->client, &LspClient::diagnosticsUpdated, this,
                     &GrammarCheckSession::updateDiagnostics);
    QObject::connect(this->client, &LspClient::stopped, this, [this] {
      // a checker which exited of its own accord checks nothing anymore,
      // whatever was chosen; the mode has to learn it to take its own spell
      // checker back, and the underlines of a dead server are none
      if (this->active) {
        this->active = false;
        this->applyDiagnostics();
        emit this->stateChanged();
      }
    });
    // the buffer of a new file has no name when the mode is built, and gains it
    // before or after, depending on how the file was reached; the same holds
    // for a save under a new name
    QObject::connect(&t, &TextEditBase::fileNameChanged, this,
                     [this](const QString &) { this->openOrRenameDocument(); });
    if (auto *const scroll_area = t.widget()) {
      this->area = scroll_area;
      scroll_area->viewport()->installEventFilter(this);
    }
    // the document is registered whether a session runs or not: the setting
    // which launches one by default is read at the construction of the mode,
    // before this constructor has a chance to run, and the session launched
    // there sends what it was given once its handshake is over
    this->openOrRenameDocument();
  }  // end of GrammarCheckSession::GrammarCheckSession

  GrammarCheckSession::~GrammarCheckSession() {
    // the client dies with this session and ends its process, which reports
    // through signals: nobody may answer them by drawing on a text edit which
    // is dying with the mode, see the `area` member
    QObject::disconnect(this->client, nullptr, this, nullptr);
    if ((this->area != nullptr) && (this->text_edit != nullptr)) {
      // a mode which is replaced leaves its text edit behind, and the lints of
      // the checker it had with it, which are the keyed contribution this
      // session wrote and nobody else will clear: the client is a child of this
      // session, so it is gone only after this body ran and still holds the
      // answer it published. Asking `applyDiagnostics` to redraw it would put
      // the underlines of a dead session back on the screen; handing the key an
      // empty list whole is what takes them off. A text edit which is destroyed
      // along with its mode needs none of this, and `area` is null by then
      this->text_edit->setExtraSelections("harper", {});
    }
  }  // end of GrammarCheckSession::~GrammarCheckSession

  bool GrammarCheckSession::isAvailable() {
    return !LspHarperSupport::serverExecutable().isEmpty();
  }  // end of GrammarCheckSession::isAvailable

  bool GrammarCheckSession::isActive() const {
    return this->active;
  }  // end of GrammarCheckSession::isActive

  void GrammarCheckSession::setActive(const bool on,
                                      const bool remembered,
                                      const bool announced) {
    if (remembered) {
      // the choice is the one the whole editor follows: a buffer opened
      // afterwards, in this mode or in another one, starts as this one does
      LspHarperSupport::setLaunchByDefault(on);
    }
    if (on == this->active) {
      // saying what already is says nothing; the entry of a menu which is
      // already checked still had its answer written above
      return;
    }
    if (!on) {
      this->active = false;
      this->client->stop();
      this->applyDiagnostics();
      if (announced) {
        this->editor->displayInformativeMessage(
            QObject::tr("grammar checking stopped"));
      }
      emit this->stateChanged();
      return;
    }
    const auto program = LspHarperSupport::serverExecutable();
    if (program.isEmpty()) {
      if (announced) {
        this->editor->displayInformativeMessage(
            QObject::tr("harper-ls was not found: install it or set the "
                        "'harper/executable' option"));
      }
      return;
    }
    auto error = QString();
    if (!this->client->launch(program, LspHarperSupport::serverArguments(),
                              error)) {
      if (announced) {
        this->editor->displayInformativeMessage(error);
      }
      return;
    }
    this->active = true;
    if (announced) {
      // the dialect is named because it is the one thing the answer changes,
      // and a person who reads a buffer of British English wants to see which
      // English the underlines were drawn against
      this->editor->displayInformativeMessage(
          QObject::tr("grammar checking with %1, reading the text as %2")
              .arg(program)
              .arg(
                  LspHarperSupport::dialectLabel(LspHarperSupport::dialect())));
    }
    emit this->stateChanged();
  }  // end of GrammarCheckSession::setActive

  void GrammarCheckSession::setDialect(const QString &name) {
    LspHarperSupport::setDialect(name);
    // the settings the client answers are what the next question of the server
    // reads; the documents, which changed by nothing, have to be handed over
    // again for the new answer to be of any use to the buffer on screen
    this->client->setConfiguration(LspHarperSupport::configuration());
    this->client->recheckDocuments();
  }  // end of GrammarCheckSession::setDialect

  void GrammarCheckSession::addToMenu(QMenu &menu, const QString &toggle) {
    // the entry says what this buffer does, not what the editor does on the
    // whole: a person who turns the checker off here turns it off for the
    // buffers opened afterwards all the same, which the tooltip says
    auto *const gc = menu.addAction(toggle);
    gc->setCheckable(true);
    gc->setChecked(this->active);
    gc->setToolTip(
        QObject::tr("Ask the grammar checker about this buffer and "
                    "underline what it finds, and take the spell "
                    "checker of the mode off it while it runs. The "
                    "choice is kept in the setting "
                    "'harper/launch-by-default' and the buffers "
                    "opened afterwards follow it"));
    QObject::connect(gc, &QAction::toggled, this,
                     [this](const bool on) { this->setActive(on); });
    auto *const v = menu.addMenu(QObject::tr("English variant"));
    v->setToolTip(
        QObject::tr("Which English the text is read as: which "
                    "spellings are held to be their own, and which "
                    "inflections are accepted. The choice is kept in "
                    "the setting 'harper/dialect' and reaches the "
                    "checker at once"));
    auto *const group = new QActionGroup(v);
    group->setExclusive(true);
    const auto current = LspHarperSupport::dialect();
    for (const auto &name : LspHarperSupport::dialects()) {
      auto *const a = v->addAction(LspHarperSupport::dialectLabel(name));
      a->setCheckable(true);
      a->setChecked(name == current);
      group->addAction(a);
      QObject::connect(a, &QAction::triggered, this,
                       [this, name] { this->setDialect(name); });
    }
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

  void GrammarCheckSession::updateDiagnostics(const QString &file) {
    if (file == this->document_file) {
      this->applyDiagnostics();
    }
  }  // end of GrammarCheckSession::updateDiagnostics

  void GrammarCheckSession::openOrRenameDocument() {
    const auto f = this->text_edit->getCompleteFileName();
    if (f == this->document_file) {
      if ((!f.isEmpty()) && (!this->client->isDocumentOpen(f))) {
        // the client is a fresh one since this session last opened the file
        this->document = this->text_edit->document();
        this->client->openDocument(f, this->document);
      }
      return;
    }
    if (!this->document_file.isEmpty()) {
      this->client->closeDocument(this->document_file,
                                  this->text_edit->document());
      this->document_file = QString();
      this->applyDiagnostics();
    }
    if (f.isEmpty()) {
      // a buffer which visits no file has no name to be read under; the checker
      // says nothing of it until the buffer is saved, which is also what the
      // diagnostics of `clangd` wait for
      return;
    }
    this->document_file = f;
    this->document = this->text_edit->document();
    this->client->openDocument(f, this->document);
  }  // end of GrammarCheckSession::openOrRenameDocument

  void GrammarCheckSession::applyDiagnostics() {
    auto selections = QList<QTextEdit::ExtraSelection>();
    if ((this->area != nullptr) && (!this->document_file.isEmpty()) &&
        (this->document != nullptr)) {
      auto &doc = *this->document;
      for (const auto &d : this->client->diagnosticsFor(this->document_file)) {
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
    this->text_edit->setExtraSelections("harper", selections);
  }  // end of GrammarCheckSession::applyDiagnostics

  QString GrammarCheckSession::diagnosticAt(const QTextDocument &doc,
                                            const int position) const {
    if (this->document_file.isEmpty()) {
      return QString();
    }
    // several lints of one run are said together: the checker has a lot to say
    // about a single word sometimes, and the tooltip is the place where it is
    // read
    auto parts = QStringList();
    for (const auto &d : this->client->diagnosticsFor(this->document_file)) {
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
        // the code of the lint, `SpellCheck` or `PronounVerbAgreement`, names
        // the rule the checker applied; it is what a person who thinks the rule
        // is wrong looks up in the documentation of the checker, so it goes
        // with the message, and the name of the checker only when the server
        // named no rule
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
