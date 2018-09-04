/*!
 * \file   MFMDataBase.hxx
 * \brief
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#ifndef LIB_TFEL_GUI_MFMDATABASE_HXX
#define LIB_TFEL_GUI_MFMDATABASE_HXX

#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel{

  namespace gui{

  /*!
   * \brief structure in charge of analysing all shared libraries found
   * in the `lib` (or `bin` on Windows) subdirectories of directories
   * listed in the `MFMHOME` environment variable.
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT  MFMDataBase {
    //! \brief constructor
    MFMDataBase();
    /*!
     * \param[in] p: parent used for the resul
     */
    QAbstractItemModel* load(QObject* const);

    //! \return the list of interfaces associated to material properties
    const QStringList& getMaterialPropertiesInterfaces() const;
    //! \return the list of interfaces associated to behaviours
    const QStringList& getBehavioursInterfaces() const;
    //! \return the list of interfaces associated to models
    const QStringList& getModelsInterfaces() const;
    //! destructor
    ~MFMDataBase();

   protected:
    //! interfaces associated to material properties
    QStringList mpInterfaces;
    //! interfaces associated to behaviours
    QStringList bInterfaces;
    //! interfaces associated to behaviours
    QStringList mInterfaces;

  };  // end of struct MFMDataBase

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_MFMDATABASE_HXX */
