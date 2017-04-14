/*! 
 * \file  LicosMaterialWizard.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 sept. 2012
 */

#ifndef _LIB_QEMACS_LICOSMATERIALWIZARD_H_
#define _LIB_QEMACS_LICOSMATERIALWIZARD_H_ 

#ifdef QEMACS_QT4
#include<QtGui/QWizard>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QWizard>
#endif /* QEMACS_QT5 */

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

    bool
    validateCurrentPage () ;

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

#endif /* _LIB_QEMACS_LICOSMATERIALWIZARD_H */

