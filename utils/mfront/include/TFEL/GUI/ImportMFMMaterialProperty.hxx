/*!
 * \file   ImportMFMMaterialProperty.hxx
 * \brief dialog selecting a material property of the MFM database.
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#ifndef LIB_TFEL_GUI_IMPORTMFMMATERIALPROPERTY_HXX
#define LIB_TFEL_GUI_IMPORTMFMMATERIALPROPERTY_HXX

#include <memory>
#include <utility>
#include <QtCore/QString>
#include <QtWidgets/QDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeView>
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief a dialog box used to import material properties from the
   * MFrontMaterials project.
   * \see the MFMDataBase class.
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportMFMMaterialProperty : QDialog {
    //! option of the dialog box
    struct Options {
      //! \brief constructor
      Options();
      //! \brief move constructor
      Options(Options&&);
      //! \brief copy constructor
      Options(const Options&);
      //! \brief move assignement
      Options& operator=(Options&&);
      //! \brief copy assignement
      Options& operator=(const Options&);
      /*!
       * \brief name of the material property. This is used to
       * initialize the name filter.
       */
      QString name;
      /*!
       * \brief name of the material. This is used to initialize the
       * material filter.
       */
      QString material;
      /*!
       * \brief if this member is not empty, the user will not be able
       * to select the interface of the material property. The material
       * properties displayed would use the given interface.
       */
      QString minterface;
    };
    /*!
     * \brief constructor
     * \param[in] q: editor widget
     * \param[in] o: options
     * \param[in] p: parent
     *
     * The constructor analyses the galleries of `MFMHOME` and
     * `MFRONTGALLERYHOME` through \c MFMDataBase::load and shows their material
     * properties in the tree view, so the dialog only reflects the contents of
     * those directories at the time it is built
     *
     * \note the loaded model and the proxy model are given the dialog as parent
     * and are released with it; the other widgets are reparented by the layout
     *
     * \note `Ok` accepts the dialog, `Cancel` rejects it and double clicking a
     * row accepts it as well
     *
     * \note `o.name` and `o.material` pre-fill the corresponding filters;
     * `o.minterface` restricts the list to that interface and hides the
     * interface selection
     */
    ImportMFMMaterialProperty(EditorWidget&,
                              const Options& o,
                              QWidget* const p = nullptr);
    /*!
     * \return the path of the library which holds the selected material
     * property, that is the fourth column of the selected row
     *
     * \note an empty string is returned when no row or several rows are
     * selected
     */
    QString getLibrary() const;
    /*!
     * \return the name of the selected material property, that is the entry
     * point of the library, which is the second column of the selected row
     *
     * \note an empty string is returned when no row or several rows are
     * selected
     */
    QString getFunction() const;
    //! destructor
    ~ImportMFMMaterialProperty() override;

   protected:
    //! tree view showing the material properties of the galleries
    QTreeView* view;
    /*!
     * \brief combo box listing the interfaces of the material properties, the
     * first entry being the regular expression `.+` which shows all of them
     *
     * \note it is only shown, and only connected to the interface filter of the
     * proxy model, when the interface is not imposed by \c Options::minterface:
     * in that case it is left out of the layout of the dialog and the interface
     * column of the tree view is hidden as well
     */
    QComboBox* isb;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMMaterialProperty

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_IMPORTMFMMATERIALPROPERTY_HXX */
