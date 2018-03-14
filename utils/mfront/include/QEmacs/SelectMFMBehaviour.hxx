  /*!
 * \file   SelectMFMBehaviour.hxx
 * \brief
 * \author Thomas Helfer
 * \date   12/03/2018
 */

#ifndef LIB_QEMACS_SELECTMFMBEHAVIOUR_HXX
#define LIB_QEMACS_SELECTMFMBEHAVIOUR_HXX

#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeView>
#include "QEmacs/BehaviourDescription.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;

  struct QEMACS_MIW_VISIBILITY_EXPORT SelectMFMBehaviour : QWidget {
    /*!
     * \brief constructor
     * \param[in] q: qemacs widget
     * \param[in] p: parent
     */
    SelectMFMBehaviour(QEmacsWidget& q, QWidget* const p = nullptr);
    //! \return the selected behaviour.
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~SelectMFMBehaviour() override;

   signals:

    void behaviourDescriptionChanged(const BehaviourDescription &);
    
   protected:
    QTreeView* view;
    // interface
    QComboBox* isb;
    // hypothesis
    QComboBox* hsb;

    BehaviourDescription getSelectedBehaviour(const QModelIndex) const;

   private:
    Q_OBJECT
  };  // end of struct SelectMFMBehaviour

}  // end of namespace qemacs

#endif /* LIB_QEMACS_SELECTMFMBEHAVIOUR_HXX */
