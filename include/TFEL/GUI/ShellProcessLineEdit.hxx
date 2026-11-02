/*!
 * \file   ShellProcessLineEdit.hxx
 * \brief input running the whole text typed with a shell.
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#ifndef LIB_TFEL_GUI_SHELLPROCESSLINEEDIT_HXX
#define LIB_TFEL_GUI_SHELLPROCESSLINEEDIT_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/ProcessLineEdit.hxx"

namespace tfel::gui {

  /*!
   * An helper class asking the user for a shell command and
   * displaying the results in a ProcessOutputFrame created as a slave
   * of the current buffer
   */
  struct TFEL_GUI_VISIBILITY_EXPORT ShellProcessLineEdit
      : public ProcessLineEdit {
    /*!
     * \param[in] l : label
     * \param[in] c : command guess
     * \param[in] m : mode used to display the results
     * \param[in] p : parent
     *
     * \note unlike \c ProcessLineEdit, the whole text typed is the shell
     * command: it is not split into a program and its arguments
     */
    ShellProcessLineEdit(const QString &,
                         const QString &,
                         const QString &,
                         EditorWidget &);
    //! destructor
    ~ShellProcessLineEdit() override;

   protected:
    /*!
     * \brief hand the text the user typed over to a shell, which runs it and
     * whose output is displayed
     *
     * The shell is \c cmd.exe on Windows, the \c SHELL environment variable
     * elsewhere.
     *
     * \note an empty text is reported in the minibuffer and runs nothing, and
     * so does a missing \c SHELL variable
     */
    void treatUserInput() override;

   private:
    Q_OBJECT

  };  // end of ShellProcessLineEdit

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SHELLPROCESSLINEEDIT_HXX */
