/*!
 * \file   tests/TestEditorWidget.cxx
 * \brief  tests of the editor widget: one height per minibuffer page, the
 *         prompt on the first line of its entry, the height propagation
 *         stopping at the widget the window sizes, the row of information lines
 *         standing under the buffers and naming the buffer on the screen, and a
 *         font change reaching every widget which read the setting.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QTextEdit>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/CommandLine.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/MultiLineLineEdit.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace {

  // the tests look at the pages of the minibuffer, at the stack of buffers and
  // at the row of information lines below them, which the editor widget holds
  // as protected members: handing them out with `using` is what the probes did,
  // and what a friend declaration would spare
  struct ExposedEditorWidget : tfel::gui::EditorWidget {
    using EditorWidget::buffers;
    using EditorWidget::eui;
    using EditorWidget::infos;
    using EditorWidget::minibuffer;
    using EditorWidget::removeBuffer;
    using EditorWidget::um;
  };  // end of struct ExposedEditorWidget

  // the shape of the application: a main window with a menu bar and the editor
  // as central widget. An editor shown as its own window reproduces none of
  // the height problems of issue #121, since the walk up the parent chain then
  // has nothing above the editor
  struct Shell : QMainWindow {
    ExposedEditorWidget* const ed;
    explicit Shell(const QSize& s) : ed(new ExposedEditorWidget()) {
      menuBar()->addMenu("File")->addAction("Open");
      setCentralWidget(ed);
      resize(s);
      ed->createEmptyBuffer();
    }
    ~Shell() override {
      // no buffer is modified: no buffer is asked what to do with it when the
      // window, hence the editor and its buffers, goes away
      pump(2);
    }
    void pump(int n = 4) {
      for (int k = 0; k != n; ++k) {
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
      }
    }
  };  // end of struct Shell

  // the visible input of the user input which the editor waits an answer from:
  // `tfel::gui::LineEdit`, which is a `MultiLineLineEdit` with the extra
  // bindings of a command line entry. `findChild<QLineEdit*>` would return the
  // stale `CustomLineEdit` the constructor replaced, which is only dropped
  // with `deleteLater`, hence the visibility test.
  QWidget* currentInput(Shell& shell) {
    auto* const cl = shell.ed->findChild<tfel::gui::CommandLine*>();
    if (cl == nullptr) {
      return nullptr;
    }
    for (auto* const w : cl->findChildren<tfel::gui::MultiLineLineEdit*>()) {
      if (w->isVisible()) {
        return w;
      }
    }
    return nullptr;
  }  // end of currentInput()

  // what the row of information lines shows: the text of the labels of the page
  // on display, joined. The row holds one page per buffer, and only the current
  // one is visible, so what is readable there is what the mode line of the
  // buffer on the screen says — never what the line of another buffer holds,
  // hidden in another page of the row.
  QString shownInformation(ExposedEditorWidget& ed) {
    auto* const page = ed.infos->currentWidget();
    if (page == nullptr) {
      return {};
    }
    QString t;
    for (auto* const l : page->findChildren<QLabel*>()) {
      t += l->text();
    }
    return t;
  }  // end of shownInformation()

  // a file of the given directory, holding the given text
  QString writeFile(const QTemporaryDir& dir, const QString& name,
                    const QString& text) {
    const auto path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      return {};
    }
    f.write(text.toUtf8());
    return path;
  }  // end of writeFile()

  bool sameFont(const QFont& got, const QFont& want) {
    // the style strategy and the style name Qt resolves are not what the code
    // asked for, so only the family and the size are compared
    return (got.family() == want.family()) &&
           (qAbs(got.pointSizeF() - want.pointSizeF()) < 0.01);
  }  // end of sameFont()

}  // end of namespace

struct TestEditorWidget : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void minibufferPagesHaveTheSameSingleLineHeight();
  void promptSitsOnTheFirstLineOfItsEntry();
  void installingAUserInputLeavesTheEditorHeightToTheWindow();
  void informationLineStandsUnderTheBuffersAndNamesThem();
  void aKilledBufferTakesItsInformationLineWithIt();
  void changeFontReachesTheWidgetsWhichReadTheSetting();

 private:
  void pump(int n = 4);
};

void TestEditorWidget::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-editor-widget");
}  // end of TestEditorWidget::initTestCase

void TestEditorWidget::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestEditorWidget::pump

void TestEditorWidget::minibufferPagesHaveTheSameSingleLineHeight() {
  Shell shell(QSize(800, 600));
  shell.show();
  shell.pump(8);
  // the informative message area is up: the minibuffer is one line tall
  const auto one_line = shell.ed->minibuffer->height();
  QVERIFY2(one_line > 0, "the minibuffer is not shown");
  // `open file` replaces the message with the input, another page of the
  // minibuffer: a page whose single line is one pixel taller than another
  // resizes the buffers every time the page changes, which is issue #121
  shell.ed->openFile();
  shell.pump(6);
  QVERIFY2(currentInput(shell) != nullptr, "no input was installed");
  QCOMPARE(shell.ed->minibuffer->height(), one_line);
  // a message asked for explicitly, while an input waits, is the third page
  // and can not impose a height of its own either
  shell.ed->displayInformativeMessage("a message over the input");
  shell.pump(6);
  QCOMPARE(shell.ed->minibuffer->height(), one_line);
}  // end of
   // TestEditorWidget::minibufferPagesHaveTheSameSingleLineHeight

void TestEditorWidget::promptSitsOnTheFirstLineOfItsEntry() {
  Shell shell(QSize(800, 600));
  shell.show();
  shell.pump(8);
  shell.ed->openFile();
  shell.pump(6);
  auto* const in = currentInput(shell);
  QVERIFY(in != nullptr);
  auto* const cl = shell.ed->findChild<tfel::gui::CommandLine*>();
  QVERIFY(cl != nullptr);
  auto* const label = cl->findChild<QLabel*>();
  QVERIFY(label != nullptr);
  // who is where, in pixels: what the top of the view port and the margin of
  // the document leave above the first line drawn in the entry, and where the
  // text of the label begins. A `QLabel` whose vertical policy is `Fixed` is
  // centred by the box whatever its own alignment says, which is how the
  // prompt came to sit in the middle of an entry of three lines.
  auto first_line_of_entry = [in]() {
    auto* const te = qobject_cast<QTextEdit*>(in);
    const auto dm = (te != nullptr) ? te->document()->documentMargin() : 0.;
    const auto vp = (te != nullptr) && (te->viewport() != nullptr)
                        ? te->viewport()->y()
                        : 0;
    return in->y() + vp + int(dm);
  };
  auto top_of_label_text = [label]() {
    const auto lc = label->contentsMargins();
    const auto room = label->height() - lc.top() - lc.bottom() -
                      QFontMetrics(label->font()).height();
    const auto align = label->alignment();
    return label->y() + lc.top() +
           ((align & Qt::AlignTop)
                ? 0
                : ((align & Qt::AlignBottom) ? room : room / 2));
  };
  QCOMPARE(top_of_label_text() - first_line_of_entry(), 0);
  auto* const te = qobject_cast<QTextEdit*>(in);
  QVERIFY(te != nullptr);
  te->setPlainText("aaa\nbbb\nccc");
  shell.pump(10);
  QCOMPARE(top_of_label_text() - first_line_of_entry(), 0);
  te->setPlainText("a\nb\nc\nd\ne\nf\ng\ng");
  shell.pump(10);
  QCOMPARE(top_of_label_text() - first_line_of_entry(), 0);
}  // end of TestEditorWidget::promptSitsOnTheFirstLineOfItsEntry

void TestEditorWidget::installingAUserInputLeavesTheEditorHeightToTheWindow() {
  Shell shell(QSize(1000, 700));
  shell.show();
  shell.pump(8);
  const auto editor_height = shell.ed->height();
  // the window gave the editor far more than its own hint: the height
  // propagation of a resized input must stop at the editor, resizing it to
  // what its own layout asks for, when it is stuck to the bottom edge, moved
  // it down and left the gray background of the window above it
  QVERIFY2(editor_height > 400, "the editor was not given the window");
  // what the editor holds under the buffers, between them and the minibuffer:
  // the row of information lines, which is one line tall and is not what a
  // growing input is owed. Accounting for it is what leaves the leftover what
  // it was.
  const auto leftover =
      shell.ed->height() - (shell.ed->buffers->height() +
                            shell.ed->infos->height() +
                            shell.ed->minibuffer->height());
  const auto line_height = shell.ed->infos->height();
  shell.ed->openFile();
  shell.pump(6);
  QVERIFY(currentInput(shell) != nullptr);
  QCOMPARE(shell.ed->height(), editor_height);
  // and nothing new appeared above the buffers or below the minibuffer: the
  // input took its second line out of the buffers, as it did before the row of
  // lines existed, and the row of lines stayed one line tall
  QCOMPARE(shell.ed->height() - (shell.ed->buffers->height() +
                                 shell.ed->infos->height() +
                                 shell.ed->minibuffer->height()),
           leftover);
  QCOMPARE(shell.ed->infos->height(), line_height);
}  // end of
   // TestEditorWidget::installingAUserInputLeavesTheEditorHeightToTheWindow

void TestEditorWidget::informationLineStandsUnderTheBuffersAndNamesThem() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto a = writeFile(dir, "alpha.txt", "first buffer\n");
  const auto b = writeFile(dir, "beta.txt", "second buffer\n");
  QVERIFY((!a.isEmpty()) && (!b.isEmpty()));
  Shell shell(QSize(1000, 700));
  shell.show();
  shell.pump(8);
  // The row of information lines is the second thing the editor lays out,
  // between the row of the file explorer panel and the buffers and the
  // minibuffer. It is one line tall and as wide as the editor: the mode line
  // names the buffer on the screen across the whole window, and the height the
  // window has over that is the text's, not the mode line's.
  auto* const row = shell.ed->infos;
  auto* const l = shell.ed->layout();
  QVERIFY2((row != nullptr) && (l != nullptr), "the editor has no layout");
  QVERIFY2(l->indexOf(row) == 1,
           "the row of information lines does not stand under the buffers");
  QVERIFY2(l->indexOf(shell.ed->minibuffer) == 2,
           "the minibuffer does not stand under the row of information lines");
  QVERIFY2(row->height() > 0, "the row of information lines is not drawn");
  QCOMPARE(row->width(), shell.ed->width());
  // The row is exactly one line of labels tall, and nothing more. The line's
  // layout stands on a widget of its own, which makes it a *top-level* layout,
  // and such a layout takes the style's default contents margins -- 9 pixels
  // top and bottom here -- where the nested layout of the buffer had none.
  // With those margins the row would be nearly as tall as two lines and the
  // text would have lost the height of a line whole, so the assertion is not
  // a bound but an equality with the height the labels of the line ask for.
  auto* const line = row->currentWidget();
  QVERIFY2(line != nullptr, "the row of information lines holds no page");
  auto tallest = 0;
  for (const auto* const l : line->findChildren<QLabel *>()) {
    tallest = qMax(tallest, l->sizeHint().height());
  }
  QVERIFY2(tallest > 0, "the information line holds no label to measure");
  QVERIFY2(qAbs(row->height() - tallest) <= 2,
           qPrintable(QString("the row of information lines is %1 px tall "
                              "against the %2 px its labels ask for")
                        .arg(row->height())
                        .arg(tallest)));
  // One page per buffer, and the page on display is the line of the buffer on
  // the screen: `insertBuffer` feeds the two stacks in the one call and
  // `setCurrentBuffer` switches them together, which is the only way the row
  // under the buffers can name the buffer which stands over them.
  shell.ed->openFile(a);
  shell.pump(4);
  auto& first = shell.ed->getCurrentBuffer();
  QCOMPARE(row->count(), shell.ed->buffers->count());
  QVERIFY2(row->currentWidget() == &first.getInformationLine(),
           "the line on display is not the line of the current buffer");
  QVERIFY2(shownInformation(*shell.ed).contains("alpha.txt"),
           qPrintable(shownInformation(*shell.ed)));
  shell.ed->openFile(b);
  shell.pump(4);
  auto& second = shell.ed->getCurrentBuffer();
  QVERIFY2(&second != &first, "opening the second file reused a buffer");
  QCOMPARE(row->count(), shell.ed->buffers->count());
  QVERIFY2(row->currentWidget() == &second.getInformationLine(),
           "changing the buffer did not change the line on display");
  QVERIFY2(shownInformation(*shell.ed).contains("beta.txt"),
           qPrintable(shownInformation(*shell.ed)));
  // and the way back: the line of the first buffer names it again, which is
  // what a line held inside the buffer could not do at the whole width of the
  // window
  shell.ed->changeBuffer(first.getBufferName());
  shell.pump(4);
  QVERIFY2(row->currentWidget() == &first.getInformationLine(),
           "the line on display did not follow the buffer back");
  QVERIFY2(shownInformation(*shell.ed).contains("alpha.txt"),
           qPrintable(shownInformation(*shell.ed)));
}  // end of TestEditorWidget::informationLineStandsUnderTheBuffersAndNamesThem

void TestEditorWidget::aKilledBufferTakesItsInformationLineWithIt() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto a = writeFile(dir, "alpha.txt", "first buffer\n");
  const auto b = writeFile(dir, "beta.txt", "second buffer\n");
  const auto c = writeFile(dir, "gamma.txt", "third buffer\n");
  QVERIFY((!a.isEmpty()) && (!b.isEmpty()) && (!c.isEmpty()));
  Shell shell(QSize(1000, 700));
  shell.show();
  shell.pump(8);
  auto* const row = shell.ed->infos;
  shell.ed->openFile(a);
  shell.pump(2);
  shell.ed->openFile(b);
  shell.pump(2);
  auto* const victim = &(shell.ed->getCurrentBuffer());
  shell.ed->openFile(c);
  shell.pump(2);
  QCOMPARE(row->count(), shell.ed->buffers->count());
  // Killing a buffer which is not the one on screen: its line stands in the row
  // under the buffers as a page of its own, and the kill has to take it out
  // there. A line left behind would keep naming `beta.txt` on the screen under
  // whatever buffer the editor makes current next, and the row would hold one
  // page more than the editor holds buffers.
  shell.ed->removeBuffer(victim);
  shell.pump(4);
  QCOMPARE(row->count(), shell.ed->buffers->count());
  QVERIFY2(row->currentWidget() ==
               &shell.ed->getCurrentBuffer().getInformationLine(),
           "the line on display is no longer the line of the current buffer");
  QVERIFY2(!shownInformation(*shell.ed).contains("beta.txt"),
           qPrintable(shownInformation(*shell.ed)));
  // and the killed buffer's line is gone from the row, not merely hidden: no
  // page of it names the file any more
  for (int i = 0; i != row->count(); ++i) {
    QString t;
    for (auto* const l : row->widget(i)->findChildren<QLabel*>()) {
      t += l->text();
    }
    QVERIFY2(!t.contains("beta.txt"),
             qPrintable(QString("page %1 still names beta.txt: %2")
                          .arg(i)
                          .arg(t)));
  }
}  // end of TestEditorWidget::aKilledBufferTakesItsInformationLineWithIt

void TestEditorWidget::changeFontReachesTheWidgetsWhichReadTheSetting() {
  Shell shell(QSize(800, 600));
  shell.show();
  shell.pump(8);
  // `getCurrentBuffer` before any user input is constructed
  auto& b = shell.ed->getCurrentBuffer();
  shell.ed->displayInformativeMessage("the informative message area");
  shell.pump(6);
  // what `Options > Select font` does once the dialog has stored the font: the
  // widgets which read `font/textedit` at construction are not children of one
  // another and Qt never replaces an explicit font with the inherited one, so
  // the font has to be handed to each of them
  const QFont big("Courier", 18);
  shell.ed->changeFont(big);
  shell.pump(8);
  QVERIFY(sameFont(b.getMainFrame().widget()->font(), big));
  QVERIFY(sameFont(shell.ed->um->font(), big));
  QVERIFY(sameFont(shell.ed->eui->font(), big));
  int labels = 0;
  // the mode line is not a child of the buffer any more: the editor shows it in
  // the row under the buffers, so the labels are looked for in the line itself.
  // The mode line shows its height of the tallest of its labels: a stale label
  // is a mode line the height of two lines. `ii` holds a pixmap and no text,
  // and is not meant to change font.
  for (auto* l : b.getInformationLine().findChildren<QLabel*>()) {
    if (l->text().isEmpty()) {
      continue;
    }
    ++labels;
    QVERIFY2(sameFont(l->font(), big),
             qPrintable(QString("label [") + l->text() + "] kept its font"));
  }
  QVERIFY2(labels > 0, "the mode line shows no label at all");
  // an input which is already waiting for an answer follows too: in the
  // reported screen a file name was being typed when the font changed
  shell.ed->openFile();
  shell.pump(6);
  auto* const in = currentInput(shell);
  QVERIFY(in != nullptr);
  const QFont small("Courier", 9);
  shell.ed->changeFont(small);
  shell.pump(8);
  QVERIFY(sameFont(in->font(), small));
  auto* const cl = shell.ed->findChild<tfel::gui::CommandLine*>();
  QVERIFY(cl != nullptr);
  auto* const label = cl->findChild<QLabel*>();
  if (label != nullptr) {
    QVERIFY(sameFont(label->font(), small));
  }
}  // end of
   // TestEditorWidget::changeFontReachesTheWidgetsWhichReadTheSetting

QTEST_MAIN(TestEditorWidget)

#include "TestEditorWidget.moc"
