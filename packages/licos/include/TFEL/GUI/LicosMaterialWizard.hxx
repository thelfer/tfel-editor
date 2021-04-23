/*!
 * \file  LicosMaterialWizard.hxx
 * \brief
 * \author Helfer Thomas
 * \date   27/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSMATERIALWIZARD_HXX
#define LIB_TFEL_GUI_LICOSMATERIALWIZARD_HXX

#include <QtWidgets/QWizard>

namespace tfel {

  namespace gui {

    // forward declaration
    struct TextEditBase;
    // forward declaration
    struct LicosThermalBehaviourWizard;
    // foward declaration
    struct LicosMechanicalBehaviourWizard;

    /*!
     * a wizard to introduce a new material
     */
    struct LicosMaterialWizard : public QWizard {
      /*!
       * Constructor
       * \param[in] t :
       */
      LicosMaterialWizard(TextEditBase &);

      bool validateCurrentPage() override;

      void writeMaterial();

     protected:
      struct MaterialPage;

      //! the textedit modified by the wizard
      TextEditBase &textEdit;

      //! the main page
      MaterialPage *mpage;

      //! wizard for the thermal behaviour
      LicosThermalBehaviourWizard *tbw;

      //! wizard for the mechanial behaviour
      LicosMechanicalBehaviourWizard *mbw;

    };  // end of struct LicosMaterialWizard

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_LICOSMATERIALWIZARD_H */
