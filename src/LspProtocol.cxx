/*!
 * \file  LspProtocol.cxx
 * \brief pure helpers of the Language Server Protocol client.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtCore/QEventLoop>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/LspProtocol.hxx"

namespace tfel::gui {

  QByteArray LspProtocol::encode(const QJsonObject &m) {
    const auto body = QJsonDocument(m).toJson(QJsonDocument::Compact);
    auto out = QByteArray("Content-Length: ");
    out += QByteArray::number(body.size());
    out += "\r\n\r\n";
    out += body;
    return out;
  }  // end of LspProtocol::encode

  QVector<QJsonObject> LspProtocol::decode(QByteArray &buffer) {
    QVector<QJsonObject> messages;
    for (;;) {
      const auto header_end = buffer.indexOf("\r\n\r\n");
      if (header_end < 0) {
        return messages;
      }
      // the headers are separated by a line feed and may come in any order;
      // only the length of the body matters to us
      auto length = -1;
      const auto headers =
          QString::fromLatin1(buffer.constData(), header_end).split("\r\n");
      for (const auto &h : headers) {
        const auto c = h.indexOf(':');
        if (c < 0) {
          continue;
        }
        const auto name = h.left(c).trimmed();
        if (name.compare("content-length", Qt::CaseInsensitive) == 0) {
          length = h.mid(c + 1).trimmed().toInt();
        }
      }
      if (length < 0) {
        // a framed block without a length can not be told apart from noise;
        // shifting a single byte and looking again is what lets a real frame
        // behind a few stray bytes still be read, which dropping the whole
        // block would throw away too
        buffer.remove(0, 1);
        continue;
      }
      if (buffer.size() < header_end + 4 + length) {
        // the body is not all here yet: the next read will complete it
        return messages;
      }
      const auto body = buffer.mid(header_end + 4, length);
      buffer.remove(0, header_end + 4 + length);
      const auto parsed = QJsonDocument::fromJson(body);
      if (parsed.isObject()) {
        messages.append(parsed.object());
      }
    }
    return messages;
  }  // end of LspProtocol::decode

  QString LspProtocol::uriOf(const QString &file) {
    return QUrl::fromLocalFile(QFileInfo(file).absoluteFilePath()).toString();
  }  // end of LspProtocol::uriOf

  QString LspProtocol::fileOfUri(const QString &uri) {
    return QUrl(uri).toLocalFile();
  }  // end of LspProtocol::fileOfUri

  QJsonObject LspProtocol::positionOf(const QTextDocument &d, int position) {
    auto c = QTextCursor(const_cast<QTextDocument *>(&d));
    c.setPosition(position);
    // `setPosition` clamps, `positionInBlock` counts QChars, which are the
    // UTF-16 code units the protocol counts
    return QJsonObject{{"line", c.blockNumber()},
                       {"character", c.positionInBlock()}};
  }  // end of LspProtocol::positionOf

  int LspProtocol::positionOf(const QTextDocument &d, int line, int character) {
    auto block = d.findBlockByNumber(line);
    if (!block.isValid()) {
      if (line < 0) {
        return 0;
      }
      block = d.lastBlock();
    }
    // the protocol clamps to the end of the line, not past the separator
    // which may follow it, nor onto the phantom separator `setPlainText`
    // leaves at the end of the document: what the line is made of, separators
    // and line breaks inside blocks aside
    return block.position() + qBound(0, character, block.text().length());
  }  // end of LspProtocol::positionOf

  QString LspProtocol::textOf(const QTextDocument &d) {
    QString text;
    for (auto b = d.begin(); b != d.end(); b = b.next()) {
      if (b != d.begin()) {
        text += '\n';
      }
      text += b.text();
    }
    return text;
  }  // end of LspProtocol::textOf

  QJsonObject LspProtocol::rangeInText(const QString &text,
                                       int start,
                                       int end) {
    auto position_at = [&text](int offset) {
      auto line = 0;
      auto line_start = 0;
      for (auto i = 0; i != offset; ++i) {
        if (text[i] == '\n') {
          ++line;
          line_start = i + 1;
        }
      }
      return QJsonObject{{"line", line}, {"character", offset - line_start}};
    };
    const auto start_offset = qBound(0, start, text.size());
    const auto end_offset = qBound(start_offset, end, text.size());
    const auto s = position_at(start_offset);
    const auto e = position_at(end_offset);
    return rangeOf(s.value("line").toInt(), s.value("character").toInt(),
                   e.value("line").toInt(), e.value("character").toInt());
  }  // end of LspProtocol::rangeInText

  QJsonObject LspProtocol::rangeOf(const int start_line,
                                   const int start_character,
                                   const int end_line,
                                   const int end_character) {
    return QJsonObject{
        {"start",
         QJsonObject{{"line", start_line}, {"character", start_character}}},
        {"end", QJsonObject{{"line", end_line}, {"character", end_character}}}};
  }  // end of LspProtocol::rangeOf

  QJsonObject LspProtocol::jsonOf(const LspDiagnostic &d) {
    auto diagnostic = QJsonObject{
        {"range",
         rangeOf(d.startLine, d.startCharacter, d.endLine, d.endCharacter)},
        {"severity", d.severity},
        {"message", d.message}};
    if (!d.source.isEmpty()) {
      diagnostic.insert(QStringLiteral("source"), d.source);
    }
    return diagnostic;
  }  // end of LspProtocol::jsonOf

  QMap<QString, QVector<LspTextEdit>> LspProtocol::editsOf(
      const QJsonValue &workspace_edit) {
    auto edits = QMap<QString, QVector<LspTextEdit>>();
    const auto add = [&edits](const QString &uri, const QJsonArray &list) {
      const auto file = LspProtocol::fileOfUri(uri);
      if (file.isEmpty()) {
        return;
      }
      auto &edits_of_the_file = edits[file];
      for (const auto &v : list) {
        const auto e = v.toObject();
        const auto range = e.value("range").toObject();
        const auto start = range.value("start").toObject();
        const auto end = range.value("end").toObject();
        auto edit = LspTextEdit();
        edit.startLine = start.value("line").toInt();
        edit.startCharacter = start.value("character").toInt();
        edit.endLine = end.value("line").toInt();
        edit.endCharacter = end.value("character").toInt();
        edit.text = e.value("newText").toString();
        edits_of_the_file.append(edit);
      }
    };
    const auto &edit = workspace_edit.toObject();
    const auto changes = edit.value("changes").toObject();
    for (auto it = changes.begin(); it != changes.end(); ++it) {
      add(it.key(), it.value().toArray());
    }
    // the richer shape, which carries whole document-syncing operations
    // alongside the edits; the ones this editor knows are the plain text
    // edits of a document the server names
    const auto document_changes = edit.value("documentChanges").toArray();
    for (const auto &v : document_changes) {
      const auto o = v.toObject();
      const auto text_document = o.value("textDocument").toObject();
      if (text_document.isEmpty()) {
        continue;
      }
      add(text_document.value("uri").toString(), o.value("edits").toArray());
    }
    return edits;
  }  // end of LspProtocol::editsOf

  namespace {

    /*!
     * \brief walk the tree of `DocumentSymbol`s a server answers with and
     * append its entries, in the order the file spells them, to the overview
     * \param[out] symbols: the overview, in document order
     * \param[in] nodes: the entries of a level of the tree
     * \param[in] scope: the names of the enclosing scopes, `::`-separated
     */
    void collect_document_symbols(QVector<LspDocumentSymbol> &symbols,
                                  const QJsonArray &nodes,
                                  const QString &scope) {
      for (const auto &v : nodes) {
        const auto o = v.toObject();
        auto symbol = LspDocumentSymbol();
        const auto name = o.value("name").toString();
        if (name.isEmpty()) {
          continue;
        }
        symbol.name = scope.isEmpty() ? name : scope + "::" + name;
        symbol.detail = o.value("detail").toString();
        symbol.kind = o.value("kind").toInt();
        // the selection range is the name itself; the range of the whole symbol
        // would point at the brace of a class or the keyword of a function
        const auto start =
            o.value("selectionRange").toObject().value("start").toObject();
        symbol.line = start.value("line").toInt();
        symbol.character = start.value("character").toInt();
        symbols.append(symbol);
        collect_document_symbols(symbols, o.value("children").toArray(),
                                 symbol.name);
      }
    }  // end of collect_document_symbols

  }  // end of anonymous namespace

  QVector<LspDocumentSymbol> documentSymbolsOf(const QJsonValue &result) {
    auto symbols = QVector<LspDocumentSymbol>();
    const auto entries = result.toArray();
    if (entries.isEmpty()) {
      return symbols;
    }
    // a server which does not answer with a tree lists the symbols with their
    // location and the name of their container: no detail, no nesting, but the
    // overview is built the same way
    if (!entries.at(0).toObject().contains("selectionRange")) {
      for (const auto &v : entries) {
        const auto o = v.toObject();
        auto symbol = LspDocumentSymbol();
        symbol.name = o.value("name").toString();
        if (symbol.name.isEmpty()) {
          continue;
        }
        const auto container = o.value("containerName").toString();
        if (!container.isEmpty()) {
          symbol.name = container + "::" + symbol.name;
        }
        symbol.kind = o.value("kind").toInt();
        const auto start = o.value("location")
                               .toObject()
                               .value("range")
                               .toObject()
                               .value("start")
                               .toObject();
        symbol.line = start.value("line").toInt();
        symbol.character = start.value("character").toInt();
        symbols.append(symbol);
      }
      return symbols;
    }
    collect_document_symbols(symbols, entries, QString());
    return symbols;
  }  // end of documentSymbolsOf

  /*!\brief the name of a `SymbolKind` of the protocol, as the overview of a
   * file names it
   *
   * \note the numbers are the ones of version 3.17 of the protocol, which
   * inserted `module` after `file` and moved everything above it; they are
   * also the ones `clangd` sends: a `class` answers `5`, a method `6`, a
   * static data member `7`, a field `8`, an enumerator `10` — the same number
   * as its own `enum`, whose `detail` is the name of the enumeration —, a
   * free function `12` and a global variable `13`.
   */
  QString symbolKindName(const int kind) {
    switch (kind) {
      case 1:
        return QString("file");
      case 2:
        return QString("module");
      case 3:
        return QString("namespace");
      case 4:
        return QString("package");
      case 5:
        return QString("class");
      case 6:
        return QString("method");
      case 7:
        return QString("field");
      case 8:
        return QString("field");
      case 9:
        return QString("constructor");
      case 10:
        return QString("enum");
      case 11:
        return QString("interface");
      case 12:
        return QString("function");
      case 13:
        return QString("variable");
      case 14:
        return QString("constant");
      case 15:
        return QString("string");
      case 16:
        return QString("number");
      case 17:
        return QString("boolean");
      case 18:
        return QString("array");
      case 19:
        return QString("object");
      case 20:
        return QString("key");
      case 21:
        return QString("null");
      case 22:
        return QString("enum member");
      case 23:
        return QString("struct");
      case 24:
        return QString("event");
      case 25:
        return QString("operator");
      case 26:
        return QString("type parameter");
    }
    return QString();
  }  // end of symbolKindName

  bool LspSemanticLegend::isValid() const {
    return !this->types.isEmpty();
  }  // end of LspSemanticLegend::isValid

  LspSemanticLegend semanticLegendOf(const QJsonValue &capabilities) {
    auto legend = LspSemanticLegend();
    const auto provider =
        capabilities.toObject().value("semanticTokensProvider");
    if (!provider.isObject()) {
      // a server which answers a plain `true`, or nothing at all, announces no
      // legend, and a legend is what the numbers of an answer are read against
      return legend;
    }
    const auto announced = provider.toObject().value("legend").toObject();
    for (const auto &v : announced.value("tokenTypes").toArray()) {
      legend.types.append(v.toString());
    }
    for (const auto &v : announced.value("tokenModifiers").toArray()) {
      legend.modifiers.append(v.toString());
    }
    return legend;
  }  // end of semanticLegendOf

  QVector<LspSemanticToken> semanticTokensOf(const QJsonValue &result,
                                             const LspSemanticLegend &legend) {
    auto tokens = QVector<LspSemanticToken>();
    // a server which saw nothing in the file answers `null`, or an object
    // naming no data; only the second shape can carry a whole file of tokens
    if (!result.isObject()) {
      return tokens;
    }
    const auto data = result.toObject().value("data").toArray();
    if (!legend.isValid()) {
      // the numbers of the answer name nothing without the legend: reading
      // them anyway would invent kinds the server never announced
      return tokens;
    }
    // the `relative` format the client asked for at the handshake, which is
    // what `clangd` speaks: five numbers per token, the line and the place
    // written as what they differ from those of the token before, so a run of
    // tokens on a line costs one line number and a column each
    auto line = 0;
    auto character = 0;
    for (auto i = 0; i + 4 < data.size(); i += 5) {
      const auto delta_line = data[i].toInt();
      const auto delta_character = data[i + 1].toInt();
      const auto length = data[i + 2].toInt();
      const auto kind = data[i + 3].toInt();
      const auto modifiers = data[i + 4].toInt();
      line += delta_line;
      // the place is relative to the previous token only on the same line; on
      // a new line it is the place itself, which is why a token following a
      // long one is not pushed to the right of its own line
      character =
          (delta_line != 0) ? delta_character : character + delta_character;
      auto token = LspSemanticToken();
      token.line = line;
      token.character = character;
      token.length = length;
      if ((kind >= 0) && (kind < legend.types.size())) {
        token.type = legend.types.at(kind);
      }
      // 31, not 32: the mask of the last bit of an `int` is `1 << 31`, which is
      // what the protocol says a modifier mask is made of, and shifting a 1 out
      // of the sign bit is not something to write
      for (auto bit = 0; (bit < legend.modifiers.size()) && (bit < 31); ++bit) {
        if ((modifiers & (1 << bit)) != 0) {
          token.modifiers.append(legend.modifiers.at(bit));
        }
      }
      tokens.append(token);
    }
    return tokens;
  }  // end of semanticTokensOf

  std::optional<LspProtocol::Change> LspProtocol::diffAgainst(
      const QString &old_text, const QString &new_text) {
    if (old_text == new_text) {
      return std::nullopt;
    }
    auto p = 0;
    const auto n = qMin(old_text.size(), new_text.size());
    while ((p != n) && (old_text[p] == new_text[p])) {
      ++p;
    }
    auto s = 0;
    while ((s != n - p) && (old_text[old_text.size() - 1 - s] ==
                            new_text[new_text.size() - 1 - s])) {
      ++s;
    }
    return Change{rangeInText(old_text, p, old_text.size() - s),
                  new_text.sliced(p, new_text.size() - p - s)};
  }  // end of LspProtocol::diffAgainst

  bool LspProtocol::boundedWait(
      const std::function<void(const std::function<void()> &)> &dispatch,
      int timeout_ms) {
    // the stop function outlives the wait whenever the request it waits for
    // is answered after the timeout: the callback the transport finally calls
    // holds a copy of it. Everything it touches therefore lives behind a
    // shared state, and the event loop it may quit is nulled out on the way
    // out, so a late stop does nothing instead of reopening a dead loop
    struct State {
      bool done = false;
      QEventLoop *loop = nullptr;
    };
    auto state = std::make_shared<State>();
    auto stop = [state] {
      state->done = true;
      if (state->loop != nullptr) {
        state->loop->quit();
      }
    };
    auto loop = QEventLoop();
    auto timer = QTimer();
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout_ms);
    state->loop = &loop;
    dispatch(stop);
    // the answer may have come before the loop could even start: the dispatch
    // is synchronous up to here, and a notification already queued on the
    // socket would be read by the loop below
    if (!state->done) {
      loop.exec();
    }
    timer.stop();
    state->loop = nullptr;
    return state->done;
  }  // end of LspProtocol::boundedWait

}  // end of namespace tfel::gui
