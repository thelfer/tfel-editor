/*!
 * \file  LicosMainBlock.hxx
 * \brief LicosMainBlock: the main block of a licos input file.
 * \author Thomas Helfer
 * \date   17/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSMAINBLOCK_HXX
#define LIB_TFEL_GUI_LICOSMAINBLOCK_HXX

#include "TFEL/GUI/StandardLicosBlock.hxx"

namespace tfel::gui {

  /*!
   * licos main block
   */
  struct LicosMainBlock : public StandardLicosBlock {
    /*!
     * \brief builds the `main` block of a licos input file
     *
     * The keys and the sub-blocks of the block are those the licos executable
     * expects in the `main` block (coupling items, output, time stepping,
     * etc.): they define the parameters and the sub-blocks the file may hold,
     * hence the indentation and the completion of the mode.
     */
    LicosMainBlock();
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSMAINBLOCK_H */
