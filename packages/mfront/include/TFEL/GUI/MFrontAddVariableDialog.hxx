/*!
 * \file   packages/mfront/include/TFEL/GUI/MFrontAddVariableDialog.hxx
 * \brief dialog which asks the name, type and array size of a new variable.
 * \author Thomas Helfer
 * \date   07/08/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTADDVARIABLEDIALOG_HXX
#define LIB_TFEL_GUI_MFRONTADDVARIABLEDIALOG_HXX

#include <QDialog>
#include "MFront/VariableDescription.hxx"

// forward declaration
class QComboBox;

namespace tfel::gui {

  // forward declaration
  struct LineEdit;
  struct EditorWidget;

  /*!
   * \brief a dialog box used to add variables
   *
   * The dialog asks for the name, the category (scalar, vector, symmetric or
   * unsymmetric tensor), the type, the array size and, for the kinds of
   * variables which can be handled by an interface, a glossary name or an
   * entry name.
   *
   * \note the dialog does not check the input itself: the variable is only
   * validated by MFront when the caller registers it, see
   * \c MFrontAddVariablesPage
   */
  struct MFrontAddVariableDialog : QDialog {
    //! \brief supported variable type
    enum VariableType {
      MATERIALPROPERTY,        //!< a material property
      PARAMETER,               //!< a parameter
      STATEVARIABLE,           //!< a state variable
      LOCALVARIABLE,           //!< a local variable
      EXTERNALSTATEVARIABLE,   //!< an external state variable
      AUXILIARYSTATEVARIABLE,  //!< an auxiliary state variable
      INTEGRATIONVARIABLE      //!< an integration variable
    };                         // end of enum VariableType
    /*!
     * \brief constructor
     * \param[in] w: editor widget, used by the line edits of the dialog
     * \param[in] t: type of the variable to be treated. It selects the title
     * of the dialog, the categories offered (a material property or an
     * external state variable can only be a scalar) and whether the glossary
     * and entry name fields are shown
     * \param[in] p: parent widget
     *
     * \note all the fields are created here and are reparented to the dialog
     * by the layouts, so the dialog owns them
     *
     * \note the array size is limited to values not less than one by an
     * integer validator and defaults to "1"
     *
     * \note the buttons of the dialog accept or reject it; the caller has to
     * query the fields once the dialog has been accepted
     */
    MFrontAddVariableDialog(EditorWidget&, const VariableType, QWidget* const);
    /*!
     * \return the description of the variable the user has typed
     *
     * The mangled name of TFEL is used for a variable whose name is not
     * ASCII. The glossary name is only set when it differs from "None" and the
     * entry name only when the field is not empty, both being ignored for a
     * local or an integration variable
     *
     * \note no validation is performed: an empty name or type yields an
     * invalid description, which MFront rejects when it is registered
     */
    mfront::VariableDescription getVariableDescription() const;
    //! \brief destructor
    ~MFrontAddVariableDialog() override;

   protected:
    /*!
     * \brief refill the list of variable types according to the category
     * currently selected
     *
     * The types are those \c mfront::SupportedTypes defines for the selected
     * category; the previously selected type is restored when it is still
     * available
     *
     * \note this is triggered by a change of the category of the variable
     */
    void updateVariableTypesList();
    /*!
     * \brief refill the list of glossary names from the TFEL glossary, with
     * "None" first
     *
     * \note the list is only shown while the entry name field is empty: a
     * variable can either use a name of the glossary or an entry name, and the
     * combo box is disabled in the latter case
     *
     * \note this is triggered by a change of the category, of the type or of
     * the entry name
     */
    void updateGlossaryNamesList();
    //! \brief variable type
    const VariableType vtype;
    //! \brief name of the variable
    LineEdit* const ne;
    //! \brief category of the variable
    QComboBox* const cb;
    //! \brief type of the variable
    QComboBox* const tb;
    //! \brief arraySize
    LineEdit* const asize;
    //! \brief list of glossary name
    QComboBox* const gb;
    //! \brief entry name of the variable
    LineEdit* const ee;

   private:
    Q_OBJECT

  };  // end of struct MFrontAddVariableDialog

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTADDVARIABLEDIALOG_HXX */
