/*!
 * \file   SelectBehaviourPage.hxx
 * \brief wizard page selecting a behaviour of a shared library.
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

namespace tfel::gui {

  /*!
   * \brief a wizard page used to select a behaviour from a shared
   * library.
   *
   * The page shows a \c LineEdit for the path of the library, an "Open" button
   * which fills it through a file dialog, and two combo boxes which list the
   * behaviours of that library and their modelling hypotheses, as they are
   * declared in the library contents read by
   * \c tfel::system::ExternalLibraryManager.
   *
   * \note the page registers the `Library`, `Behaviour` and `Hypothesis`
   * fields of the wizard as required ones, so the wizard only accepts the page
   * when the three of them are filled
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT SelectBehaviourPage
      : public QWizardPage {
    /*!
     * \brief constructor
     * \param[in] q: editor widget, given to the \c LineEdit of the page, which
     * handles the emacs style short cuts
     *
     * \note the widgets of the page are created with the page as parent and are
     * released with it
     *
     * \note the widgets are connected so that changing the library refreshes
     * the list of behaviours, changing the behaviour refreshes the list of
     * modelling hypotheses, and each of those changes emits
     * \c behaviourDescriptionChanged
     */
    SelectBehaviourPage(EditorWidget &);

    /*!
     * \return the identifier of the material properties page if the selected
     * behaviour declares material properties, the identifier of the summary
     * page otherwise, that is 1 or 2; the summary page is also returned when no
     * behaviour can be selected yet
     *
     * \note the behaviour is instantiated to read its material properties
     */
    int nextId() const override;

    /*!
     * \return true if the current selection describes a behaviour which can be
     * instantiated, false otherwise, which keeps the user on the page
     *
     * \note an invalid selection yields an empty description, which can not be
     * instantiated
     */
    bool validatePage() override;
    /*!
     * \return the description of the behaviour defined by the library, the
     * behaviour and the modelling hypothesis fields, with the interface read
     * from \c tfel::system::ExternalLibraryManager
     *
     * \note an empty description is returned when the library or the behaviour
     * can not be analysed; the material properties are not filled by this
     * method
     */
    BehaviourDescription getBehaviourDescription() const;
    //! destructor
    ~SelectBehaviourPage();

   private slots:

    /*!
     * \brief update the list of behaviours when a library
     * is selected.
     *
     * The slot is connected to \c LineEdit::textChanged of the library field:
     * the entry points of the library whose kind of material knowledge is a
     * behaviour are listed and the first one is selected.
     *
     * \note an empty library field or a library which can not be analysed
     * leaves the previous list untouched; the failure is reported with \c debug
     */
    virtual void updateBehaviourList();
    /*!
     * \brief update the list of modelling hypotheses when a behaviour
     * is selected.
     *
     * The slot is connected to \c QComboBox::currentTextChanged of the
     * behaviour box: the hypotheses supported by the behaviour of the selected
     * library are listed and the first one is selected.
     *
     * \note the list is cleared first and stays empty when the library or the
     * behaviour is not set, or can not be analysed
     */
    virtual void updateModellingHypotheses();

    /*!
     * \brief ask the user for a shared library and show its path in the library
     * field; this slot is connected to the "Open" button
     *
     * The file dialog starts in the current directory and only proposes the
     * shared libraries of the current platform, that is the files with the
     * default library prefix and suffix given by
     * \c mfront::LibraryDescription
     *
     * \note a cancelled dialog, whose path is empty, leaves the field untouched
     */
    virtual void selectLibrary();

   signals:

    /*!
     * \brief emitted when the library, the behaviour or the modelling
     * hypothesis changes
     * \param[in] bd: description of the behaviour which results from the
     * new selection
     *
     * \note an empty description is emitted while the selection is still
     * incomplete; \c ImportBehaviourWizard listens to this signal to rebuild
     * the list of material properties shown on its second page
     */
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

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SELECTBEHAVIOURPAGE_HXX */
