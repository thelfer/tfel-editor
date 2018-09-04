/*!
 * \file   ImplicitDSLWizard.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#ifndef LIB_TFEL_GUI_IMPLICITDSLWIZARD_HXX
#define LIB_TFEL_GUI_IMPLICITDSLWIZARD_HXX

#include <QtWidgets/QWizard>

namespace tfel{

  namespace gui{

  // forward declaration
  struct EditorWidget;

  struct BehaviourMetaDataPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] q: editor widget
     * \param[in] p: parent
     */
    BehaviourMetaDataPage(EditorWidget&, QWizard *const);

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
     * \param[in] q: editor widget
     * \param[in] p: parent
     */
    ImplicitDSLWizard(EditorWidget &, QWidget *const p = nullptr);
    //! destructor
    ~ImplicitDSLWizard() override;

   protected:
    BehaviourMetaDataPage *md = nullptr;

   private:
    Q_OBJECT
  };  // end of struct ImplicitDSLWizard

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_IMPLICITDSLWIZARD_HXX */
