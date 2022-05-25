/*!
 * \file   DirectoryMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtGui/QSyntaxHighlighter>
#include "TFEL/GUI/EditorWidget.hxx"
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
      this->directoryFormat.setForeground(Qt::blue);
      this->executableFileFormat.setForeground(Qt::darkGreen);
    }  // end of DirectorySyntaxHighlighter
    /*!
     * \brief highlight the current line
     * \param[in] l: line
     */
    void highlightBlock(const QString& l) override {
      const auto ls = l.split(QRegExp("\\s+"), QString::SkipEmptyParts);
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
    QObject::connect(&(this->watcher), &QFileSystemWatcher::directoryChanged,
                     this, [this] { this->updateDirectoryDescription(); });
    this->updateDirectoryDescription(d);
  }  // end of DirectoryMajorMode

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
    new DirectorySyntaxHighlighter(this->directory, d);
  }  // end of setSyntaxHighlighter

  void DirectoryMajorMode::format() {}  // end of format

  bool DirectoryMajorMode::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if (((m == Qt::NoModifier) && (k == Qt::Key_Return)) ||
        ((m == Qt::NoModifier) && (k == Qt::Key_Enter))) {
      auto tc = this->textEdit.textCursor();
      tc.select(QTextCursor::LineUnderCursor);
      const auto l =
          tc.selectedText().split(QRegExp("\\s+"), QString::SkipEmptyParts);
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
      mf = std::max(mf, ei.fileName().size());
      mo = std::max(mo, ei.owner().size());
    }
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
    this->textEdit.setReadOnly(true);
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
    this->watcher.addPath(this->directory);
    this->updateDirectoryDescription();
    this->setSyntaxHighlighter(this->textEdit.document());
  }  // end of updateDirectoryDescription()

  DirectoryMajorMode::~DirectoryMajorMode() = default;

}  // end of namespace tfel::gui
