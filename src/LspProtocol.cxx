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
    const auto s = qBound(0, start, text.size());
    const auto e = qBound(s, end, text.size());
    return QJsonObject{{"start", position_at(s)}, {"end", position_at(e)}};
  }  // end of LspProtocol::rangeInText

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
