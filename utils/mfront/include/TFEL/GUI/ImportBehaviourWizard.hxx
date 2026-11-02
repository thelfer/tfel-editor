/*!
 * \file   ImportBehaviourWizard.hxx
 * \brief wizard importing a behaviour of a shared library.
 * \author THOMAS HELFER
 * \date   02/05/2017
 */

#ifndef LIB_TFEL_GUI_MTEST_IMPORTBEHAVIOURWIZARD_HXX
#define LIB_TFEL_GUI_MTEST_IMPORTBEHAVIOURWIZARD_HXX

#include <memory>
#include <vector>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizard>
#include "TFEL/GUI/LineEdit.hxx"
#include "TFEL/GUI/BehaviourDescription.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct SelectBehaviourPage;
  // forward declaration
  struct MaterialPropertiesSelectionPage;
  // forward declaration
  struct BehaviourSummaryPage;

  /*!
   * \brief the wizard used to import a behaviour from a shared library the user
   * points at, and insert its description in a buffer
   *
   * The wizard runs the three pages of the import: \c SelectBehaviourPage,
   * where the library, the behaviour and the modelling hypothesis are chosen,
   * \c MaterialPropertiesSelectionPage, where the material properties of that
   * behaviour are defined, and \c BehaviourSummaryPage, which ends the wizard
   *
   * \note the material properties page is refreshed each time
   * \c SelectBehaviourPage::behaviourDescriptionChanged is emitted; the summary
   * page is a placeholder which holds nothing
   *
   * \see \c mtest::MTestMajorMode and \c castem::CastemMajorMode, which show
   * the wizard and insert the resulting description in the current buffer
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT ImportBehaviourWizard : public QWizard {
    /*!
     * \brief constructor
     * \param[in] t: text edit the wizard is a dialog of, and which provides the
     * editor widget given to the line edits of the pages
     *
     * The three pages are created for the editor widget of \c t, added to the
     * wizard and hence owned by it, and shown in that order
     *
     * \note the material properties page is filled immediately, with the empty
     * description the first page starts with
     */
    ImportBehaviourWizard(TextEditBase &);
    /*!
     * \return the description of the selected behaviour: the `Library`,
     * `Behaviour` and `Hypothesis` fields of the wizard, the interface read
     * from \c tfel::system::ExternalLibraryManager for that library and
     * behaviour, and the material properties defined on the second page
     *
     * \note an empty description is returned when the interface can not be
     * analysed, which makes the selection unusable for the caller; the
     * material properties left undefined by the user are not part of the result
     *
     * \note this method is meant to be called once the wizard has been accepted
     */
    BehaviourDescription getSelectedBehaviour() const;
    //! destructor
    ~ImportBehaviourWizard() override;

    //! first page, in charge of selecting the behaviour
    SelectBehaviourPage *sb;
    //! second page, in charge of defining the material properties
    MaterialPropertiesSelectionPage *mp;
    //! last page, which ends the wizard and displays nothing
    BehaviourSummaryPage *c;

   private:
    Q_OBJECT
  };  // end of struct ImportBehaviourWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTEST_IMPORTBEHAVIOURWIZARD_HXX */
