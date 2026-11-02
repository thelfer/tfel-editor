/*!
 * \file   MFMDataBase.hxx
 * \brief analyses the libraries of the MFM galleries and fills a model.
 * \author Thomas Helfer
 * \date   21/02/2018
 */

#ifndef LIB_TFEL_GUI_MFMDATABASE_HXX
#define LIB_TFEL_GUI_MFMDATABASE_HXX

#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  /*!
   * \brief structure in charge of analysing all shared libraries found
   * in the `lib` (or `bin` on Windows) subdirectories of directories
   * listed in the `MFMHOME` environment variable.
   *
   * The libraries of `MFRONTGALLERYHOME` are analysed in the same way, after
   * those of `MFMHOME`.
   *
   * \note a file is analysed when its name has the prefix and the suffix of a
   * shared library of the current platform; it is then handed to the
   * \c tfel::system::ExternalLibraryManager, which lists its entry points.
   * A library or an entry point which can not be analysed is skipped
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT MFMDataBase {
    //! \brief constructor: the database holds nothing until \c load is called
    MFMDataBase();
    /*!
     * \brief analyse the libraries of the directories and build the model of
     * their contents
     * \param[in] p: parent used for the resulting model, which Qt owns
     *
     * The model has one row per entry point and five columns: the kind of
     * material knowledge (\c MaterialProperty, \c Behaviour or \c Model), the
     * name of the entry point, the material it belongs to, the path of the
     * library which holds it and its interface. The rows are inserted at the
     * top, so the last library analysed comes first.
     *
     * \return the model, whose ownership is given to \c p by Qt; it is empty,
     * and the lists of interfaces stay empty, when the environment variables
     * are not defined
     *
     * \note the lists of interfaces are only filled by this method: querying
     * them before \c load has run returns empty lists
     *
     * \note the horizontal header of the model reads \c Type, \c Name,
     * \c Material, \c Library and \c Interface for these five columns; each
     * cell holds a plain string, which is what the views display and sort on
     *
     * \note the galleries are analysed again on each call: the model is not
     * shared between the callers
     */
    QAbstractItemModel* load(QObject* const);

    /*!
     * \return the list of interfaces associated to material properties, in the
     * order they have been found, without duplicates; empty before \c load has
     * been called
     */
    const QStringList& getMaterialPropertiesInterfaces() const;
    /*!
     * \return the list of interfaces associated to behaviours, in the order
     * they have been found, without duplicates; empty before \c load has been
     * called
     */
    const QStringList& getBehavioursInterfaces() const;
    /*!
     * \return the list of interfaces associated to models, in the order they
     * have been found, without duplicates; empty before \c load has been called
     */
    const QStringList& getModelsInterfaces() const;
    //! destructor
    ~MFMDataBase();

   protected:
    //! interfaces associated to material properties
    QStringList mpInterfaces;
    //! interfaces associated to behaviours
    QStringList bInterfaces;
    //! interfaces associated to models
    QStringList mInterfaces;
  };  // end of struct MFMDataBase

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFMDATABASE_HXX */
