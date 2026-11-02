/*!
 * \file   ProcessLineEdit.hxx
 * \brief input which runs a command and shows its output.
 * \author Thomas Helfer
 * \date   15/02/2018
 */

#ifndef LIB_TFEL_GUI_PROCESSLINEEDIT_HXX
#define LIB_TFEL_GUI_PROCESSLINEEDIT_HXX

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CommandLine.hxx"

namespace tfel::gui {

  /*!
   * An helper class asking the user for a command and displaying the
   * results in a ProcessOutputFrame created as a slave of the current
   * buffer
   */
  struct TFEL_GUI_VISIBILITY_EXPORT ProcessLineEdit : public CommandLine {
    /*!
     * \param[in] l : label
     * \param[in] c : command guess
     * \param[in] m : mode used to display the results
     * \param[in] p : parent
     *
     * \note \c c is written in the line edit, so the user has to complete it,
     * or to validate it as is, to run the command. An empty \c m means the
     * output frame keeps the mode of a plain text
     */
    ProcessLineEdit(const QString &,
                    const QString &,
                    const QString &,
                    EditorWidget &);
    //! \brief destructor
    ~ProcessLineEdit() override;

   protected:
    /*!
     * \brief run the command the user typed
     *
     * The text is split on blanks: the first word is the program, the other
     * ones are its arguments.
     *
     * \note an empty text is reported in the minibuffer and runs nothing
     */
    void treatUserInput() override;

    /*!
     * \brief start the given command and display its output in a secondary
     * task of the current buffer
     * \param[in] on : name the secondary task is titled with
     * \param[in] c : program to start
     * \param[in] args : arguments of the program
     *
     * The program is started in the directory of the main frame of the current
     * buffer, or in the current directory when that directory does not exist
     * any more. In the arguments, a ".", a "$(cmd)" and a "%cmd%" are replaced
     * by that directory, so that a command can refer to the directory of the
     * buffer it was asked for.
     *
     * \note an empty \c c is reported in the minibuffer and starts nothing
     *
     * \note when \c mode is not empty, the output frame takes that major mode
     * and, if it is a \c ProcessOutputMajorModeBase, records the directory, the
     * command and its arguments so that the process can be restarted from the
     * context menu
     */
    virtual void run(const QString &, const QString &, const QStringList &);
    //! major mode for the process output frame
    const QString mode;

   private:
    Q_OBJECT
  };  // end of ProcessLineEdit

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_PROCESSLINEEDIT_HXX */
