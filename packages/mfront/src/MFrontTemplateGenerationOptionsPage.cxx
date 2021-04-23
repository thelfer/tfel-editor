/*!
 * \file   MFrontTemplateGenerationOptionsPage.cxx
 * \brief
 * \author th202608
 * \date   07/09/2019
 */

#include "TFEL/GUI/MFrontBehaviourWizard.hxx"
#include "TFEL/GUI/MFrontTemplateGenerationOptionsPage.hxx"

namespace tfel {

  namespace gui {

    MFrontTemplateGenerationOptionsPage::MFrontTemplateGenerationOptionsPage(
        MFrontBehaviourWizard *const p)
        : QWizardPage(p) {
      this->setTitle(QObject::tr("Select generation options."));
      this->setSubTitle(
          QObject::tr("This page allows the selection various options "
                      "for the generation of the code"));
    }  // end of
       // MFrontTemplateGenerationOptionsPage::MFrontTemplateGenerationOptionsPage

    bool MFrontTemplateGenerationOptionsPage::validatePage() { return true; }

    void MFrontTemplateGenerationOptionsPage::initializePage() {}

    int MFrontTemplateGenerationOptionsPage::nextId() const { return -1; }

    mfront::MFrontTemplateGenerationOptions
    MFrontTemplateGenerationOptionsPage::getTemplateGenerationOptions() const {
      return {};
    }  // end of
       // MFrontTemplateGenerationOptionsPage::getTemplateGenerationOptions

    MFrontTemplateGenerationOptionsPage::
        ~MFrontTemplateGenerationOptionsPage() = default;

  }  // end of namespace gui

}  // end of namespace tfel
