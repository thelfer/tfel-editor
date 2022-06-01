/*!
 * \file
 * packages/mfront/include/TFEL/GUI/MFrontAddInelasticFlowDialog.hxx
 * \brief
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTADDINELASTICFLOWDIALOG_HXX
#define LIB_TFEL_GUI_MFRONTADDINELASTICFLOWDIALOG_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QDialog>

// forward declaration
class QComboBox;
class QListView;

namespace tfel::gui {

  // forward declaration
  struct MFrontBehaviourWizard;

  //! \brief a dialog box used to add variables
  struct MFrontAddInelasticFlowDialog : QDialog {
    /*!
     * \brief a structure containing all the options selected by the
     * user.
     */
    struct InelasticFlowDescription {
      //! \brief name of the inelastic flow
      QString inelastic_flow;
      //! \brief selected stress criterion
      QString stress_criterion;
      //! \brief selected plastic potential (empty if the flow is associated)
      QString plastic_potential;
      //! \brief selected isotropic hardening rules
      QStringList isotropic_hardening_rules;
      //! \brief selected kinematic hardening rules
      QStringList kinematic_hardening_rules;
    };  // end of struct InelasticFlowDescription
    /*!
     * \brief constructor
     * \param[in] p: parent widget
     */
    MFrontAddInelasticFlowDialog(MFrontBehaviourWizard* const);
    //! \return the options selected by the user
    InelasticFlowDescription getInelasticFlowDescription() const;
    //! \brief destructor
    ~MFrontAddInelasticFlowDialog() override;

   protected:
    //! list of inelastic flows
    QComboBox* const inelastic_flows;
    //! list of stress criteria
    QComboBox* const stress_criteria;
    //! list of plastic potentials
    QComboBox* const plastic_potentials;
    //! list of selected isotropic hardening rules
    QListView* const isotropic_hardening_rules_selected;
    //! list of available isotropic hardening rules
    QComboBox* const isotropic_hardening_rules;
    //! list of selected kinematic hardening rules
    QListView* const kinematic_hardening_rules_selected;
    //! list of available kinematic hardening rules
    QComboBox* const kinematic_hardening_rules;

   private:
    Q_OBJECT

  };  // end of struct MFrontAddInelasticFlowDialog

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTADDINELASTICFLOWDIALOG_HXX */
