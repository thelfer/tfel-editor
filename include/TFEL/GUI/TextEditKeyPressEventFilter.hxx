/*!
 * \file  TextEditKeyPressEventFilter.hxx
 * \brief abstract base class of the key press event filters.
 * \author Thomas Helfer
 * \date   02/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITKEYPRESSEVENTFILTER_HXX
#define LIB_TFEL_GUI_TEXTEDITKEYPRESSEVENTFILTER_HXX

#include <QtCore/QObject>
#include <QtGui/QKeyEvent>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;

  /*!
   * An abstract class using by TextEditBase to filter key press
   * event.
   *
   * A filter is called on every key press the text area receives,
   * whatever the modifiers: it is up to it to ignore those it does not
   * handle, as the incremental search filter does
   *
   * \note the filter is installed with
   * `TextEditBase::setKeyPressEventFilter` and removed with
   * `TextEditBase::removeKeyPressEventFilter`, which a mouse press in
   * the text area also triggers; the text area owns the filter as a Qt
   * child of it
   *
   * \note only the Emacs key bindings consult the filter:
   * `TextEditBase::handleKeyPressEventWithEmacsShortCuts` calls
   * `filterKeyPressEvent` before the key bindings do; the Qt short cut
   * style, `TextEditBase::handleKeyPressEventWithQtShortCuts`, never
   * calls it
   */
  struct TFEL_GUI_VISIBILITY_EXPORT TextEditKeyPressEventFilter
      : public QObject {
    /*!
     * \brief constructor
     * \param[in] t : text area whose key press events are filtered
     *
     * \note the filter is parented to that text area, which destroys it
     * with itself unless it is removed first
     */
    TextEditKeyPressEventFilter(TextEditBase&);

    /*!
     * \brief handle a key press of the text area before the Emacs key
     * bindings of `TextEditBase` do
     * \param[in] ev : key press event
     *
     * \return true if the filter consumed the event, in which case the
     * text area does not treat it at all; false hands it over to the key
     * bindings of `TextEditBase` and to the wrapped Qt widget, which then
     * treat it as usual
     *
     * \note the filters of this tree remove themselves from the text
     * area to end whatever interaction they drive, and return false for
     * the event which ends it when that event shall still reach the text
     * area
     */
    virtual bool filterKeyPressEvent(QKeyEvent* const) = 0;

    /*!
     * \return true if this object can be set : if something goes wrong
     * in the constructor, this must be reported through this method
     *
     * \note a filter reporting itself not ok is refused by
     * `TextEditBase::setKeyPressEventFilter`, which does not install it
     * and leaves its caller destroy it
     */
    virtual bool isOk() const = 0;
    //! destructor
    ~TextEditKeyPressEventFilter() override;

   private:
    Q_OBJECT
  };  // end of TextEditKeyPressEventFilter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_TEXTEDITKEYPRESSEVENTFILTER_HXX */
