/*!
 * \file   tests/TestMultiLineLineEdit.cxx
 * \brief  tests of the height of a multi line entry: one fixed height per
 *         line, shared between the frame and the document margin.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

#include "TFEL/GUI/MultiLineLineEdit.hxx"

// the invariants of issue #121, seen from the public interface: a line of a
// multi line entry is as tall whether the widget is drawn with a frame or
// without one, since the command line builds a frameless input and the
// message area a framed one and the minibuffer may not change height when it
// switches from one to the other; the height of the widget is
// `singleLineHeight + (lines - 1) * font height`; and it is measured in the
// width the text is drawn in, not in the width of a view port which was never
// laid out.
struct TestMultiLineLineEdit : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void singleLineHeightsMatchWithAndWithoutFrame();
  void heightGrowsOneLinePerLine();
  void heightMeasuredInWidgetWidthNotInViewportWidth();
  void readOnlyTextShowsTheStart();

 private:
  void pump(int n = 4);
};

void TestMultiLineLineEdit::initTestCase() {
  // the widgets read `font/textedit` from the settings when they are built:
  // the tests of one application must not read, nor write, the settings of
  // another
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-multi-line-line-edit");
}  // end of TestMultiLineLineEdit::initTestCase

void TestMultiLineLineEdit::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestMultiLineLineEdit::pump

namespace {

  // an entry is never shown as a window of its own in the application, and a
  // window 19 pixels tall does not stay 19 pixels tall on a live display: the
  // window manager has its own opinion about that. The host plays the role of
  // the minibuffer: the widget is as tall as its content asks, in a window
  // tall enough for nothing else to decide its height.
  struct Host : QWidget {
    explicit Host(QWidget* const w) {
      auto* const l = new QVBoxLayout(this);
      l->setContentsMargins(0, 0, 0, 0);
      l->addWidget(w);
      resize(1000, 900);
      show();
    }
  };  // end of struct Host

}  // end of namespace

void TestMultiLineLineEdit::singleLineHeightsMatchWithAndWithoutFrame() {
  // the entries belong to their host: `addWidget` reparents them and the host
  // deletes them, so they live on the heap
  auto* const framed = new tfel::gui::MultiLineLineEdit;
  auto* const frameless = new tfel::gui::MultiLineLineEdit;
  framed->setFrameStyle(int(QFrame::StyledPanel) | int(QFrame::Sunken));
  frameless->setFrameStyle(QFrame::NoFrame);
  Host host_framed(framed);
  Host host_frameless(frameless);
  pump();
  QVERIFY2(framed->frameWidth() > 0, "the framed input has no frame to spare");
  QCOMPARE(frameless->frameWidth(), 0);
  // what `alignDocumentMargin` is there for: the frame takes its part of the
  // padding of a line and the document margin holds the rest, so that both
  // show a single line of the same height
  QCOMPARE(framed->height(), frameless->height());
  QCOMPARE(framed->firstLineTop(), frameless->firstLineTop());
  // with more than one line the frame enters the height once only: the extra
  // lines are pure text
  framed->setText("aaa\nbbb\nccc");
  frameless->setText("aaa\nbbb\nccc");
  pump();
  QCOMPARE(framed->height(), frameless->height());
}  // end of TestMultiLineLineEdit::singleLineHeightsMatchWithAndWithoutFrame

void TestMultiLineLineEdit::heightGrowsOneLinePerLine() {
  auto* const e = new tfel::gui::MultiLineLineEdit;
  e->setFrameStyle(int(QFrame::StyledPanel) | int(QFrame::Sunken));
  Host host(e);
  pump();
  const auto single = e->fontMetrics().height() +
                      2 * static_cast<int>(e->document()->documentMargin()) +
                      2 * e->frameWidth();  // what `singleLineHeight` computes
  QVERIFY2(e->height() >= single, "the widget is shorter than one line");
  const auto lines = [](const int n) {
    QStringList l;
    for (int k = 1; k <= n; ++k) {
      l << QString::number(k);
    }
    return l.join('\n');
  };
  const auto height_of = [e, &lines](const int n) {
    e->setPlainText(lines(n));
    for (int k = 0; k != 6; ++k) {
      QCoreApplication::processEvents();
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    return e->height();
  };
  // what one line costs is how the font resolves on the platform of the moment
  // (`fontMetrics().height()` and the document line spacing need not agree,
  // and on a live display the session font is not the offscreen one); what
  // must hold everywhere is that every line costs the same: one fixed height
  // per line.
  const auto height_1 = height_of(1);
  const auto height_2 = height_of(2);
  const auto line = height_2 - height_1;
  QVERIFY(line > 0);
  auto previous = height_2;
  for (int n = 3; n <= 9; ++n) {
    const auto h = height_of(n);
    QCOMPARE(h - previous, line);
    previous = h;
  }
  // the content is never allowed past `max_lines`, ten lines: a taller input
  // would take the room of the document the user is editing. Up to ten lines
  // each one is paid for; past it, the widget stops growing and the scroll bar
  // takes over.
  const auto height_10 = height_of(10);
  QVERIFY(height_10 > previous);  // the ninth line is not the last free one
  QCOMPARE(height_of(11), height_10);
  QCOMPARE(height_of(12), height_10);
}  // end of TestMultiLineLineEdit::heightGrowsOneLinePerLine

void TestMultiLineLineEdit::heightMeasuredInWidgetWidthNotInViewportWidth() {
  // the vertical bar shown for a content which does not fit the height of the
  // moment, or a view port which was laid out before the widget was resized
  // while hidden, both give the view port a width the text is not drawn in:
  // counting lines there asks for a line more, which asks for a bar more,
  // which latches the input one line too tall. Two widgets given the same
  // text in the same width can not disagree on their height, however the
  // widths of their view ports were reached.
  const auto text =
      QString("the quick brown fox jumps over the lazy dog, ") +
      "then it sits and waits for the rain to pass, which it does not,";
  tfel::gui::MultiLineLineEdit shown_first;
  shown_first.resize(400, 20);
  shown_first.show();
  pump();
  shown_first.setText(text);
  pump();
  tfel::gui::MultiLineLineEdit shown_last;
  shown_last.resize(120, 20);  // the view port learns this width
  shown_last.resize(400, 20);  // the widget moves on, the view port may not
  shown_last.setText(text);
  pump();
  shown_last.show();
  pump();
  QCOMPARE(shown_last.height(), shown_first.height());
}  // end of
   // TestMultiLineLineEdit::heightMeasuredInWidgetWidthNotInViewportWidth

void TestMultiLineLineEdit::readOnlyTextShowsTheStart() {
  // a read only widget shows a text rather than receiving one: the cursor
  // goes to the start, as the view follows the cursor and the end placement
  // an editable widget wants would leave a text longer than the height of
  // the message area displaying its tail only — the whole message of the
  // minibuffer being one line of "clangd" when it is a path wrapped over
  // three lines
  const auto message = QStringLiteral(
      "using /home/user/project/build/compile_commands.json "
      "to start clangd");
  tfel::gui::MultiLineLineEdit read_only;
  read_only.setReadOnly(true);
  read_only.setText(message);
  QCOMPARE(read_only.text(), message);
  QCOMPARE(read_only.textCursor().position(), 0);
  // an editable widget receives a text to type into: there, as in a
  // `QLineEdit`, the cursor is where the next key will act, that is the end
  tfel::gui::MultiLineLineEdit editable;
  editable.setText(message);
  QCOMPARE(editable.textCursor().position(), static_cast<int>(message.size()));
}  // end of TestMultiLineLineEdit::readOnlyTextShowsTheStart

QTEST_MAIN(TestMultiLineLineEdit)

#include "TestMultiLineLineEdit.moc"
