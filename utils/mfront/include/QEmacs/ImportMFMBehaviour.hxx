/*!
 * \file   ImportMFMBehaviour.hxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#ifndef LIB_QEMACS_IMPORTMFMBEHAVIOUR_HXX
#define LIB_QEMACS_IMPORTMFMBEHAVIOUR_HXX

#include <QtWidgets/QDialog>
#include "QEmacs/BehaviourDescription.hxx"
#include "QEmacs/SelectMFMBehaviour.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;

  struct QEMACS_MIW_VISIBILITY_EXPORT ImportMFMBehaviour : QDialog {
    //! a simple alias
    using Options = SelectMFMBehaviour::Options;
    /*!
     * \brief constructor
     * \param[in] q: qemacs widget
     * \param[in] o: options
     * \param[in] p: parent
     */
    ImportMFMBehaviour(QEmacsWidget& q,
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_IMPORTMFMBEHAVIOUR_HXX */
