/*!
 * \file   tests/TestLineSeparators.cxx
 * \brief  tests of the invariant that the text of a buffer of the editor holds
 *         no Unicode line separator: a `QTextDocument` keeps such a character
 *         inside a block while the layout breaks the displayed line on it, so
 *         that the line is drawn on two rows, is highlighted on both, and that
 *         `Ctrl-E` does not reach the end of.
 * \author Thomas Helfer
 * \date   09/11/2026
 */

#include <memory>
#include <QtCore/QFile>
#include <QtCore/QMimeData>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtTest/QtTest>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QClipboard>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>
#include <QtGui/QTextLayout>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MultiLineLineEdit.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/Utilities.hxx"

namespace {

  //! the U+2028 LINE SEPARATOR, invisible on the screen
  const QChar line_separator = QChar(0x2028);

  //! the U+0085 NEXT LINE character
  const QChar next_line = QChar(0x0085);

  // write the file as any program outside the editor would, through a handle
  // the editor knows nothing about
  void writeOutside(const QString &file, const QString &content) {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::WriteOnly | QIODevice::Truncate));
    handle.write(content.toUtf8());
    handle.close();
  }  // end of writeOutside()

  // the text of the document, one entry per block: what a block holds is the
  // question, since a character which is not the block separator of the
  // document may still break the displayed line
  QStringList blocksOf(const QTextDocument *const doc) {
    auto l = QStringList();
    for (auto b = doc->begin(); b != doc->end(); b = b.next()) {
      l << b.text();
    }
    return l;
  }  // end of blocksOf()

  // the number of rows a block is drawn on: the document and the layout
  // disagreeing on that is the whole defect
  int rowsOf(QTextDocument *const doc, const int line) {
    // asking for the size of the document is what makes the layout lay the
    // document out, which the document of a widget nobody painted yet has not
    // done
    doc->documentLayout()->documentSize();
    const auto b = doc->findBlockByNumber(line);
    if (!b.isValid()) {
      return -1;
    }
    auto *const l = b.layout();
    return (l != nullptr) ? l->lineCount() : -1;
  }  // end of rowsOf()

  // an entry is never shown as a window of its own in the application, and a
  // window manager has its own opinion about the height of one: the host plays
  // the role of the minibuffer
  struct Host : QWidget {
    explicit Host(QWidget *const w) {
      auto *const l = new QVBoxLayout(this);
      l->setContentsMargins(0, 0, 0, 0);
      l->addWidget(w);
      resize(1000, 900);
      show();
    }
  };  // end of struct Host

}  // end of anonymous namespace

//! tests of the line separators a text may carry into a document
struct TestLineSeparators : QObject {
  Q_OBJECT

 private slots:

  void initTestCase();

  void theHelperTurnsTheSeparatorsIntoNewLines();

  void openingAFileTurnsALineSeparatorIntoANewLine();

  void ctrlEReachesTheEndOfTheCurrentLine();

  void pastingTextCarryingALineSeparatorIsNormalized();

  void theMinibufferInputKeepsNoLineSeparator();

 private:
  void pump(int n = 4);

  QTemporaryDir directory;
  std::unique_ptr<tfel::gui::EditorWidget> editor;

  // write the file as any program outside the editor would, and open it
  QString openFile(const QString &name, const QString &content);
  tfel::gui::PlainTextEdit &mainFrame();
};

void TestLineSeparators::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-line-separators");
  auto settings = QSettings();
  settings.remove("positions in files");
  settings.sync();
  QVERIFY(directory.isValid());
}  // end of TestLineSeparators::initTestCase

void TestLineSeparators::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestLineSeparators::pump

QString TestLineSeparators::openFile(const QString &name,
                                     const QString &content) {
  const auto file = directory.filePath(name);
  writeOutside(file, content);
  if (editor == nullptr) {
    editor = std::make_unique<tfel::gui::EditorWidget>();
    editor->show();
    pump();
  }
  editor->openFile(file);
  pump();
  return file;
}  // end of TestLineSeparators::openFile

tfel::gui::PlainTextEdit &TestLineSeparators::mainFrame() {
  return editor->getCurrentBuffer().getMainFrame();
}  // end of TestLineSeparators::mainFrame

void TestLineSeparators::theHelperTurnsTheSeparatorsIntoNewLines() {
  // an ordinary text is left alone, the same string back
  const auto plain = QStringLiteral("nothing\nto change\n");
  QVERIFY(tfel::gui::normalizeLineSeparators(plain) == plain);
  QCOMPARE(
      tfel::gui::normalizeLineSeparators("a" + QString(line_separator) + "b"),
      QString("a\nb"));
  QCOMPARE(tfel::gui::normalizeLineSeparators("a" + QString(next_line) + "b"),
           QString("a\nb"));
  // the U+2029 PARAGRAPH SEPARATOR is the one separator Qt does turn into the
  // end of a block on its own: it is a line break already
  QCOMPARE(
      tfel::gui::normalizeLineSeparators("a" + QString(QChar(0x2029)) + "b"),
      QString("a" + QString(QChar(0x2029)) + "b"));
  // a separator at the very end of a line is what the defect is about: it
  // breaks the displayed line of that very line, it does not add a character
  // to it
  QCOMPARE(tfel::gui::normalizeLineSeparators("text" + QString(line_separator)),
           QString("text\n"));
}  // end of TestLineSeparators::theHelperTurnsTheSeparatorsIntoNewLines

void TestLineSeparators::openingAFileTurnsALineSeparatorIntoANewLine() {
  // a file whose first line ends with the invisible separator: on the screen it
  // reads as three lines, the second of them empty
  const auto text = QStringLiteral("- first item") + line_separator +
                    QStringLiteral("\n- second item\n");
  const auto file = openFile("separators.txt", text);
  auto &t = mainFrame();
  QVERIFY(t.getCompleteFileName() == file);
  auto *const doc = t.document();
  // the invariant: no block of a document of the editor holds a character the
  // layout breaks a displayed line on
  for (const auto &b : blocksOf(doc)) {
    QVERIFY2(!b.contains(line_separator), qPrintable(b));
    QVERIFY2(!b.contains(next_line), qPrintable(b));
  }
  QCOMPARE(blocksOf(doc), QStringList()
                              << QStringLiteral("- first item") << QString()
                              << QStringLiteral("- second item") << QString());
  // and the first line is drawn on a single row: that is what the highlight of
  // the current line, and `Ctrl-E`, go by
  QCOMPARE(rowsOf(doc, 0), 1);
  QCOMPARE(rowsOf(doc, 1), 1);
}  // end of TestLineSeparators::openingAFileTurnsALineSeparatorIntoANewLine

void TestLineSeparators::ctrlEReachesTheEndOfTheCurrentLine() {
  const auto text = QStringLiteral("- first item") + line_separator +
                    QStringLiteral("\n- second item\n");
  openFile("ctrl-e.txt", text);
  auto &t = mainFrame();
  auto c = t.textCursor();
  c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  c.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, 4);
  t.setTextCursor(c);
  QTest::keyClick(t.widget(), Qt::Key_E, Qt::ControlModifier);
  pump();
  const auto after = t.textCursor();
  QCOMPARE(after.blockNumber(), 0);
  // the end of the line the cursor was on, not of the block the document
  // believes the line is
  QCOMPARE(after.positionInBlock(), 12);
}  // end of TestLineSeparators::ctrlEReachesTheEndOfTheCurrentLine

void TestLineSeparators::pastingTextCarryingALineSeparatorIsNormalized() {
  const auto text = QStringLiteral("- first item") + line_separator +
                    QStringLiteral("\n- second item\n");
  openFile("pasted.txt", text);
  auto &t = mainFrame();
  auto *const clipboard = QApplication::clipboard();
  QVERIFY(clipboard != nullptr);
  clipboard->setText("head" + QString(line_separator) + "tail");
  pump();
  auto c = t.textCursor();
  c.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  t.setTextCursor(c);
  t.paste();
  pump();
  // the pasted text makes two lines of the document, not one the layout breaks
  // by itself
  QCOMPARE(blocksOf(t.document()),
           QStringList() << QStringLiteral("- first item") << QString()
                         << QStringLiteral("- second item")
                         << QStringLiteral("head") << QStringLiteral("tail"));
  QCOMPARE(rowsOf(t.document(), 3), 1);
  QCOMPARE(rowsOf(t.document(), 4), 1);
}  // end of TestLineSeparators::pastingTextCarryingALineSeparatorIsNormalized

void TestLineSeparators::theMinibufferInputKeepsNoLineSeparator() {
  // the entry belongs to its host, which deletes it
  auto *const input = new tfel::gui::MultiLineLineEdit;
  Host host(input);
  pump();
  // an input is as tall as the rows its text makes: a separator the document
  // keeps inside a block while the layout breaks the line on it is a row the
  // entry pays for and which `text` says nothing about
  input->setText("first" + QString(line_separator) + "second");
  pump();
  QCOMPARE(input->text(), QString("first\nsecond"));
  QCOMPARE(input->document()->blockCount(), 2);
  // the same text through the clipboard: pasting is the way such a character
  // reaches an input of the editor in the first place
  auto *const clipboard = QApplication::clipboard();
  QVERIFY(clipboard != nullptr);
  clipboard->setText("pasted" + QString(next_line) + "here");
  pump();
  input->selectAll();
  input->paste();
  pump();
  QCOMPARE(input->text(), QString("pasted\nhere"));
  QCOMPARE(input->document()->blockCount(), 2);
}  // end of TestLineSeparators::theMinibufferInputKeepsNoLineSeparator

QTEST_MAIN(TestLineSeparators)

#include "TestLineSeparators.moc"
