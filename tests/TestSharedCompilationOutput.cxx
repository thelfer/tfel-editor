/*!
 * \file   tests/TestSharedCompilationOutput.cxx
 * \brief  tests of the category of the major modes and of the sharing of the
 *         compilation output between the buffers which edit a source file.
 * \author Thomas Helfer
 * \date   09/11/2026
 */

#include <QtCore/QFileInfo>
#include <QtCore/QLibrary>
#include <QtCore/QTemporaryDir>
#include <QtCore/QFile>
#include <QtTest/QtTest>
#include <QtWidgets/QWidget>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/MajorMode.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/SecondaryTask.hxx"

namespace {

  using Category = tfel::gui::MajorMode::Category;

  void pump(int n = 4) {
    for (int k = 0; k != n; ++k) {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
  }  // end of pump()

  /*!
   * \brief an editor of its own, shown with an empty buffer
   *
   * \note the editor is a member and not a base class: `show` and the creation
   * of the first buffer are reached once the widget is entirely built
   *
   * \note the editor is shown because the tabs of the secondary tasks of a
   * buffer are only built when that buffer is visible, which is the case of the
   * current buffer of a shown editor
   */
  struct Editor {
    tfel::gui::EditorWidget ed;
    Editor() : ed(nullptr) {
      ed.setMinimumSize(600, 400);
      ed.resize(800, 600);
      ed.show();
      ed.createEmptyBuffer();
      pump(8);
    }
    ~Editor() {
      // no buffer is modified: nothing is asked of any buffer when the editor,
      // hence its buffers and the frames they hold, goes away
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
   * \brief create a frame displaying the output of a command, the way
   * `ProcessLineEdit` and `MFrontMajorMode::runMFront` do
   * \param[in] ed: editor
   * \param[in] b: buffer the frame is created for
   * \param[in] m: major mode of the frame, empty leaves it without any
   * \param[in] mode_first: install the mode before attaching the frame to the
   * buffer, as `ProcessLineEdit` does, or the other way round, as the `MFront`
   * mode does
   *
   * The process is never started: the sharing is decided by the mode of the
   * frame, not by what it holds, and a test which ran a compiler would depend
   * on the tool chain of whoever runs it.
   *
   * \note the frame is returned: it is owned by the Qt hierarchy and by the
   * secondary task manager, which drops it once no buffer holds it any more
   */
  tfel::gui::ProcessOutputFrame* openOutputFrame(tfel::gui::EditorWidget& ed,
                                                 tfel::gui::Buffer& b,
                                                 const QString& m,
                                                 const bool mode_first) {
    auto* const frame = new tfel::gui::ProcessOutputFrame(ed, b);
    if (mode_first) {
      if (!m.isEmpty()) {
        frame->setMajorMode(m);
      }
      b.attachSecondaryTask("*output*", frame);
    } else {
      b.attachSecondaryTask("*output*", frame);
      if (!m.isEmpty()) {
        frame->setMajorMode(m);
      }
    }
    return frame;
  }  // end of openOutputFrame()

  /*!
   * \return true when the given buffer holds the given secondary task
   *
   * The registry of the secondary tasks is what states the sharing. The tab
   * widget of a buffer only says what that buffer shows, and a buffer which is
   * not the current one of the stack is not visible.
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

  //! \return the names of the buffers whose main frame wears a mode of the
  //! given category
  QStringList namesOfCategory(tfel::gui::EditorWidget& ed, const Category c) {
    auto names = QStringList{};
    for (auto* const b : ed.getBuffersOfCategory(c)) {
      names << b->getBufferName();
    }
    return names;
  }  // end of namesOfCategory()

}  // end of namespace

struct TestSharedCompilationOutput : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void theModesAreClassified();
  void aCompilationOutputIsSharedWithSourceFiles();
  void theOrderOfTheTwoStepsDoesNotMatter();
  void aNonCompilationOutputStaysWithItsBuffer();
  void aBufferBecomingProgrammingIsGivenTheOutputs();
  void aSourceFileOpenedAfterTheOutputIsGivenIt();
  void aDirectorySharesNothing();
  void aBufferWithoutAMajorModeSharesNothing();
  void theTextModesShareNothing();

 private:
  QTemporaryDir dir;
};

void TestSharedCompilationOutput::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-shared-compilation-output");
  QVERIFY2(dir.isValid(), "no temporary directory to write the files in");
}  // end of TestSharedCompilationOutput::initTestCase

void TestSharedCompilationOutput::theModesAreClassified() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "widget.cpp", "int main() { return 0; }\n");
  const auto txt = writeFile(dir, "notes.txt", "some prose\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  QVERIFY2(source.getMainFrame().hasMajorMode(),
           "a C++ file is given no major mode");
  // the files of a project are written as the `C++` mode claims them, that is
  // with one of the suffixes of its proxy: a buffer wearing no mode is of no
  // category, so the names the test writes are those suffixes
  QCOMPARE(source.getMainFrame().getMajorMode().getName(), QString("C++"));
  // the whole C/C++ family, which is what `CMajorModeBase` is, is what a
  // compilation output is about
  QCOMPARE(source.getMainFrame().getMajorMode().getCategory(),
           Category::PROGRAMMING);
  // the mode which displays such an output is not one of them: the frames which
  // wear it are secondary tasks, never the main frame of a buffer
  ed.openFile(txt);
  auto& notes = ed.getCurrentBuffer();
  auto* const frame = new tfel::gui::ProcessOutputFrame(ed, notes);
  QVERIFY2(frame->setMajorMode("compilation-output") != nullptr,
           "the compilation-output mode is not registered");
  QCOMPARE(frame->getMajorMode().getCategory(), Category::MISC);
  QVERIFY(frame->displaysCompilationOutput());
  // a buffer visiting a file no mode claims, and an empty buffer, are of no
  // category at all: no list of a category holds them
  QVERIFY(!notes.getMainFrame().hasMajorMode());
  QCOMPARE(namesOfCategory(ed, Category::PROGRAMMING), QStringList()
                                                           << "widget.cpp");
  QCOMPARE(namesOfCategory(ed, Category::TEXT_EDITING), QStringList());
  QCOMPARE(namesOfCategory(ed, Category::MISC), QStringList());
}  // end of TestSharedCompilationOutput::theModesAreClassified

void TestSharedCompilationOutput::aCompilationOutputIsSharedWithSourceFiles() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "a.cpp", "int main() { return 0; }\n");
  const auto h = writeFile(dir, "widget.h", "class Widget;\n");
  const auto txt = writeFile(dir, "notes.txt", "some prose\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  ed.openFile(h);
  auto& header = ed.getCurrentBuffer();
  ed.openFile(txt);
  auto& notes = ed.getCurrentBuffer();
  auto* const frame = openOutputFrame(ed, source, "compilation-output", true);
  // the buffer which asked for the compilation, as every other buffer editing a
  // source file of the same project, holds the very same task
  QVERIFY(holdsTask(ed, source, frame));
  QVERIFY2(holdsTask(ed, header, frame),
           "the output of a compilation reached only the file which asked for "
           "it: the sharing is what the PROGRAMMING category is for");
  // and the tab of that other buffer shows it, under the same title
  QCOMPARE(header.getSecondaryTaskIndex(frame), 0);
  QCOMPARE(header.getSecondaryTaskTitle(frame), QString("*output*"));
  // a buffer which is not of that category is left alone
  QVERIFY(!holdsTask(ed, notes, frame));
  QCOMPARE(notes.getSecondaryTaskIndex(frame), -1);
  // the two programming buffers are the ones the sharing was asked for
  QCOMPARE(namesOfCategory(ed, Category::PROGRAMMING),
           (QStringList() << "a.cpp"
                          << "widget.h"));
}  // end of
   // TestSharedCompilationOutput::aCompilationOutputIsSharedWithSourceFiles

void TestSharedCompilationOutput::theOrderOfTheTwoStepsDoesNotMatter() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "first.cpp", "int a();\n");
  const auto h = writeFile(dir, "second.h", "int b();\n");
  ed.openFile(cc);
  auto& first = ed.getCurrentBuffer();
  ed.openFile(h);
  auto& second = ed.getCurrentBuffer();
  // `ProcessLineEdit` gives the frame its mode, then attaches it: the sharing
  // happens on the attachment
  auto* const before = openOutputFrame(ed, first, "compilation-output", true);
  QVERIFY2(holdsTask(ed, second, before),
           "a frame attached after its mode was installed is not shared");
  // `MFrontMajorMode::runMFront` attaches the frame, then gives it its mode:
  // the sharing happens when the mode is installed
  auto* const after = openOutputFrame(ed, first, "compilation-output", false);
  QVERIFY2(holdsTask(ed, second, after),
           "a frame which received its mode after being attached is not "
           "shared: the two orders have to agree");
  // and both frames are held by the buffer they were created for
  QVERIFY(holdsTask(ed, first, before));
  QVERIFY(holdsTask(ed, first, after));
  QCOMPARE(second.getSecondaryTaskIndex(before), 0);
  QCOMPARE(second.getSecondaryTaskIndex(after), 1);
}  // end of
   // TestSharedCompilationOutput::theOrderOfTheTwoStepsDoesNotMatter

void TestSharedCompilationOutput::aNonCompilationOutputStaysWithItsBuffer() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "stay.cpp", "int a();\n");
  const auto h = writeFile(dir, "stay.h", "int b();\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  ed.openFile(h);
  auto& header = ed.getCurrentBuffer();
  // the output of a `grep`, of an `mtest` or of an interactive session is about
  // the buffer which asked for it
  auto* const grep_frame = openOutputFrame(ed, source, "grep-output", true);
  QVERIFY(!grep_frame->displaysCompilationOutput());
  QVERIFY(holdsTask(ed, source, grep_frame));
  QVERIFY2(!holdsTask(ed, header, grep_frame),
           "the output of a grep was shared with another buffer");
  // a frame which wears no mode at all displays nothing in particular
  auto* const plain_frame = openOutputFrame(ed, source, {}, true);
  QVERIFY(!plain_frame->displaysCompilationOutput());
  QVERIFY(!holdsTask(ed, header, plain_frame));
}  // end of
   // TestSharedCompilationOutput::aNonCompilationOutputStaysWithItsBuffer

void TestSharedCompilationOutput::
    aBufferBecomingProgrammingIsGivenTheOutputs() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "later.cpp", "int a();\n");
  const auto txt = writeFile(dir, "later.txt", "some prose\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  auto* const frame = openOutputFrame(ed, source, "compilation-output", true);
  // a buffer opened while a compilation output is displayed does not get it as
  // long as it wears no mode of the programming category
  ed.openFile(txt);
  auto& notes = ed.getCurrentBuffer();
  QVERIFY(!notes.getMainFrame().hasMajorMode());
  QVERIFY(!holdsTask(ed, notes, frame));
  // and it gets it once it wears such a mode, which is what `M-x c++-mode`, or
  // `Options > Major mode`, does
  QVERIFY(notes.getMainFrame().setMajorMode("C++") != nullptr);
  pump(2);
  QVERIFY2(holdsTask(ed, notes, frame),
           "a buffer which reached the programming category was not given the "
           "compilation output the other buffers hold");
  QCOMPARE(notes.getSecondaryTaskIndex(frame), 0);
}  // end of
   // TestSharedCompilationOutput::aBufferBecomingProgrammingIsGivenTheOutputs

void TestSharedCompilationOutput::aSourceFileOpenedAfterTheOutputIsGivenIt() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "before.cpp", "int a();\n");
  const auto other = writeFile(dir, "after.cpp", "int b();\n");
  ed.openFile(cc);
  auto& first = ed.getCurrentBuffer();
  auto* const frame = openOutputFrame(ed, first, "compilation-output", true);
  // the mode of a buffer visiting a file is installed by `setFileName`, inside
  // the constructor of the text edit, where nothing can be connected to the
  // signal announcing it yet: the sharing is therefore looked at once the
  // buffer is created, not only when its mode changes afterwards
  ed.openFile(other);
  auto& second = ed.getCurrentBuffer();
  QCOMPARE(second.getMainFrame().getMajorMode().getName(), QString("C++"));
  QVERIFY2(holdsTask(ed, second, frame),
           "a source file opened while a compilation output was displayed was "
           "left without it");
  QCOMPARE(second.getSecondaryTaskIndex(frame), 0);
}  // end of
   // TestSharedCompilationOutput::aSourceFileOpenedAfterTheOutputIsGivenIt

void TestSharedCompilationOutput::aDirectorySharesNothing() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "dir.cpp", "int a();\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  auto* const frame = openOutputFrame(ed, source, "compilation-output", true);
  // the buffer listing a directory wears the `Directory` mode, which is of no
  // particular category: the output of a compilation says nothing about it
  ed.openFile(QFileInfo(dir.path()).absoluteFilePath());
  auto& listing = ed.getCurrentBuffer();
  QVERIFY(listing.getMainFrame().hasMajorMode());
  QCOMPARE(listing.getMainFrame().getMajorMode().getCategory(), Category::MISC);
  QVERIFY(!holdsTask(ed, listing, frame));
}  // end of TestSharedCompilationOutput::aDirectorySharesNothing

void TestSharedCompilationOutput::aBufferWithoutAMajorModeSharesNothing() {
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "empty.cpp", "int a();\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  auto* const frame = openOutputFrame(ed, source, "compilation-output", true);
  // an empty buffer has no major mode: it edits the source of nothing yet, and
  // `getMajorMode` would throw on it
  ed.createEmptyBuffer();
  auto& fresh = ed.getCurrentBuffer();
  QVERIFY(!fresh.getMainFrame().hasMajorMode());
  QVERIFY2(!holdsTask(ed, fresh, frame),
           "a buffer with no major mode was given the output of a compilation");
  // the buffer which asked for the compilation still holds it
  QVERIFY(holdsTask(ed, source, frame));
}  // end of
   // TestSharedCompilationOutput::aBufferWithoutAMajorModeSharesNothing

void TestSharedCompilationOutput::theTextModesShareNothing() {
  // the packaged modes are plugins: the only way to see what they answer is to
  // load the built library, which registers the mode for the rest of the run,
  // so no case may follow this one
#ifdef TFEL_EDITOR_LATEX_MODE
  auto latex = QLibrary{QStringLiteral(TFEL_EDITOR_LATEX_MODE)};
  QVERIFY2(latex.load(), qPrintable(latex.errorString()));
#else  /* TFEL_EDITOR_LATEX_MODE */
  QSKIP("this build does not name the library of the LaTeX mode");
#endif /* TFEL_EDITOR_LATEX_MODE */
#ifdef TFEL_EDITOR_MARKDOWN_MODE
  auto markdown = QLibrary{QStringLiteral(TFEL_EDITOR_MARKDOWN_MODE)};
  QVERIFY2(markdown.load(), qPrintable(markdown.errorString()));
#else  /* TFEL_EDITOR_MARKDOWN_MODE */
  QSKIP("this build does not name the library of the Markdown mode");
#endif /* TFEL_EDITOR_MARKDOWN_MODE */
  Editor e;
  auto& ed = e.ed;
  const auto cc = writeFile(dir, "text.cpp", "int a();\n");
  const auto tex = writeFile(dir, "report.tex", "\\section{A title}\n");
  const auto md = writeFile(dir, "notes.md", "# A title\n");
  ed.openFile(cc);
  auto& source = ed.getCurrentBuffer();
  auto* const frame = openOutputFrame(ed, source, "compilation-output", true);
  // the output of the compiler which built `text.cpp` has nothing to do in the
  // buffer of a report, whatever the command which wrote it
  ed.openFile(tex);
  auto& report = ed.getCurrentBuffer();
  QCOMPARE(report.getMainFrame().getMajorMode().getName(), QString("LaTeX"));
  QCOMPARE(report.getMainFrame().getMajorMode().getCategory(),
           Category::TEXT_EDITING);
  QVERIFY(!holdsTask(ed, report, frame));
  ed.openFile(md);
  auto& notes = ed.getCurrentBuffer();
  QCOMPARE(notes.getMainFrame().getMajorMode().getName(), QString("Markdown"));
  QCOMPARE(notes.getMainFrame().getMajorMode().getCategory(),
           Category::TEXT_EDITING);
  QVERIFY(!holdsTask(ed, notes, frame));
  QCOMPARE(namesOfCategory(ed, Category::TEXT_EDITING),
           (QStringList() << "report.tex"
                          << "notes.md"));
  QCOMPARE(namesOfCategory(ed, Category::PROGRAMMING), QStringList()
                                                           << "text.cpp");
}  // end of TestSharedCompilationOutput::theTextModesShareNothing

QTEST_MAIN(TestSharedCompilationOutput)

#include "TestSharedCompilationOutput.moc"
