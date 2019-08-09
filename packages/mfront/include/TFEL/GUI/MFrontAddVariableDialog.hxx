/*!
 * \file   packages/mfront/include/TFEL/GUI/MFrontAddVariableDialog.hxx
 * \brief
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTADDVARIABLEDIALOG_HXX
#define LIB_TFEL_GUI_MFRONTADDVARIABLEDIALOG_HXX

#include <QDialog>
#include "MFront/VariableDescription.hxx"

// forward declaration
class QComboBox;

namespace tfel {

  namespace gui {

    // forward declaration
    struct LineEdit;
    struct EditorWidget;

    //! \brief a dialog box used to add variables
    struct MFrontAddVariableDialog : QDialog {
      //! \brief supported variable type
      enum VariableType {
        MATERIALPROPERTY,
        INTERNALSTATEVARIABLE,
        AUXILIARYSTATEVARIABLE,
        INTEGRATIONVARIABLE,
        EXTERNALSTATEVARIABLE
      }; // end of enum VariableType
      /*!
       * \brief constructor
       * \param[in] t: type of the variable to be treated
       * \param[in] p: parent widget
       */
      MFrontAddVariableDialog(EditorWidget&,
                              const VariableType,
                              QWidget* const);
      //! \return the variable description
      mfront::VariableDescription getVariableDescription() const;
      //! \brief destructor
      ~MFrontAddVariableDialog() override;

     protected:
      void updateVariableTypesList();
      void updateGlossaryNamesList();
      //! \brief variable type
      const VariableType vtype;
      //! \brief name of the variable
      LineEdit* const ne;
      //! \brief category of the variable
      QComboBox* const cb;
      //! \brief type of the variable
      QComboBox* const tb;
      //! \brief list of glossary name
      QComboBox* const gb;
      //! \brief entry name of the variable
      LineEdit* const ee;

     private:
      Q_OBJECT

    }; // end of struct MFrontAddVariableDialog

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTADDVARIABLEDIALOG_HXX */
