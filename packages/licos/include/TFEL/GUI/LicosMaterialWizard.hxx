/*!
 * \file  LicosMaterialWizard.hxx
 * \brief wizard which inserts a Material block for the materials named.
 * \author Thomas Helfer
 * \date   27/09/2012
 */

#ifndef LIB_TFEL_GUI_LICOSMATERIALWIZARD_HXX
#define LIB_TFEL_GUI_LICOSMATERIALWIZARD_HXX

#include <QtWidgets/QWizard>

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct LicosThermalBehaviourWizard;
  // foward declaration
  struct LicosMechanicalBehaviourWizard;

  /*!
   * a wizard to introduce a new material
   */
  struct LicosMaterialWizard : public QWizard {
    /*!
     * Constructor
     * \param[in] t : text edit the material is to be inserted in, and the
     * parent of the wizard in the Qt sense
     *
     * \note a single page asks the names of the materials, the names of the
     * material descriptions they inherit from, and offers the buttons which
     * would call the behaviour wizards
     */
    LicosMaterialWizard(TextEditBase &);

    /*!
     * \brief check the page the user is on before the wizard moves to the next
     * one
     * \return true when the "MaterialName" field can be split into at least one
     * material name
     *
     * \note that split keeps the empty parts, so the list is not empty even
     * when the user has entered nothing; the implementation of \c QWizard,
     * which checks the mandatory fields, is not called
     *
     * \note the user cannot leave the page with an empty name anyway: the `*`
     * which marks the "MaterialName" field as mandatory is what keeps the
     * "Next" button disabled
     */
    bool validateCurrentPage() override;

    /*!
     * \brief insert at the cursor of the text edit a `Material` block which
     * declares the materials the user has named, each of them between simple
     * quotes, and inherits from the material descriptions the user has named
     *
     * \note the fields of the wizard are read again here: the caller shall only
     * call this method once the wizard has been accepted
     *
     * \note the buttons of the page which would launch the behaviour wizards do
     * nothing, so no behaviour is ever written
     */
    void writeMaterial();

   protected:
    /*!
     * \brief the page of the wizard: it asks the names of the materials and
     * those of the material descriptions they inherit from
     */
    struct MaterialPage;

    //! the textedit modified by the wizard
    TextEditBase &textEdit;

    /*!
     * the main page
     *
     * \note the wizard owns it: \c addPage reparents the page to the wizard
     */
    MaterialPage *mpage;

    /*!
     * wizard for the thermal behaviour
     *
     * \note it is left null: no such wizard is ever created, the page only
     * offers the button which would open it
     */
    LicosThermalBehaviourWizard *tbw;

    /*!
     * wizard for the mechanical behaviour
     *
     * \note it is left null: that type is only forward declared and no object
     * of it is ever created
     */
    LicosMechanicalBehaviourWizard *mbw;

  };  // end of struct LicosMaterialWizard

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSMATERIALWIZARD_HXX */
