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
#include "QEmacs/BehaviourDescription.hxx"

namespace qemacs {


  struct ImportMFMBehaviour : QDialog {
    /*!
     * \brief constructor
     * \param[in] p: parent
     */
    ImportMFMBehaviour(QWidget* const p = nullptr);
    //! \return the selected behaviour.
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportMFMBehaviour() override;

   protected:
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
