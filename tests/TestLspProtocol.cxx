/*!
 * \file   tests/TestLspProtocol.cxx
 * \brief  tests of the pure helpers of the Language Server Protocol client:
 *         framing, URIs, positions, the incremental diff, the bounded wait,
 *         the answer to the question for a compilation database.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <chrono>

#include "TFEL/GUI/LspCxxSupport.hxx"
#include "TFEL/GUI/LspProtocol.hxx"

// everything tested here is pure: no server, no process, no window system,
// only a `QTextDocument` of `QtGui` and a temporary directory of files
struct TestLspProtocol : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void testEncode();
  void testDecodeRoundTrip();
  void testDecodeChunked();
  void testDecodeGlued();
  void testDecodeKeepsPartialTail();
  void testDecodeDropsNoise();
  void testUriRoundTrip();
  void testPositionOfDocumentPosition();
  void testPositionOfProtocolPosition();
  void testTextOf();
  void testRangeInText();
  void testDiffAgainst();
  void testBoundedWait();
  void testCompileCommandsArguments();
};

void TestLspProtocol::initTestCase() {
  // the settings of the application under test are none of these tests'
  // business, and `LspProtocol` reads none anyway
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-lsp-protocol");
}  // end of TestLspProtocol::initTestCase

void TestLspProtocol::testEncode() {
  const auto framed =
      tfel::gui::LspProtocol::encode(QJsonObject{{"id", 1}, {"method", "foo"}});
  const auto body = QJsonDocument(QJsonObject{{"id", 1}, {"method", "foo"}})
                        .toJson(QJsonDocument::Compact);
  QVERIFY(framed.startsWith("Content-Length: "));
  // the length counts the bytes of the body, not the characters of the text
  QVERIFY(framed.contains(QByteArray::number(body.size())));
  // the headers end, and the body follows, whole
  QVERIFY(framed.contains("\r\n\r\n"));
  QCOMPARE(framed.mid(framed.indexOf("\r\n\r\n") + 4), body);
}  // end of TestLspProtocol::testEncode

void TestLspProtocol::testDecodeRoundTrip() {
  const auto message = QJsonObject{
      {"jsonrpc", "2.0"}, {"id", 7}, {"method", "textDocument/hover"}};
  auto buffer = tfel::gui::LspProtocol::encode(message);
  const auto messages = tfel::gui::LspProtocol::decode(buffer);
  QCOMPARE(messages.size(), 1);
  QCOMPARE(messages.at(0), message);
  // a stream which holds nothing but complete messages comes out empty
  QVERIFY(buffer.isEmpty());
}  // end of TestLspProtocol::testDecodeRoundTrip

void TestLspProtocol::testDecodeChunked() {
  const auto message = QJsonObject{{"id", 1}, {"result", "a payload"}};
  const auto framed = tfel::gui::LspProtocol::encode(message);
  // a server writes when it wants: a message may arrive by any pieces, the
  // header cut short, the body cut short, both whole
  for (int cut = 1; cut != framed.size(); ++cut) {
    auto buffer = framed.left(cut);
    QVERIFY2(
        tfel::gui::LspProtocol::decode(buffer).isEmpty(),
        qPrintable(QStringLiteral("a partial frame decoded at %1").arg(cut)));
    buffer += framed.mid(cut);
    const auto messages = tfel::gui::LspProtocol::decode(buffer);
    QCOMPARE(messages.size(), 1);
    QCOMPARE(messages.at(0), message);
    QVERIFY(buffer.isEmpty());
  }
}  // end of TestLspProtocol::testDecodeChunked

void TestLspProtocol::testDecodeGlued() {
  // two servers' worth of traffic in a single read: both messages come out,
  // in order
  auto buffer = tfel::gui::LspProtocol::encode(QJsonObject{{"id", 1}});
  buffer += tfel::gui::LspProtocol::encode(QJsonObject{{"id", 2}});
  const auto messages = tfel::gui::LspProtocol::decode(buffer);
  QCOMPARE(messages.size(), 2);
  QCOMPARE(messages.at(0).value("id").toInt(), 1);
  QCOMPARE(messages.at(1).value("id").toInt(), 2);
}  // end of TestLspProtocol::testDecodeGlued

void TestLspProtocol::testDecodeKeepsPartialTail() {
  const auto first = tfel::gui::LspProtocol::encode(QJsonObject{{"id", 1}});
  auto buffer = first;
  buffer += tfel::gui::LspProtocol::encode(QJsonObject{{"id", 2}}).left(10);
  const auto messages = tfel::gui::LspProtocol::decode(buffer);
  // the complete message is decoded, the partial one is left in the buffer
  // for the next read, neither lost nor corrupted
  QCOMPARE(messages.size(), 1);
  QCOMPARE(messages.at(0).value("id").toInt(), 1);
  auto complete =
      buffer + tfel::gui::LspProtocol::encode(QJsonObject{{"id", 2}}).mid(10);
  const auto next = tfel::gui::LspProtocol::decode(complete);
  QCOMPARE(next.size(), 1);
  QCOMPARE(next.at(0).value("id").toInt(), 2);
}  // end of TestLspProtocol::testDecodeKeepsPartialTail

void TestLspProtocol::testDecodeDropsNoise() {
  // a framed block whose headers hold no length, or which holds no json
  // object, is noise: dropped, not returned, and not fatal to what follows
  auto buffer = QByteArray("X-Other: 3\r\n\r\njunk");
  buffer += tfel::gui::LspProtocol::encode(QJsonObject{{"id", 3}});
  auto messages = tfel::gui::LspProtocol::decode(buffer);
  QVERIFY(messages.size() >= 1);
  QCOMPARE(messages.last().value("id").toInt(), 3);
  auto not_an_object = QByteArray("Content-Length: 2\r\n\r\n[]");
  not_an_object += tfel::gui::LspProtocol::encode(QJsonObject{{"id", 4}});
  messages = tfel::gui::LspProtocol::decode(not_an_object);
  QCOMPARE(messages.size(), 1);
  QCOMPARE(messages.at(0).value("id").toInt(), 4);
}  // end of TestLspProtocol::testDecodeDropsNoise

void TestLspProtocol::testUriRoundTrip() {
  for (const auto &file :
       {QStringLiteral("/tmp/a.cxx"), QStringLiteral("/tmp/a b/c++_ü.cxx"),
        QStringLiteral("/tmp/目录/x.hxx")}) {
    const auto uri = tfel::gui::LspProtocol::uriOf(file);
    QVERIFY2(uri.startsWith("file:"), qPrintable(uri));
    QCOMPARE(tfel::gui::LspProtocol::fileOfUri(uri), file);
  }
  // a relative name gets an absolute uri: the server and the editor must
  // speak of the same file the same way
  QVERIFY(tfel::gui::LspProtocol::uriOf("a.cxx").startsWith("file:///"));
}  // end of TestLspProtocol::testUriRoundTrip

void TestLspProtocol::testPositionOfDocumentPosition() {
  QTextDocument doc;
  // U+1F600 is a surrogate pair, two QChars, which is what the protocol
  // counts: the column of `b` is 3, not 2
  doc.setPlainText("a\U0001F600b\ncd");
  auto position = tfel::gui::LspProtocol::positionOf(doc, 0);
  QCOMPARE(position.value("line").toInt(), 0);
  QCOMPARE(position.value("character").toInt(), 0);
  position = tfel::gui::LspProtocol::positionOf(doc, 3);
  QCOMPARE(position.value("line").toInt(), 0);
  QCOMPARE(position.value("character").toInt(), 3);
  // the second line counts from its own start
  position = tfel::gui::LspProtocol::positionOf(doc, 6);
  QCOMPARE(position.value("line").toInt(), 1);
  QCOMPARE(position.value("character").toInt(), 1);
}  // end of TestLspProtocol::testPositionOfDocumentPosition

void TestLspProtocol::testPositionOfProtocolPosition() {
  QTextDocument doc;
  doc.setPlainText("abcdef\nxy");
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, 0, 0), 0);
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, 0, 6), 6);
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, 1, 1), 8);
  // a server which has not seen the latest change may answer with a position
  // the document has grown past: it is clamped to the end of the line, never
  // onto the separator behind it, never out of the document
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, 1, 42), 9);
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, 12, 0), 7);
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, 0, -3), 0);
  QCOMPARE(tfel::gui::LspProtocol::positionOf(doc, -1, 0), 0);
  // the round trip: what the first overload gives, the second one takes back
  for (int p = 0; p != doc.characterCount(); ++p) {
    const auto position = tfel::gui::LspProtocol::positionOf(doc, p);
    QCOMPARE(
        tfel::gui::LspProtocol::positionOf(doc, position.value("line").toInt(),
                                           position.value("character").toInt()),
        p);
  }
}  // end of TestLspProtocol::testPositionOfProtocolPosition

void TestLspProtocol::testTextOf() {
  QTextDocument doc;
  doc.setPlainText("ab\ncd\n\nef");
  // the paragraph separators of the document come back as line feeds, the
  // empty line included; `setPlainText` keeps no trace beyond the blocks
  QCOMPARE(tfel::gui::LspProtocol::textOf(doc), QString("ab\ncd\n\nef"));
  // a line separator, U+2028, inserted inside a block is not a line of its
  // own for the protocol, and `textOf` must not turn it into one, or every
  // offset behind it would move: this is why `toRawText` can not be used
  auto cursor = QTextCursor(&doc);
  cursor.setPosition(2);
  cursor.insertText(QString(QChar(0x2028)));
  const auto text = tfel::gui::LspProtocol::textOf(doc);
  QVERIFY(text.contains(QChar(0x2028)));
  QCOMPARE(text, QString("ab") + QChar(0x2028) + QString("\ncd\n\nef"));
}  // end of TestLspProtocol::testTextOf

void TestLspProtocol::testRangeInText() {
  const auto text = QStringLiteral("abcdef\nxy\nlast");
  auto range = tfel::gui::LspProtocol::rangeInText(text, 0, 3);
  QCOMPARE(range.value("start").toObject().value("line").toInt(), 0);
  QCOMPARE(range.value("start").toObject().value("character").toInt(), 0);
  QCOMPARE(range.value("end").toObject().value("line").toInt(), 0);
  QCOMPARE(range.value("end").toObject().value("character").toInt(), 3);
  // a range which spans lines: the column counts from the line it is on
  range = tfel::gui::LspProtocol::rangeInText(text, 3, 9);
  QCOMPARE(range.value("start").toObject().value("character").toInt(), 3);
  QCOMPARE(range.value("end").toObject().value("line").toInt(), 1);
  // offset 9 is the second line feed: the end of the line `xy`, one past its
  // last character
  QCOMPARE(range.value("end").toObject().value("character").toInt(), 2);
  // out of bounds offsets are clamped, the end never precedes the start
  range = tfel::gui::LspProtocol::rangeInText(text, -5, 1000);
  QCOMPARE(range.value("start").toObject().value("character").toInt(), 0);
  QCOMPARE(tfel::gui::LspProtocol::rangeInText(text, 7, 4)
               .value("end")
               .toObject()
               .value("character")
               .toInt(),
           0);
}  // end of TestLspProtocol::testRangeInText

void TestLspProtocol::testDiffAgainst() {
  // no change, no diff
  QVERIFY(!tfel::gui::LspProtocol::diffAgainst("same", "same").has_value());
  // an insertion in the middle: the smallest range, which here holds nothing
  const auto inserted =
      tfel::gui::LspProtocol::diffAgainst("hello world", "hello big world");
  QVERIFY(inserted.has_value());
  QCOMPARE(inserted->text, QString("big "));
  QCOMPARE(inserted->range.value("start").toObject().value("line").toInt(), 0);
  QCOMPARE(inserted->range.value("start").toObject().value("character").toInt(),
           6);
  QCOMPARE(inserted->range.value("end").toObject().value("character").toInt(),
           6);
  // a deletion: a non empty range of the old text, an empty new text; the
  // `()` of `int main() {}` is what goes away
  const auto removed =
      tfel::gui::LspProtocol::diffAgainst("int main() {}", "int main {}");
  QVERIFY(removed.has_value());
  QCOMPARE(removed->text, QString());
  QCOMPARE(removed->range.value("start").toObject().value("character").toInt(),
           8);
  QCOMPARE(removed->range.value("end").toObject().value("character").toInt(),
           10);
  // a change which spans lines
  const auto spanning =
      tfel::gui::LspProtocol::diffAgainst("one\ntwo\nthree", "one\n2\nthree");
  QVERIFY(spanning.has_value());
  QCOMPARE(spanning->range.value("start").toObject().value("line").toInt(), 1);
  QCOMPARE(spanning->text, QString("2"));
  // a whole new text: the range covers everything, the text is everything
  const auto replaced = tfel::gui::LspProtocol::diffAgainst("aaa", "bbb");
  QVERIFY(replaced.has_value());
  QCOMPARE(replaced->text, QString("bbb"));
  // the diff is what the server applies to the snapshot: replaying it here
  // must give the new text back, which is the whole point of the exercise
  auto replay = QStringLiteral("hello world");
  const auto change =
      tfel::gui::LspProtocol::diffAgainst(replay, "hello big world");
  replay.replace(6, 0, change->text);
  QCOMPARE(replay, QString("hello big world"));
}  // end of TestLspProtocol::testDiffAgainst

void TestLspProtocol::testBoundedWait() {
  // a stop called before the loop even starts: the wait returns true without
  // ever blocking
  auto started = QElapsedTimer();
  started.start();
  QVERIFY(tfel::gui::LspProtocol::boundedWait(
      [](const std::function<void()> &stop) { stop(); }, 5000));
  QVERIFY(started.elapsed() < 1000);
  // nobody ever answers: the timeout expires, the wait gives up
  auto elapsed = QElapsedTimer();
  elapsed.start();
  QVERIFY(!tfel::gui::LspProtocol::boundedWait(
      [](const std::function<void()> &) {}, 100));
  // a grace on the timer: what matters is that the wait ended, not that Qt
  // hits the millisecond
  QVERIFY(elapsed.elapsed() >= 90);
  QVERIFY(elapsed.elapsed() < 5000);
  // an answer which comes from the event loop, while waiting
  QVERIFY(tfel::gui::LspProtocol::boundedWait(
      [](const std::function<void()> &stop) {
        QTimer::singleShot(20, [stop] { stop(); });
      },
      5000));
  // the stop function may outlive the wait: a callback holding it, called
  // after the timeout, must do nothing, not reopen a dead loop, not crash
  auto late = std::function<void()>();
  QVERIFY(!tfel::gui::LspProtocol::boundedWait(
      [&late](const std::function<void()> &stop) { late = stop; }, 50));
  late();
  late = nullptr;
}  // end of TestLspProtocol::testBoundedWait

void TestLspProtocol::testCompileCommandsArguments() {
  // `clangd` is told which directory to look a compilation database in, which
  // is the whole trick of turning the answer of a file picker into an option
  auto directory = QTemporaryDir();
  QVERIFY(directory.isValid());
  const auto database =
      QDir(directory.path()).filePath("compile_commands.json");
  {
    auto handle = QFile(database);
    QVERIFY(handle.open(QIODevice::WriteOnly));
    handle.write("[]");
    handle.close();
  }
  auto error = QString();
  // a directory is taken as it stands
  QCOMPARE(tfel::gui::LspCxxSupport::compileCommandsArguments(directory.path(),
                                                              error),
           QStringList{QStringLiteral("--compile-commands-dir=") +
                       directory.path()});
  QVERIFY(error.isEmpty());
  // a file is only the directory it lives in
  QCOMPARE(tfel::gui::LspCxxSupport::compileCommandsArguments(database, error),
           QStringList{QStringLiteral("--compile-commands-dir=") +
                       directory.path()});
  QVERIFY(error.isEmpty());
  // an empty answer is the plain session: the server searches as it likes
  QVERIFY(tfel::gui::LspCxxSupport::compileCommandsArguments(QStringLiteral(""),
                                                             error)
              .isEmpty());
  QVERIFY(error.isEmpty());
  QVERIFY(tfel::gui::LspCxxSupport::compileCommandsArguments("   ", error)
              .isEmpty());
  QVERIFY(error.isEmpty());
  // an answer which points nowhere is refused, and says so: a session which
  // is launched with a database the user does not have would leave them
  // looking for why the answers are wrong
  QVERIFY(tfel::gui::LspCxxSupport::compileCommandsArguments(
              QStringLiteral("/does/not/exist/compile_commands.json"), error)
              .isEmpty());
  QVERIFY(!error.isEmpty());
  // the answer is made absolute: the server does not start in the directory of
  // the buffer, and a relative answer would mean nothing to it
  const auto relative = QDir(QDir::currentPath()).relativeFilePath(database);
  QCOMPARE(tfel::gui::LspCxxSupport::compileCommandsArguments(relative, error),
           QStringList{QStringLiteral("--compile-commands-dir=") +
                       directory.path()});
  QVERIFY(error.isEmpty());
}  // end of TestLspProtocol::testCompileCommandsArguments

QTEST_GUILESS_MAIN(TestLspProtocol)

#include "TestLspProtocol.moc"
