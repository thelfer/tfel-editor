/*!
 * \file   SelectMFMBehaviour.hxx
 * \brief widget listing the material knowledges of the databases.
 * \author Thomas Helfer
 * \date   12/03/2018
 */

#ifndef LIB_TFEL_GUI_SELECTMFMBEHAVIOUR_HXX
#define LIB_TFEL_GUI_SELECTMFMBEHAVIOUR_HXX

#include <QtWidgets/QComboBox>
#include <QtWidgets/QTreeView>
#include "TFEL/GUI/BehaviourDescription.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief a widget aiming at selecting a behaviour of the `MFrontMaterials`
   * database
   *
   * The widget shows the contents of the galleries in a tree view backed by
   * \c MFMDataBase and \c MFMFilterProxyModel, restricted to the behaviours,
   * and completes it with a combo box listing the modelling hypotheses
   * supported by the selected behaviour.
   *
   * \note the tree view is read only and the selection is made row by row; the
   * library column is only shown through the "Show library path" check box, and
   * the type column is hidden with no way to show it again
   */
  struct TFEL_GUI_MIW_VISIBILITY_EXPORT SelectMFMBehaviour : QWidget {
    //! option of the dialog box
    struct TFEL_GUI_MIW_VISIBILITY_EXPORT Options {
      //! \brief constructor
      Options();
      //! \brief move constructor
      Options(Options&&);
      //! \brief copy constructor
      Options(const Options&);
      //! \brief move assignement
      Options& operator=(Options&&);
      //! \brief copy assignement
      Options& operator=(const Options&);
      /*!
       * \brief name of the material property. This is used to
       * initialize the name filter.
       *
       * \note the filter of this widget applies to the names of the
       * behaviours of the galleries
       */
      QString name;
      /*!
       * \brief name of the material. This is used to initialize the
       * material filter.
       */
      QString material;
      /*!
       * \brief if this member is not empty, the user will not be able
       * to select the interface of the material property. The material
       * properties displayed would use the given interface.
       *
       * \note the interface column of the tree view is hidden as well in
       * that case
       */
      QString minterface;
    };
    /*!
     * \brief constructor
     * \param[in] q: editor widget
     * \param[in] o: options
     * \param[in] p: parent
     *
     * The constructor analyses the galleries of `MFMHOME` and
     * `MFRONTGALLERYHOME` through \c MFMDataBase::load and shows their
     * behaviours in the tree view, so the widget only reflects the contents of
     * those directories at the time it is built
     *
     * \note the loaded model and the proxy model are given this widget as
     * parent and are released with it; the other widgets are reparented by the
     * layout
     *
     * \note `o.name` and `o.material` pre-fill the corresponding filters;
     * `o.minterface` restricts the list to that interface and hides the
     * interface selection
     */
    SelectMFMBehaviour(EditorWidget& q,
                       const Options& o,
                       QWidget* const p = nullptr);
    /*!
     * \return the description of the selected behaviour: its name, the path of
     * the library which holds it, its interface, as they are shown in the tree
     * view, and the modelling hypothesis selected in the combo box
     *
     * \note an empty description is returned when no row or several rows are
     * selected; the material properties are not filled by this method
     */
    BehaviourDescription getSelectedBehaviour() const;
    //! \brief destructor
    ~SelectMFMBehaviour() override;

   signals:

    /*!
     * \brief emitted when the user double clicks a row of the tree view
     * \param[in] bd: description of the behaviour of that row
     *
     * \note the wizards use this signal to move to the next page without the
     * user having to validate the page
     */
    void doubleClicked(const BehaviourDescription&);

    /*!
     * \brief emitted when the current row of the tree view changes
     * \param[in] bd: description of the behaviour of the new current row
     *
     * \note the signal is also emitted when the row is invalid, in which case
     * an empty description is emitted
     */
    void behaviourDescriptionChanged(const BehaviourDescription&);

   protected:
    //! tree view showing the behaviours of the galleries
    QTreeView* view;
    /*!
     * \brief combo box listing the interfaces of the behaviours, the first
     * entry being the regular expression `.+` which shows all of them
     *
     * \note it is only shown, and only connected to the interface filter of the
     * proxy model, when the interface is not imposed by \c Options::minterface:
     * in that case it is left out of the layout of the widget and the interface
     * column of the tree view is hidden as well
     */
    QComboBox* isb;
    //! combo box listing the modelling hypotheses of the selected behaviour
    QComboBox* hsb;

    /*!
     * \return the description built from the row \c i of the tree view: the
     * name, the library and the interface of that row and the modelling
     * hypothesis currently selected in \c hsb
     *
     * \note the row is read through the model of the view, which is the proxy
     * model, so \c i must be an index of that model; an index of an invalid row
     * yields empty fields
     */
    BehaviourDescription getSelectedBehaviour(const QModelIndex) const;

   private:
    Q_OBJECT
  };  // end of struct SelectMFMBehaviour

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SELECTMFMBEHAVIOUR_HXX */
