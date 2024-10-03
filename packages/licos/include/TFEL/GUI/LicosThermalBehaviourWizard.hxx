/*!
 * \file  LicosThermalBehaviourWizard.hxx
 * \brief
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
     * \param[in] t :
     */
    LicosThermalBehaviourWizard(TextEditBase&);

    bool validateCurrentPage() override;

   protected:
    //! the textedit modified by the wizard
    TextEditBase& textEdit;

  };  // end of struct LicosThermalBehaviourWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSTHERMALBEHAVIOURWIZARD_H */
