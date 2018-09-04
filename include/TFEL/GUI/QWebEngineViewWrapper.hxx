/*! 
 * \file  QWebEngineViewWrapper.hxx
 * \brief
 * \author Helfer Thomas
 * \date   07/12/2012
 */

#ifndef LIB_QWEBENGINEVIEWWRAPPER_HXX
#define LIB_QWEBENGINEVIEWWRAPPER_HXX 

#include<QtWebEngineWidgets/QWebEngineView>
#include"TFEL/GUI/Config.hxx"

namespace tfel{

  namespace gui{

  struct TFEL_GUI_VISIBILITY_EXPORT QWebEngineViewWrapper
      : public QWidget {
    QWebEngineViewWrapper(QWebEngineView *, QWidget *);

    void focusInEvent(QFocusEvent *) override;

   protected:
    QWebEngineView *const wrappedObject;

   private:
    Q_OBJECT
  }; // end of QWebEngineViewWrapper

} // end of namespace gui
}// end of namespace tfel

#endif /* LIB_QWEBENGINEVIEWWRAPPER_HXX */

