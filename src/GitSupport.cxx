/*!
 * \file  GitSupport.cxx
 * \brief which git repository a file belongs to.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "TFEL/GUI/GitSupport.hxx"

namespace tfel::gui {

  QString GitSupport::repositoryRootOf(const QString &path) {
    if (path.isEmpty()) {
      return {};
    }
    const auto info = QFileInfo(path);
    if (info.fileName() == QLatin1String(".git")) {
      // the entry itself is the bookkeeping of a repository: neither the
      // directory of a checkout nor the file of a linked working tree is a
      // working file of it
      return {};
    }
    // The search starts at the directory the file stands in. A path naming a
    // directory is asked about itself: the directory of a repository is a
    // working directory of it, and a buffer visiting a directory shows the
    // indicator as a buffer visiting one of its files does.
    auto current =
        info.isDir() ? QDir::cleanPath(info.absoluteFilePath())
                     : QDir::cleanPath(info.absolutePath());
    for (;;) {
      if (current.isEmpty()) {
        return {};
      }
      if (QDir(current).dirName() == QLatin1String(".git")) {
        // the internals of a repository are no working files of it: a buffer
        // visiting `COMMIT_EDITMSG` or `HEAD` names no working tree
        return {};
      }
      // `.git` is a directory of a regular checkout and a file of a linked
      // working tree or of a submodule: `exists` asks both questions at once
      if (QFileInfo::exists(QDir(current).absoluteFilePath(".git"))) {
        return QDir(current).absolutePath();
      }
      // `QFileInfo::path` is a lexical operation, which is what lets the ascent
      // go over a directory holding nothing yet: the directory of a file which
      // was not written is as much a place of a repository as one which is.
      const auto parent = QFileInfo(current).path();
      if (parent == current) {
        // the root of the file system was reached and holds no repository
        return {};
      }
      current = parent;
    }
  }  // end of GitSupport::repositoryRootOf

}  // end of namespace tfel::gui
