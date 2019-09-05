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

namespace mfront {
  // forward declaration
  struct AbstractBehaviourDSL;
}

namespace tfel {

  namespace gui {

    // forward declarations
    struct EditorWidget;
    struct TextEditBase;
    struct MFrontBehaviourWizard;

    struct MFrontAddVariablesPage : public QWizardPage {
      //! a simple alias
      using VariableDescriptionContainer =
          mfront::VariableDescriptionContainer;

      /*!
       * \brief constructor
       * \param[in] w: editor widget
       * \param[in] cd: current document
       * \param[in] p: parent
       */
      MFrontAddVariablesPage(EditorWidget &,
                             TextEditBase &,
                             MFrontBehaviourWizard *const);

      bool validatePage() override;

      void initializePage() override;

      int nextId() const override;
      /*!
       * \brief complete the behaviour description
       * \param[in] dsl: behaviour DSL
       */
      virtual void complete(mfront::AbstractBehaviourDSL&) const;
      //! \brief write the output of the wizard
      virtual void write() const;


      //! \return the selected  material properties
      VariableDescriptionContainer getMaterialProperties() const;
      //! \return the selected  parameters
      VariableDescriptionContainer getParameters() const;
      //! \return the selected  state variables
      VariableDescriptionContainer getStateVariables() const;
      //! \return the selected  auxiliary state variables
      VariableDescriptionContainer getAuxiliaryStateVariables() const;
      //! \return the selected  external state variables
      VariableDescriptionContainer getExternalStateVariables() const;
      //! \return the selected  local variables
      VariableDescriptionContainer getLocalVariables() const;
      //! \return the selected  integration variables
      VariableDescriptionContainer getIntegrationVariables() const;

      //! destructor
      ~MFrontAddVariablesPage() override;

     protected:
      //! \brief material properties
      VariableDescriptionContainer materialProperties;
      //! \brief parameters
      VariableDescriptionContainer parameters;
      //! \brief state variables
      VariableDescriptionContainer stateVariables;
      //! \brief auxiliary state variables
      VariableDescriptionContainer auxiliaryStateVariables;
      //! \brief external state variables
      VariableDescriptionContainer externalStateVariables;
      //! \brief local variables
      VariableDescriptionContainer localVariables;
      //! \brief integration variables
      VariableDescriptionContainer integrationVariables;
      //!\brief underlying editor
      EditorWidget &editor;
      //!\brief document
      TextEditBase &d;

      template <void (mfront::BehaviourDescription::*f)(
          const mfront::BehaviourDescription::Hypothesis,
          const mfront::VariableDescription &,
          const mfront::BehaviourData::RegistrationStatus)>
      void addVariable(mfront::VariableDescriptionContainer &,
                       const mfront::VariableDescription &);

     private:
      Q_OBJECT
    };  // end of struct MFrontAddVariablesPage

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX */
