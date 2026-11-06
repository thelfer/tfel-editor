/*!
 * \file   tests/TestFileOnDiskModification.cxx
 * \brief  tests of the watch on the visited file: the warning banner shown
 *         when another process modified the file, its dismissal, the reload
 *         it offers, and the save which asks before overwriting such a
 *         modification.
 * \author Thomas Helfer
 * \date   06/11/2026
 */

#include <memory>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>
#include <QtTest/QtTest>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/MultiLineLineEdit.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  // a question the minibuffer still lays out: an input which was answered
  // lingers as a live child of the editor until its `deleteLater` runs, so
  // `findChild` alone can return the previous question (see the note on
  // answering a minibuffer question in the project documentation)
  tfel::gui::CommandLine *laidOutQuestion(tfel::gui::EditorWidget &editor,
                                          const QString &name) {
    for (auto *const cl : editor.findChildren<tfel::gui::CommandLine *>()) {
      if ((cl->objectName() == name) && (cl->parentWidget() != nullptr) &&
          (cl->parentWidget()->layout() != nullptr) &&
          (cl->parentWidget()->layout()->indexOf(cl) >= 0)) {
        return cl;
      }
    }
    return nullptr;
  }  // end of laidOutQuestion()

  // the visible line edit of a question, the widget `setText` answers it with
  tfel::gui::LineEdit *lineEditOf(tfel::gui::CommandLine &question) {
    for (auto *const line : question.findChildren<tfel::gui::LineEdit *>()) {
      if (line->isVisible()) {
        return line;
      }
    }
    return nullptr;
  }  // end of lineEditOf()

  // write the file as another process would: through a handle the editor
  // knows nothing about
  void writeOutside(const QString &file, const QString &content) {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::WriteOnly | QIODevice::Truncate));
    handle.write(content.toUtf8());
    handle.close();
  }  // end of writeOutside()

}  // end of anonymous namespace

//! tests of the watch on the file visited by a buffer
struct TestFileOnDiskModification : QObject {
  Q_OBJECT

 private slots:

  void initTestCase();

  void openingAFileRecordsItsState();

  void theBannerWarnsAboutAModificationOnDisk();

  void theBannerFreezesTheBufferWhileItIsUp();

  void aKeyboardSaveAsksBeforeOverwriting();

  void savingANewFileIsNotAConflict();

 private:
  void pump(int n = 4);

  QTemporaryDir directory;
  std::unique_ptr<tfel::gui::EditorWidget> editor;

  // the buffer visiting `file`, built once and shared: the cases follow the
  // life of that file, from open to reload
  QString openBuffer(const QString &name, const QString &content);
  tfel::gui::Buffer &buffer();
  tfel::gui::PlainTextEdit &mainFrame();
  QWidget *banner();
};

void TestFileOnDiskModification::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-file-on-disk-modification");
  auto settings = QSettings();
  settings.remove("positions in files");
  settings.sync();
  QVERIFY(directory.isValid());
}  // end of TestFileOnDiskModification::initTestCase

void TestFileOnDiskModification::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestFileOnDiskModification::pump

QString TestFileOnDiskModification::openBuffer(const QString &name,
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
}  // end of openBuffer

tfel::gui::Buffer &TestFileOnDiskModification::buffer() {
  return editor->getCurrentBuffer();
}  // end of TestFileOnDiskModification::buffer

tfel::gui::PlainTextEdit &TestFileOnDiskModification::mainFrame() {
  return buffer().getMainFrame();
}  // end of TestFileOnDiskModification::mainFrame

QWidget *TestFileOnDiskModification::banner() {
  return buffer().findChild<QWidget *>("file-modified-banner");
}  // end of TestFileOnDiskModification::banner

void TestFileOnDiskModification::openingAFileRecordsItsState() {
  const auto file = openBuffer("watched.txt", "first line\n");
  auto &t = mainFrame();
  QVERIFY(!t.fileModifiedOnDisk());
  QVERIFY(banner() != nullptr);
  QVERIFY(!banner()->isVisible());
  // the check of the timer, run by hand: a file nobody touched agrees
  // with the buffer visiting it
  t.checkFileModifiedOnDisk();
  QVERIFY(!banner()->isVisible());
}  // end of TestFileOnDiskModification::openingAFileRecordsItsState

void TestFileOnDiskModification::theBannerWarnsAboutAModificationOnDisk() {
  const auto file = openBuffer("banner.txt", "first line\n");
  auto &t = mainFrame();
  auto *const bar = banner();
  // another process writes over the file
  writeOutside(file, "first line\nsecond line\n");
  QVERIFY(t.fileModifiedOnDisk());
  t.checkFileModifiedOnDisk();
  QVERIFY2(bar->isVisible(), "the buffer does not warn about the file");
  QVERIFY2(t.isReadOnly(), "a warned-about buffer still accepts edits");
  auto *const message = bar->findChild<QLabel *>();
  QVERIFY(message != nullptr);
  QVERIFY(message->text().contains("has been modified on disk"));
  QVERIFY(message->text().contains(file));
  // dismissing the warning silences the banner for this state of the
  // file, without forgetting that the disk and the buffer disagree; the
  // buffer is editable again, the warning only freezes it while it is up
  bar->findChild<QToolButton *>()->click();
  pump();
  QVERIFY(!bar->isVisible());
  QVERIFY(!t.isReadOnly());
  QVERIFY(t.fileModifiedOnDisk());
  t.checkFileModifiedOnDisk();
  QVERIFY(!bar->isVisible());
  // a further change is worth a warning again
  writeOutside(file, "first line\nsecond line\nthird line\n");
  t.checkFileModifiedOnDisk();
  QVERIFY2(bar->isVisible(), "a new change does not warn again");
  // the Reload button replaces the text of the buffer by the file, resets
  // the modified flag and dismisses the banner
  const auto original = t.document()->toPlainText();
  QVERIFY(!original.contains("second line"));
  bar->findChild<QPushButton *>()->click();
  pump();
  QVERIFY(!bar->isVisible());
  QVERIFY(!t.isReadOnly());
  QVERIFY(t.document()->toPlainText().contains("second line"));
  QVERIFY(!t.document()->isModified());
  QVERIFY(!t.fileModifiedOnDisk());
  t.checkFileModifiedOnDisk();
  QVERIFY(!bar->isVisible());
}  // end of theBannerWarnsAboutAModificationOnDisk

void TestFileOnDiskModification::theBannerFreezesTheBufferWhileItIsUp() {
  const auto file = openBuffer("frozen.txt", "one\ntwo\nthree\n");
  auto &t = mainFrame();
  QVERIFY(!t.isReadOnly());
  // an edit of the buffer first, so that the undo history is not empty
  // when the freeze comes and the frozen buffer has something to walk
  // back to if undo slipped through its gate
  QTest::keyClick(t.widget(), Qt::Key_X);
  pump();
  QVERIFY(t.document()->isModified());
  // another process writes over the file, the warning goes up
  writeOutside(file, "one\ntwo\nthree\nfour\n");
  t.checkFileModifiedOnDisk();
  auto *const bar = banner();
  QVERIFY(bar->isVisible());
  QVERIFY2(t.isReadOnly(), "a warned-about buffer still accepts edits");
  // navigation keeps working: `End` reaches the widget even in a read-only
  // buffer, as the compilation output and directory buffers already show
  auto c = t.textCursor();
  c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  t.setTextCursor(c);
  QTest::keyClick(t.widget(), Qt::Key_End);
  pump();
  QVERIFY(t.textCursor().position() > 0);
  // and so does selecting a word
  c = t.textCursor();
  c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
  t.setTextCursor(c);
  QVERIFY(t.textCursor().hasSelection());
  QVERIFY(t.textCursor().selectedText() ==
          t.document()->toPlainText().left(3));
  // a typed character inserts nothing while the banner is up
  const auto before = t.document()->toPlainText();
  QVERIFY(before.contains("x"));
  QTest::keyClick(t.widget(), Qt::Key_Z);
  pump();
  QVERIFY2(t.document()->toPlainText() == before,
           "a key mutated a frozen buffer");
  // undo is bound by the same gate: the frozen buffer can not walk its
  // text back through the undo history either
  QTest::keyClick(t.widget(), Qt::Key_U, Qt::ControlModifier);
  pump();
  QVERIFY2(t.document()->toPlainText() == before,
           "undo mutated a frozen buffer");
  // the Reload button hands the buffer back, with the file's text
  bar->findChild<QPushButton *>()->click();
  pump();
  QVERIFY(!bar->isVisible());
  QVERIFY(!t.isReadOnly());
  QTest::keyClick(t.widget(), Qt::Key_Z);
  pump();
  QVERIFY(t.document()->toPlainText() != before);
  t.document()->setModified(false);
}  // end of theBannerFreezesTheBufferWhileItIsUp

void TestFileOnDiskModification::aKeyboardSaveAsksBeforeOverwriting() {
  const auto file = openBuffer("keyboard.txt", "alpha\n");
  auto &t = mainFrame();
  // the buffer edits its copy of the file...
  auto c = t.textCursor();
  c.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  c.insertText("beta\n");
  t.setTextCursor(c);
  // ... while another process writes over the file
  writeOutside(file, "written by someone else\n");
  // the save stops short of overwriting what it did not read
  t.save();
  pump();
  QVERIFY(editor->hasUserInput());
  auto *const question = laidOutQuestion(*editor, "file-modified-on-disk");
  QVERIFY2(question != nullptr, "the save asks nothing in the minibuffer");
  {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::ReadOnly));
    QVERIFY(QString::fromUtf8(handle.readAll()) == "written by someone else\n");
  }
  QVERIFY(t.document()->isModified());
  // answering 'n' aborts the save and leaves everything in place
  auto *const answer = lineEditOf(*question);
  QVERIFY(answer != nullptr);
  answer->setText("n");
  question->userEditingFinished();
  pump();
  QVERIFY(!editor->hasUserInput());
  {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::ReadOnly));
    QVERIFY(QString::fromUtf8(handle.readAll()) == "written by someone else\n");
  }
  QVERIFY(t.document()->isModified());
  // answering 'y' writes the buffer over the file, without asking twice;
  // the warning may be up in the meantime, and a frozen buffer can still
  // be saved: the freeze stops keys, not the decision it is waiting for
  t.checkFileModifiedOnDisk();
  QVERIFY(banner()->isVisible());
  QVERIFY(t.isReadOnly());
  t.save();
  pump();
  auto *const reopening = laidOutQuestion(*editor, "file-modified-on-disk");
  QVERIFY(reopening != nullptr);
  auto *const yes = lineEditOf(*reopening);
  QVERIFY(yes != nullptr);
  yes->setText("y");
  reopening->userEditingFinished();
  pump();
  QVERIFY(!editor->hasUserInput());
  {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::ReadOnly));
    QVERIFY(QString::fromUtf8(handle.readAll()) == "alpha\nbeta\n");
  }
  QVERIFY(!t.document()->isModified());
  QVERIFY(!t.fileModifiedOnDisk());
  QVERIFY(!banner()->isVisible());
  QVERIFY2(!t.isReadOnly(), "the overwritten buffer stayed frozen");
}  // end of aKeyboardSaveAsksBeforeOverwriting

void TestFileOnDiskModification::savingANewFileIsNotAConflict() {
  // a buffer visiting nothing, saved under a name the disk does not know:
  // there is nothing to overwrite and nothing to be asked about
  editor->createEmptyBuffer();
  pump();
  auto &t = mainFrame();
  auto c = t.textCursor();
  c.insertText("fresh text\n");
  t.setTextCursor(c);
  const auto file = directory.filePath("new-file.txt");
  t.writeFile(file);
  pump();
  QVERIFY(!editor->hasUserInput());
  {
    auto handle = QFile(file);
    QVERIFY(handle.open(QIODevice::ReadOnly));
    QVERIFY(QString::fromUtf8(handle.readAll()) == "fresh text\n");
  }
  QVERIFY(!t.document()->isModified());
  QVERIFY(!t.fileModifiedOnDisk());
  t.checkFileModifiedOnDisk();
  QVERIFY(!banner()->isVisible());
}  // end of savingANewFileIsNotAConflict

QTEST_MAIN(TestFileOnDiskModification)

#include "TestFileOnDiskModification.moc"
