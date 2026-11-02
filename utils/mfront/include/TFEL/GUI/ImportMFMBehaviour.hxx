/*!
 * \file   ImportMFMBehaviour.hxx
 * \brief dialog selecting a behaviour of the MFM database.
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#ifndef LIB_TFEL_GUI_IMPORTMFMBEHAVIOUR_HXX
#define LIB_TFEL_GUI_IMPORTMFMBEHAVIOUR_HXX

#include <QtWidgets/QDialog>
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/SelectMFMBehaviour.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief a dialog box which lets the user select a behaviour of the
   * `MFrontMaterials` database or of the gallery, with the
   * \c SelectMFMBehaviour widget and the usual `Ok` and `Cancel` buttons
   *
   * \note unlike \c ImportMFMBehaviourWizard, this dialog does not ask for the
   * material properties of the selected behaviour
   *
   * \note no major mode uses this dialog: the `mtest` and `castem` modes show
   * \c ImportMFMBehaviourWizard instead
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportMFMBehaviour : QDialog {
    //! a simple alias
    using Options = SelectMFMBehaviour::Options;
    /*!
     * \brief constructor
     * \param[in] q: editor widget
     * \param[in] o: options
     * \param[in] p: parent
     *
     * \note the selection widget and the button box are built with no parent
     * and are given one by the layout of the dialog, which owns them; `Ok`
     * accepts the dialog, `Cancel` rejects it
     */
    ImportMFMBehaviour(EditorWidget& q,
                       const Options& o,
                       QWidget* const p = nullptr);
    /*!
     * \return the description of the behaviour selected in the widget of the
     * dialog, which is empty when no single row of the list is selected
     *
     * \note the dialog does not query the interface of the behaviour nor its
     * material properties, so only what the list shows is filled
     */
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportMFMBehaviour() override;

   protected:
    //! widget in charge of selecting the behaviour
    SelectMFMBehaviour* b;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMBehaviour

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_IMPORTMFMBEHAVIOUR_HXX */
