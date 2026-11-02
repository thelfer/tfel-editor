/*!
 * \file   MaterialPropertyDescription.hxx
 * \brief description of a material property of the MFM database.
 * \author Thomas Helfer
 * \date   05/03/2018
 */

#ifndef LIB_MATERIALPROPERTYDESCRIPTION_HXX
#define LIB_MATERIALPROPERTYDESCRIPTION_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>
#include "TFEL/Utilities/GenTypeBase.hxx"
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  /*!
   * \brief structure describing a constant material property
   */
  struct ConstantMaterialPropertyDescription {
    //! name of the material property, as it is seen by the behaviour
    QString name;
    //! value of the material property
    double value;
  };  // end of struct ConstantMaterialPropertyDescription

  /*!
   * \brief structure describing a `Cast3M` material property
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT CastemMaterialPropertyDescription {
    /*!
     * \brief constructor
     * \param[in] n: name
     * \param[in] l: library
     * \param[in] f: function
     *
     * \note the arguments of the function are read from
     * \c tfel::system::ExternalLibraryManager; the list is empty when the
     * library or the function can not be analysed
     */
    CastemMaterialPropertyDescription(const QString&,
                                      const QString&,
                                      const QString&);
    //! \brief move constructor
    CastemMaterialPropertyDescription(CastemMaterialPropertyDescription&&);
    //! \brief copy constructor
    CastemMaterialPropertyDescription(const CastemMaterialPropertyDescription&);
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
    //! list of arguments, that is the names of the variables of `Cast3M`
    QStringList arguments;
  };  // end of struct CastemMaterialPropertyDescription

  /*!
   * \brief a simple alias: a material property is either a constant or a
   * `Cast3M` one, and an empty variant is used for a material property the
   * user has not defined yet
   */
  using MaterialPropertyDescription =
      tfel::utilities::GenType<ConstantMaterialPropertyDescription,
                               CastemMaterialPropertyDescription>;

}  // end of namespace tfel::gui

#endif /* LIB_MATERIALPROPERTYDESCRIPTION_HXX */
