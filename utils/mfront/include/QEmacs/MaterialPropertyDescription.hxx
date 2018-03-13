/*!
 * \file   MaterialPropertyDescription.hxx
 * \brief    
 * \author th202608
 * \date   05/03/2018
 */

#ifndef LIB_MATERIALPROPERTYDESCRIPTION_HXX
#define LIB_MATERIALPROPERTYDESCRIPTION_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>
#include "TFEL/Utilities/GenTypeBase.hxx"
#include "QEmacs/QEmacsMFrontImportConfig.hxx"

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
  struct QEMACS_MIW_VISIBILITY_EXPORT
      CastemMaterialPropertyDescription {
    /*!
     * \brief constructor
     * \param[in] n: name
     * \param[in] l: library
     * \param[in] f: function
     */
    CastemMaterialPropertyDescription(const QString&,
                                      const QString&,
                                      const QString&);
    //! \brief move constructor
    CastemMaterialPropertyDescription(
        CastemMaterialPropertyDescription&&);
    //! \brief copy constructor
    CastemMaterialPropertyDescription(
        const CastemMaterialPropertyDescription&);
    //! \brief move assignement
    CastemMaterialPropertyDescription& operator=(
        CastemMaterialPropertyDescription&&);
    //! \brief standard assignement
    CastemMaterialPropertyDescription& operator=(
        const CastemMaterialPropertyDescription&);
    //! name of the material property
    QString name;
    //! shared library
    QString library;
    //! function
    QString function;
    //! list of arguments
    QStringList arguments;
  };  // end of struct CastemMaterialPropertyDescription



  //! a simple alias
  using MaterialPropertyDescription =
      tfel::utilities::GenType<ConstantMaterialPropertyDescription,
                               CastemMaterialPropertyDescription>;

} // end of namespace qemacs

#endif /* LIB_MATERIALPROPERTYDESCRIPTION_HXX */
