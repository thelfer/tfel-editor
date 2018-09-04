/*! 
 * \file  LicosMainBlock.hxx
 * \brief
 * \author Helfer Thomas
 * \date   17/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSMAINBLOCK_HXX
#define LIB_TFEL_GUI_LICOSMAINBLOCK_HXX 

#include"TFEL/GUI/StandardLicosBlock.hxx"

namespace tfel{

  namespace gui
{

  /*!
   * licos main block
   */
  struct LicosMainBlock
    : public StandardLicosBlock
  {
    LicosMainBlock();
  };
  
} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_LICOSMAINBLOCK_H */

