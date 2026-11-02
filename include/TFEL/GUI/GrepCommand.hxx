/*!
 * \file  GrepCommand.hxx
 * \brief command based on the grep utility.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_GREPCOMMAND_HXX
#define LIB_TFEL_GUI_GREPCOMMAND_HXX

#include "TFEL/GUI/Command.hxx"

namespace tfel::gui {

  //! \brief a command based on the grep utility
  struct GrepCommand : public Command {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the command is run on, it is also the parent
     * of the command
     */
    GrepCommand(EditorWidget&);

    /*!
     * \brief ask the user for a grep command in the minibuffer
     *
     * The command proposed starts with "grep -nH -e " and the results are
     * displayed with the "grep-output" major mode.
     *
     * \note the input is owned by the editor once \c setUserInput has been
     * given it
     */
    void execute() override;
    //! \brief destructor
    ~GrepCommand() override;
  };  // end of GrepCommand

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GREPCOMMAND_HXX */
