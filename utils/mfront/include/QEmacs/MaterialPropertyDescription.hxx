/*!
 * \file   MaterialPropertyDescription.hxx
 * \brief    
 * \author th202608
 * \date   05/03/2018
 */

#ifndef LIB_MATERIALPROPERTYDESCRIPTION_HXX
#define LIB_MATERIALPROPERTYDESCRIPTION_HXX

#include <QtCore/QString>
#include "TFEL/Utilities/GenTypeBase.hxx"

namespace qemacs{

  /*!
   * \brief structure describing a constant material property
   */
  struct ConstantMaterialPropertyDescription {
    QString name;
    double value;
  };  // end of struct ConstantMaterialPropertyDescription

  /*!
   * \brief structure describing a `Cast3M` material property
   */
  struct CastemMaterialPropertyDescription {
    QString name;
    QString library;
    QString function;
  };  // end of struct CastemMaterialPropertyDescription

  //! a simple alias
  using MaterialPropertyDescription =
      tfel::utilities::GenType<ConstantMaterialPropertyDescription,
                               CastemMaterialPropertyDescription>;

} // end of namespace qemacs

#endif /* LIB_MATERIALPROPERTYDESCRIPTION_HXX */
