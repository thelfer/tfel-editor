/*!
 * \file   SelectMFMBehaviourPage.hxx
 * \brief wizard page selecting a behaviour from the MFM database.
 * \author Thomas Helfer
 * \date   11/03/2018
 */

#ifndef LIB_TFEL_GUI_SELECTMFMBEHAVIOURPAGE_HXX
#define LIB_TFEL_GUI_SELECTMFMBEHAVIOURPAGE_HXX

#include <QtWidgets/QWizardPage>
#include "TFEL/GUI/BehaviourDescription.hxx"
#include "TFEL/GUI/SelectMFMBehaviour.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief a wizard page used to select a behaviour from a shared
   * library.
   *
   * The page holds a single \c SelectMFMBehaviour widget, which lists the
   * behaviours of the `MFrontMaterials` database and of the gallery, and
   * forwards its signals to the wizard.
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT SelectMFMBehaviourPage
      : public QWizardPage {
    //! a simple alias
    using Options = SelectMFMBehaviour::Options;

    /*!
     * \brief constructor
     * \param[in] q: editor widget, given to the selection widget
     * \param[in] o: options used to build that widget, that is the name and
     * material filters it starts with and the interface it is restricted to
     *
     * \note the selection widget is a child of the page through the layout
     * which holds it
     *
     * \note the signals of the page are emitted when the corresponding signal
     * of the selection widget is: a double click emits
     * \c behaviourDescriptionChanged as well
     */
    SelectMFMBehaviourPage(EditorWidget &, const Options &);

    /*!
     * \return the identifier of the material properties page if the selected
     * behaviour declares material properties, the identifier of the summary
     * page otherwise, that is 1 or 2; the summary page is also returned when no
     * valid behaviour is selected
     *
     * \note the behaviour is instantiated to read its material properties
     */
    int nextId() const override;

    /*!
     * \return true if the selected behaviour can be instantiated, false
     * otherwise, which keeps the user on the page
     */
    bool validatePage() override;

    /*!
     * \return the description of the behaviour selected in the widget of the
     * page, which is empty when no single row of the list is selected
     *
     * \note the interface is not read from the library by this method, it is
     * the one of the list; \c ImportMFMBehaviourWizard::getSelectedBehaviour
     * queries it afterwards
     */
    BehaviourDescription getBehaviourDescription() const;

    //! destructor
    ~SelectMFMBehaviourPage() override;

   signals:

    /*!
     * \brief emitted when the selection of the widget of the page changes
     * \param[in] bd: description of the behaviour which results from the new
     * selection
     *
     * \note the material properties page of the wizard listens to it to rebuild
     * its list of material properties
     */
    void behaviourDescriptionChanged(const BehaviourDescription &);

    /*!
     * \brief emitted when the user double clicks a behaviour of the list
     * \param[in] bd: description of that behaviour
     *
     * \note \c ImportMFMBehaviourWizard uses it to refresh the material
     * properties and go to the next page at once
     */
    void doubleClicked(const BehaviourDescription &);

   private:
    //! widget in charge of selecting the behaviour
    SelectMFMBehaviour *sb;
    Q_OBJECT
  };  // end of struct SelectMFMBehaviourPage

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SELECTMFMBEHAVIOURPAGE_HXX */
