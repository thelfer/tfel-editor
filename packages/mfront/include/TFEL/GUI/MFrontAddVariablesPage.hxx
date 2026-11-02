/*!
 * \file   MFrontAddVariablesPage.hxx
 * \brief wizard page collecting the variables of the behaviour being created.
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
}  // namespace mfront

namespace tfel::gui {

  // forward declarations
  struct EditorWidget;
  struct TextEditBase;
  struct MFrontBehaviourWizard;

  /*!
   * \brief a wizard page used to add material properties, parameters, state
   * variables and the other kinds of variables to the behaviour being created
   *
   * The page shows one button per kind of variable the DSL selected on the
   * behaviour page allows the user to define; each button opens a
   * \c MFrontAddVariableDialog and the variables which have been accepted are
   * kept by the page.
   *
   * \note the page registers no field with the wizard, so it is never
   * disabled and \c validatePage always succeeds
   */
  struct MFrontAddVariablesPage : public QWizardPage {
    //! a simple alias
    using VariableDescriptionContainer = mfront::VariableDescriptionContainer;

    /*!
     * \brief constructor
     * \param[in] w: editor widget
     * \param[in] cd: current document, which the variables are written in
     * \param[in] p: parent, that is the wizard the page belongs to
     */
    MFrontAddVariablesPage(EditorWidget &,
                           TextEditBase &,
                           MFrontBehaviourWizard *const);

    //! \return always true: the page accepts to be left in any case
    bool validatePage() override;

    /*!
     * \brief build the buttons of the page when it is shown
     *
     * Only the kinds of variables the DSL description of the wizard allows the
     * user to define get a button. When the wizard can not be reached or its
     * DSL can not be created, the page shows a message instead of the buttons
     * and a warning box is raised in the first case.
     *
     * \note the layout is created here, and \c QWizardPage only honours the
     * first one it is given, so the buttons of a page visited a second time,
     * when the user goes back and forth, are those of the first visit
     */
    void initializePage() override;

    //! \return the identifier of the template generation options page
    int nextId() const override;
    /*!
     * \brief complete the behaviour description
     * \param[in] dsl: behaviour DSL to which the variables collected by the
     * page are added, kind by kind; a kind with no variable is skipped
     */
    virtual void complete(mfront::AbstractBehaviourDSL &) const;
    /*!
     * \brief write the output of the wizard
     *
     * One declaration per variable is inserted at the cursor of the document,
     * with the array size when it is greater than one, followed by the calls
     * to \c setGlossaryName and \c setEntryName when the variable has such a
     * name.
     *
     * \note the editor's cursor is left at the end of the inserted text
     */
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

    /*!
     * \brief destructor
     *
     * The buttons and the dialogs opened from them are Qt children of the page
     * or of the wizard and are released by Qt
     */
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

    /*!
     * \brief add a variable to the given container, after checking that MFront
     * accepts it together with the variables already selected
     * \tparam f: the \c mfront::BehaviourDescription method which registers
     * the kind of variable being added
     * \param[in] variables : variables of the same kind already selected; they
     * are registered again, in an empty behaviour description, so that the
     * whole list is checked at once
     * \param[in] v : variable to add
     *
     * The check is made on the behaviour description of the DSL of the wizard,
     * for the undefined modelling hypothesis.
     *
     * \note when MFront rejects the variable, the reason is shown in a message
     * box, but the variable is inserted in the container all the same: the
     * insertion follows the \c try/\c catch block and is not skipped by the
     * exception
     *
     * \note nothing is added, and a message box is raised, when the wizard or
     * its DSL can not be reached
     */
    template <void (mfront::BehaviourDescription::*f)(
        const mfront::BehaviourDescription::Hypothesis,
        const mfront::VariableDescription &,
        const mfront::BehaviourData::RegistrationStatus)>
    void addVariable(mfront::VariableDescriptionContainer &,
                     const mfront::VariableDescription &);

   private:
    Q_OBJECT
  };  // end of struct MFrontAddVariablesPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTADDVARIABLES_HXX */
