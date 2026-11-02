/*!
 * \file   MFrontTemplateGenerationOptionsPage.hxx
 * \brief wizard page selecting the options of the code template generation.
 * \author Thomas Helfer
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
     * \param[in] p: parent, that is the wizard the page belongs to
     *
     * \note the page shows a title and a subtitle only: no widget is created
     * and no layout is set
     */
    MFrontTemplateGenerationOptionsPage(MFrontBehaviourWizard *const);
    //! \return always true: the page has nothing to check
    bool validatePage() override;
    /*!
     * \brief does nothing: the page has no widget to initialise, so there is
     * no state to reset when the user comes back to it
     */
    void initializePage() override;
    /*!
     * \return -1, the identifier Qt uses for no page: this page is the last one
     * of the wizard and accepting it closes the dialog
     */
    int nextId() const override;
    /*!
     * \return the options selected by the user
     *
     * \note the page has no widget yet, so default options are always returned
     */
    mfront::MFrontTemplateGenerationOptions getTemplateGenerationOptions()
        const;
    /*!
     * \brief destructor
     *
     * \note the page owns no widget of its own
     */
    ~MFrontTemplateGenerationOptionsPage() override;

   private:
    Q_OBJECT
  };  // end of struct MFrontAddVariablesPage

}  // end of namespace tfel::gui

#endif /* LIB_MFRONTTEMPLATEGENERATIONOPTIONSPAGE_HXX */
