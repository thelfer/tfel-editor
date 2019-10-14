/*!
 * \file   MFrontBehaviourWizardPages.hxx
 * \brief
 * \author th202608
 * \date   01/09/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTBEHAVIOURWIZARDPAGES_HXX
#define LIB_TFEL_GUI_MFRONTBEHAVIOURWIZARDPAGES_HXX

namespace tfel {

  namespace gui {

    /*!
     * \brief index of the pages used by the `MFrontBehaviourWizard`
     * wizard.
     * \note we don't use strongly typed enum, because the latter does
     * not allow conversion to int.
     */
    struct MFrontBehaviourWizardPages {
      static const int METADATA = 0;
      static const int BEHAVIOUR = 1;
      static const int STANDARDELASTOVISCOPLASTICITY = 2;
      static const int ADDVARIABLES = 3;
      static const int TEMPLATEGENERATIONOPTIONS = 4;
    };  // end of MFrontBehaviourWizardPages

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTBEHAVIOURWIZARDPAGES_HXX */
