/*!
 * \file   ImportMFMBehaviour.hxx
 * \brief
 * \author Thomas Helfer
 * \date   19/02/2018
 */

#ifndef LIB_QEMACS_IMPORTMFMBEHAVIOUR_HXX
#define LIB_QEMACS_IMPORTMFMBEHAVIOUR_HXX

#include <memory>
#include <utility>
#include <QtCore/QString>
#include <QtWidgets/QDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeView>

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;

  struct ImportMFMBehaviour : QDialog {
    /*!
     * \brief structure describing the selected behaviour
     */
    struct BehaviourDescription {
      QString library;
      QString behaviour;
      QString mfront_interface;
      QString hypothesis;
    };  // end of BehaviourDescription
    /*!
     * \brief constructor
     * \param[in] w: qemacs widget
     * \param[in] p: parent
     */
    ImportMFMBehaviour(QEmacsWidget&, QWidget* const p);
    //! \return the selected behaviour.
    BehaviourDescription getSelectedBehaviour() const;
    //! desctructor
    ~ImportMFMBehaviour() override;

   protected:
    QEmacsWidget& qemacs;
    QTreeView* view;
    // interface
    QComboBox* isb;
    // hypothesis
    QComboBox* hsb;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMBehaviour

}  // end of namespace qemacs

#endif /* LIB_QEMACS_IMPORTMFMBEHAVIOUR_HXX */
