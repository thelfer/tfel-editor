/*!
 * \file  QAbstractScrollAreaWrapper.hxx
 * \brief widget showing a `QAbstractScrollArea` as a secondary task.
 * \author Thomas Helfer
 * \date   07/12/2012
 */

#ifndef LIB_QABSTRACTSCROLLAREAWRAPPER_HXX
#define LIB_QABSTRACTSCROLLAREAWRAPPER_HXX

#include <QtWidgets/QAbstractScrollArea>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief widget displaying a `QAbstractScrollArea` as a secondary
   * task
   *
   * The scroll area is placed in a layout of this widget, as done for
   * the main text editing areas, and the focus received by the wrapper
   * is forwarded to the wrapped widget.
   *
   * \note `Buffer::attachSecondaryTask` uses this wrapper when a
   * secondary task is a scroll area: the tab widget holds the wrapper
   * whereas callers may hold the wrapped widget, so
   * `Buffer::getSecondaryTaskIndex` and `SecondaryTaskManager` rely on
   * `getWrappedObject` to reconcile the two identities.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT QAbstractScrollAreaWrapper
      : public QWidget {
    /*!
     * \brief constructor
     * \param[in] s : scroll area wrapped by this widget
     * \param[in] p : parent widget
     *
     * \note `s` is reparented in the layout of this widget and is thus
     * destroyed with it
     */
    QAbstractScrollAreaWrapper(QAbstractScrollArea *, QWidget *);

    /*!
     * \return the wrapped scroll area, that is, the pointer given to
     * the constructor
     */
    QAbstractScrollArea *getWrappedObject();

    /*!
     * \brief transfer the focus to the wrapped scroll area
     *
     * \note triggered whenever this wrapper itself receives the focus;
     * the event is ignored, it is not propagated to `QWidget`
     */
    virtual void focusInEvent(QFocusEvent *) override;

   protected:
    //! scroll area wrapped by this widget, its value is fixed at
    //! construction
    QAbstractScrollArea *const wrappedObject;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_QABSTRACTSCROLLAREAWRAPPER_HXX */
