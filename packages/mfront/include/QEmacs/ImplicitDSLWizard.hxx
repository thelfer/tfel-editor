/*!
 * \file   ImplicitDSLWizard.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#ifndef LIB_QEMACS_IMPLICITDSLWIZARD_HXX
#define LIB_QEMACS_IMPLICITDSLWIZARD_HXX

#include <QtWidgets/QWizard>

namespace qemacs{

  struct BehaviourMetaDataPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] p: parent
     */
    BehaviourMetaDataPage(QWizard *const);

    bool validatePage() override;
    //!
    int nextId() const override;
    //! destructor
    ~BehaviourMetaDataPage() override;

   private:
    Q_OBJECT
  }; // end of struct BehaviourMetaDataPage

  struct ImplicitDSLWizard : public QWizard {
    /*!
     * \brief constructor
     * \param[in] p: parent
     */
    ImplicitDSLWizard(QWidget *const p = nullptr);
    //! destructor
    ~ImplicitDSLWizard() override;

   protected:
    BehaviourMetaDataPage *md = nullptr;

   private:
    Q_OBJECT
  };  // end of struct ImplicitDSLWizard

} // end of namespace qemacs

#endif /* LIB_QEMACS_IMPLICITDSLWIZARD_HXX */
