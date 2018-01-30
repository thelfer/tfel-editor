/*! 
 * \file  LicosMainBlock.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#ifndef LIB_QEMACS_LICOSMAINBLOCK_HXX
#define LIB_QEMACS_LICOSMAINBLOCK_HXX 

#include"QEmacs/StandardLicosBlock.hxx"

namespace qemacs
{

  /*!
   * licos main block
   */
  struct LicosMainBlock
    : public StandardLicosBlock
  {
    LicosMainBlock();
  };
  
} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSMAINBLOCK_H */
