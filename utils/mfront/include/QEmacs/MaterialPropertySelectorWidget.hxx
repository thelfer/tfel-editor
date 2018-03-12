/*!
 * \file   include/QEmacs/MaterialPropertySelectorWidget.hxx
 * \brief
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#ifndef LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX
#define LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX

#include <QtCore/QString>
#include <QtWidgets/QGroupBox>
#include "TFEL/Utilities/GenTypeBase.hxx"
#include "QEmacs/QEmacsLineEdit.hxx"
#include "QEmacs/MaterialPropertyDescription.hxx"
#include "QEmacs/QEmacsMFrontImportWidgetsConfig.hxx"

namespace qemacs {

  /*!
   * \brief a widget aiming at selecting material property
   */
  struct QEMACS_MIW_VISIBILITY_EXPORT MaterialPropertySelectorWidget
      : QGroupBox {
    /*!
     * \brief constructor
     * \param[in] q: qemacs widget
     * \param[in] n: name
     * \param[in] m: material
     * \param[in] p: parent widget
     */
    MaterialPropertySelectorWidget(QEmacsWidget &,
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
    //! qemacs widget
    QEmacsWidget &qemacs;
    //! external name of the material property
    QString name;
    //! material name
    QString material;
    /*!
     * \brief value used for a constant material property.
     */
    QEmacsLineEdit *cv;
    /*!
     * \brief a line edit to display the library path for a `Cast3M`
     * material property.
     */
    QEmacsLineEdit *cle;
    /*!
     * \brief a line edit to display the function for a `Cast3M`
     * material property.
     */
    QEmacsLineEdit *cfe;

   private:
    Q_OBJECT
  };  // end of struct MaterialPropertySelectorWidget

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX */
