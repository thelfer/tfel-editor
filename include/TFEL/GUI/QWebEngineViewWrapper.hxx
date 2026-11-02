/*!
 * \file  QWebEngineViewWrapper.hxx
 * \brief widget showing a `QWebEngineView` as a secondary task.
 * \author Thomas Helfer
 * \date   07/12/2012
 */

#ifndef LIB_QWEBENGINEVIEWWRAPPER_HXX
#define LIB_QWEBENGINEVIEWWRAPPER_HXX

#include <QtWebEngineWidgets/QWebEngineView>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief a widget which shows a `QWebEngineView` as a secondary task of a
   * buffer
   *
   * A `QWebEngineView` is neither a `QPlainTextEdit` nor a
   * `QAbstractScrollArea`, so `Buffer` cannot attach it to a secondary task nor
   * give it the focus: this wrapper holds the view in its layout and forwards
   * the focus to it, as `QAbstractScrollAreaWrapper` does for scroll areas.
   *
   * \note this class is only compiled when Qt's `WebEngine` and
   * `WebEngineWidgets` components are found, which never happens in the default
   * configuration of the project (see `src/CMakeLists.txt`); no other class of
   * the editor refers to it.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT QWebEngineViewWrapper : public QWidget {
    /*!
     * \param[in] s : view displayed by the wrapper, reparented in its layout
     * \param[in] p : parent widget
     */
    QWebEngineViewWrapper(QWebEngineView *, QWidget *);

    /*!
     * \brief give the focus to the wrapped view
     * \param[in] e : event, ignored
     *
     * \note the event is not propagated to `QWidget`: the wrapper itself never
     * keeps the focus
     */
    void focusInEvent(QFocusEvent *) override;

   protected:
    //! web page displayed by the wrapper, never null, owned by the wrapper
    QWebEngineView *const wrappedObject;

   private:
    Q_OBJECT
  };  // end of QWebEngineViewWrapper

}  // end of namespace tfel::gui

#endif /* LIB_QWEBENGINEVIEWWRAPPER_HXX */
