/*!
 * \file   include/QEmacs/MaterialPropertySelectorWidget.hxx
 * \brief
 * \author Thomas Helfer
 * \date   01/03/2018
 */

#ifndef LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX
#define LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX

#include <QtCore/QString>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include "TFEL/Utilities/GenTypeBase.hxx"
#include "QEmacs/MaterialPropertyDescription.hxx"

namespace qemacs {

  /*!
   * \brief a widget aiming at selecting material property
   */
  struct MaterialPropertySelectorWidget : QGroupBox {
    /*!
     * \brief constructor
     * \param[in] n: name
     * \param[in] m: material
     * \param[in] p: parent widget
     */
    MaterialPropertySelectorWidget(const QString &,
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
    //! external name of the material property
    QString name;
    //! material name
    QString material;
    /*!
     * \brief value used for a constant material property.
     */
    QLineEdit *cv;
    /*!
     * \brief a line edit to display the library path for a `Cast3M`
     * material property.
     */
    QLineEdit *cle;
    /*!
     * \brief a line edit to display the function for a `Cast3M`
     * material property.
     */
    QLineEdit *cfe;

   private:
    Q_OBJECT
  };  // end of struct MaterialPropertySelectorWidget

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX */
