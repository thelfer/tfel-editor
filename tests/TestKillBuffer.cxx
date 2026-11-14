/*!
 * \file   tests/TestKillBuffer.cxx
 * \brief  tests of what the kill of a buffer does to the things which name it:
 *         a question the minibuffer waits an answer from, and a secondary task
 *         whose widget the tab widget of the killed buffer held.
 * \author Thomas Helfer
 * \date   14/11/2026
 */

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryDir>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTabWidget>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/SecondaryTask.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  void pump(int n = 4) {
    for (int k = 0; k != n; ++k) {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
  }  // end of pump()

  /*!
   * \brief an editor which records what it displays in the minibuffer
   *
   * \note the reason the message of a refusal is worth reading is that the
   * refusal is otherwise silent: the buffer stays, and nothing else says why
   * the kill did not happen
   */
  struct ReportingEditor : tfel::gui::EditorWidget {
    ReportingEditor() : EditorWidget(nullptr) {}
    QStringList messages;
    using tfel::gui::EditorWidget::removeBuffer;
    void displayInformativeMessage(const QString& m) override {
      this->messages << m;
      tfel::gui::EditorWidget::displayInformativeMessage(m);
    }
  };  // end of struct ReportingEditor

  /*!
   * \brief an editor of its own, shown with an empty buffer
   *
   * \note the editor is shown because the tabs of the secondary tasks of a
   * buffer are only built when that buffer is visible, and because a buffer is
   * only registered as a secondary task of a visible editor the same way the
   * modes do it
   */
  struct Editor {
    ReportingEditor ed;
    Editor() {
      ed.setMinimumSize(600, 400);
      ed.resize(800, 600);
      ed.show();
      ed.createEmptyBuffer();
      pump(8);
    }
    ~Editor() {
      // no buffer is modified: nothing is asked of any buffer when the editor,
      // hence its buffers, goes away
      pump(4);
    }
  };  // end of struct Editor

  //! \return the absolute name of a file written in the given directory
  QString writeFile(QTemporaryDir& d, const QString& n, const QString& body) {
    const auto p = d.filePath(n);
    QFile f(p);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return {};
    }
    f.write(body.toUtf8());
    f.close();
    return QFileInfo(p).absoluteFilePath();
  }  // end of writeFile()

  /*!
   * \brief true when the widget is still laid out
   *
   * \note dropping a widget of the minibuffer, an input which was answered
   * included, only takes it out of the layout and asks for its deletion: the
   * widget stays a child, and a live one, until the event loop runs that
   * deletion. What the layout still holds is what the editor shows
   */
  bool isLaidOut(QWidget& w) {
    auto* const parent = w.parentWidget();
    auto* const layout = (parent != nullptr) ? parent->layout() : nullptr;
    return (layout != nullptr) && (layout->indexOf(&w) >= 0);
  }  // end of isLaidOut()

  //! \return the question the editor waits an answer from
  tfel::gui::CommandLine* liveInput(tfel::gui::EditorWidget& ed) {
    for (auto* const input : ed.findChildren<tfel::gui::CommandLine*>()) {
      if (isLaidOut(*input)) {
        return input;
      }
    }
    return nullptr;
  }  // end of liveInput()

  //! \return the entry of the given question
  tfel::gui::LineEdit* entryOf(tfel::gui::CommandLine& cl) {
    for (auto* const line : cl.findChildren<tfel::gui::LineEdit*>()) {
      if (isLaidOut(*line)) {
        return line;
      }
    }
    return nullptr;
  }  // end of entryOf()

  void press(QWidget* const w, const int k) {
    if (w == nullptr) {
      return;
    }
    auto press_event = QKeyEvent(QEvent::KeyPress, k, Qt::NoModifier);
    QApplication::sendEvent(w, &press_event);
    auto release_event = QKeyEvent(QEvent::KeyRelease, k, Qt::NoModifier);
    QApplication::sendEvent(w, &release_event);
  }  // end of press()

  /*!
   * \return true when the registry of the buffer holds the task of the given
   * widget
   *
   * \note an entry whose widget has been destroyed names nothing: it matches no
   * widget, which is exactly what the sharing of a task amounts to once the
   * buffer which held the widget is gone
   */
  bool holdsTask(tfel::gui::EditorWidget& ed,
                 tfel::gui::Buffer& b,
                 QWidget* const w) {
    for (const auto& t : ed.getSecondaryTasks(&b)) {
      if (t.w == w) {
        return true;
      }
    }
    return false;
  }  // end of holdsTask()

}  // end of namespace

struct TestKillBuffer : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void aWaitingPromptKeepsItsBufferAlive();
  void theAnswerOfAPromptMayKillTheBufferItWasRaisedIn();
  void aKilledBufferTakesASharedOutputWithIt();

 private:
  QTemporaryDir dir;
};

void TestKillBuffer::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-kill-buffer");
  QVERIFY2(dir.isValid(), "no temporary directory to write the files in");
}  // end of TestKillBuffer::initTestCase

void TestKillBuffer::aWaitingPromptKeepsItsBufferAlive() {
  Editor e;
  auto& ed = e.ed;
  const auto first = writeFile(dir, "asked-1.txt", "first\n");
  const auto second = writeFile(dir, "asked-2.txt", "second\n");
  ed.openFile(first);
  pump();
  ed.openFile(second);
  pump();
  auto* const asked = ed.getBufferVisitingFile(first);
  auto* const other = ed.getBufferVisitingFile(second);
  QVERIFY(asked != nullptr);
  QVERIFY(other != nullptr);
  // the question of `Ctrl-x b` is raised in the buffer which is current, and
  // `Tab` shows the list of the buffers as a `*completions*` secondary task of
  // that very buffer
  ed.changeBuffer(asked->getId());
  pump();
  ed.changeBuffer();
  pump();
  auto* const question = liveInput(ed);
  QVERIFY2(question != nullptr, "`Ctrl-x b` asked no question");
  QCOMPARE(question->getBuffer(), asked);
  QVERIFY(question->isWaitingForAnAnswer());
  press(entryOf(*question), Qt::Key_Tab);
  pump();
  QVERIFY2(!ed.getSecondaryTasks(asked).empty(),
           "the question showed no completion list as a secondary task of its "
           "own buffer");
  // the question does not block: the user looks at another buffer, as the
  // `Buffers` menu does, and the kill of the buffer the question belongs to is
  // asked for from there
  ed.changeBuffer(other->getId());
  pump();
  const auto count = ed.getBuffersNames().size();
  // Killing that buffer is refused: the completion list is a page of it, and
  // the answer of the question would name a buffer which is gone
  ed.removeBuffer(asked);
  pump(8);
  QCOMPARE(ed.getBuffersNames().size(), count);
  QVERIFY(ed.hasUserInputWaitingOn(*asked));
  QVERIFY2(ed.messages.contains("minibuffer is busy"),
           "the refusal of the kill said nothing");
  // Answering the question spends the protection, and the kill which follows
  // goes through
  auto* const entry = entryOf(*question);
  QVERIFY(entry != nullptr);
  entry->setText("asked-2.txt");
  question->userEditingFinished();
  pump(8);
  QVERIFY(!ed.hasUserInput());
  ed.removeBuffer(asked);
  pump(8);
  QCOMPARE(ed.getBuffersNames().size(), count - 1);
  QVERIFY(ed.getBufferVisitingFile(first) == nullptr);
}  // end of
   // TestKillBuffer::aWaitingPromptKeepsItsBufferAlive

void TestKillBuffer::theAnswerOfAPromptMayKillTheBufferItWasRaisedIn() {
  Editor e;
  auto& ed = e.ed;
  const auto file = writeFile(dir, "modified.txt", "body\n");
  ed.openFile(file);
  pump();
  auto* const buffer = ed.getBufferVisitingFile(file);
  QVERIFY(buffer != nullptr);
  // a modified buffer is not one the editor kills without asking
  buffer->getMainFrame().document()->setModified(true);
  const auto count = ed.getBuffersNames().size();
  ed.closeCurrentBuffer(true);
  pump();
  auto* const question = liveInput(ed);
  QVERIFY2(question != nullptr,
           "killing a modified buffer asked for no confirmation");
  // the confirmation is raised in the buffer it asks about, and it is the very
  // prompt whose answer kills that buffer: an input which has been answered is
  // no longer waiting, which is what keeps this flow from being stopped by the
  // protection it names its own buffer with
  auto* const entry = entryOf(*question);
  QVERIFY(entry != nullptr);
  entry->setText("y");
  question->userEditingFinished();
  pump(8);
  QCOMPARE(ed.getBuffersNames().size(), count - 1);
}  // end of TestKillBuffer::theAnswerOfAPromptMayKillTheBufferItWasRaisedIn

void TestKillBuffer::aKilledBufferTakesASharedOutputWithIt() {
  Editor e;
  auto& ed = e.ed;
  const auto first = writeFile(dir, "shared-1.cpp", "int one();\n");
  const auto second = writeFile(dir, "shared-2.cpp", "int two();\n");
  const auto note = writeFile(dir, "shared-note.txt", "prose\n");
  ed.openFile(first);
  pump();
  auto* const source = ed.getBufferVisitingFile(first);
  QVERIFY(source != nullptr);
  ed.openFile(second);
  pump();
  auto* const header = ed.getBufferVisitingFile(second);
  QVERIFY(header != nullptr);
  // one frame, attached to the two source buffers: the sharing a compilation
  // output is about
  auto* const frame = new tfel::gui::ProcessOutputFrame(ed, *source);
  QVERIFY(frame->setMajorMode("compilation-output") != nullptr);
  source->attachSecondaryTask("*output*", frame);
  header->attachSecondaryTask("*output*", frame);
  pump();
  QVERIFY(holdsTask(ed, *source, frame));
  QVERIFY(holdsTask(ed, *header, frame));
  // the frame is a single widget, so only the tab widget of one of the two
  // buffers holds the page: adding a tab moves it, which is what makes the
  // buffer holding it possibly different from the one which asked for the
  // output, and the two assertions below state which one it is
  auto* const holder =
      (source->getSecondaryTaskIndex(frame) >= 0) ? source : header;
  auto* const other = (holder == source) ? header : source;
  QVERIFY(holder->getSecondaryTaskIndex(frame) >= 0);
  QVERIFY2(other->getSecondaryTaskIndex(frame) < 0,
           "the page of a shared task stands in the tab widget of both buffers "
           "at once");
  // a buffer of no category is made current: the crash this test pins was the
  // switch to a buffer naming a widget another buffer had taken in and died
  // with, so the kill must not reach the current buffer
  ed.openFile(note);
  pump(8);
  QVERIFY(&ed.getCurrentBuffer() != holder);
  auto alive = QPointer<tfel::gui::ProcessOutputFrame>(frame);
  ed.removeBuffer(holder);
  pump(8);
  QVERIFY2(alive.isNull(),
           "the widget of a shared task outlived the buffer whose tab widget "
           "held it");
  // the surviving buffer still names the task it was given, but the entry names
  // nothing: it matches neither the dead widget nor any other one
  QVERIFY(!holdsTask(ed, *other, frame));
  // and switching to that buffer, which is what re-adds a tab per task the
  // registry holds, hands nothing to the tab widget
  ed.changeBuffer(other->getId());
  pump(8);
  QCOMPARE(&ed.getCurrentBuffer(), other);
  QVERIFY(other->getSecondaryTaskIndex(frame) < 0);
  // and the tab widget of that buffer shows no page left over from the task
  auto* const tabs = other->findChild<QTabWidget*>();
  QVERIFY(tabs != nullptr);
  QCOMPARE(tabs->count(), 0);
}  // end of
   // TestKillBuffer::aKilledBufferTakesASharedOutputWithIt

QTEST_MAIN(TestKillBuffer)

#include "TestKillBuffer.moc"
