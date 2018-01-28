/*! 
 * \file  LicosMaterialWizard.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 sept. 2012
 */

#ifndef LIB_QEMACS_LICOSMATERIALWIZARD_HXX
#define LIB_QEMACS_LICOSMATERIALWIZARD_HXX 

#include<QtWidgets/QWizard>

namespace qemacs{

  // forward declaration
  class QEmacsTextEditBase;

  // forward declaration
  struct LicosThermalBehaviourWizard;

  // foward declaration
  struct LicosMechanicalBehaviourWizard;

  /*!
   * a wizard to introduce a new material
   */
  struct LicosMaterialWizard
    : public QWizard
  {
    /*!
     * Constructor
     * \param[in] t : 
     */
    LicosMaterialWizard(QEmacsTextEditBase&);

    bool validateCurrentPage() override;

    void writeMaterial();

  protected:

    struct MaterialPage;

    //! the textedit modified by the wizard 
    QEmacsTextEditBase& textEdit;

    //! the main page
    MaterialPage *mpage;

    //! wizard for the thermal behaviour
    LicosThermalBehaviourWizard *tbw;

    //! wizard for the mechanial behaviour
    LicosMechanicalBehaviourWizard *mbw;

  }; // end of struct LicosMaterialWizard

} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSMATERIALWIZARD_H */

