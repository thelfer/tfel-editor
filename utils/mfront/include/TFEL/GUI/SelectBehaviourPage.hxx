/*!
 * \file   SelectBehaviourPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_TFEL_GUI_SELECTBEHAVIOURPAGE_HXX
#define LIB_TFEL_GUI_SELECTBEHAVIOURPAGE_HXX

#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizardPage>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"

namespace tfel{

  namespace gui{

    /*!
     * \brief a wizard page used to select a behaviour from a shared
     * library.
     */
    struct TFEL_GUI_MIW_VISIBILITY_EXPORT SelectBehaviourPage
        : public QWizardPage {
      SelectBehaviourPage(EditorWidget &);

      int nextId() const override;

      bool validatePage() override;
      //! \return the current behaviour description
      BehaviourDescription getBehaviourDescription() const;
      //! destructor
      ~SelectBehaviourPage();

     private slots:

      /*!
       * \brief update the list of behaviours when a library
       * is selected.
       */
      virtual void updateBehaviourList();
      /*!
       * \brief update the list of modelling hypotheses when a behaviour
       * is selected.
       */
      virtual void updateModellingHypotheses();

      virtual void selectLibrary();

     signals:

      void behaviourDescriptionChanged(const BehaviourDescription &);

     protected:
      //! list of behaviours declared in the selected library
      QComboBox *bl;
      //! list of modelling hypothesis for the selected behaviour
      QComboBox *mh;
      //! the library location
      LineEdit *le;
      //! select library button
      QPushButton *slb;

     private:
      Q_OBJECT
    };  // end of struct SelectBehaviourPage

  }  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_SELECTBEHAVIOURPAGE_HXX */
