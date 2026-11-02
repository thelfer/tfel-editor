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
     * the user accepts it; the flows are not displayed once added
     */
    MFrontStandardElastoViscoPlasticityBrickPage(EditorWidget &,
                                                 TextEditBase &,
                                                 MFrontBehaviourWizard *const);

    //! \return always true: the page accepts to be left in any case
    bool validatePage() override;

    //! \return the identifier of the page which adds variables
    int nextId() const override;
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
    //! list of inelastic flows
    std::vector<MFrontAddInelasticFlowDialog::InelasticFlowDescription>
        inelastic_flows;
    //! \brief list of stress potentials
    QComboBox *const stress_potentials;
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
