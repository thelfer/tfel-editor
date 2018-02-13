/*!
 * \file   DirectoryMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   10/02/2018
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QSyntaxHighlighter>
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/DirectoryMajorMode.hxx"

namespace qemacs {

  /*!
   * \brief structure in charge document handled by the
   * DirectoryMajorMode major mode
   */
  struct DirectorySyntaxHighlighter: public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param[in] d: directory path
     * \param[in] p: document treated
     */
    DirectorySyntaxHighlighter(const QString& d, QTextDocument* p)
        : QSyntaxHighlighter(p), directory(d) {
      this->directoryFormat.setForeground(Qt::blue);
    }  // end of DirectorySyntaxHighlighter
    /*!
     * \brief highlight the current line
     * \param[in] l: line
     */
    void highlightBlock(const QString& l) override {
      QFileInfo fi(this->directory+QDir::separator()+l);
      if(fi.isDir()){
        this->setFormat(0,l.size(),this->directoryFormat);
      } else {
        this->setFormat(0, l.size(), this->fileFormat);
      }
    }  // end of highlightBlock
    //! destructor
    ~DirectorySyntaxHighlighter() override = default;

   private:
    //! path the directory being treated
    QString directory;
    QTextCharFormat directoryFormat;
    QTextCharFormat fileFormat;
  };  // end of DirectorySyntaxHighlighter

  DirectoryMajorMode::DirectoryMajorMode(const QString& d,
                                         QEmacsWidget& w,
                                         QEmacsBuffer& b,
                                         QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w, b, t, &t) {
    QObject::connect(&(this->watcher),
                     &QFileSystemWatcher::directoryChanged, this,
                     [this] { this->updateDirectoryDescription(); });
    this->updateDirectoryDescription(d);
  }  // end of DirectoryMajorMode::DirectoryMajorMode

  QString DirectoryMajorMode::getName() const {
    return "directory-mode";
  }  // end of DirectoryMajorMode::getName

  QString DirectoryMajorMode::getDescription() const {
    return "major mode used to interact with a directory content";
  }  // end of DirectoryMajorMode::getDescription

  QIcon DirectoryMajorMode::getIcon() const {
    return QIcon::fromTheme("folder");
  } // end of DirectoryMajorMode::getIcon

  void DirectoryMajorMode::setSyntaxHighlighter(QTextDocument* d) {
    new DirectorySyntaxHighlighter(this->directory,d);
  }  // end of DirectoryMajorMode::setSyntaxHighlighter

  void DirectoryMajorMode::format() {
  }  // end of DirectoryMajorMode::format

  bool DirectoryMajorMode::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if (((m == Qt::AltModifier) && (k == Qt::Key_M)) ||
        ((m == Qt::NoModifier) && (k == Qt::Key_Enter))) {
      auto tc = this->textEdit.textCursor();
      tc.select(QTextCursor::LineUnderCursor);
      const auto path =
          this->directory + QDir::separator() + tc.selectedText();
      QFileInfo fi(path);
      if (fi.isFile()) {
        this->qemacs.openFile(path);
      } else if (fi.isDir()) {
        this->updateDirectoryDescription(path);
      }
      return true;
    }
    return QEmacsMajorModeBase::keyPressEvent(e);
  }  // end of DirectoryMajorMode::keyPressEvent

  void DirectoryMajorMode::updateDirectoryDescription() {
    QFileInfo fn(this->directory);
    if (!fn.isDir()) {
      return;
    }
    this->textEdit.setReadOnly(false);
    this->textEdit.setFileName(this->directory);
    auto tc = this->textEdit.textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    tc.removeSelectedText();
    const auto eis = QDir(this->directory).entryInfoList();
    for (const auto& ei : eis) {
      const auto efn = ei.fileName();
      if (efn != ".") {
        tc.insertText(efn + '\n');
      }
    }
    this->textEdit.document()->setModified(false);
    this->textEdit.setReadOnly(true);
  }  // end of DirectoryMajorMode::updateDirectoryDescription

  void DirectoryMajorMode::updateDirectoryDescription(
      const QString& d) {
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
  }  // end of DirectoryMajorMode::updateDirectoryDescription()

  DirectoryMajorMode::~DirectoryMajorMode() = default;

}  // end of namespace qemacs
