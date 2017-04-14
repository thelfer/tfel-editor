/*! 
 * \file  LicosThermalBehaviourWizard.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 sept. 2012
 */

#ifndef _LIB_QEMACS_LICOSTHERMALBEHAVIOURWIZARD_H_
#define _LIB_QEMACS_LICOSTHERMALBEHAVIOURWIZARD_H_ 

#ifdef QEMACS_QT4
#include<QtGui/QWizard>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QWizard>
#endif /* QEMACS_QT5 */

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

    bool
    validateCurrentPage () ;

  protected:
    
    //! the textedit modified by the wizard 
    QEmacsTextEditBase& textEdit;

  }; // end of struct LicosThermalBehaviourWizard

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LICOSTHERMALBEHAVIOURWIZARD_H */

