/*!
 * \file   src/FileExplorer.cxx
 * \brief  panel showing the file system, which opens in a buffer the file it is
 *         double clicked on.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtGui/QIcon>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FileExplorer.hxx"

namespace tfel::gui {

  FileExplorer::FileExplorer(EditorWidget &e, QWidget *const p)
      : QWidget(p),
        editor(e),
        model(new QFileSystemModel(this)),
        tree(new QTreeView(this)),
        path(new QLabel(this)),
        up(new QToolButton(this)),
        choose(new QToolButton(this)) {
    // the model watches the directories it has looked at, so an entry created,
    // renamed or removed on disk reaches the tree by itself: nothing here has
    // to poll the file system or hold a watcher of its own. Its filter drops
    // the two entries every directory names for itself and its parent: the
    // panel has a button of its own for the parent, and the file system is not
    // a place one reaches by clicking on `..`
    this->model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    this->tree->setModel(this->model);
    // The model names five columns; only the name of an entry is of any use in
    // a panel whose whole width is a third of a window, and there is no header
    // to name a column the entries are not sorted by
    for (int c = 1; c != this->model->columnCount(); ++c) {
      this->tree->setColumnHidden(c, true);
    }
    this->tree->setHeaderHidden(true);
    this->tree->setUniformRowHeights(true);
    this->tree->setSelectionMode(QAbstractItemView::SingleSelection);
    // the double click is what the request names: the default action of a tree
    // view on a directory, expanding it, is left to the view and is not
    // disturbed by opening it, which is what the test of a directory in
    // openFileAt is for
    QObject::connect(this->tree, &QTreeView::doubleClicked, this,
                     &FileExplorer::openFileAt);
    // the header of the panel: the two ways out of the directory which is shown
    // and the name of that directory, since the tree is rooted at it and shows
    // nothing of its own above its first entry
    this->up->setIcon(QIcon::fromTheme("go-up"));
    this->up->setToolTip(
        QObject::tr("Show the directory which holds this one"));
    this->up->setAutoRaise(true);
    QObject::connect(this->up, &QToolButton::clicked, this,
                     &FileExplorer::goUp);
    this->choose->setIcon(QIcon::fromTheme("document-open"));
    this->choose->setToolTip(
        QObject::tr("Show the contents of another directory"));
    this->choose->setAutoRaise(true);
    QObject::connect(this->choose, &QToolButton::clicked, this,
                     &FileExplorer::chooseDirectory);
    // the name of the directory has to be cut rather than to push the panel
    // wider: a widget whose minimumSizeHint is as wide as its text is a panel
    // which can not be shrunk, and shrinking it is what the user is owed
    this->path->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    this->path->setTextFormat(Qt::PlainText);
    auto *const header = new QWidget(this);
    auto *const hl = new QHBoxLayout(header);
    hl->setContentsMargins(2, 0, 2, 0);
    hl->setSpacing(2);
    hl->addWidget(this->up);
    hl->addWidget(this->choose);
    hl->addWidget(this->path, 1);
    auto *const vl = new QVBoxLayout(this);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    vl->addWidget(header);
    vl->addWidget(this->tree, 1);
    this->setRootDirectory(QDir::current().absolutePath());
    // the tree wears the font of the text edits, as the widgets which read
    // that setting themselves do, and follows it afterwards through
    // changeFont, which is what MainWindow::selectFont hands the new font to
    QSettings settings;
    if (settings.contains("font/textedit")) {
      auto f = settings.value("font/textedit").value<QFont>();
      f.setStyleStrategy(QFont::PreferAntialias);
      this->changeFont(f);
    }
  }  // end of FileExplorer::FileExplorer

  FileExplorer::~FileExplorer() = default;

  QString FileExplorer::rootDirectory() const {
    return this->model->rootPath();
  }  // end of FileExplorer::rootDirectory

  void FileExplorer::setRootDirectory(const QString &d) {
    if (d.isEmpty()) {
      return;
    }
    const QFileInfo info(d);
    if ((not info.exists()) or (not info.isDir())) {
      return;
    }
    // the path of the model is what makes it look at a directory: an index
    // asked for a directory it was never rooted at is valid, has no row, and
    // says nothing of the fact that nothing was read. The load itself is not
    // waited for: the entries reach the tree as the model reads them, which is
    // what the model tells the view through layoutChanged
    this->model->setRootPath(d);
    this->tree->setRootIndex(this->model->index(d));
    this->updateHeader();
  }  // end of FileExplorer::setRootDirectory

  void FileExplorer::changeFont(const QFont &f) {
    this->tree->setFont(f);
    this->path->setFont(f);
  }  // end of FileExplorer::changeFont

  void FileExplorer::openFileAt(const QModelIndex &i) {
    if (not i.isValid()) {
      return;
    }
    const auto f = this->model->filePath(i);
    if (f.isEmpty()) {
      return;
    }
    // a directory is what a double click expands, never what it opens: the view
    // has already handled the expansion when this slot is reached
    if (QFileInfo(f).isDir()) {
      return;
    }
    this->editor.openFile(f);
  }  // end of FileExplorer::openFileAt

  void FileExplorer::goUp() {
    const auto current = this->model->rootPath();
    if (current.isEmpty()) {
      return;
    }
    const auto parent = QFileInfo(current).path();
    // the directory which holds / is /, and a panel rooted at the root of the
    // file system has nowhere above it to go
    if (parent == current) {
      return;
    }
    this->setRootDirectory(parent);
  }  // end of FileExplorer::goUp

  void FileExplorer::chooseDirectory() {
    const auto d = QFileDialog::getExistingDirectory(
        this, QObject::tr("Choose a Directory"), this->model->rootPath());
    if (d.isEmpty()) {
      return;
    }
    this->setRootDirectory(d);
  }  // end of FileExplorer::chooseDirectory

  void FileExplorer::updateHeader() {
    const auto d = this->model->rootPath();
    this->path->setText(d);
    // the line of the header is cut where the panel ends, so the whole name is
    // only to be found in its tip
    this->path->setToolTip(d);
    this->up->setEnabled(QFileInfo(d).path() != d);
  }  // end of FileExplorer::updateHeader

}  // end of namespace tfel::gui
