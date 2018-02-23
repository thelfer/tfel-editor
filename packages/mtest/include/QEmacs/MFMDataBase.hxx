/*!
 * \file   MFMDataBase.hxx
 * \brief
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#ifndef LIB_QEMACS_MFMDATABASE_HXX
#define LIB_QEMACS_MFMDATABASE_HXX

#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>

namespace qemacs {

  /*!
   * \brief structure in charge of analysing all shared libraries found
   * in the `lib` (or `bin` on Windows) subdirectories of directories
   * listed in the `MFMHOME` environment variable.
   */
  struct MFMDataBase {
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MFMDATABASE_HXX */
