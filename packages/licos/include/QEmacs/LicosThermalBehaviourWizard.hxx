/*! 
 * \file  LicosThermalBehaviourWizard.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 sept. 2012
 */

#ifndef LIB_QEMACS_LICOSTHERMALBEHAVIOURWIZARD_HXX
#define LIB_QEMACS_LICOSTHERMALBEHAVIOURWIZARD_HXX 

#include<QtWidgets/QWizard>

namespace qemacs{

  //! foward declaration
  class QEmacsTextEditBase;

  /*!
   * a wizard to introduce a new material
   */
  struct LicosThermalBehaviourWizard
    : public QWizard
  {
    /*!
     * Constructor
     * \param[in] t : 
     */
    LicosThermalBehaviourWizard(QEmacsTextEditBase&);

    bool validateCurrentPage() override;

  protected:
    
    //! the textedit modified by the wizard 
    QEmacsTextEditBase& textEdit;

  }; // end of struct LicosThermalBehaviourWizard

} // end of namespace qemacs

#endif /* LIB_QEMACS_LICOSTHERMALBEHAVIOURWIZARD_H */
