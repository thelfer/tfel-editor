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

namespace qemacs {

  /*!
   * \brief a widget aiming at selecting material property
   */
  struct MaterialPropertySelectorWidget : QGroupBox {
    /*!
     * \brief constructor
     * \param[in] n: name
     * \param[in] p: parent widget
     */
    MaterialPropertySelectorWidget(const QString&,
                                   QWidget* const = nullptr);
    //! destructor
    ~MaterialPropertySelectorWidget() override;

   public slots:

    //! import from a user library
    void import();
    //! import from mfront materials
    void importFromMFM();

   protected:
    //! externale name of the material property
    QString ename;

   private:
    Q_OBJECT
  };  // end of struct MaterialPropertySelectorWidget

} // end of namespace qemacs

#endif /* LIB_QEMACS_MATERIALPROPERTYSELECTORWIDGET_HXX */
