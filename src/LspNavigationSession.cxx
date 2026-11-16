/*!
 * \file  LspNavigationSession.cxx
 * \brief the conversation of a buffer with the language server of its language.
 * \author Thomas Helfer
 * \date   14/11/2026
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
#include <QtCore/QTimer>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPalette>
#include <algorithm>
#include <memory>

#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/LspClient.hxx"
#include "TFEL/GUI/LspNavigationSession.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

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

    // what the two questions about one tracked word bring back: the two places
    // to jump to, and the countdown which says the answer is complete. The
    // places are kept, not only counted, because a `Ctrl-Left click` on the very
    // word the mouse rests on is the common way to jump, and the click which
    // follows the icon of the jump shall not ask the server anything
    struct WordAnswers {
      //! the or-ing of `HAS_DEFINITION` and `HAS_DECLARATION`
      int capabilities = 0;
      //! the questions still to come back
      int pending = 2;
      QVector<LspLocation> definitions;
      QVector<LspLocation> declarations;
    };  // end of struct WordAnswers

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
      RenameSymbolInput(LspNavigationSession &m,
                        EditorWidget &w,
                        const QString &word,
                        const int symbol_position)
          : CommandLine(QObject::tr("rename %1 to :").arg(word), w),
            session(&m),
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
        if ((!name.isEmpty()) && (this->session != nullptr)) {
          this->session->renameToAt(name, this->position);
        }
      }  // end of treatUserInput

     private:
      //! the session which asked; null when the buffer it belongs to is gone
      QPointer<LspNavigationSession> session;
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
      DocumentSymbolInput(LspNavigationSession &m,
                          EditorWidget &w,
                          const QString &file,
                          const QVector<LspDocumentSymbol> &ss)
          : CommandLine(QObject::tr("go to symbol :"), w),
            session(&m),
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
        if ((answer.isEmpty()) || (this->session == nullptr)) {
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
          this->session->report(
              QObject::tr("no symbol named '%1' in this file").arg(answer));
          return;
        }
        // the first of several: the list said which ones the name visits, the
        // message says which one was gone to, and pressing the key again with
        // a longer name is how the next one is picked
        this->session->goToDocumentSymbol(*matches.at(0));
      }  // end of treatUserInput

      void inputTextChanged(const QString &t) override {
        // the base emits the signal the list is hidden on; showing the
        // narrowed list behind it is what makes the overview follow the name
        // being typed instead of vanishing under it
        CommandLine::inputTextChanged(t);
        this->showCompletions(this->heading, this->narrowed(t.trimmed()));
      }  // end of inputTextChanged

     private:
      //! the session which asked; null when the buffer it belongs to is gone
      QPointer<LspNavigationSession> session;
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
    // a change of a document, at the places of that document rather than at the
    // lines and columns of the protocol: whoever applies changes wants places,
    // and converting them once is what lets the changes of one file be ordered
    // and applied from the end of the text to its start
    struct DocumentEdit {
      //! where the change begins
      int start;
      //! where it ends
      int end;
      //! what stands there afterwards
      QString text;
    };  // end of struct DocumentEdit

    // how many characters two texts hold in common from their start
    int common_prefix(const QString &a, const QString &b) {
      const auto n = qMin(a.size(), b.size());
      auto i = 0;
      while ((i < n) && (a[i] == b[i])) {
        ++i;
      }
      return i;
    }  // end of common_prefix()

    // how many characters two texts hold in common from their end, of the given
    // number at most
    int common_suffix(const QString &a, const QString &b, const int limit) {
      const auto n = qMin(limit, qMin(a.size(), b.size()));
      auto i = 0;
      while ((i < n) && (a[a.size() - 1 - i] == b[b.size() - 1 - i])) {
        ++i;
      }
      return i;
    }  // end of common_suffix()

    /*!\brief the changes one answer of the protocol makes in a document
     * \param[out] edits : the changes, at the places of the document
     * \param[in] doc: the document the server was talking about
     * \param[in] text: its text, which is the text the server was handed
     * \param[in] change: the change, at the lines and columns of the protocol
     *
     * A change which names a place of the file is taken as it came. A change
     * which names the whole file is not: that is how \c jedi renames a symbol and
     * how \c autopep8 reformats a document, both of which answer the text of the
     * file with the one thing changed in it. Applying that as it stands is right
     * and wasteful — it throws the cursor to the end of the file, marks a buffer
     * modified down to its last line, and reports one edit for a change of two
     * letters — so what the two texts hold in common at their ends goes first,
     * and the lines the answer changes are then taken one by one, since what
     * stands between them is text the file already holds.
     *
     * \note the places of the text of a document and the places of its
     * `toPlainText` are the same numbers: a block separator is one character in
     * both, and no block of the editor holds a character which `toPlainText`
     * would write as more than one, see \c normalizeLineSeparators
     */
    void documentChangesOf(QVector<DocumentEdit> &edits,
                           const QTextDocument &doc,
                           const QString &text,
                           const LspTextEdit &change) {
      const auto start = LspProtocol::positionOf(doc, change.startLine,
                                                 change.startCharacter);
      auto end = LspProtocol::positionOf(doc, change.endLine,
                                         change.endCharacter);
      if (end < start) {
        end = start;
      }
      if ((start != 0) ||
          (end < doc.findBlockByNumber(doc.blockCount() - 1).position())) {
        // a change of a place the server named, which is the shape a \c clangd
        // rename and every quick fix come as: it is applied where it says
        edits.append({start, end, change.text});
        return;
      }
      const auto old_text = text.mid(start, end - start);
      const auto &new_text = change.text;
      const auto head = common_prefix(old_text, new_text);
      const auto tail =
          common_suffix(old_text, new_text, old_text.size() - head);
      const auto kept_old = old_text.mid(head, old_text.size() - head - tail);
      const auto kept_new = new_text.mid(head, new_text.size() - head - tail);
      if ((kept_old.isEmpty()) && (kept_new.isEmpty())) {
        // the file already says what the server would have said: the answer is
        // the text under another shape, not a change to make
        return;
      }
      const auto kept_start = start + head;
      const auto old_lines = kept_old.split(QLatin1Char('\n'));
      const auto new_lines = kept_new.split(QLatin1Char('\n'));
      if ((old_lines.size() == new_lines.size()) && (old_lines.size() > 1)) {
        // several lines differ and their number is left alone, which is what a
        // rename of a few occurrences, or a reformat of a few lines, reads as:
        // one change each, and the text between them, which the answer repeats
        // for nothing, is left where it is
        auto offset = 0;
        for (auto i = 0; i != old_lines.size(); ++i) {
          const auto &o = old_lines.at(i);
          const auto &n = new_lines.at(i);
          // the length of a line is a distance between two places of the
          // document, and the places of a document are `int`s
          const int line_length = static_cast<int>(o.size());
          if (o != n) {
            const auto line_head = common_prefix(o, n);
            const auto line_tail =
                common_suffix(o, n, line_length - line_head);
            edits.append({kept_start + offset + line_head,
                          kept_start + offset + line_length - line_tail,
                          n.mid(line_head, n.size() - line_head - line_tail)});
          }
          // the line and the separator which ends it
          offset += line_length + 1;
        }
        return;
      }
      edits.append({kept_start, end - tail, kept_new});
    }  // end of documentChangesOf()

  }  // end of namespace
  void LspNavigationSession::openOrRenameDocument() {
    if (this->lsp_client == nullptr) {
      return;
    }
    // the words tracked on the previous file, and what a server said of them,
    // are none of the renamed buffer's business: the tracking starts over
    this->resetNavigation();
    const auto f = this->textEdit.getCompleteFileName();
    if (f == this->documentFile) {
      if ((!f.isEmpty()) && (!this->lsp_client->isDocumentOpen(f))) {
        // the client is a fresh one since this mode last opened the file
        this->document = this->textEdit.document();
        this->lsp_client->openDocument(f, this->document);
        this->tokens_timer->start();
      }
      this->startSessionIfNeeded();
      return;
    }
    if (!this->documentFile.isEmpty()) {
      this->lsp_client->closeDocument(this->documentFile,
                                  this->textEdit.document());
      this->documentFile = QString();
      this->applyDiagnostics();
    }
    if (f.isEmpty()) {
      return;
    }
    this->documentFile = f;
    this->document = this->textEdit.document();
    // the document is registered on the client, ready to be sent: whether a
    // session runs to receive it is what `startClangdIfNeeded` decides on the
    // project of the file, and the entries of the "C++" menu decide on the
    // answer of whoever asks for one
    this->lsp_client->openDocument(f, this->document);
    this->startSessionIfNeeded();
  }  // end of openOrRenameDocument
  void LspNavigationSession::updateDiagnostics(const QString &f) {
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

  void LspNavigationSession::applyDiagnostics() {
    auto selections = QList<QTextEdit::ExtraSelection>();
    if ((this->lsp_client != nullptr) && (!this->documentFile.isEmpty())) {
      auto &doc = *this->textEdit.document();
      for (const auto &d :
           this->lsp_client->diagnosticsFor(this->documentFile)) {
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
    this->textEdit.setExtraSelections(this->server_name, selections);
  }  // end of applyDiagnostics

  void LspNavigationSession::refreshSemanticTokens() {
    if ((this->lsp_client == nullptr) ||
        (!this->semanticTokensWanted())) {
      // the setting is read on every question, so a person who turns it off
      // sees the text go back to what the highlighter draws on the next settle
      // of the buffer, without a new session or a restart
      this->clearSemanticTokens();
      return;
    }
    if ((!this->lsp_client->isReady()) ||
        (!this->lsp_client->hasSemanticTokens()) ||
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
    auto self = QPointer<LspNavigationSession>(this);
    this->lsp_client->requestSemanticTokens(
        this->documentFile,
        [self, sequence, revision](const QVector<LspSemanticToken> &tokens) {
          if (self.isNull()) {
            return;
          }
          self->applySemanticTokens(tokens, sequence, revision);
        });
  }  // end of refreshSemanticTokens

  void LspNavigationSession::applySemanticTokens(
      const QVector<LspSemanticToken> &tokens,
      const int sequence,
      const qint64 revision) {
    if ((this->document == nullptr) || (this->tokens_request != sequence) ||
        (this->document->revision() != revision) ||
        (!this->semanticTokensWanted())) {
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
    this->textEdit.setExtraSelections(this->tokens_key, selections);
  }  // end of applySemanticTokens

  void LspNavigationSession::clearSemanticTokens() {
    // ++, not a reset: a request which is on its way carries the number it was
    // issued with, and finds this one past it when its answer comes back
    ++(this->tokens_request);
    this->textEdit.setExtraSelections(this->tokens_key,
                                      QList<QTextEdit::ExtraSelection>());
  }  // end of clearSemanticTokens

  QString LspNavigationSession::diagnosticAt(const QTextDocument &doc,
                                     const int position) const {
    if ((this->lsp_client == nullptr) || (this->documentFile.isEmpty())) {
      return QString();
    }
    // the same conversion `applyDiagnostics` draws with: what is echoed has
    // to be what is underlined, down to the collapsed ranges, which are
    // drawn over a character and have to be pointed at over it too
    for (const auto &d : this->lsp_client->diagnosticsFor(this->documentFile)) {
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

  void LspNavigationSession::echoDiagnosticAtCursor() {
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

  void LspNavigationSession::resetNavigation() {
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
  bool LspNavigationSession::eventFilter(QObject *const o, QEvent *const e) {
    if ((this->mouse_press_consumed) &&
        (e->type() == QEvent::MouseButtonRelease)) {
      // the release of a click the mode answered with a jump is kept from the
      // text edit too: let through, it would move the cursor to the place the
      // click was pressed on, after the jump had put it somewhere else
      this->mouse_press_consumed = false;
      return true;
    }
    if ((this->lsp_client != nullptr) && (this->navigation_timer != nullptr)) {
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
    return QObject::eventFilter(o, e);
  }  // end of eventFilter

  bool LspNavigationSession::mousePressEvent(QMouseEvent *const e) {
    // `Ctrl-Left click` is the mouse's `Alt-.`: the icon of the jump the mode
    // put under the pointer promised a place to go. Any other click — and any
    // click of a buffer which asks nothing to a server — is the plain click the
    // text edit makes of it itself
    const auto m = e->modifiers();
    if ((this->lsp_client == nullptr) || (this->document == nullptr) ||
        (this->documentFile.isEmpty()) || (e->button() != Qt::LeftButton) ||
        ((m & Qt::ControlModifier) == Qt::NoModifier) ||
        ((m & (Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier)) !=
         Qt::NoModifier)) {
      this->mouse_press_consumed = false;
      return false;
    }
    const auto handled = this->jumpFromMouse(
        this->textEdit.cursorForPosition(e->pos()).position());
    // the press of the click which jumped is consumed, and so is its release,
    // see `eventFilter`; a click which found nothing goes back to the text edit
    // whole, cursor and all
    this->mouse_press_consumed = handled;
    return handled;
  }  // end of mousePressEvent

  void LspNavigationSession::refreshTrackedWords() {
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
    if ((this->lsp_client == nullptr) || (!this->lsp_client->isReady()) ||
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

  void LspNavigationSession::askTrackedWord(const bool track_cursor) {
    auto &word = track_cursor ? this->cursor_word : this->hovered_word;
    // the word the mouse rests on is very often the one the cursor has been
    // sitting on: what the cursor's question learned is the mouse's answer
    auto &other = track_cursor ? this->hovered_word : this->cursor_word;
    if ((other.start == word.start) && (other.revision == word.revision) &&
        (other.capabilities >= 0)) {
      word.capabilities = other.capabilities;
      word.definitions = other.definitions;
      word.declarations = other.declarations;
      if (track_cursor) {
        this->applyTrackedCursor();
      } else {
        this->applyTrackedHover();
      }
      return;
    }
    // the two questions of one word are one answer: it is defined or
    // declared, or it is neither, and that is known once both are back. A
    // server which has no declaration question is waited for one answer only
    auto answer = std::make_shared<WordAnswers>();
    const auto declarations = this->declarationSupported();
    answer->pending = declarations ? 2 : 1;
    auto self = QPointer<LspNavigationSession>(this);
    const auto start = word.start;
    const auto revision = word.revision;
    auto note = [self, answer, track_cursor, start, revision](
                    const int capability,
                    const QVector<LspLocation> &locations) {
      if (capability == HAS_DEFINITION) {
        answer->definitions = locations;
      } else {
        answer->declarations = locations;
      }
      if (!locations.isEmpty()) {
        answer->capabilities |= capability;
      }
      if ((--answer->pending) != 0) {
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
      w.capabilities = answer->capabilities;
      // the places themselves go with the verdict: a `Ctrl-Left click` on this
      // word is the jump the icon under the pointer promised, and it is then
      // answered out of what is already here
      w.definitions = answer->definitions;
      w.declarations = answer->declarations;
      if (track_cursor) {
        self->applyTrackedCursor();
      } else {
        self->applyTrackedHover();
      }
    };
    // any position within the identifier names it to the server, its start will
    // do; a client which is not answering calls the reply back at once, empty,
    // which settles the answer as much as a server would
    this->lsp_client->requestDefinition(
        this->documentFile, *this->document, start,
        [note](const QVector<LspLocation> &locations) {
          note(HAS_DEFINITION, locations);
        });
    if (declarations) {
      this->lsp_client->requestDeclaration(
          this->documentFile, *this->document, start,
          [note](const QVector<LspLocation> &locations) {
            note(HAS_DECLARATION, locations);
          });
    }
  }  // end of askTrackedWord

  void LspNavigationSession::applyTrackedCursor() {
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
    this->textEdit.setExtraSelections(this->navigation_key, selections);
  }  // end of applyTrackedCursor

  void LspNavigationSession::applyTrackedHover() {
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

  bool LspNavigationSession::jumpFromMouse(const int position) {
    const auto range = identifier_range(*this->document, position);
    if (range.x() < 0) {
      // no identifier under the pointer: a click on a brace, a blank or a word
      // of a comment is the plain click the text edit makes of it anyway
      return false;
    }
    const auto revision = this->document->revision();
    auto definitions = QVector<LspLocation>();
    auto declarations = QVector<LspLocation>();
    if ((this->hovered_word.start == range.x()) &&
        (this->hovered_word.revision == revision) &&
        (this->hovered_word.capabilities >= 0)) {
      // the word has been asked about already — which is what the icon of the
      // jump under the pointer is made of — and its answer holds the places to
      // go: the click which follows the icon asks nothing and lands at once
      definitions = this->hovered_word.definitions;
      declarations = this->hovered_word.declarations;
    } else if (this->lsp_client->isReady()) {
      // a word the mouse never rested on, or one whose answer is still on its
      // way: the questions are asked at once and waited for together, as the
      // two of the context menu are, and the wait is as short. A session which
      // is not up is not waited for: a click which starts nothing and moves
      // nothing is a click, not a request
      auto answer = std::make_shared<WordAnswers>();
      const auto with_declarations = this->declarationSupported();
      answer->pending = with_declarations ? 2 : 1;
      auto waiter = std::make_shared<std::function<void()>>();
      LspProtocol::boundedWait(
          [this, range, answer, with_declarations,
           waiter](const std::function<void()> &stop) {
            // one countdown shared by the answers: the last one back ends
            // the wait, and a late one is a late one only
            *waiter = [answer, stop] {
              if ((--(answer->pending)) == 0) {
                stop();
              }
            };
            this->lsp_client->requestDefinition(
                this->documentFile, *this->document, range.x(),
                [answer, waiter](const QVector<LspLocation> &locations) {
                  answer->definitions = locations;
                  (*waiter)();
                });
            if (with_declarations) {
              this->lsp_client->requestDeclaration(
                  this->documentFile, *this->document, range.x(),
                  [answer, waiter](const QVector<LspLocation> &locations) {
                    answer->declarations = locations;
                    (*waiter)();
                  });
            }
          },
          1000);
      definitions = answer->definitions;
      declarations = answer->declarations;
    }
    // the definition wins, as it does for `Alt-.`: the declaration of a symbol
    // which nothing defines is what `clangd` answers the definition question
    // with anyway, so the second question only speaks for a symbol whose body
    // lives where the first question could not follow it
    if (!definitions.isEmpty()) {
      this->jumpTo(definitions.at(0));
      return true;
    }
    if (!declarations.isEmpty()) {
      this->jumpTo(declarations.at(0));
      return true;
    }
    // nothing to jump to, in the same words `Alt-.` says it; the click itself is
    // handed back to the text edit, which places the cursor where it was pressed
    this->report(QStringLiteral("no definition found"));
    return false;
  }  // end of jumpFromMouse
  void LspNavigationSession::completeAtCursor() {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (!this->serverIsReachable()) {
      this->report(this->notAnswering());
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->openOrRenameDocument();
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    if (!this->lsp_client->isDocumentOpen(this->documentFile)) {
      this->lsp_client->openDocument(this->documentFile,
                                     this->textEdit.document());
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
          this->lsp_client->requestCompletion(
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
  }  // end of completeAtCursor

  void LspNavigationSession::jumpToDefinition() {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (!this->serverIsReachable()) {
      this->report(this->notAnswering());
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    auto answer = std::make_shared<QVector<LspLocation>>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->lsp_client->requestDefinition(
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

  void LspNavigationSession::showReferences() {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (!this->serverIsReachable()) {
      this->report(this->notAnswering());
      return;
    }
    if (this->documentFile.isEmpty()) {
      this->report("this buffer visits no file yet");
      return;
    }
    auto answer = std::make_shared<QVector<LspLocation>>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->lsp_client->requestReferences(
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
    menu->setObjectName(this->references_name);
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

  void LspNavigationSession::showDocumentSymbols() {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (!this->serverIsReachable()) {
      this->report(this->notAnswering());
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
          this->lsp_client->requestDocumentSymbols(
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
      if (in_time) {
        this->report(QStringLiteral("%1 found no symbol in this file")
                         .arg(this->server_name));
      } else {
        this->report(this->notAnswering());
      }
      return;
    }
    this->editor.setUserInput(new DocumentSymbolInput(
        *this, this->editor, this->documentFile, *answer));
  }  // end of showDocumentSymbols

  void LspNavigationSession::renameSymbol() {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (!this->serverIsReachable()) {
      this->report(this->notAnswering());
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

  void LspNavigationSession::renameToAt(const QString &name,
                                        const int position) {
    if (this->lsp_client == nullptr) {
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
          this->lsp_client->requestRename(
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
      this->report(QStringLiteral("%1 did not answer the rename")
                       .arg(this->server_name));
      return;
    }
    if (answer->edits.isEmpty()) {
      // clangd refuses a name which is not a valid identifier, points at no
      // symbol, or names the symbol it already has; its own reason is the
      // message worth showing, and an empty edit list with no reason says the
      // symbol is already named what was asked
      this->report(answer->error.isEmpty()
                       ? QStringLiteral("nothing to rename")
                       : QStringLiteral("%1 refused the rename: %2")
                             .arg(this->server_name, answer->error));
      return;
    }
    const auto count = this->applyWorkspaceEdit(answer->edits);
    this->report(QStringLiteral("%1 occurrences renamed in %2 file(s)")
                     .arg(count)
                     .arg(answer->edits.size()));
  }  // end of renameToAt

  int LspNavigationSession::applyWorkspaceEdit(
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
      // the positions of the protocol are those of the text as the server knows
      // it, which is what the buffer holds: the changes are converted once, and
      // a server which answered the whole file has its answer cut back to the
      // lines it actually changes, see `documentChangesOf`
      const auto text = doc->toPlainText();
      const auto &of_this_file = it.value();
      auto changes = QVector<DocumentEdit>();
      for (const auto &change : of_this_file) {
        documentChangesOf(changes, *doc, text, change);
      }
      // applied from the end of the document to its start, so that no change
      // moves the place the next one is to cut
      std::sort(changes.begin(), changes.end(),
                [](const DocumentEdit &a, const DocumentEdit &b) {
                  return a.start > b.start;
                });
      auto cursor = QTextCursor(doc);
      for (const auto &change : std::as_const(changes)) {
        cursor.setPosition(change.start);
        cursor.setPosition(change.end, QTextCursor::KeepAnchor);
        cursor.insertText(change.text);
        ++count;
      }
    }
    this->editor.changeBuffer(origin);
    return count;
  }  // end of applyWorkspaceEdit

  void LspNavigationSession::goToDocumentSymbol(
      const LspDocumentSymbol &symbol) {
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

  void LspNavigationSession::jumpTo(const LspLocation &location) {
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

  void LspNavigationSession::showHover() {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (!this->serverIsReachable()) {
      this->report(this->notAnswering());
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
          this->lsp_client->requestHover(
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

  void LspNavigationSession::showSignatureHelp(const bool asked_by_user) {
    if (this->lsp_client == nullptr) {
      return;
    }
    if (asked_by_user) {
      if (!this->serverIsReachable()) {
        this->report(this->notAnswering());
        return;
      }
      if (this->documentFile.isEmpty()) {
        this->report("this buffer visits no file yet");
        return;
      }
    } else if ((!this->lsp_client->isReady()) ||
               (this->documentFile.isEmpty()) ||
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
    auto self = QPointer<LspNavigationSession>(this);
    this->lsp_client->requestSignatureHelp(
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

  void LspNavigationSession::watchSignatureHelp(const QKeyEvent &e) {
    if ((this->lsp_client == nullptr) || (this->signature_timer == nullptr) ||
        (!this->lsp_client->isReady()) || (this->documentFile.isEmpty())) {
      return;
    }
    const auto text = e.text();
    // the character the key puts in the document is what the server listed as
    // a trigger, not the key itself: `<` and `(` carry modifiers on this
    // keyboard and none on that one
    if (text.isEmpty() ||
        (!this->lsp_client->signatureHelpTriggers().contains(text.at(0)))) {
      return;
    }
    this->signature_timer->start();
  }  // end of watchSignatureHelp

  void LspNavigationSession::completeContextMenu(QMenu *const m,
                                         const QTextCursor &tc) {
    if ((this->lsp_client == nullptr) || (this->documentFile.isEmpty()) ||
        (this->document == nullptr) || (tc.document() != this->document)) {
      return;
    }
    // a right click neither launches a server nor waits for a handshake: the
    // menu would show up late, and no question has been asked of the server
    // yet for that to be paid for
    if ((!this->lsp_client->isReady())) {
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
          this->lsp_client->requestDefinition(
              this->documentFile, *this->document, tc.position(),
              [locations, waiter](const QVector<LspLocation> &l) {
                *locations = l;
                (*waiter)();
              });
          this->lsp_client->requestCodeActions(
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
      if (action.edits.isEmpty()) {
        continue;
      }
      if (!this->codeActionOffered(action)) {
        // the whole-buffer kinds of a language, which its menu and its commands
        // hold entries for and a context menu has no business offering
        continue;
      }
      fixes.append(&action);
      if (fixes.size() == 6) {
        break;
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

  QCompleter *LspNavigationSession::getCompleter() {
    if ((this->lspCompleter != nullptr) && (this->lspCompletionActive)) {
      if (!this->lspCompleter->popup()->isVisible()) {
        // the popup was closed under our feet: by Escape, by a focus change,
        // by a click elsewhere. The next Tab must go back to being a Tab
        this->lspCompletionActive = false;
      } else {
        return this->lspCompleter;
      }
    }
    return nullptr;
  }  // end of getCompleter

  int LspNavigationSession::getMinimalCompletionLength() {
    return (this->lspCompletionActive) ? 1 : 3;
  }  // end of getMinimalCompletionLength

  bool LspNavigationSession::completeCurrentWord(TextEditBase &t,
                                                 const QString &w) {
    if ((this->lspCompleter == nullptr) || (!this->lspCompletionActive)) {
      // the popup of the server is not up: whatever completion the mode has of
      // its own is the one to apply, and this session has nothing to say
      return false;
    }
    // the whole word under the cursor goes, not only the missing tail: a server
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
    return true;
  }  // end of completeCurrentWord

  LspNavigationSession::LspNavigationSession(EditorWidget &w,
                                             Buffer &b,
                                             TextEditBase &t,
                                             const QString &name,
                                             const QString &language,
                                             QObject *const p)
      : QObject(p),
        editor(w),
        buffer(b),
        textEdit(t),
        lsp_client(&LspClient::get(w, name)),
        server_name(name),
        navigation_key(name + QStringLiteral("-navigation")),
        tokens_key(name + QStringLiteral("-tokens")),
        references_name(name + QStringLiteral("-references")) {
    // the client of an editor widget is built by the first session of its
    // language and shared by every buffer of that language, which is what a
    // server with a shared index or a shared project expects; what it is told
    // about the files it is handed is the same for every one of those buffers
    this->lsp_client->setLanguageId(language);
    // the diagnostics of the server are drawn as they are published, and the
    // buffer of the file they name is the only one they concern
    QObject::connect(this->lsp_client, &LspClient::diagnosticsUpdated, this,
                     &LspNavigationSession::updateDiagnostics);
    // the buffer of a new file has no name when the session is built, and gains
    // it before or after, depending on how the file was reached; the same holds
    // for a save under a new name
    QObject::connect(&t, &TextEditBase::fileNameChanged, this,
                     [this](const QString &) {
                       this->openOrRenameDocument();
                     });
    // the word under the cursor is underlined and the word under the mouse asks
    // for the icon of the jump: both are questions of the moment, so one
    // settling timer is shared by the two, restarted by every event which may
    // have moved a position, and the questions are only asked when it goes off
    this->navigation_timer = new QTimer(this);
    this->navigation_timer->setSingleShot(true);
    this->navigation_timer->setInterval(150);
    QObject::connect(this->navigation_timer, &QTimer::timeout, this,
                     &LspNavigationSession::refreshTrackedWords);
    // typing `(` or `,` asks what call the cursor is inside of; the wait is what
    // keeps a fast typist from one request per character, and the answer goes to
    // the minibuffer, not to the text
    this->signature_timer = new QTimer(this);
    this->signature_timer->setSingleShot(true);
    this->signature_timer->setInterval(120);
    QObject::connect(this->signature_timer, &QTimer::timeout, this,
                     [this] { this->showSignatureHelp(false); });
    // what the identifiers of the file are is a question about the whole file,
    // which a server answers only once it has parsed it: the wait is therefore
    // much longer than the two above, and the questions are asked on their timer
    // alone, restarted by everything which may have changed the text or woken the
    // server up — the handshake, a file parsed, a keystroke
    this->tokens_timer = new QTimer(this);
    this->tokens_timer->setSingleShot(true);
    this->tokens_timer->setInterval(700);
    QObject::connect(this->tokens_timer, &QTimer::timeout, this,
                     &LspNavigationSession::refreshSemanticTokens);
    QObject::connect(this->lsp_client, &LspClient::ready, this,
                     [this] { this->tokens_timer->start(); });
    QObject::connect(t.document(), &QTextDocument::contentsChanged, this,
                     [this] { this->tokens_timer->start(); });
    QObject::connect(&t, &TextEditBase::cursorPositionChanged, this, [this] {
      this->cursor_tracking = true;
      this->navigation_timer->start();
    });
    // a save is the one moment the file of the buffer starts saying what the
    // buffer says, and a linter which runs its program over that file — `pylint`
    // is one, and it holds the results it had while the buffer was dirty — needs
    // to be told that its turn has come round again. The document reaching
    // "not modified" is the editor's own cue of that moment: the save which
    // wrote the file is what leaves it, and an undo which comes back to the
    // saved state asks the server for one read of a file it agrees with
    QObject::connect(t.document(), &QTextDocument::modificationChanged, this,
                     [this] {
                       if ((this->document != nullptr) &&
                           (!this->document->isModified())) {
                         this->noteFileSaved();
                       }
                     });
    // a stopped session knows nothing anymore: the underlines and the mouse
    // cursor of the one which is gone go away with it
    QObject::connect(this->lsp_client, &LspClient::stopped, this,
                     &LspNavigationSession::resetNavigation);
    // the moves of the mouse are the events of the viewport, which reports them
    // only when the tracking is on
    if (auto *const area = t.widget()) {
      area->setMouseTracking(true);
      area->viewport()->setMouseTracking(true);
      area->viewport()->installEventFilter(this);
    }
  }  // end of LspNavigationSession::LspNavigationSession

  LspNavigationSession::~LspNavigationSession() {
    // `textEdit.document()` must not be called from here: the session is
    // destroyed as the child of a mode which is itself a child of the text edit
    // widget, whose own Qt widget is gone by then, and the virtual call would
    // read it back through a dangling pointer, the lifetime trap of issue #20 in
    // a new dress. The `QPointer` says what is alive; a dead document has
    // released its reference on the client by itself, see
    // `LspClient::openDocument`
    if ((this->lsp_client != nullptr) && (!this->documentFile.isEmpty()) &&
        (this->document != nullptr)) {
      this->lsp_client->closeDocument(this->documentFile,
                                      this->document.data());
    }
  }  // end of ~LspNavigationSession

  LspClient &LspNavigationSession::client() {
    return *this->lsp_client;
  }  // end of client

  QString LspNavigationSession::visitedFile() const {
    return this->documentFile;
  }  // end of visitedFile

  EditorWidget &LspNavigationSession::editorWidget() {
    return this->editor;
  }  // end of editorWidget

  QTextDocument *LspNavigationSession::documentOfBuffer() const {
    return this->document.data();
  }  // end of documentOfBuffer

  void LspNavigationSession::report(const QString &m) {
    this->editor.displayInformativeMessage(m);
  }  // end of report

  QString LspNavigationSession::menuName() const {
    return QString();
  }  // end of menuName

  QString LspNavigationSession::startCommandName() const {
    return QString();
  }  // end of startCommandName

  bool LspNavigationSession::declarationSupported() const {
    return true;
  }  // end of declarationSupported

  void LspNavigationSession::startSessionIfNeeded() {
    // the generic code starts nothing: whether a file is worth a session
    // nobody asked for is the language's business, not the protocol's
  }  // end of startSessionIfNeeded

  bool LspNavigationSession::semanticTokensWanted() const {
    return true;
  }  // end of semanticTokensWanted

  bool LspNavigationSession::semanticFaceOf(const LspSemanticToken &token,
                                            QTextCharFormat &format) const {
    // a restrained set, which colors what a regular expression can not tell
    // apart and leaves the rest of the text as the highlighter already drew it
    //
    // the names are those of the legend the server announced, not of the
    // protocol: `clangd` names eleven of its kinds, three of them `variable`,
    // and a name which is not in the list simply gets no face
    //
    // `auto` is answered as the type it is deduced to be: coloring that word
    // would ink a keyword over, which the highlighter owns
    if (token.modifiers.contains(QStringLiteral("deduced"))) {
      return false;
    }
    if (token.modifiers.contains(QStringLiteral("deprecated"))) {
      // what the server says a name is not worth using anymore: the face of a
      // strike-out, whatever else the name is
      format.setFontStrikeOut(true);
      return true;
    }
    const auto &type = token.type;
    if (type == QLatin1String("function") || type == QLatin1String("method")) {
      format.setForeground(QColor(Qt::darkCyan));
      return true;
    }
    if (type == QLatin1String("macro")) {
      // not bold: the bold dark magenta is the keyword face of the highlighter,
      // and the two sets of words which get it never overlap
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
  }  // end of semanticFaceOf

  bool LspNavigationSession::keyPressEvent(QKeyEvent *const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_Tab)) {
      this->completeAtCursor();
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
    // rectangle mark of `TextEditBase` sits behind the Ctrl-x prefix chain and
    // Ctrl-R belongs to the search history
    if ((m == Qt::AltModifier) && (k == Qt::Key_R)) {
      this->renameSymbol();
      return true;
    }
    // '?' is a shifted key: the platform puts ShiftModifier alongside
    // AltModifier, as the Alt-< and Alt-> bindings of TextEditBase learned the
    // hard way
    if ((m & Qt::AltModifier) && !(m & Qt::ControlModifier) &&
        (k == Qt::Key_Question)) {
      this->showHover();
      return true;
    }
    // Alt-S asks the parameter list of the call the cursor stands in; plain Alt,
    // since the Ctrl-x chain writes the buffer with `s` and Ctrl-S is the
    // incremental search
    if ((m == Qt::AltModifier) && (k == Qt::Key_S)) {
      this->showSignatureHelp(true);
      return true;
    }
    // Alt-O opens the overview of the file: which symbol to go to is a question
    // of the whole buffer, not of the word under the cursor, so it is the one of
    // the keys of a session which asks nothing about a position; plain Alt, since
    // Ctrl-O is the history of the minibuffer and Alt-O belongs to nobody
    if ((m == Qt::AltModifier) && (k == Qt::Key_O)) {
      this->showDocumentSymbols();
      return true;
    }
    return false;
  }  // end of keyPressEvent

  void LspNavigationSession::askSemanticTokens() {
    this->tokens_timer->start();
  }  // end of askSemanticTokens

  bool LspNavigationSession::formatDocument() {
    auto why = QString();
    return formatDocument(why);
  }  // end of formatDocument

  bool LspNavigationSession::formatDocument(QString &why) {
    why = QStringLiteral("no server answered");
    if ((this->lsp_client == nullptr) || (this->documentFile.isEmpty()) ||
        (this->document == nullptr)) {
      // no file, hence no document the protocol knows, hence nothing to ask
      // about: the mode is left to the formatter of its own language
      why = QStringLiteral("this buffer visits no file yet");
      return false;
    }
    if (!this->lsp_client->isReady()) {
      // a right click, a menu entry, a key: none of them launches a server, and
      // a reformat is not worth waiting a handshake for
      why = QStringLiteral("%1 is not started").arg(this->server_name);
      return false;
    }
    struct Answer {
      QMap<QString, QVector<LspTextEdit>> edits;
    };
    auto answer = std::make_shared<Answer>();
    const auto in_time = LspProtocol::boundedWait(
        [this, answer](const std::function<void()> &stop) {
          this->lsp_client->requestFormatting(
              this->documentFile, *this->document, 4, true,
              [answer, stop](
                  const QMap<QString, QVector<LspTextEdit>> &edits,
                  const QString &) {
                answer->edits = edits;
                stop();
              });
        },
        3000);
    if (!in_time) {
      // a server which is busy formatting a file the person is waiting on is
      // none's business to wait for; the mode says what it can do about it,
      // which is nothing
      why = notAnswering();
      return false;
    }
    // an answer which changes nothing — the common answer of a buffer which is
    // already formatted — applies nothing and says nothing: a message per key
    // press on such a buffer is what makes a key unusable
    const auto applied = this->applyWorkspaceEdit(answer->edits);
    if (applied == 0) {
      why = QStringLiteral("%1 changed nothing").arg(this->server_name);
    }
    return applied > 0;
  }  // end of formatDocument

  void LspNavigationSession::noteFileSaved() {
    if ((this->lsp_client == nullptr) || (this->documentFile.isEmpty())) {
      // nothing under a name the server knows, so no file of it which could be
      // read again
      return;
    }
    this->lsp_client->didSaveDocument(this->documentFile);
  }  // end of noteFileSaved

  bool LspNavigationSession::sortImports(QString &why) {
    why = QStringLiteral("no server answered");
    if ((this->lsp_client == nullptr) || (this->documentFile.isEmpty()) ||
        (this->document == nullptr)) {
      // no document under a name the protocol speaks, hence no question to ask
      why = QStringLiteral("this buffer visits no file yet");
      return false;
    }
    if (!this->lsp_client->isReady()) {
      why = notAnswering();
      return false;
    }
    // the whole file, and only the whole file: what gets sorted is the block of
    // the imports at the top of it, but a range is what the question is about,
    // and a server which reads the range has to see the imports inside of it.
    // The kinds name the one change wanted, and the answer is picked over for
    // it below, since a server is bound by that list no more than it is bound
    // by the range
    const auto stop = this->document->characterCount() - 1;
    struct Answer {
      QVector<LspCodeAction> actions;
    };
    auto answer = std::make_shared<Answer>();
    const auto in_time = LspProtocol::boundedWait(
        [this, stop, answer](const std::function<void()> &done) {
          this->lsp_client->requestCodeActions(
              this->documentFile, *this->document, 0, stop,
              [answer, done](const QVector<LspCodeAction> &a) {
                answer->actions = a;
                done();
              },
              {QStringLiteral("source.organizeImports")});
        },
        5000);
    if (!in_time) {
      why = notAnswering();
      return false;
    }
    for (const auto &action : std::as_const(answer->actions)) {
      if (!action.kind.startsWith(QLatin1String("source.organizeImports"))) {
        // the quick fixes of the line and the whole-file fixes of the same
        // program, which came along because the answer decides what is offered,
        // not the list the question named
        continue;
      }
      if (action.edits.isEmpty()) {
        continue;
      }
      if (this->applyWorkspaceEdit(action.edits) == 0) {
        why = QStringLiteral("the imports are already sorted");
        return false;
      }
      return true;
    }
    why = QStringLiteral("no server offered a change sorting the imports");
    return false;
  }  // end of sortImports

  bool LspNavigationSession::codeActionOffered(
      const LspCodeAction & /*action*/) const {
    // every change the server carries text for is a change worth an entry: the
    // answers of `clangd` are the quick fixes of a diagnostic and the refactors
    // of the range the menu was opened on, which is what a context menu is for
    return true;
  }  // end of codeActionOffered

  QString LspNavigationSession::notAnswering() const {
    return QStringLiteral("%1 is not answering").arg(this->server_name);
  }  // end of notAnswering

  bool LspNavigationSession::serverIsReachable() {
    if (this->lsp_client->isIdle()) {
      auto message =
          QStringLiteral("%1 is not started: use the '%2' menu")
              .arg(this->server_name)
              .arg(this->menuName());
      if (!this->startCommandName().isEmpty()) {
        message += QStringLiteral(" or the '%1' command")
                       .arg(this->startCommandName());
      }
      this->editor.displayInformativeMessage(message);
      return false;
    }
    if (this->lsp_client->isReady()) {
      return true;
    }
    this->editor.displayInformativeMessage(
        QStringLiteral("contacting %1...").arg(this->server_name));
    return this->lsp_client->waitUntilReady(5000);
  }  // end of serverIsReachable

}  // end of namespace tfel::gui
