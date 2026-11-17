/*!
 * \file   tests/TestFileExplorer.cxx
 * \brief  tests of the file explorer panel of the editor and of the `Display`
 *         menu which shows it
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FileExplorer.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/MainWindow.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

// The panel is a class of the core library, not a private widget of the
// window, and the tree it is built from is found with `findChild`: what the
// request asks for is on the screen, and that is what is asserted here, never
// a member of `MainWindow`.
struct TestFileExplorer : QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void theDisplayMenuStandsBetweenEditAndBuffers();
  void thePanelIsHiddenUntilTheEntryIsTriggered();
  void aShownPanelIsAboutAThirdOfTheEditor();
  void thePanelEndsAboveTheInformationLineAndTheMinibuffer();
  void thePanelKeepsTheWidthTheUserGaveIt();
  void theTicksOfTheEntriesNameWhatIsOnTheScreen();
  void thePanelIsRootedAtTheDirectoryOfTheBuffer();
  void aBufferChangeLeavesThePanelWhereItIs();
  void goingUpRootsThePanelAtTheParentDirectory();
  void aDoubleClickOpensTheFileItNames();
  void aDoubleClickOnADirectoryOpensNothing();
  void theAuxiliaryTasksEntryHidesAndShowsTheTasksOfTheBuffer();

 private:
  //! \brief run the event loop, and the deferred deletions it holds
  static void pump(int n = 4);

  //! \return the titles of the menus of the menu bar, in order
  static QStringList menuTitles(tfel::gui::MainWindow &);
  //! \return the menu of the given title, null when the bar holds none
  static QMenu *menu(tfel::gui::MainWindow &, const QString &title);
  //! \return the entry whose text holds the given words, null when none
  static QAction *entry(QMenu *, const QString &words);
  //! \return the row of the given name among the children of the parent index
  static int rowOf(QAbstractItemModel *, const QModelIndex &, const QString &);
  //! \return the row of the given name, waiting up to 3 s for the listing
  static int waitForRow(QTreeView *, const QString &, const QModelIndex &);
  //! \return the centre of a row, once the view has laid it out
  static QPoint placeOf(QTreeView *, const QModelIndex &);
  /*!
   * \brief press the left button twice at the given row, the way a hand does
   *
   * `QTest::mouseDClick` sends the double click event and nothing else, and in
   * an inactive window — which every window of an offscreen run is — a view
   * which was never clicked ignores it: the file stays unopened and the
   * directory unexpanded. The single click a real double click always begins
   * with is what the recipe adds first, measured the same way offscreen and
   * on a live display.
   */
  static void doubleClickOn(QTreeView *, const QModelIndex &);
  /*!
   * \brief write a file of the given directory and return its name
   *
   * A helper which answers something can not use `QVERIFY`: the macro
   * expands to a bare `return`. The caller asserts on the name being
   * non-empty.
   */
  static QString write(QTemporaryDir &, const QString &name,
                       const QString &text);
};

void TestFileExplorer::initTestCase() {
  QCoreApplication::setOrganizationName("tfel-editor-tests");
  QCoreApplication::setApplicationName("test-file-explorer");
}  // end of TestFileExplorer::initTestCase

void TestFileExplorer::pump(int n) {
  for (int k = 0; k != n; ++k) {
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  }
}  // end of TestFileExplorer::pump

QStringList TestFileExplorer::menuTitles(tfel::gui::MainWindow &w) {
  auto s = QStringList{};
  // the list is bound to a name: `qAsConst` of a temporary is a deleted
  // overload, and a range-for over a temporary of `operator<<` is a
  // use-after-scope
  const auto actions = w.menuBar()->actions();
  for (const auto &a : actions) {
    s << a->text();
  }
  return s;
}  // end of TestFileExplorer::menuTitles

QMenu *TestFileExplorer::menu(tfel::gui::MainWindow &w, const QString &title) {
  const auto actions = w.menuBar()->actions();
  for (const auto &a : actions) {
    if (a->text() == title) {
      return a->menu();
    }
  }
  return nullptr;
}  // end of TestFileExplorer::menu

QAction *TestFileExplorer::entry(QMenu *m, const QString &words) {
  if (m == nullptr) {
    return nullptr;
  }
  const auto actions = m->actions();
  for (const auto &a : actions) {
    if (a->text().contains(words)) {
      return a;
    }
  }
  return nullptr;
}  // end of TestFileExplorer::entry

int TestFileExplorer::rowOf(QAbstractItemModel *m,
                            const QModelIndex &parent, const QString &name) {
  for (int i = 0; i != m->rowCount(parent); ++i) {
    if (m->index(i, 0, parent).data(Qt::DisplayRole).toString() == name) {
      return i;
    }
  }
  return -1;
}  // end of TestFileExplorer::rowOf

int TestFileExplorer::waitForRow(QTreeView *view,
                                 const QString &name,
                                 const QModelIndex &parent) {
  // the listing of a directory reaches a `QFileSystemModel` through a socket
  // notifier, which `processEvents` never expires: `QTest::qWait` is what waits
  // for it, the same trap the output of a pipe carries
  auto row = -1;
  for (int i = 0; (i != 30) && (row == -1); ++i) {
    QTest::qWait(100);
    row = rowOf(view->model(), parent, name);
  }
  return row;
}  // end of TestFileExplorer::waitForRow

QPoint TestFileExplorer::placeOf(QTreeView *view, const QModelIndex &i) {
  // `visualRect` of a row the view has not laid out is an empty rectangle, and
  // a click at its centre is a click on the blank part of the viewport
  for (int k = 0; k != 20; ++k) {
    const auto r = view->visualRect(i);
    if (r.isValid() && (r.height() > 0) && (view->viewport()->height() > 0)) {
      return r.center();
    }
    QTest::qWait(50);
  }
  return QPoint(-1, -1);
}  // end of TestFileExplorer::placeOf

void TestFileExplorer::doubleClickOn(QTreeView *view, const QModelIndex &i) {
  const auto where = placeOf(view, i);
  QVERIFY2(where.x() >= 0, "the row of the tree was never laid out");
  QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, where);
  pump(3);
  QTest::mouseDClick(view->viewport(), Qt::LeftButton, {}, where);
  pump(6);
}  // end of TestFileExplorer::doubleClickOn

QString TestFileExplorer::write(QTemporaryDir &dir,
                                const QString &name, const QString &text) {
  const auto path = dir.filePath(name);
  QFile f(path);
  if (not f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return QString();
  }
  f.write(text.toUtf8());
  f.close();
  return path;
}  // end of TestFileExplorer::write

void TestFileExplorer::theDisplayMenuStandsBetweenEditAndBuffers() {
  tfel::gui::MainWindow w;
  const auto titles = menuTitles(w);
  const auto edit = titles.indexOf("Edit");
  const auto display = titles.indexOf("Display");
  const auto buffers = titles.indexOf("Buffers");
  QVERIFY2((edit >= 0) && (display >= 0) && (buffers >= 0),
           qPrintable(titles.join(' ')));
  QCOMPARE(display, edit + 1);
  QCOMPARE(buffers, display + 1);
  // the two entries of the menu, and nothing else: no separator, no submenu. An
  // entry which carries a submenu can not be clicked — Qt opens the submenu
  // instead of triggering the entry — which is the trap the spelling menu of
  // issue #131 paid for
  auto *const m = menu(w, "Display");
  QVERIFY(m != nullptr);
  const auto actions = m->actions();
  QCOMPARE(actions.size(), 2);
  for (const auto &a : actions) {
    QVERIFY2(not a->isSeparator(), "the menu holds a separator");
    QVERIFY2(a->menu() == nullptr, "an entry of the menu is a submenu");
    QVERIFY2(a->isCheckable(), "an entry of the menu is not checkable");
  }
  QVERIFY2(actions.at(0)->text().contains("Explorer Panel"),
           qPrintable(actions.at(0)->text()));
  QVERIFY2(actions.at(1)->text().contains("Auxiliary"),
           qPrintable(actions.at(1)->text()));
}  // end of TestFileExplorer::theDisplayMenuStandsBetweenEditAndBuffers

void TestFileExplorer::thePanelIsHiddenUntilTheEntryIsTriggered() {
  tfel::gui::MainWindow w;
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (ed != nullptr) && (m != nullptr));
  auto *const show = entry(m, "Explorer Panel");
  QVERIFY(show != nullptr);
  // hidden at the start, and the buffers have the whole width: the request asks
  // for a panel which is not there until it is asked for. What the panel takes
  // its width from is the buffers, not the window: the editor keeps its own
  // width whatever the entry does, which the two last assertions say of the
  // editor as a whole.
  auto &b = ed->getCurrentBuffer();
  QVERIFY2(ex->isVisible() == false, "the panel is shown at the start");
  const auto whole = b.getMainFrame().width();
  const auto editor_width = ed->width();
  QVERIFY2(whole > 800,
           qPrintable(QString("the buffer is %1 px wide").arg(whole)));
  show->trigger();
  pump(4);
  QVERIFY2(ex->isVisible(), "the entry did not show the panel");
  QVERIFY2(b.getMainFrame().width() < whole,
           "the panel took nothing from the buffers");
  QCOMPARE(ed->width(), editor_width);
  show->trigger();
  pump(4);
  QVERIFY2(ex->isVisible() == false, "the entry did not hide the panel");
  QCOMPARE(b.getMainFrame().width(), whole);
  QCOMPARE(ed->width(), editor_width);
}  // end of TestFileExplorer::thePanelIsHiddenUntilTheEntryIsTriggered

void TestFileExplorer::aShownPanelIsAboutAThirdOfTheEditor() {
  tfel::gui::MainWindow w;
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  auto *const splitter = qobject_cast<QSplitter *>(ex->parentWidget());
  QVERIFY((ex != nullptr) && (ed != nullptr) && (m != nullptr) &&
          (splitter != nullptr));
  const auto window_height = w.height();
  const auto editor_height = ed->height();
  const auto editor_width = ed->width();
  m->actions().at(0)->trigger();
  pump(6);
  // The ratio is that of the panel to the editor it stands in, not to the
  // window: the panel shares the width of the editor with the buffers, and on a
  // live display the window holds the border the window manager gave it, which
  // the editor has nothing to do with.
  const auto ratio = double(ex->width()) / double(ed->width());
  QVERIFY2((ratio > 0.28) && (ratio < 0.39),
           qPrintable(QString("the panel is %1 px of %2: %3")
                        .arg(ex->width())
                        .arg(ed->width())
                        .arg(ratio)));
  // the two sides of the splitter and its handle do not exceed the editor: the
  // sizes handed to `QSplitter::setSizes` add up to the width of the splitter,
  // since that call resizes the splitter itself to their sum
  const auto buffer_width = ed->getCurrentBuffer().getMainFrame().width();
  QVERIFY2(ex->width() + buffer_width + splitter->handleWidth() <=
                   ed->width() + 2,
           qPrintable(QString("%1 + %2 px of a %3 px editor")
                        .arg(ex->width())
                        .arg(buffer_width)
                        .arg(ed->width())));
  // the height of the window is the business of the window, not of the panel:
  // showing it takes a third of the width and nothing of the height, which is
  // what the minibuffer of issue #121 rests on
  QCOMPARE(w.height(), window_height);
  QCOMPARE(ed->height(), editor_height);
  QCOMPARE(ed->width(), editor_width);
}  // end of TestFileExplorer::aShownPanelIsAboutAThirdOfTheEditor

void TestFileExplorer::
    thePanelEndsAboveTheInformationLineAndTheMinibuffer() {
  tfel::gui::MainWindow w;
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (ed != nullptr) && (m != nullptr));
  m->actions().at(0)->trigger();
  pump(6);
  // The row of the panel is the row of the buffers: the two stand in one
  // `QSplitter`, the first thing the layout of the editor holds. Under it
  // stands the information line of the buffer on the screen, and under that the
  // minibuffer, which holds the informative message area and the line the user
  // types in. The panel therefore ends where the text of the buffer does, above
  // the two lines of the window, and not beside the whole editor.
  auto *const row = ex->parentWidget();
  auto *const l = ed->layout();
  QVERIFY2((row != nullptr) && (l != nullptr), "the editor has no layout");
  QVERIFY2(l->indexOf(row) == 0,
           "the row of the panel is not what the editor lays out first");
  auto *const line = l->itemAt(1)->widget();
  auto *const mini = l->itemAt(2)->widget();
  QVERIFY2((line != nullptr) && (mini != nullptr),
           "the editor does not lay out three rows");
  // the information line is the row of labels, the minibuffer the row which
  // holds what the user reads and types: nothing else is on their two places
  QVERIFY2(line->findChild<QLabel *>() != nullptr,
           "the row under the buffers holds no label");
  QVERIFY2(line->findChild<tfel::gui::LineEdit *>() == nullptr,
           "the row under the buffers is no information line");
  auto *const bar = mini->findChild<tfel::gui::LineEdit *>();
  QVERIFY2(bar != nullptr, "the last row of the editor is no minibuffer");
  QVERIFY2(bar->isReadOnly(), "the informative message area is editable");
  // the panel is as tall as the row it stands in, and that row ends where the
  // information line begins: the two edges meet, which is what "above" means
  QCOMPARE(ex->height(), row->height());
  QCOMPARE(line->geometry().top(), row->geometry().bottom() + 1);
  QCOMPARE(mini->geometry().top(), line->geometry().bottom() + 1);
  // and the two lines keep the whole width of the editor, as they do when no
  // panel is shown: the third is taken out of the row of the buffers only
  QCOMPARE(line->width(), ed->width());
  QCOMPARE(mini->width(), ed->width());
  const auto left_of_the_line = line->mapTo(ed, QPoint(0, 0));
  const auto bottom_of_the_panel = ex->mapTo(ed, QPoint(0, ex->height()));
  QCOMPARE(left_of_the_line.x(), 0);
  QCOMPARE(bar->width(), ed->width());
  QCOMPARE(bottom_of_the_panel.y(), left_of_the_line.y());
}  // end of TestFileExplorer::thePanelEndsAboveTheInformationLineAnd...

void TestFileExplorer::thePanelKeepsTheWidthTheUserGaveIt() {
  tfel::gui::MainWindow w;
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const m = menu(w, "Display");
  auto *const splitter = qobject_cast<QSplitter *>(ex->parentWidget());
  QVERIFY((ex != nullptr) && (m != nullptr) && (splitter != nullptr));
  auto *const show = entry(m, "Explorer Panel");
  QVERIFY(show != nullptr);
  show->trigger();
  pump(4);
  // what a drag of the handle answers is what `setSizes` answers: the user is
  // given 400 px, and the panel is owed to keep them across a hide and a show,
  // since the splitter remembers the width a hidden child was last given
  const auto total = splitter->width();
  splitter->setSizes({400, total - 400});
  pump(4);
  QVERIFY2(qAbs(ex->width() - 400) <= 4,
           qPrintable(QString("the drag gave %1 px").arg(ex->width())));
  show->trigger();
  pump(4);
  show->trigger();
  pump(4);
  QVERIFY2(qAbs(ex->width() - 400) <= 4,
           qPrintable(QString("after a hide and a show: %1 px")
                        .arg(ex->width())));
}  // end of TestFileExplorer::thePanelKeepsTheWidthTheUserGaveIt

void TestFileExplorer::theTicksOfTheEntriesNameWhatIsOnTheScreen() {
  tfel::gui::MainWindow w;
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ed != nullptr) && (m != nullptr));
  auto *const explorer_entry = entry(m, "Explorer Panel");
  auto *const tasks_entry = entry(m, "Auxiliary");
  QVERIFY((explorer_entry != nullptr) && (tasks_entry != nullptr));
  // nothing ticks a checkable entry but the code, so the two entries answer
  // what the window says from the moment it is built, not from the moment its
  // menu is raised
  QVERIFY2(explorer_entry->isChecked() == false, "the panel is ticked, hidden");
  QVERIFY2(tasks_entry->isChecked() == false, "the tasks are ticked, hidden");
  explorer_entry->trigger();
  pump(4);
  QVERIFY2(explorer_entry->isChecked(), "showing the panel left it unticked");
  // a buffer which holds no auxiliary task has nothing to show: the entry says
  // so instead of ticking itself over an empty page
  tasks_entry->trigger();
  pump(4);
  QVERIFY2(tasks_entry->isChecked() == false,
           "the entry ticked itself on a buffer with no task");
  // and a task attached by other means — a compilation, a session — reaches the
  // tick when the menu is opened, which is what `aboutToShow` is for: the entry
  // is not what made the page appear
  // the frame is what a compilation, a session of the interpreter, or the
  // overview of a file hands the buffer, and the entry of the menu is the way
  // back to it once its tab was closed. The title form is what attaches it: the
  // one which names no title only re-shows a widget the registry already holds
  auto &b = ed->getCurrentBuffer();
  auto *const frame = new tfel::gui::ProcessOutputFrame(*ed, b);
  b.attachSecondaryTask("*output*", frame);
  pump(4);
  QVERIFY2(b.areSecondaryTasksVisible(),
           "attaching a task left its page hidden");
  QVERIFY2(tasks_entry->isChecked() == false, "the tick jumped ahead");
  QMetaObject::invokeMethod(m, "aboutToShow");
  pump(2);
  QVERIFY2(tasks_entry->isChecked(),
           "opening the menu did not ask what is on the screen");
}  // end of TestFileExplorer::theTicksOfTheEntriesNameWhatIsOnTheScreen

void TestFileExplorer::thePanelIsRootedAtTheDirectoryOfTheBuffer() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto path = write(dir, "first.txt", "the file of the window\n");
  QVERIFY(not path.isEmpty());
  tfel::gui::MainWindow w(QStringList() << path);
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (m != nullptr));
  // the panel builds itself rooted at the working directory, like the widgets
  // which read a setting when they are built, and stays there until it is shown
  QCOMPARE(ex->rootDirectory(), QDir::current().absolutePath());
  m->actions().at(0)->trigger();
  pump(6);
  // the first show is what roots it, at the directory of the file the buffer on
  // the screen visits: the buffers the window was given are known by then
  QCOMPARE(QDir(ex->rootDirectory()).absolutePath(),
           QFileInfo(path).absolutePath());
  // a name which is no directory leaves the panel where it was: a panel which
  // went blank on a typo is worse than one which stayed put
  ex->setRootDirectory(QString());
  QCOMPARE(QDir(ex->rootDirectory()).absolutePath(),
           QFileInfo(path).absolutePath());
  ex->setRootDirectory(dir.filePath("no_such_directory"));
  QCOMPARE(QDir(ex->rootDirectory()).absolutePath(),
           QFileInfo(path).absolutePath());
  ex->setRootDirectory(path);
  QCOMPARE(QDir(ex->rootDirectory()).absolutePath(),
           QFileInfo(path).absolutePath());
}  // end of TestFileExplorer::thePanelIsRootedAtTheDirectoryOfTheBuffer

void TestFileExplorer::aBufferChangeLeavesThePanelWhereItIs() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto first = write(dir, "first.txt", "the first file\n");
  const auto second = write(dir, "second.txt", "the second file\n");
  QVERIFY((not first.isEmpty()) && (not second.isEmpty()));
  tfel::gui::MainWindow w(QStringList() << first);
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (ed != nullptr) && (m != nullptr));
  m->actions().at(0)->trigger();
  pump(6);
  const auto root = ex->rootDirectory();
  const auto width = ex->width();
  const auto name = ed->getCurrentBuffer().getBufferName();
  // The panel is what the user pointed at, not what the buffers are going
  // through: nothing connects it to the current buffer, and opening a file of
  // another directory, then coming back, leaves the root, the width and the
  // visibility of the panel exactly as they were.
  ed->openFile(second);
  pump(6);
  QCOMPARE(ex->rootDirectory(), root);
  QCOMPARE(ex->width(), width);
  QVERIFY2(ex->isVisible(), "opening a file hid the panel");
  ed->changeBuffer(name);
  pump(6);
  QCOMPARE(ex->rootDirectory(), root);
  QCOMPARE(ex->width(), width);
  QVERIFY2(ex->isVisible(), "changing the buffer hid the panel");
}  // end of TestFileExplorer::aBufferChangeLeavesThePanelWhereItIs

void TestFileExplorer::goingUpRootsThePanelAtTheParentDirectory() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto path = write(dir, "first.txt", "the file of the window\n");
  QVERIFY(not path.isEmpty());
  tfel::gui::MainWindow w(QStringList() << path);
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (m != nullptr));
  m->actions().at(0)->trigger();
  pump(6);
  const auto directory = QDir(ex->rootDirectory()).absolutePath();
  ex->goUp();
  pump(4);
  QCOMPARE(QDir(ex->rootDirectory()).absolutePath(),
           QFileInfo(directory).absolutePath());
  QVERIFY2(ex->rootDirectory() != directory, "going up stayed where it was");
}  // end of TestFileExplorer::goingUpRootsThePanelAtTheParentDirectory

void TestFileExplorer::aDoubleClickOpensTheFileItNames() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto opened_at_start = write(dir, "first.txt", "the first file\n");
  const auto note = write(dir, "note.txt", "hello explorer\n");
  QVERIFY((not opened_at_start.isEmpty()) && (not note.isEmpty()));
  tfel::gui::MainWindow w(QStringList() << opened_at_start);
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (ed != nullptr) && (m != nullptr));
  auto *const view = ex->findChild<QTreeView *>();
  QVERIFY2(view != nullptr, "the panel holds no tree view");
  m->actions().at(0)->trigger();
  pump(6);
  const auto root = view->rootIndex();
  const auto row = waitForRow(view, "note.txt", root);
  QVERIFY2(row >= 0, "the file never reached the tree");
  QCOMPARE(ed->getBuffersNames().size(), 1);
  doubleClickOn(view, view->model()->index(row, 0, root));
  // the whole point of the panel: the name in the tree becomes a buffer holding
  // the contents of the file it names
  QCOMPARE(ed->getBuffersNames().size(), 2);
  auto &b = ed->getCurrentBuffer();
  QVERIFY2(b.getBufferRawName().endsWith("note.txt"),
           qPrintable(b.getBufferRawName()));
  QCOMPARE(b.getMainFrame().document()->toPlainText(),
           QString("hello explorer\n"));
  // the same file again: what reaches the screen is the buffer which already
  // holds it, not a second one reading the file a second time
  doubleClickOn(view, view->model()->index(row, 0, root));
  QCOMPARE(ed->getBuffersNames().size(), 2);
  QVERIFY2(ed->getCurrentBuffer().getBufferRawName().endsWith("note.txt"),
           qPrintable(ed->getCurrentBuffer().getBufferRawName()));
}  // end of TestFileExplorer::aDoubleClickOpensTheFileItNames

void TestFileExplorer::aDoubleClickOnADirectoryOpensNothing() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto opened_at_start = write(dir, "first.txt", "the first file\n");
  QVERIFY(QDir(dir.path()).mkpath("sub"));
  const auto inside = write(dir, "sub/inside.txt", "inside the directory\n");
  QVERIFY((not opened_at_start.isEmpty()) && (not inside.isEmpty()));
  tfel::gui::MainWindow w(QStringList() << opened_at_start);
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ex = w.findChild<tfel::gui::FileExplorer *>();
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ex != nullptr) && (ed != nullptr) && (m != nullptr));
  auto *const view = ex->findChild<QTreeView *>();
  QVERIFY2(view != nullptr, "the panel holds no tree view");
  m->actions().at(0)->trigger();
  pump(6);
  const auto root = view->rootIndex();
  const auto row = waitForRow(view, "sub", root);
  QVERIFY2(row >= 0, "the directory never reached the tree");
  const auto before = ed->getBuffersNames().size();
  const auto directory = view->model()->index(row, 0, root);
  // the name of the row, not its branch decoration: the decoration is what
  // `QTreeView` expands on its own account, the name is where a hand points
  doubleClickOn(view, directory);
  QCOMPARE(ed->getBuffersNames().size(), before);
  QVERIFY2(view->isExpanded(directory),
           "a double click on the name of a directory left it closed");
  QVERIFY2(waitForRow(view, "inside.txt", directory) >= 0,
           "the tree did not reach the contents of the directory");
}  // end of TestFileExplorer::aDoubleClickOnADirectoryOpensNothing

void TestFileExplorer::
    theAuxiliaryTasksEntryHidesAndShowsTheTasksOfTheBuffer() {
  tfel::gui::MainWindow w;
  w.resize(1000, 700);
  w.show();
  pump(8);
  auto *const ed = w.findChild<tfel::gui::EditorWidget *>();
  auto *const m = menu(w, "Display");
  QVERIFY((ed != nullptr) && (m != nullptr));
  auto *const tasks_entry = entry(m, "Auxiliary");
  QVERIFY(tasks_entry != nullptr);
  auto &b = ed->getCurrentBuffer();
  // the frame is what a compilation, a session of the interpreter, or the
  // overview of a file hands the buffer: the entry of the menu is the way back
  // to it once the tab was closed. The title form attaches it; the
  // one-argument form is what a mode re-shows an already registered widget
  // with, and on a widget the registry does not hold it does nothing at all,
  // silently.
  auto *const frame = new tfel::gui::ProcessOutputFrame(*ed, b);
  b.attachSecondaryTask("*output*", frame);
  pump(4);
  QVERIFY2(b.areSecondaryTasksVisible(),
           "attaching a task left its page hidden");
  tasks_entry->trigger();
  pump(4);
  QVERIFY2(b.areSecondaryTasksVisible() == false,
           "the entry did not hide the auxiliary tasks");
  QVERIFY2(tasks_entry->isChecked() == false,
           "the tick disagreed with the page it hides");
  tasks_entry->trigger();
  pump(4);
  QVERIFY2(b.areSecondaryTasksVisible(), "the entry did not show them back");
  QVERIFY2(tasks_entry->isChecked(), "the tick disagreed with the shown page");
}  // end of TestFileExplorer::theAuxiliaryTasksEntryHidesAndShowsTheTasksOf
    // TheBuffer

QTEST_MAIN(TestFileExplorer)

#include "TestFileExplorer.moc"
