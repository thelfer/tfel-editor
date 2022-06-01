/*!
 * \file   MFrontTemplateGenerationOptionsPage.hxx
 * \brief
 * \author th202608
 * \date   07/09/2019
 */

#ifndef LIB_MFRONTTEMPLATEGENERATIONOPTIONSPAGE_HXX
#define LIB_MFRONTTEMPLATEGENERATIONOPTIONSPAGE_HXX

#include <QtWidgets/QWizard>
#include "MFront/MFrontTemplateGenerationOptions.hxx"

namespace tfel::gui {

  //! forward declaration
  struct MFrontBehaviourWizard;

  /*!
   * \brief page allowing the user to select options used for the
   * generation of the template.
   */
  struct MFrontTemplateGenerationOptionsPage : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] p: parent
     */
    MFrontTemplateGenerationOptionsPage(MFrontBehaviourWizard *const);
    bool validatePage() override;
    void initializePage() override;
    int nextId() const override;
    //! \return the options selected by the user
    mfront::MFrontTemplateGenerationOptions getTemplateGenerationOptions()
        const;
    //! \brief destructor
    ~MFrontTemplateGenerationOptionsPage() override;

   private:
    Q_OBJECT
  };  // end of struct MFrontAddVariablesPage

}  // end of namespace tfel::gui

#endif /* LIB_MFRONTTEMPLATEGENERATIONOPTIONSPAGE_HXX */
