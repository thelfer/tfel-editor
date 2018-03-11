/*!
 * \file   ImportMFMMaterialProperty.hxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#ifndef LIB_QEMACS_IMPORTMFMMATERIALPROPERTY_HXX
#define LIB_QEMACS_IMPORTMFMMATERIALPROPERTY_HXX

#include <memory>
#include <utility>
#include <QtCore/QString>
#include <QtWidgets/QDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeView>
// #include "QEmacs/MaterialPropertyDescription.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;

  /*!
   * \brief a dialog box used to import material properties from the
   * MFrontMaterials project.
   * \see the MFMDataBase class.
   */
  struct ImportMFMMaterialProperty : QDialog {
    //! option of the dialog box
    struct Options {
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
     * \param[in] q: qemacs widget
     * \param[in] o: options
     * \param[in] p: parent
     */
    ImportMFMMaterialProperty(QEmacsWidget&,
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_IMPORTMFMMATERIALPROPERTY_HXX */
