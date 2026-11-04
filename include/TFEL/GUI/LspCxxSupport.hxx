/*!
 * \file  LspCxxSupport.hxx
 * \brief the C and C++ side of the Language Server Protocol: finding
 *        `clangd`, and deciding what a C++ file is.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#ifndef LIB_TFEL_GUI_LSPCXXSUPPORT_HXX
#define LIB_TFEL_GUI_LSPCXXSUPPORT_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief where `clangd` is and whether the C++ mode should ask it anything
   *
   * \note the settings live in `QSettings`, as the rest of the editor:
   * `clangd/enable` (true by default), `clangd/executable` (an absolute path,
   * tried first), `clangd/arguments` (a string list appended to what the
   * client needs)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LspCxxSupport {
    //! \return the setting `clangd/enable`, true when unset
    static bool isEnabled();
    /*!
     * \return the executable of `clangd`: the setting `clangd/executable`,
     * else `clangd` in the path, else the highest `clangd-NN` of the usual
     * binary directories, else the highest `/usr/lib/llvm-NN/bin/clangd`; an
     * empty string when there is none
     *
     * \note the versioned names matter: distributions package `clangd` as
     * `clangd-15` and the unversioned name is not always there
     */
    static QString serverExecutable();
    //! \return the setting `clangd/arguments`, empty when unset
    static QStringList serverArguments();
    /*!
     * \brief the arguments which hand a compilation database to \c clangd
     * \param[in] path : the database file or the directory holding it; may be
     * empty, which means that the server has to look for a database itself
     * \param[out] error : why the answer can not be used, empty otherwise
     * \return the arguments to give the server, empty when the search is left
     * to it
     *
     * \note \c clangd is told which \e directory to look in, never which file,
     * so the file of the answer loses its name and the directory is taken as
     * it stands; the answer is made absolute, and a leading `~` expanded,
     * because the server starts in a directory of the editor's, not of the
     * buffer's, and a relative or abbreviated answer would mean nothing to it
     */
    static QStringList compileCommandsArguments(const QString &path,
                                                QString &error);
    /*!
     * \return true if the file name is one of the C and C++ extensions the
     * `C++` major mode is registered for
     */
    static bool isCxxSourceFile(const QString &file_name);
  };  // end of struct LspCxxSupport

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LSPCXXSUPPORT_HXX */
