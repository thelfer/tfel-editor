/*!
 * \file   MFrontBehaviourWizard.hxx
 * \brief
 * \author Thomas Helfer
 * \date   26/02/2018
 */

#ifndef LIB_TFEL_GUI_MFRONTBEHAVIOURWIZARD_HXX
#define LIB_TFEL_GUI_MFRONTBEHAVIOURWIZARD_HXX

#include <QtWidgets/QWizard>

namespace tfel {

  namespace gui {

    // forward declarations
    struct EditorWidget;
    struct TextEditBase;
    struct MFrontMetaDataPage;
    struct MFrontBehaviourPage;
    struct MFrontAddVariablesPage;

    struct MFrontBehaviourWizard : public QWizard {
      /*!
       * \brief constructor
       * \param[in] w: editor
       * \param[in] cd: current document
       * \param[in] p: parent
       */
      MFrontBehaviourWizard(EditorWidget &,
                            TextEditBase &,
                            QWidget *const p = nullptr);
      //! \brief write the output of the wizard
      virtual void write() const;
      //! destructor
      ~MFrontBehaviourWizard() override;

     protected:
      //! \brief meta data
      MFrontMetaDataPage *md = nullptr;
      //! \brief integration method
      MFrontBehaviourPage *im = nullptr;
      //! \brief integration method
      MFrontAddVariablesPage *variables = nullptr;
      //! \brief document
      TextEditBase &d;

     private:
      Q_OBJECT
    };  // end of struct MFrontBehaviourWizard

  }  // end of namespace gui

}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTBEHAVIOURWIZARD_HXX */
