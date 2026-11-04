/*!
 * \file  CxxMajorMode.cxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtGui/QTextDocument>
#include <QtWidgets/QListView>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolTip>
#include <QtCore/QStringListModel>
#include <memory>

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

    // run the client, start it if the buffer asks for it the first time, and
    // wait for the handshake; true when requests can be sent
    bool serverIsReachable(LspClient &c, TextEditBase &t) {
      auto error = QString();
      if (!c.ensureStarted(error)) {
        t.getEditorWidget().displayInformativeMessage(error);
        return false;
      }
      if (c.isReady()) {
        return true;
      }
      t.getEditorWidget().displayInformativeMessage(
          QStringLiteral("contacting clangd..."));
      return c.waitUntilReady(5000);
    }  // end of serverIsReachable()

  }  // end of namespace

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
    this->client->openDocument(f, this->document);
    // the server is started with the first C++ buffer: the diagnostics are
    // the reason one opens a C++ file in this editor, and waiting for a key
    // press to start asking for them would show none until then
    auto error = QString();
    if (!this->client->ensureStarted(error)) {
      this->editor.displayInformativeMessage(error);
    }
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
    if (!serverIsReachable(*this->client, this->textEdit)) {
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
    if (!serverIsReachable(*this->client, this->textEdit)) {
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
  }  // end of jumpToDefinition

  void CxxMajorMode::showHover() {
    if (this->client == nullptr) {
      return;
    }
    if (!serverIsReachable(*this->client, this->textEdit)) {
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

  static StandardMajorModeProxy<CxxMajorMode> proxy(
      "C++",
      QVector<QRegularExpression>()
          << suffixRegExp("cxx") << suffixRegExp("hxx") << suffixRegExp("ixx")
          << suffixRegExp("cpp") << suffixRegExp("hpp") << suffixRegExp("ipp")
          << suffixRegExp("CC") << suffixRegExp("hh") << suffixRegExp("c")
          << suffixRegExp("h") << suffixRegExp("cu"),
      ":/tfel/editor/languages/cxx.png");

}  // end of namespace tfel::gui
