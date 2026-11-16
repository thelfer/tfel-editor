/*!
 * \file   MFrontStandardElastoViscoPlasticityBrickPage.hxx
 * \brief wizard page for the stress potential and inelastic flows of a brick.
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX
#define LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX

#include <vector>
#include <QtWidgets/QWizard>
#include "TFEL/GUI/MFrontAddInelasticFlowDialog.hxx"

// forward declaration
class QComboBox;
class QPushButton;

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  struct TextEditBase;
  struct LineEdit;
  struct MFrontBehaviourWizard;

  /*!
   * \brief a wizard page used to specify the options of the
   * \c StandardElastoViscoPlasticity brick: a stress potential and one or more
   * inelastic flows
   *
   * The page is only reached when that brick has been selected on the behaviour
   * page.
   */
  struct MFrontStandardElastoViscoPlasticityBrickPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] w: editor widget, used to report an internal error
     * \param[in] cd: current document, which the options are written in
     * \param[in] p: parent, that is the wizard the page belongs to
     *
     * \note the combo box lists the stress potentials registered in MFront and
     * preselects \c Hooke
     *
     * \note the "Add an inelastic flow" button opens a
     * \c MFrontAddInelasticFlowDialog, whose result is stored by the page when
     * the user accepts it. The flows which have been added are listed by the
     * combo box which stands below that button: one of them can be selected and
     * modified, which reopens the dialog prefilled with what the page holds, or
     * removed
     */
    MFrontStandardElastoViscoPlasticityBrickPage(EditorWidget &,
                                                 TextEditBase &,
                                                 MFrontBehaviourWizard *const);

    //! \return always true: the page accepts to be left in any case
    bool validatePage() override;

    //! \return the identifier of the page which adds variables
    int nextId() const override;
    /*!
     * \brief ask a new inelastic flow and add it to the list of the page
     *
     * The flow is asked through a \c MFrontAddInelasticFlowDialog and, when the
     * user accepts it, the answer is stored by the page and the entry which
     * names it is selected.
     *
     * \note an internal error is reported when the page is not attached to a
     * wizard, which is what the dialog needs to build the behaviour description
     * the lists of its options are read from
     */
    void addInelasticFlow();
    /*!
     * \brief modify the inelastic flow which the list of the page selects
     *
     * The \c MFrontAddInelasticFlowDialog is reopened prefilled with the
     * description the page holds and what the user answers replaces it, the
     * selection of the list being kept.
     *
     * \note nothing happens when the list selects nothing, which the button of
     * the page makes impossible: an empty list has nothing to modify
     */
    void modifyInelasticFlow();
    /*!
     * \brief remove the inelastic flow which the list of the page selects
     *
     * The entry which takes the place of the removed one is selected, the last
     * entry of the list when the removed one was the last one.
     *
     * \note nothing happens when the list selects nothing, as for a modify
     */
    void removeInelasticFlow();
    /*!
     * \brief write the output of the wizard, that is the body of the
     * \c @Brick declaration of the brick: the stress potential followed by the
     * inelastic flows, each with the options of its criterion, plastic
     * potential, hardening rules and flow itself, as a comment the user has to
     * complete
     *
     * The options are those the corresponding bricks of MFront declare; the
     * options which hold the criterion, the plastic potential and the hardening
     * rules are skipped as they are written explicitely.
     *
     * \note the text is inserted at the cursor of the document, which is left
     * at the end of it
     *
     * \note nothing is written beyond the opening brace when the behaviour
     * description of the wizard can not be built, and an internal error is
     * reported when the page is not attached to a wizard
     */
    virtual void write() const;
    /*!
     * \brief destructor
     *
     * \note the dialogs opened by the page are created on the stack by the
     * slots which show them
     */
    ~MFrontStandardElastoViscoPlasticityBrickPage() override;

   protected:
    /*!
     * \brief rebuild the combo box which lists the inelastic flows
     * \param[in] i: the entry to select, a negative value selecting none
     *
     * The two buttons which modify and remove a flow are enabled by the same
     * move: they act on the selected flow and have nothing to act on when the
     * list is empty.
     */
    void updateInelasticFlowsList(const int);
    //! list of inelastic flows
    std::vector<MFrontAddInelasticFlowDialog::InelasticFlowDescription>
        inelastic_flows;
    //! \brief list of stress potentials
    QComboBox *const stress_potentials;
    /*!
     * \brief list of the inelastic flows which have been added, which is also
     * the selection the two buttons below it act on
     */
    QComboBox *const inelastic_flows_list;
    //! \brief button which reopens the dialog with the selected inelastic flow
    QPushButton *const modify_inelastic_flow;
    //! \brief button which takes the selected inelastic flow out of the list
    QPushButton *const remove_inelastic_flow;
    //! \brief editor
    EditorWidget &editor;
    //!\brief document
    TextEditBase &d;

   private:
    Q_OBJECT
  };  // end of struct MFrontStandardElastoViscoPlasticityBrickPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX \
        */
