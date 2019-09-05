/*!
 * \file   packages/mfront/include/TFEL/GUI/MFrontAddInelasticFlowDialog.hxx
 * \brief
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTADDINELASTICFLOWDIALOG_HXX
#define LIB_TFEL_GUI_MFRONTADDINELASTICFLOWDIALOG_HXX

#include <QDialog>

// forward declaration
class QComboBox;

namespace tfel {

  namespace gui {

    //! \brief a dialog box used to add variables
    struct MFrontAddInelasticFlowDialog : QDialog {
      /*!
       * \brief constructor
       * \param[in] p: parent widget
       */
      MFrontAddInelasticFlowDialog(QWidget* const);
      //! \brief destructor
      ~MFrontAddInelasticFlowDialog() override;

     protected:
      //! list of inelastic flows
      QComboBox* const inelastic_flows;
      //! list of stress criteria
      QComboBox* const stress_criteria;

     private:
      Q_OBJECT

    }; // end of struct MFrontAddInelasticFlowDialog

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTADDINELASTICFLOWDIALOG_HXX */
