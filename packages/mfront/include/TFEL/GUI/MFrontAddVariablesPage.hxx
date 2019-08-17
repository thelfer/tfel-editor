/*!
 * \file   MFrontAddVariablesPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX
#define LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX

#include <QtWidgets/QWizard>
#include "MFront/VariableDescription.hxx"
#include "MFront/BehaviourDescription.hxx"

// forward declaration
class QComboBox;

namespace tfel {

  namespace gui {

    // forward declaration
    struct EditorWidget;

    struct MFrontAddVariablesPage : public QWizardPage {
      /*!
       * \brief constructor
       * \param[in] w: editor widget
       * \param[in] p: parent
       */
      MFrontAddVariablesPage(EditorWidget &, QWizard *const);

      bool validatePage() override;

      int nextId() const override;
      //! \brief write the output of the wizard
      virtual void write() const;
      //! destructor
      ~MFrontAddVariablesPage() override;

     protected:
      //! \brief material properties
      mfront::VariableDescriptionContainer mps;

      template <void (mfront::BehaviourDescription::*f)(
          const mfront::BehaviourDescription::Hypothesis,
          const mfront::VariableDescription &,
          const mfront::BehaviourData::RegistrationStatus)>
      void addVariable(
          mfront::VariableDescriptionContainer &,
          const mfront::VariableDescription &);

     private:
      Q_OBJECT
      };  // end of struct MFrontAddVariablesPage

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX */
