/*!
 * \file   MFrontStandardElastoViscoPlasticityBrickPage.hxx
 * \brief
 * \author Thomas Helfer
 * \date   27/07/2019
 */

#ifndef LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX
#define LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX

#include <vector>
#include <QtWidgets/QWizard>
#include "TFEL/GUI/MFrontAddInelasticFlowDialog.hxx"

// forward declaration
class QComboBox;

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  struct TextEditBase;
  struct LineEdit;
  struct MFrontBehaviourWizard;

  struct MFrontStandardElastoViscoPlasticityBrickPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] w: editor widget
     * \param[in] cd: current document
     * \param[in] p: parent
     */
    MFrontStandardElastoViscoPlasticityBrickPage(EditorWidget &,
                                                 TextEditBase &,
                                                 MFrontBehaviourWizard *const);

    bool validatePage() override;

    int nextId() const override;
    //! \brief write the output of the wizard
    virtual void write() const;
    //! \brief destructor
    ~MFrontStandardElastoViscoPlasticityBrickPage() override;

   protected:
    //! list of inelastic flows
    std::vector<MFrontAddInelasticFlowDialog::InelasticFlowDescription>
        inelastic_flows;
    //! \brief list of stress potentials
    QComboBox *const stress_potentials;
    //! \brief editor
    EditorWidget &editor;
    //!\brief document
    TextEditBase &d;

   private:
    Q_OBJECT
  };  // end of struct MFrontStandardElastoViscoPlasticityBrickPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTSTANDARDELASTOVISCOPLASTICITYBRICKPAGE_HXX \
        */
