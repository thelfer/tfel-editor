/*!
 * \file   include/TFEL/GUI/MaterialPropertySelectorWidget.hxx
 * \brief
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#ifndef LIB_TFEL_GUI_MATERIALPROPERTYSELECTORWIDGET_HXX
#define LIB_TFEL_GUI_MATERIALPROPERTYSELECTORWIDGET_HXX

#include <QtCore/QString>
#include <QtWidgets/QGroupBox>
#include "TFEL/Utilities/GenTypeBase.hxx"
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/MaterialPropertyDescription.hxx"
#include "TFEL/GUI/MFrontImportWidgetsConfig.hxx"

namespace tfel::gui {

  /*!
   * \brief a widget aiming at selecting material property
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT MaterialPropertySelectorWidget
      : QGroupBox {
    /*!
     * \brief constructor
     * \param[in] q: editor widget
     * \param[in] n: name
     * \param[in] m: material
     * \param[in] p: parent widget
     */
    MaterialPropertySelectorWidget(EditorWidget &,
                                   const QString &,
                                   const QString &,
                                   QWidget *const = nullptr);
    //! \return the selected material property
    MaterialPropertyDescription getMaterialProperty() const;
    //! destructor
    ~MaterialPropertySelectorWidget() override;

   public slots:

    //! import from a user library
    void import();
    //! import from mfront materials
    void importFromMFM();

   protected:
    //! editor widget
    EditorWidget &editor;
    //! external name of the material property
    QString name;
    //! material name
    QString material;
    /*!
     * \brief value used for a constant material property.
     */
    LineEdit *cv;
    /*!
     * \brief a line edit to display the library path for a `Cast3M`
     * material property.
     */
    LineEdit *cle;
    /*!
     * \brief a line edit to display the function for a `Cast3M`
     * material property.
     */
    LineEdit *cfe;

   private:
    Q_OBJECT
  };  // end of struct MaterialPropertySelectorWidget

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MATERIALPROPERTYSELECTORWIDGET_HXX */
