/*!
 * \file   ImportMFMBehaviour.hxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#ifndef LIB_TFEL_GUI_IMPORTMFMBEHAVIOUR_HXX
#define LIB_TFEL_GUI_IMPORTMFMBEHAVIOUR_HXX

#include <QtWidgets/QDialog>
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/SelectMFMBehaviour.hxx"

namespace tfel {

  namespace gui {

    // forward declaration
    struct EditorWidget;

    struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportMFMBehaviour : QDialog {
      //! a simple alias
      using Options = SelectMFMBehaviour::Options;
      /*!
       * \brief constructor
       * \param[in] q: editor widget
       * \param[in] o: options
       * \param[in] p: parent
       */
      ImportMFMBehaviour(EditorWidget& q,
                         const Options& o,
                         QWidget* const p = nullptr);
      //! \return the selected behaviour.
      BehaviourDescription getSelectedBehaviour() const;
      //! destructor
      ~ImportMFMBehaviour() override;

     protected:
      SelectMFMBehaviour* b;

     private:
      Q_OBJECT
    };  // end of struct ImportMFMBehaviour

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_IMPORTMFMBEHAVIOUR_HXX */
