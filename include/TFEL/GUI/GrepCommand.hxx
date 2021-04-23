/*!
 * \file  GrepCommand.hxx
 * \brief
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_GREPCOMMAND_HXX
#define LIB_TFEL_GUI_GREPCOMMAND_HXX

#include "TFEL/GUI/Command.hxx"

namespace tfel {

  namespace gui {

    /*!
     * A command based on the grep utility
     */
    struct GrepCommand : public Command {
      GrepCommand(EditorWidget&);

      void execute() override;

      ~GrepCommand() override;

    };  // end of GrepCommand

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_GREPCOMMAND_HXX */
