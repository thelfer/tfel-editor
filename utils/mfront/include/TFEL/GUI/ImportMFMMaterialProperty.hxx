/*!
 * \file   ImportMFMMaterialProperty.hxx
 * \brief
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

namespace tfel {

  namespace gui {

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
        //! \brief constructor
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
       */
      ImportMFMMaterialProperty(EditorWidget&,
                                const Options& o,
                                QWidget* const p = nullptr);
      //! \return the selected library
      QString getLibrary() const;
      //! \return the selected function
      QString getFunction() const;
      //! destructor
      ~ImportMFMMaterialProperty() override;

     protected:
      QTreeView* view;
      // interface
      QComboBox* isb;

     private:
      Q_OBJECT
    };  // end of struct ImportMFMMaterialProperty

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_IMPORTMFMMATERIALPROPERTY_HXX */
