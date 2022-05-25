/*!
 * \file  GrepCommand.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_GREPCOMMAND_HXX
#define LIB_TFEL_GUI_GREPCOMMAND_HXX

#include "TFEL/GUI/Command.hxx"

namespace tfel::gui {

  //! \brief a command based on the grep utility
  struct GrepCommand : public Command {
    GrepCommand(EditorWidget&);

    void execute() override;
    //! \brief destructor
    ~GrepCommand() override;
  };  // end of GrepCommand

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GREPCOMMAND_HXX */
