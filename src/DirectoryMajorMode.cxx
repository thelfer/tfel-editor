/*!
 * \file   DirectoryMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QTextEdit>
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/GitStatusClient.hxx"
#include "TFEL/GUI/GitSupport.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/DirectoryMajorMode.hxx"

namespace tfel::gui {

  /*!
   * \brief structure in charge document handled by the
   * DirectoryMajorMode major mode
   */
  struct DirectorySyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param[in] d: directory path
     * \param[in] p: document treated
     */
    DirectorySyntaxHighlighter(const QString& d, QTextDocument* p)
        : QSyntaxHighlighter(p), directory(d) {
      // The pure blue of a name is given to git: the two inks of the states of a
      // file, which `updateGitColors` draws over these faces, are the blue of an
      // entry no commit holds and the dark red of one a working tree holds
      // differently from the commit its branch names. A directory which wore that
      // same blue would make the ink of a name say two different things — and the
      // directory face is what a reader of this listing reads first. `darkBlue` is
      // the same hue one step down, which is the face the file explorer panel does
      // not need and the listing can tell from a new file.
      this->directoryFormat.setForeground(Qt::darkBlue);
      this->executableFileFormat.setForeground(Qt::darkGreen);
    }  // end of DirectorySyntaxHighlighter
    /*!
     * \brief highlight the current line
     * \param[in] l: line
     */
    void highlightBlock(const QString& l) override {
      const auto ls = l.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      if (ls.isEmpty()) {
        return;
      }
      QFileInfo fi(this->directory + QDir::separator() + ls[0]);
      if (fi.isDir()) {
        this->setFormat(0, ls[0].size(), this->directoryFormat);
      } else if (fi.isFile()) {
        if (fi.isExecutable()) {
          this->setFormat(0, ls[0].size(), this->executableFileFormat);
        } else {
          this->setFormat(0, ls[0].size(), this->standardFileFormat);
        }
      }
    }  // end of highlightBlock
    //! destructor
    ~DirectorySyntaxHighlighter() override = default;

   private:
    //! path the directory being treated
    QString directory;
    QTextCharFormat directoryFormat;
    QTextCharFormat executableFileFormat;
    QTextCharFormat standardFileFormat;
  };  // end of DirectorySyntaxHighlighter

  DirectoryMajorMode::DirectoryMajorMode(const QString& d,
                                         EditorWidget& w,
                                         Buffer& b,
                                         TextEditBase& t)
      : MajorModeBase(w, b, t, &t) {
    // the guard of the destructor: see the `area` member
    this->area = t.widget();
    QObject::connect(&(this->watcher), &QFileSystemWatcher::directoryChanged,
                     this, [this] { this->updateDirectoryDescription(); });
    // The state of a file is the answer of a process, and that answer reaches the
    // window after the listing was written: the signal which repaints the entries
    // of the file explorer panel is what redraws the names of a listing here. The
    // answer of the repository shown is asked for by
    // `updateDirectoryDescription` below, and the repositories of the buffers are
    // what the client watches, which a buffer visiting a directory is — the name
    // it visits is a directory, and the root of a repository is asked of a
    // directory as of a file.
    QObject::connect(&GitStatusClient::get(w),
                     &GitStatusClient::repositoryStatusesChanged, this,
                     [this](const QString& root) {
                       if (root == this->repository_root) {
                         this->updateGitColors();
                       }
                     });
    this->updateDirectoryDescription(d);
  }  // end of DirectoryMajorMode

  DirectoryMajorMode::~DirectoryMajorMode() {
    // The band of the current line is redrawn by the text edit itself, the lints
    // of a proofreader and the faces of clangd clear their own contribution, and
    // the names of a listing have to do the same: `directory-git` is not a face of
    // the highlighter, which the document drops with the mode, and a contribution
    // left behind keeps colouring the text of whatever mode comes next — a buffer
    // given another mode by hand shows names still wearing the state their
    // *previous* text had.
    // A text edit which is destroyed along with its mode needs none of this, and
    // must not be asked: the mode is a child of it and is deleted by
    // `QObjectPrivate::deleteChildren` of its `~QWidget`, which runs *after* the
    // members of `TextEditBase` — among them the map of the keyed contributions —
    // are gone. `area` is null by then, because the Qt widget of the listing is a
    // child made before the mode and so is taken away first: the same guard
    // `GrammarCheckSession` needs, and for the same reason
    if (this->area != nullptr) {
      this->textEdit.setExtraSelections(QLatin1String("directory-git"), {});
    }
  }  // end of ~DirectoryMajorMode

  QString DirectoryMajorMode::getName() const {
    return "directory-mode";
  }  // end of getName

  QString DirectoryMajorMode::getDescription() const {
    return "major mode used to interact with a directory content";
  }  // end of getDescription

  QIcon DirectoryMajorMode::getIcon() const {
    return QIcon::fromTheme("folder");
  }  // end of getIcon

  void DirectoryMajorMode::setSyntaxHighlighter(QTextDocument* d) {
    this->setHighlighter(new DirectorySyntaxHighlighter(this->directory, d));
  }  // end of setSyntaxHighlighter

  void DirectoryMajorMode::format() {}  // end of format

  bool DirectoryMajorMode::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if (((m == Qt::NoModifier) && (k == Qt::Key_Return)) ||
        ((m == Qt::NoModifier) && (k == Qt::Key_Enter))) {
      // the name asked of the file system is the first token of the *line*,
      // never of a selection: `select(BlockUnderCursor)` names, on any block
      // but the first of the document, the `U+2029` PARAGRAPH SEPARATOR which
      // precedes that block too, and the `\s+` of `QRegularExpression` is the
      // ASCII whitespace class of PCRE — unlike the JavaScript `\s` of the
      // `QRegExp` this split with until the Qt 6 port, which knew `U+2029` as
      // whitespace. The token of every line but the first was therefore
      // `<U+2029>name`, a name no directory holds, and since a listing begins
      // with the `..` entry, no file and no sub-directory could be reached
      // with this key any more. `QTextBlock::text` is the text of the line and
      // nothing else: no separator of a neighbour, whatever the cursor holds.
      const auto l = this->textEdit.textCursor().block().text().split(
          QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      if (l.isEmpty()) {
        return true;
      }
      const auto path = this->directory + QDir::separator() + l[0];
      QFileInfo fi(path);
      if (fi.isFile()) {
        this->editor.openFile(path);
      } else if (fi.isDir()) {
        this->updateDirectoryDescription(path);
      }
      return true;
    }
    return MajorModeBase::keyPressEvent(e);
  }  // end of keyPressEvent

  void DirectoryMajorMode::updateDirectoryDescription() {
    QFileInfo fn(this->directory);
    if (!fn.isDir()) {
      return;
    }
    auto mf = int{};
    auto mo = int{};
    const auto eis = QDir(this->directory).entryInfoList();
    for (const auto& ei : eis) {
      mf = std::max(static_cast<qsizetype>(mf), ei.fileName().size());
      mo = std::max(static_cast<qsizetype>(mo), ei.owner().size());
    }
    // The line the cursor stands on is taken before the rewrite and put back
    // after it. Each of the insertions below happens at the place the cursor of
    // the text edit holds, and so pushes that cursor along: a listing rewritten
    // without this leaves the cursor on the empty line the last newline opens,
    // which is a buffer whose next `Enter` names nothing — and a hand which was
    // reading the fifth line of a long directory is sent to its end by the file
    // system watcher noticing something. `QTextCursor::blockNumber` is not used:
    // it asks the layout, which is only brought up to date when the text is
    // painted, and a listing rewritten programmatically was never painted since.
    const auto document = this->textEdit.document();
    const auto placed = this->textEdit.textCursor().position();
    const auto line = document->findBlock(placed).blockNumber();
    this->textEdit.setReadOnly(false);
    this->textEdit.setFileName(this->directory);
    auto tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    for (const auto& ei : eis) {
      const auto efn = ei.fileName();
      if (efn != ".") {
        const auto eo = ei.owner();
        const auto d = ei.lastModified();
        tc.insertText(efn.leftJustified(mf + 1, ' ') +
                      eo.leftJustified(mo + 1, ' ') + d.toString() + '\n');
      }
    }
    this->textEdit.document()->setModified(false);
    // the listing has fewer lines than the one it replaces as soon as the
    // directory lost an entry: the last line of the new listing is the furthest
    // the place asked for can stand
    const auto kept = document->findBlockByNumber(
        std::min(line, document->blockCount() - 1));
    tc.setPosition(kept.position());
    this->textEdit.setTextCursor(tc);
    this->textEdit.setReadOnly(true);
    // the rewrite moved the lines of the listing, so the places the states of the
    // entries are drawn on moved with them: what is known about the entries is
    // drawn again, and the answers are already had — drawing asks no program, it
    // reads what the client of the window holds
    this->updateGitColors();
  }  // end of updateDirectoryDescription

  void DirectoryMajorMode::updateDirectoryDescription(const QString& d) {
    QFileInfo fn(d);
    if (!fn.isDir()) {
      return;
    }
    if (!this->directory.isEmpty()) {
      this->watcher.removePath(this->directory);
    }
    this->directory = d;
    // The entries of one listing are of one repository, so the ascent which finds
    // the `.git` entry is made here, once, and not once per line; the answer is
    // also what tells the signal of the client which of its trees this buffer is
    // about. `repositoryRootOf` runs no program: it only ascends the names.
    this->repository_root = GitSupport::repositoryRootOf(d);
    this->watcher.addPath(this->directory);
    // a directory which is entered is looked at from its first line, and that is
    // the line the rewrite above keeps: the cursor is moved before it, not after,
    // because the rewrite is what decides where the cursor stands
    auto tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    this->textEdit.setTextCursor(tc);
    this->updateDirectoryDescription();
    this->setSyntaxHighlighter(this->textEdit.document());
    if (!this->repository_root.isEmpty()) {
      // a listing which just appeared on the screen is a moment a person is
      // looking at: the first answer is asked for now, and not at the next tick of
      // the poll, up to five seconds off. Asking is not waiting: the colors of the
      // entries which have one come with the signal, and `updateGitColors` below
      // draws the answers this client holds already, which is what makes entering
      // a directory of an answered repository color it at once.
      GitStatusClient::get(this->editor).refresh(this->repository_root);
    }
  }  // end of updateDirectoryDescription()

  GitStatusClient::State DirectoryMajorMode::stateOfEntry(
      const QString& name) const {
    if (this->repository_root.isEmpty()) {
      return GitStatusClient::Unknown;
    }
    if (name == QLatin1String("..")) {
      // the parent entry names a directory the listing does not show, and often
      // enough a directory out of the repository of the one it shows; and the
      // parent of the directory looked at is never what a reader wants the state
      // of, which is a question about the entries this line is a listing of
      return GitStatusClient::Unknown;
    }
    return GitStatusClient::get(this->editor).statusIn(
        this->repository_root, this->directory + QDir::separator() + name);
  }  // end of stateOfEntry

  void DirectoryMajorMode::updateGitColors() {
    auto selections = QList<QTextEdit::ExtraSelection>();
    if (!this->repository_root.isEmpty()) {
      auto* const document = this->textEdit.document();
      for (auto block = document->begin(); block.isValid();
           block = block.next()) {
        // the name of the entry is the first word of the line: the listing lays
        // its columns with spaces, and the name is the first of them. `block` is
        // the text of the line and nothing else, so no separator of a neighbour
        // reaches the split, the trap of `Enter` on this same line
        const auto words =
            block.text().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (words.isEmpty()) {
          continue;
        }
        const auto color =
            GitStatusClient::colorOf(this->stateOfEntry(words[0]));
        if (!color.isValid()) {
          // nothing to say about the entry: the face of its kind, which the
          // highlighter of the mode inked, is kept
          continue;
        }
        auto format = QTextCharFormat();
        format.setForeground(color);
        auto selection = QTextEdit::ExtraSelection();
        auto cursor = QTextCursor(document);
        cursor.setPosition(block.position());
        cursor.setPosition(block.position() + words[0].size(),
                           QTextCursor::KeepAnchor);
        selection.cursor = cursor;
        selection.format = format;
        selections << selection;
      }
    }
    // the keyed contribution, never the whole list: the band of the current line
    // is another source, and a directory buffer whose names are all it has to say
    // removes its own contribution and leaves that band where it is
    this->textEdit.setExtraSelections("directory-git", selections);
  }  // end of updateGitColors

}  // end of namespace tfel::gui
