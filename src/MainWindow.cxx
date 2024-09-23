/*!
 * \file   src/MainWindow.cxx
 * \brief
 * \author Helfer Thomas
 * \date   27/06/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QApplication>
#include <QtGui/QCloseEvent>
#include "TFEL/GUI/Debug.hxx"
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/HunspellDictionariesManager.hxx"
#include "TFEL/GUI/ShortCutStyle.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/MainWindow.hxx"

namespace tfel::gui {

  MainWindow::MainWindow(const QStringList &f, QWidget *const p)
      : QMainWindow(p) {
    auto &s = ShortCutStyle::getShortCutStyle();
    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setStyleSheet("background: rgba(255,255,255,100%)");
    auto *e = new EditorWidget(this);
    this->setCentralWidget(e);
    this->createMainMenu();
    QObject::connect(&s, &ShortCutStyle::shortCutStyleChanged, this,
                     &MainWindow::updateOptionsMenu);
    QObject::connect(e, &EditorWidget::closed, this, &MainWindow::close);
    QObject::connect(e, &EditorWidget::bufferNameChanged, this,
                     &MainWindow::updateBuffersMenu);
    QObject::connect(e, &EditorWidget::bufferAdded, this,
                     &MainWindow::updateBuffersMenu);
    QObject::connect(e, &EditorWidget::bufferRemoved, this,
                     &MainWindow::updateBuffersMenu);
    QObject::connect(e, &EditorWidget::currentBufferChanged, this,
                     &MainWindow::createMainMenu);
    QObject::connect(e, &EditorWidget::currentBufferMajorModeChanged, this,
                     &MainWindow::createMainMenu);
    QObject::connect(e, &EditorWidget::updatedMenu, this,
                     &MainWindow::createMainMenu);
    QObject::connect(e, &EditorWidget::newTreatedFile, this,
                     &MainWindow::addToRecentFiles);
    if (f.empty()) {
      e->createEmptyBuffer();
    } else {
      for (const auto &fn : f) {
        e->openFile(fn);
      }
    }
  }  // end of MainWindow::MainWindow

  void MainWindow::addToRecentFiles(const QString &f) {
    QSettings s;
    auto rf = s.value("recent files").toStringList();
    int p = 0;
    // remove previous appearance of the file
    p = rf.indexOf(f);
    while (p != -1) {
      rf.removeAt(p);
      p = rf.indexOf(f);
    }
    rf.push_back(f);
    if (rf.size() >= 50) {
      const auto file = rf.front();
      if (s.contains("positions in files")) {
        auto pl =
            s.value("positions in files").value<QMap<QString, QVariant>>();
        if (pl.contains(file)) {
          pl.remove(file);
          s.setValue("positions in files", pl);
        }
      }
      rf.pop_front();
    }
    s.setValue("recent files", rf);
  }  // end of MainWindow::addToRecentFiles

  void MainWindow::closeEvent(QCloseEvent *ev) {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      QObject::disconnect(e, &EditorWidget::closed, this, &MainWindow::close);
      if (e->close()) {
        ev->accept();
      } else {
        QObject::connect(e, &EditorWidget::closed, this, &MainWindow::close);
        ev->ignore();
      }
    }
  }  // end of MainWindow::close

  void MainWindow::openFile() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    const auto &f = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (f.isEmpty()) {
      return;
    }
    e->openFile(f);
  }  // end of MainWindow::openFile

  void MainWindow::selectFont() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      QSettings s;
      QFontDialog fd;
      fd.setCurrentFont(s.value("font/textedit").value<QFont>());
      if (fd.exec()) {
        auto f = fd.selectedFont();
        f.setStyleStrategy(QFont::PreferAntialias);
        s.setValue("font/textedit", f);
        e->changeMainFramesFont(f);
      }
    }
  }  // end of MainWindow::selectFont

  void MainWindow::createActions() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    this->na = new QAction(QObject::tr("N&ew"), this);
    this->na->setStatusTip(QObject::tr("New buffer"));
    this->na->setIcon(QIcon::fromTheme("document-new"));
    this->na->setIconVisibleInMenu(true);
    QObject::connect(this->na, &QAction::triggered, e,
                     &EditorWidget::createEmptyBuffer);
    this->oa = new QAction(QObject::tr("O&pen"), this);
    this->oa->setStatusTip(QObject::tr("Open a file"));
    this->oa->setIcon(QIcon::fromTheme("document-open"));
    this->oa->setIconVisibleInMenu(true);
    QObject::connect(this->oa, &QAction::triggered, this,
                     &MainWindow::openFile);
    this->sa = new QAction(QObject::tr("S&ave current buffer"), this);
    this->sa->setStatusTip(QObject::tr("Save the current buffer"));
    this->sa->setIcon(QIcon::fromTheme("document-save"));
    this->sa->setIconVisibleInMenu(true);
    QObject::connect(this->sa, &QAction::triggered, e,
                     &EditorWidget::saveCurrentBuffer);
    this->ka = new QAction(QObject::tr("C&lose current buffer"), this);
    this->ka->setStatusTip(QObject::tr("Close the current buffer"));
    this->ka->setIcon(QIcon::fromTheme("window-close"));
    QObject::connect(this->ka, &QAction::triggered, e,
                     &EditorWidget::closeCurrentBuffer);
    this->pra = new QAction(QObject::tr("Print current buffer"), this);
    this->pra->setIcon(QIcon::fromTheme("document-print"));
    this->pra->setIconVisibleInMenu(true);
    this->pra->setStatusTip(QObject::tr("Print the current buffer"));
    QObject::connect(this->pra, &QAction::triggered, this, &MainWindow::print);
    this->saa = new QAction(QObject::tr("S&ave all buffers"), this);
    this->saa->setStatusTip(QObject::tr("Save all buffers"));
    this->saa->setIcon(QIcon::fromTheme("document-save"));
    QObject::connect(this->saa, &QAction::triggered, e,
                     &EditorWidget::saveAllBuffers);
    // edit actions
    this->ua = new QAction(QObject::tr("U&ndo"), this);
    this->ua->setIcon(QIcon::fromTheme("edit-undo"));
    this->ua->setIconVisibleInMenu(true);
    QObject::connect(this->ua, &QAction::triggered, this, &MainWindow::undo);
    this->ra = new QAction(QObject::tr("R&edo"), this);
    this->ra->setIcon(QIcon::fromTheme("edit-redo"));
    this->ra->setIconVisibleInMenu(true);
    QObject::connect(this->ra, &QAction::triggered, this, &MainWindow::redo);
    this->sea = new QAction(QObject::tr("S&elect All"), this);
    this->sea->setIcon(QIcon::fromTheme("edit-select-all"));
    this->sea->setIconVisibleInMenu(true);
    QObject::connect(this->sea, &QAction::triggered, this,
                     &MainWindow::selectAll);
    this->ca = new QAction(QObject::tr("C&ut"), this);
    this->ca->setStatusTip(QObject::tr("Cut the selected text"));
    this->ca->setIcon(QIcon::fromTheme("edit-cut"));
    this->ca->setIconVisibleInMenu(true);
    QObject::connect(this->ca, &QAction::triggered, this, &MainWindow::cut);
    this->ca2 = new QAction(QObject::tr("C&opy"), this);
    this->ca2->setStatusTip(QObject::tr("Copy the selected text to clipboard"));
    this->ca2->setIcon(QIcon::fromTheme("edit-copy"));
    this->ca2->setIconVisibleInMenu(true);
    QObject::connect(this->ca2, &QAction::triggered, this, &MainWindow::copy);
    this->pa = new QAction(QObject::tr("P&aste"), this);
    this->pa->setStatusTip(QObject::tr("Paste from clipboard"));
    this->pa->setIcon(QIcon::fromTheme("edit-paste"));
    this->pa->setIconVisibleInMenu(true);
    QObject::connect(this->pa, &QAction::triggered, this, &MainWindow::paste);
    // exit actions
    this->ea = new QAction(QObject::tr("E&xit"), this);
    this->ea->setStatusTip(QObject::tr("Exit editor"));
    this->ea->setIcon(QIcon::fromTheme("window-close"));
    this->ea->setIconVisibleInMenu(true);
    QObject::connect(this->ea, &QAction::triggered, this, &MainWindow::close);
    // font actions
    this->fa = new QAction(QObject::tr("Main Frames' F&ont"), this);
    this->fa->setIcon(QIcon::fromTheme("preferences-desktop-font"));
    this->fa->setStatusTip(QObject::tr("Select Font"));
    QObject::connect(this->fa, &QAction::triggered, this,
                     &MainWindow::selectFont);
    this->esca =
        new QAction(QObject::tr("Change to emacs's style shortcuts"), this);
    this->esca->setStatusTip(QObject::tr("Use emacs's style shortcuts"));
    this->esca->setIcon(QIcon::fromTheme("preferences-desktop-keyboard"));
    QObject::connect(this->esca, &QAction::triggered, this,
                     &MainWindow::useEmacsShortCuts);
    this->qsca =
        new QAction(QObject::tr("Change to Qt's style shortcuts"), this);
    this->qsca->setStatusTip(QObject::tr("Use Qt's style shortcuts"));
    this->qsca->setIcon(QIcon::fromTheme("preferences-desktop-keyboard"));
    QObject::connect(this->qsca, &QAction::triggered, this,
                     &MainWindow::useQtShortCuts);
    // help actions
    this->aa = new QAction(QObject::tr("&About"), this);
    this->aa->setStatusTip(QObject::tr("Show the application's About box"));
    this->aa->setIcon(QIcon::fromTheme("help-about"));
    this->aa->setIconVisibleInMenu(true);
    connect(this->aa, &QAction::triggered, this, &MainWindow::about);
    this->aa2 = new QAction(QObject::tr("About &Qt"), this);
    this->aa2->setStatusTip(QObject::tr("Show the Qt library's About box"));
    connect(this->aa2, &QAction::triggered, qApp, &QApplication::aboutQt);
  }  // end of MainWindow::createActions

  void MainWindow::updateOptionsMenu() {
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
    using HunspellDictionaries = HunspellDictionariesManager;
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
    auto &s = ShortCutStyle::getShortCutStyle();
    this->om->clear();
    // font
    this->om->addAction(this->fa);
#ifdef TFEL_GUI_HUNSPELL_SUPPORT
    // available dictionaries
    auto &dm = HunspellDictionaries::getHunspellDictionariesManager();
    const auto dicts = dm.getAvailableDictionnaries();
    if (!dicts.isEmpty()) {
      auto *const d = this->om->addMenu(QObject::tr("Dictionaries"));
      d->setIcon(QIcon::fromTheme("accessories-dictionary"));
      for (const auto &di : dicts) {
        auto *a = d->addAction(di);
        a->setData(di);
        QObject::connect(a, &QAction::triggered, this, [di, this]() {
          auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
          if (e != nullptr) {
            auto &t = e->getCurrentBuffer().getMainFrame();
            t.setSpellCheckLanguage(di);
          }
        });
      }
    }
#endif /* TFEL_GUI_HUNSPELL_SUPPORT */
    // short cuts
    if (s.getStyle() != ShortCutStyle::EMACS) {
      this->om->addAction(this->esca);
    }
    if (s.getStyle() != ShortCutStyle::QT) {
      this->om->addAction(this->qsca);
    }
  }  // end of MainWindow::updateOptionsMenu

  void MainWindow::updateBuffersMenu() {
    clearMenu(this->bm);
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      const auto &bnames = e->getBuffersNames();
      const auto &bicons = e->getBuffersIcons();
      const auto &bids = e->getBuffersIds();
      if ((bnames.size() != bicons.size()) || (bnames.size() != bids.size())) {
        return;
      }
      for (int i = 0; i != bicons.size(); ++i) {
        const auto &n = bnames[i];
        const auto &ic = bicons[i];
        const int id = bids[i];
        auto *a = this->bm->addAction(n);
        if (!ic.isNull()) {
          a->setIcon(ic);
          a->setIconVisibleInMenu(true);
        }
        QObject::connect(a, &QAction::triggered, this, [this, id]() {
          auto *qw = qobject_cast<EditorWidget *>(this->centralWidget());
          if (qw != nullptr) {
            qw->changeBuffer(id);
          }
        });
      }
    }
  }  // end of MainWindow::updateBuffersMenu

  void MainWindow::useEmacsShortCuts() {
    auto &s = ShortCutStyle::getShortCutStyle();
    s.setStyle(ShortCutStyle::EMACS);
    this->updateOptionsMenu();
  }  // end of MainWindow::useEmacsShortCuts

  void MainWindow::useQtShortCuts() {
    auto &s = ShortCutStyle::getShortCutStyle();
    s.setStyle(ShortCutStyle::QT);
    this->updateOptionsMenu();
  }  // end of MainWindow::useQtShortCuts

  void MainWindow::createMainMenu() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    QSettings settings;
    clearMenuBar(this->menuBar());
    this->createActions();
#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
// https://bugreports.qt.io/browse/QTBUG-46812
// https://codereview.qt-project.org/#/c/170352/
//
#endif
    this->fm = this->menuBar()->addMenu(QObject::tr("File"));
    this->fm->addAction(this->na);
    this->fm->addAction(this->oa);
    if (settings.contains("recent files")) {
      const auto files = settings.value("recent files").toStringList();
      debug("recent files:", files);
      auto *rfm = this->fm->addMenu(QIcon::fromTheme("document-open-recent"),
                                    QObject::tr("&Recent Files"));
      //      rfm->setIconVisibleInMenu(true);
      this->createRecentFilesMenu(rfm, files);
    }
    this->fm->addAction(this->sa);
    this->fm->addAction(this->ka);
    this->fm->addSeparator();
    this->fm->addAction(this->pra);
    this->fm->addSeparator();
    this->fm->addAction(this->saa);
    this->fm->addSeparator();
    this->fm->addAction(this->ea);
    // edit menu
    this->em = this->menuBar()->addMenu(QObject::tr("Edit"));
    this->em->addAction(this->ua);
    this->em->addAction(this->ra);
    this->em->addSeparator();
    this->em->addAction(this->ca);
    this->em->addAction(this->ca2);
    this->em->addAction(this->pa);
    this->em->addAction(this->sea);
    // buffer menu
    this->bm = this->menuBar()->addMenu(QObject::tr("Buffers"));
    this->updateBuffersMenu();
    // editor menu
    this->cm = this->menuBar()->addMenu(QObject::tr("Editor"));
    auto *const qcmds = this->cm->addMenu(QObject::tr("Commands"));
    auto *const lqcmd = qcmds->addAction(QObject::tr("Launch"));
    QObject::connect(lqcmd, &QAction::triggered, e,
                     static_cast<void (EditorWidget::*)(void)>(
                         &EditorWidget::launchCommand));
    auto *const qmm = this->cm->addMenu(QObject::tr("Major Mode"));
    auto &mf = MajorModeFactory::getMajorModeFactory();
    for (const auto &mn : mf.getAvailableMajorModesNames()) {
      auto *const qmma = qmm->addAction(mn);
      qmma->setIcon(mf.getMajorModeIcon(mn));
      QObject::connect(qmma, &QAction::triggered, e, [e, mn] {
        e->getCurrentBuffer().getMainFrame().setMajorMode(mn);
      });
    }
    // menu associated with the current major mode
    for (const auto &m : e->getCurrentBufferSpecificMenus()) {
      this->menuBar()->addMenu(m);
    }
    // options
    this->om = this->menuBar()->addMenu(QObject::tr("Options"));
    this->updateOptionsMenu();
    // help menu
    this->hm = this->menuBar()->addMenu(QObject::tr("Help"));
    this->hm->addAction(this->aa);
    this->hm->addAction(this->aa2);
  }  // end of MainWindow::createMainMenu

  void MainWindow::createRecentFilesMenu(QMenu *const m,
                                         const QStringList &files) {
    auto &mf = MajorModeFactory::getMajorModeFactory();
    auto n = int{};
    const auto s = files.size();
    if (s == 0) {
      return;
    }
    for (int i = s - 1; (i >= 0) && (n != 5); --i, ++n) {
      const auto &f = files.at(i);
      QFileInfo fi(f);
      if ((fi.exists()) && (fi.isFile()) && (fi.isReadable())) {
        auto *rf = m->addAction(fi.fileName());
        const auto mn = mf.getMajorModeNameForFile(fi.fileName());
        rf->setIcon(mf.getMajorModeIcon(mn));
        const auto path = fi.absoluteFilePath();
        QObject::connect(rf, &QAction::triggered, this, [this, path] {
          auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
          if (e != nullptr) {
            e->openFile(path);
          }
        });
      }
    }
  }  // end of MainWindow::createRecentFilesMenu

  void MainWindow::about() {
    QMessageBox::about(this, tr("About "),
                       tr("The <b></b> is a simple "
                          "qt based text editor."));
  }  // end of MainWindow::about()

  void MainWindow::cut() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      auto &t = e->getCurrentBuffer().getMainFrame();
      t.cut();
    }
  }  // end of MainWindow::cut

  void MainWindow::undo() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      auto &t = e->getCurrentBuffer().getMainFrame();
      t.undo();
    }
  }  // end of MainWindow::undo

  void MainWindow::redo() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      auto &t = e->getCurrentBuffer().getMainFrame();
      t.redo();
    }
  }  // end of MainWindow::redo

  void MainWindow::selectAll() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      auto &t = e->getCurrentBuffer().getMainFrame();
      t.selectAll();
    }
  }  // end of MainWindow::selectAll

  void MainWindow::copy() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      auto &t = e->getCurrentBuffer().getMainFrame();
      t.copy();
    }
  }  // end of MainWindow::copy

  void MainWindow::paste() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      auto &t = e->getCurrentBuffer().getMainFrame();
      t.paste();
    }
  }  // end of MainWindow::paste

  void MainWindow::print() {
    auto *e = qobject_cast<EditorWidget *>(this->centralWidget());
    if (e != nullptr) {
      e->print();
    }
  }  // end of MainWindow::print

  QSize MainWindow::sizeHint() const {
    return QSize(800, 600);
  }  // end of MainWindow::sizeHint()

}  // end of namespace tfel::gui
