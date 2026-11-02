/*!
 * \file   ImportMFMBehaviourWizard.hxx
 * \brief wizard importing a behaviour of the MFM database.
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_TFEL_GUI_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX
#define LIB_TFEL_GUI_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX

#include <memory>
#include <vector>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizard>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/SelectMFMBehaviourPage.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct MaterialPropertiesSelectionPage;
  // forward declaration
  struct BehaviourSummaryPage;

  /*!
   * \brief the wizard used to import a behaviour of the `MFrontMaterials`
   * database or of the gallery, and insert its description in a buffer
   *
   * The wizard runs the three pages of the import:
   * \c SelectMFMBehaviourPage, where the behaviour is picked in the list of the
   * behaviours of the galleries, \c MaterialPropertiesSelectionPage, where the
   * material properties of that behaviour are defined, and
   * \c BehaviourSummaryPage, which ends the wizard
   *
   * \note the material properties page is refreshed each time
   * \c SelectMFMBehaviourPage::behaviourDescriptionChanged is emitted; a double
   * click on a behaviour refreshes it and moves to the next page at once
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportMFMBehaviourWizard
      : public QWizard {
    //! a simple alias
    using Options = SelectMFMBehaviourPage::Options;

    /*!
     * \brief constructor
     * \param[in] t: text edit the wizard is a dialog of, and which provides the
     * editor widget given to the widgets of the pages
     * \param[in] o: options of the selection page, that is the name and
     * material filters it starts with and the interface the list is restricted
     * to; the default value leaves all the behaviours of all the interfaces
     * selectable
     *
     * The three pages are created for the editor widget of \c t, added to the
     * wizard and hence owned by it, and shown in that order
     *
     * \note the material properties page is filled at construction from the
     * description the first page starts with: the selection widget pre-selects
     * the first row of the list, so the selectors of that behaviour, whose
     * modelling hypothesis is still empty, may already be shown
     */
    ImportMFMBehaviourWizard(TextEditBase &, const Options & = Options());
    /*!
     * \return the description selected on the first page, with the interface
     * read from \c tfel::system::ExternalLibraryManager for that library and
     * behaviour, and the material properties defined on the second page
     *
     * \note an empty description is returned when the interface can not be
     * analysed, which makes the selection unusable for the caller; the material
     * properties left undefined by the user are not part of the result
     *
     * \note this method is meant to be called once the wizard has been accepted
     */
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportMFMBehaviourWizard() override;

   protected:
    //! first page, in charge of selecting the behaviour
    SelectMFMBehaviourPage *sb;
    //! second page, in charge of defining the material properties
    MaterialPropertiesSelectionPage *mp;
    //! last page, which ends the wizard and displays nothing
    BehaviourSummaryPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportMFMBehaviourWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTEST_IMPORTMFMBEHAVIOURWIZARD_HXX */
