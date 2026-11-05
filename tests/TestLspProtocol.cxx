/*!
 * \file   tests/TestLspProtocol.cxx
 * \brief  tests of the pure helpers of the Language Server Protocol client:
 *         framing, URIs, positions, the incremental diff, the bounded wait,
 *         the answer to the question for a compilation database, the overview
 *         of a file, the legend and the answer of the semantic tokens.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
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
  void testDefaultCompileCommandsFile();
  void testSearchCompileCommandsFile();
  void testDocumentSymbolsOf();
  void testSemanticLegendOf();
  void testSemanticTokensOf();
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

namespace {

  // create a file, and the directories which hold it; `content` is small and
  // meaningless, the tests only care that the file exists
  bool makeFile(const QString &path, const QByteArray &content = "[]") {
    const auto info = QFileInfo(path);
    if (!QDir().mkpath(info.absolutePath())) {
      return false;
    }
    auto handle = QFile(path);
    if (!handle.open(QIODevice::WriteOnly)) {
      return false;
    }
    handle.write(content);
    handle.close();
    return true;
  }  // end of makeFile()

}  // end of namespace

void TestLspProtocol::testDefaultCompileCommandsFile() {
  using tfel::gui::LspCxxSupport;
  // what the editor holds before `--compile-commands-file` is parsed, or when
  // it is absent: nothing, which hands the choice of the database to the search
  LspCxxSupport::setDefaultCompileCommandsFile(QString());
  QVERIFY(LspCxxSupport::defaultCompileCommandsFile().isEmpty());
  // the value of the option, blank around it removed the way the command line
  // has no business carrying it in the first place
  LspCxxSupport::setDefaultCompileCommandsFile(
      "  /somewhere/compile_commands.json  ");
  QCOMPARE(LspCxxSupport::defaultCompileCommandsFile(),
           QStringLiteral("/somewhere/compile_commands.json"));
  LspCxxSupport::setDefaultCompileCommandsFile(QString());
  QVERIFY(LspCxxSupport::defaultCompileCommandsFile().isEmpty());
}  // end of TestLspProtocol::testDefaultCompileCommandsFile

void TestLspProtocol::testSearchCompileCommandsFile() {
  using tfel::gui::LspCxxSupport;
  auto root = QTemporaryDir();
  QVERIFY(root.isValid());
  const auto r = QDir::cleanPath(root.path());
  auto error = QString();
  // an empty place to start from is no place to search
  QVERIFY(LspCxxSupport::searchCompileCommandsFile(QString()).isEmpty());
  QVERIFY(LspCxxSupport::searchCompileCommandsFile("  ").isEmpty());
  QVERIFY(
      LspCxxSupport::searchCompileCommandsFile("/does/not/exist").isEmpty());
  // the database of the directory itself, and of the project the directory
  // lives in: `src` has no `CMakeLists.txt` of its own but its parent does, so
  // the parent is still part of the project and holds the database
  const auto proj = r + "/proj";
  QVERIFY(makeFile(proj + "/CMakeLists.txt", "project(p)\n"));
  QVERIFY(makeFile(proj + "/compile_commands.json"));
  const auto deep = proj + "/src/engine";
  QVERIFY(makeFile(proj + "/src/CMakeLists.txt", "add_subdirectory(engine)\n"));
  QVERIFY(makeFile(deep + "/cxx.cxx", "int main(){}\n"));
  QCOMPARE(LspCxxSupport::searchCompileCommandsFile(proj),
           proj + "/compile_commands.json");
  // several project directories up: the ascent follows the `CMakeLists.txt` of
  // `src` and `proj`, `engine` having none of its own
  QCOMPARE(LspCxxSupport::searchCompileCommandsFile(deep),
           proj + "/compile_commands.json");
  // the boundary of the project: without a `CMakeLists.txt` above, a database
  // one level up belongs to no project and is not the editor's business
  const auto iso = r + "/iso/one/two";
  QVERIFY(makeFile(r + "/iso/one/compile_commands.json"));
  QVERIFY(makeFile(iso + "/cxx.cxx", "int x;\n"));
  QVERIFY(LspCxxSupport::searchCompileCommandsFile(iso).isEmpty());
  // an in-source project with its build directory inside: the project ends at
  // the top `CMakeLists.txt`, and the `build` sub-directory of the last
  // project directory is searched before the search gives up
  const auto inner = r + "/inner";
  QVERIFY(makeFile(inner + "/CMakeLists.txt", "project(i)\n"));
  QVERIFY(makeFile(inner + "/build/compile_commands.json"));
  QVERIFY(makeFile(inner + "/src/cxx.cxx", "int y;\n"));
  QCOMPARE(LspCxxSupport::searchCompileCommandsFile(inner + "/src"),
           inner + "/build/compile_commands.json");
  // an out-of-source build next to the project: the parent of the project has
  // no `CMakeLists.txt`, is no more part of the source tree, and its own
  // `build*` sub-directories are the last place looked into
  const auto par = r + "/par";
  QVERIFY(makeFile(par + "/prj/CMakeLists.txt", "project(o)\n"));
  QVERIFY(makeFile(par + "/build-tfel/compile_commands.json"));
  QVERIFY(makeFile(par + "/prj/src/cxx.cxx", "int z;\n"));
  QCOMPARE(LspCxxSupport::searchCompileCommandsFile(par + "/prj/src"),
           par + "/build-tfel/compile_commands.json");
  // when several build directories coexist, the one named `build` wins over
  // the others, whatever its place in the name order; take its database away
  // and `build` has nothing to offer, the name order of the remaining
  // candidates takes over
  const auto pri = r + "/pri";
  QVERIFY(makeFile(pri + "/CMakeLists.txt", "project(m)\n"));
  QVERIFY(makeFile(pri + "/build-z/compile_commands.json"));
  QVERIFY(makeFile(pri + "/build-a/compile_commands.json"));
  QVERIFY(makeFile(pri + "/build/compile_commands.json"));
  QCOMPARE(LspCxxSupport::searchCompileCommandsFile(pri),
           pri + "/build/compile_commands.json");
  QVERIFY(QFile(pri + "/build/compile_commands.json").remove());
  QCOMPARE(LspCxxSupport::searchCompileCommandsFile(pri),
           pri + "/build-a/compile_commands.json");
  // a project with no database at all, wherever it was built
  const auto bare = r + "/bare";
  QVERIFY(makeFile(bare + "/CMakeLists.txt", "project(b)\n"));
  QVERIFY(makeFile(bare + "/src/cxx.cxx", "int w;\n"));
  QVERIFY(LspCxxSupport::searchCompileCommandsFile(bare + "/src").isEmpty());
  // and whatever the answer is, the same option as an explicit answer names
  const auto found = LspCxxSupport::searchCompileCommandsFile(deep);
  QVERIFY(!found.isEmpty());
  QCOMPARE(LspCxxSupport::compileCommandsArguments(found, error),
           QStringList{QStringLiteral("--compile-commands-dir=") + proj});
  QVERIFY(error.isEmpty());
}  // end of TestLspProtocol::testSearchCompileCommandsFile

void TestLspProtocol::testDocumentSymbolsOf() {
  // the two shapes a `textDocument/documentSymbol` answer takes, and the one
  // overview both are turned into: the symbols in the order the file spells
  // them, each named by the scopes it lives in
  const auto range = [](const int line, const int character) {
    return QJsonObject{
        {"start", QJsonObject{{"line", line}, {"character", character}}},
        {"end", QJsonObject{{"line", line}, {"character", character + 1}}}};
  };
  const auto tree = QJsonArray{
      QJsonObject{{"name", QStringLiteral("Widget")},
                  {"detail", QStringLiteral("struct")},
                  {"kind", 5},
                  {"range", range(0, 0)},
                  {"selectionRange", range(0, 7)},
                  {"children", QJsonArray{QJsonObject{
                                   {"name", QStringLiteral("paint")},
                                   {"detail", QStringLiteral("void (int)")},
                                   {"kind", 6},
                                   {"range", range(1, 2)},
                                   {"selectionRange", range(1, 7)}}}}},
      QJsonObject{{"name", QStringLiteral("Widget::paint")},
                  {"detail", QStringLiteral("void (int)")},
                  {"kind", 6},
                  {"range", range(4, 0)},
                  {"selectionRange", range(4, 13)}}};
  const auto symbols = tfel::gui::documentSymbolsOf(tree);
  QCOMPARE(symbols.size(), 3);
  // the depth-first walk of the tree: a child is named after the scope it is
  // written in, and the definition out of that scope keeps the name the file
  // spells it with, which is the same one, twice and in the right order
  QCOMPARE(symbols.at(0).name, QStringLiteral("Widget"));
  QCOMPARE(symbols.at(0).detail, QStringLiteral("struct"));
  QCOMPARE(symbols.at(0).line, 0);
  QCOMPARE(symbols.at(0).character, 7);
  QCOMPARE(symbols.at(1).name, QStringLiteral("Widget::paint"));
  QCOMPARE(symbols.at(1).kind, 6);
  QCOMPARE(symbols.at(2).name, QStringLiteral("Widget::paint"));
  QCOMPARE(symbols.at(2).line, 4);
  QCOMPARE(symbols.at(2).character, 13);
  // the same file for a server which was not asked to answer with a tree: the
  // entries are flat, the container is named instead of nesting, and nothing
  // at all is said beside the name
  const auto flat = QJsonArray{
      QJsonObject{
          {"name", QStringLiteral("Widget")},
          {"kind", 5},
          {"containerName", QStringLiteral("")},
          {"location", QJsonObject{{"uri", QStringLiteral("file:///w.cxx")},
                                   {"range", range(0, 0)}}}},
      QJsonObject{
          {"name", QStringLiteral("paint")},
          {"kind", 6},
          {"containerName", QStringLiteral("Widget")},
          {"location", QJsonObject{{"uri", QStringLiteral("file:///w.cxx")},
                                   {"range", range(1, 2)}}}}};
  const auto flattened = tfel::gui::documentSymbolsOf(flat);
  QCOMPARE(flattened.size(), 2);
  QCOMPARE(flattened.at(0).name, QStringLiteral("Widget"));
  QVERIFY(flattened.at(0).detail.isEmpty());
  QCOMPARE(flattened.at(1).name, QStringLiteral("Widget::paint"));
  // the name of a symbol is where its name is written, not where its definition
  // begins: the selection range of a class starts at its `class` keyword
  QCOMPARE(flattened.at(1).line, 1);
  QCOMPARE(flattened.at(1).character, 2);
  // an answer which is nothing at all, and an answer which holds entries with
  // no name, are an overview with no entry
  QVERIFY(tfel::gui::documentSymbolsOf(QJsonValue()).isEmpty());
  QVERIFY(tfel::gui::documentSymbolsOf(QJsonArray{QJsonObject{{"kind", 6}}})
              .isEmpty());
  // the kinds the overview names a symbol by, in the numbering the servers of
  // today send: a `class` answers 5, not the 4 of the version of the protocol
  // which had no `module` before it
  QCOMPARE(tfel::gui::symbolKindName(3), QStringLiteral("namespace"));
  QCOMPARE(tfel::gui::symbolKindName(5), QStringLiteral("class"));
  QCOMPARE(tfel::gui::symbolKindName(6), QStringLiteral("method"));
  QCOMPARE(tfel::gui::symbolKindName(12), QStringLiteral("function"));
  QCOMPARE(tfel::gui::symbolKindName(13), QStringLiteral("variable"));
  QVERIFY(tfel::gui::symbolKindName(0).isEmpty());
}  // end of TestLspProtocol::testDocumentSymbolsOf

void TestLspProtocol::testSemanticLegendOf() {
  // the legend is what the numbers of an answer are read against, so a server
  // which announces none is a server which offers no semantic tokens at all
  const auto capabilities = QJsonObject{
      {"semanticTokensProvider",
       QJsonObject{
           {"full", QJsonObject{{"delta", true}}},
           {"range", false},
           {"legend",
            QJsonObject{
                {"tokenTypes",
                 QJsonArray{QStringLiteral("variable"),
                            QStringLiteral("variable"), QStringLiteral("class"),
                            QStringLiteral("function"),
                            QStringLiteral("macro")}},
                {"tokenModifiers", QJsonArray{QStringLiteral("declaration"),
                                              QStringLiteral("deprecated"),
                                              QStringLiteral("deduced"),
                                              QStringLiteral("readonly")}}}}}}};
  const auto legend = tfel::gui::semanticLegendOf(capabilities);
  QVERIFY(legend.isValid());
  // the names of the server, in its own order, duplicates and all: `variable`
  // is the name of two kinds of its legend, which is why reading a token goes
  // by the number, never by the name
  QCOMPARE(legend.types.size(), 5);
  QCOMPARE(legend.types.at(0), QStringLiteral("variable"));
  QCOMPARE(legend.types.at(1), QStringLiteral("variable"));
  QCOMPARE(legend.types.at(4), QStringLiteral("macro"));
  QCOMPARE(
      legend.modifiers,
      (QStringList{QStringLiteral("declaration"), QStringLiteral("deprecated"),
                   QStringLiteral("deduced"), QStringLiteral("readonly")}));
  // what servers which offer nothing answer: no capabilities at all, a provider
  // which is a plain `true`, or a provider with no legend; all three are the
  // absence of the question, since an answer of numbers without a legend to
  // read them by says nothing
  QVERIFY(!tfel::gui::semanticLegendOf(QJsonObject{}).isValid());
  QVERIFY(!tfel::gui::semanticLegendOf(
               QJsonObject{{"semanticTokensProvider", QJsonValue(true)}})
               .isValid());
  QVERIFY(
      !tfel::gui::semanticLegendOf(
           QJsonObject{{"semanticTokensProvider",
                        QJsonObject{{"full", QJsonObject{{"delta", false}}}}}})
           .isValid());
}  // end of TestLspProtocol::testSemanticLegendOf

void TestLspProtocol::testSemanticTokensOf() {
  const auto legend = [this] {
    auto l = tfel::gui::LspSemanticLegend();
    l.types = QStringList{QStringLiteral("variable"),
                          QStringLiteral("variable"), QStringLiteral("class"),
                          QStringLiteral("function"), QStringLiteral("macro")};
    l.modifiers =
        QStringList{QStringLiteral("declaration"), QStringLiteral("deprecated"),
                    QStringLiteral("deduced"), QStringLiteral("readonly")};
    return l;
  }();
  // `clangd` answers five numbers per token, the line and the place of the name
  // written as what they differ from those of the previous token: two tokens on
  // the first line and one on the third, the last of them deprecated and
  // readonly, which are the bits 1 and 3 of its mask. On a new line the place
  // is not a difference but the place itself, which is why the first token of a
  // line is answered with a place of 0
  const auto data = QJsonArray{0, 4, 3, 0, 0,    // line 0, character 4
                               0, 6, 4, 3, 0,    // line 0, character 10
                               2, 0, 5, 4, 10};  // line 2, character 0
  const auto tokens =
      tfel::gui::semanticTokensOf(QJsonObject{{"data", data}}, legend);
  QCOMPARE(tokens.size(), 3);
  QCOMPARE(tokens.at(0).line, 0);
  QCOMPARE(tokens.at(0).character, 4);
  QCOMPARE(tokens.at(0).length, 3);
  QCOMPARE(tokens.at(0).type, QStringLiteral("variable"));
  QVERIFY(tokens.at(0).modifiers.isEmpty());
  // the place of the second token is counted from the place of the first, since
  // both are on the same line; the line number of the third starts over from
  // nothing, which is why a negative difference is not one
  QCOMPARE(tokens.at(1).character, 10);
  QCOMPARE(tokens.at(1).type, QStringLiteral("function"));
  QCOMPARE(tokens.at(2).line, 2);
  QCOMPARE(tokens.at(2).character, 0);
  QCOMPARE(tokens.at(2).type, QStringLiteral("macro"));
  QCOMPARE(tokens.at(2).modifiers, (QStringList{QStringLiteral("deprecated"),
                                                QStringLiteral("readonly")}));
  // the two names of the legend which are both `variable` read as the same
  // word: the distinction is lost on purpose, the face a name gets is keyed on
  // the word the server says, not on which of its kinds it was
  QCOMPARE(tokens.at(0).type, legend.types.at(1));
  // a kind number the legend does not name, and a modifier bit past the list of
  // the modifiers: neither invents a name the server never announced
  const auto beyond = QJsonArray{0, 0, 2, 99, 1 << 20};
  const auto unnamed =
      tfel::gui::semanticTokensOf(QJsonObject{{"data", beyond}}, legend);
  QCOMPARE(unnamed.size(), 1);
  QVERIFY(unnamed.at(0).type.isEmpty());
  QVERIFY(unnamed.at(0).modifiers.isEmpty());
  // a truncated answer — the protocol says five numbers per token, and a server
  // which sent 18 of them said nothing about the last token — and an answer
  // which names no data or is `null`: both are no tokens
  const auto truncated = tfel::gui::semanticTokensOf(
      QJsonObject{{"data", QJsonArray{0, 0, 1, 0, 0, 0, 0, 1, 0}}}, legend);
  QCOMPARE(truncated.size(), 1);
  QCOMPARE(tfel::gui::semanticTokensOf(QJsonObject{}, legend).size(), 0);
  QCOMPARE(tfel::gui::semanticTokensOf(QJsonValue(), legend).size(), 0);
  // and an answer of numbers without the legend to read them by is an answer
  // which says nothing: reading it anyway would name kinds the server never
  // offered
  QCOMPARE(tfel::gui::semanticTokensOf(QJsonObject{{"data", data}},
                                       tfel::gui::LspSemanticLegend())
               .size(),
           0);
}  // end of TestLspProtocol::testSemanticTokensOf

QTEST_GUILESS_MAIN(TestLspProtocol)

#include "TestLspProtocol.moc"
