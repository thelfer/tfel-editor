/*!
 * \file  LicosThermalBehaviourWizard.hxx
 * \brief a wizard to introduce a thermal behaviour, declared but unused.
 * \author Thomas Helfer
 * \date   27/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSTHERMALBEHAVIOURWIZARD_HXX
#define LIB_TFEL_GUI_LICOSTHERMALBEHAVIOURWIZARD_HXX

#include <QtWidgets/QWizard>

namespace tfel::gui {

  //! foward declaration
  struct TextEditBase;

  //! a wizard to introduce a new thermal material
  struct LicosThermalBehaviourWizard : public QWizard {
    /*!
     * Constructor
     * \param[in] t : text edit the thermal behaviour is to be inserted in, and
     * the parent of the wizard in the Qt sense
     *
     * \note no implementation of this constructor is provided by the mode: the
     * class is declared only, is never instantiated, and the member which would
     * hold such a wizard, \c LicosMaterialWizard::tbw, is left null
     */
    LicosThermalBehaviourWizard(TextEditBase&);

    /*!
     * \brief check the page the user is on before the wizard moves to the next
     * one
     *
     * \note no implementation of this override is provided by the mode either;
     * the meaning it would give to a valid page is not known
     */
    bool validateCurrentPage() override;

   protected:
    //! the textedit modified by the wizard
    TextEditBase& textEdit;

  };  // end of struct LicosThermalBehaviourWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSTHERMALBEHAVIOURWIZARD_H */
