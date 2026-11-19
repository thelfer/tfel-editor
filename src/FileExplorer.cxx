/*!
 * \file   src/FileExplorer.cxx
 * \brief  panel showing the file system, which opens in a buffer the file it is
 *         double clicked on.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QPointer>
#include <QtCore/QSettings>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FileExplorer.hxx"
#include "TFEL/GUI/GitStatusClient.hxx"
#include "TFEL/GUI/GitSupport.hxx"

namespace tfel::gui {

  namespace {

    /*!
     * \brief the model of the file system of the panel, coloured by what git
     *        thinks of the entries it shows
     *
     * A model of the file system which is asked something the file system does
     * not hold — is this file one the repository has never seen? — answers that
     * question at the one place the answer is drawn: the state of a file is the
     * git client's, and the tree only asks what a colour is.
     *
     * \note The colours themselves are answered by `GitStatusClient::colorOf`,
     * the one place which says what a state looks like: the entries of this panel
     * and the names of a directory listing are the same question about the same
     * file and can not be allowed to wear two different inks.
     */
    struct GitColoredFileSystemModel : QFileSystemModel {
      /*!
       * \brief constructor
       * \param[in] e : editor whose git client answers what an entry is
       * \param[in] p : parent object, the panel the model belongs to
       */
      GitColoredFileSystemModel(EditorWidget &e, QObject *const p)
          : QFileSystemModel(p),
            client(&GitStatusClient::get(e)) {}

      //! \brief the repository the entries of the panel are asked of
      void setRepositoryRoot(const QString &r) { this->repository_root = r; }

      /*!
       * \return what an index says for the given role
       * \param[in] index : index of an entry of the tree
       * \param[in] role : role the view asks for
       *
       * Only the colour of the text is ever answered differently, and only for
       * the two states the question distinguishes: an entry which is of no
       * repository, which the answer of a repository holds nothing about, and an
       * entry whose state was never asked, wear the colour the desktop gives
       * them, which is the honest drawing of "nothing is known".
       */
      QVariant data(const QModelIndex &index, const int role) const override {
        if ((role != Qt::ForegroundRole) || (!index.isValid()) ||
            (this->repository_root.isEmpty())) {
          return QFileSystemModel::data(index, role);
        }
        auto *const c = this->client.data();
        if (c == nullptr) {
          return QFileSystemModel::data(index, role);
        }
        const auto file = this->filePath(index);
        if (file.isEmpty()) {
          return QFileSystemModel::data(index, role);
        }
        const auto color = GitStatusClient::colorOf(
            c->statusIn(this->repository_root, file));
        if (color.isValid()) {
          return QBrush(color);
        }
        return QFileSystemModel::data(index, role);
      }  // end of data

     private:
      //! the client the state of an entry is asked of
      QPointer<GitStatusClient> client;
      //! the repository the entries are asked of, empty when they are of none
      QString repository_root;
    };  // end of struct GitColoredFileSystemModel

  }  // end of anonymous namespace

  FileExplorer::FileExplorer(EditorWidget &e, QWidget *const p)
      : QWidget(p),
        editor(e),
        model(new GitColoredFileSystemModel(e, this)),
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
    // a panel whose whole width is a fraction of a window, and there is no
    // header to name a column the entries are not sorted by
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
    // The colours of the entries are not data the file system holds: they follow
    // the answer of the client, so the answer is what says when the tree has to
    // be drawn again. The tree caches nothing of the colour of a row, so asking
    // its viewport to repaint is the whole of the redraw; the state of the root
    // the entries are asked of is taken at the same moment, because the answer
    // naming a root is the answer the rows of this panel are about.
    QObject::connect(&GitStatusClient::get(e),
                     &GitStatusClient::repositoryStatusesChanged, this,
                     &FileExplorer::refreshStatusColors);
    // the tree wears the font of the text edits, as the widgets which read
    // that setting themselves do, and follows it afterwards through
    // changeFont, which is what EditorWidget::changeFont hands the new font to
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
    // the entries of the new directory are of another repository, or of none, so
    // the root the colours are asked of changes with the directory shown
    this->refreshStatusColors();
  }  // end of FileExplorer::setRootDirectory

  void FileExplorer::refreshStatusColors() {
    auto *const m = dynamic_cast<GitColoredFileSystemModel *>(this->model);
    if (m == nullptr) {
      return;
    }
    m->setRepositoryRoot(GitSupport::repositoryRootOf(this->model->rootPath()));
    // The tree holds no colour of its own: a row is drawn from what the model
    // answers at the moment it is drawn, so asking the viewport to be redrawn is
    // the whole of the redraw — and it is what the model itself can not do, since
    // it knows nothing of the view which shows it.
    this->tree->viewport()->update();
  }  // end of FileExplorer::refreshStatusColors

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
