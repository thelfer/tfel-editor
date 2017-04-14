/*! 
 * \file  LicosMainBlock.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 17 sept. 2012
 */

#ifndef _LIB_QEMACS_LICOSMAINBLOCK_H_
#define _LIB_QEMACS_LICOSMAINBLOCK_H_ 

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

#endif /* _LIB_QEMACS_LICOSMAINBLOCK_H */

