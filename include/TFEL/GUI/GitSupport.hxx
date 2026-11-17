/*!
 * \file  GitSupport.hxx
 * \brief the little the editor knows about git: which repository a file
 *        belongs to.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#ifndef LIB_TFEL_GUI_GITSUPPORT_HXX
#define LIB_TFEL_GUI_GITSUPPORT_HXX

#include <QtCore/QString>

#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief the questions about git which are worth asking without git
   *
   * The first step of the git support is to say whether the file a buffer
   * visits stands in the working tree of a repository, so that the information
   * line of the buffer can name it beside the name of the buffer. That
   * question is answered by looking for the `.git` entry of a repository up the
   * hierarchy of the directories of the file, which is the very rule `git`
   * itself uses to find the repository a working file belongs to, and which
   * needs no program and no subprocess.
   *
   * \note Nothing here asks git what it thinks of the file: a file which is in
   * a working tree and not tracked yet, and a file which is unchanged since the
   * last commit, are answered the same way. What the state of a file is —
   * modified, staged, ahead of the branch, and what its differences with the
   * committed text are — needs the program, and is the next step of this
   * support.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT GitSupport {
    /*!
     * \brief find the root of the repository the given path belongs to
     * \param[in] path : name of a file or of a directory, absolute or not
     * \return the root of the repository the path belongs to, an empty string
     * when it belongs to no git working tree
     *
     * \note The search starts at the directory the path stands in — at the path
     * itself when it names a directory — and ascends to the root of the file
     * system. A directory holding a `.git` entry is the root of a repository:
     * that entry is a directory of a regular checkout and a file of a linked
     * working tree or of a submodule, which is why its existence is what is
     * asked, and not whether it is a directory.
     *
     * \note The ascent is lexical: it asks the directories which the name names
     * and never resolves a symbolic link, which is what keeps the search cheap.
     * It is what `git` does too, and a repository reached through a link is
     * found where the link stands, not where it points.
     *
     * \note The file named needs no existence: a buffer visiting a file which
     * was not written yet asks the question of the directory it is about to be
     * written in, which is what makes the indicator name a new file of a
     * project as soon as it is opened.
     *
     * \note An empty path answers nothing, and so does a path standing inside
     * the `.git` entry of a repository: the files of the internals of a
     * repository are no working files of it.
     */
    static QString repositoryRootOf(const QString &path);
  };  // end of struct GitSupport

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GITSUPPORT_HXX */
