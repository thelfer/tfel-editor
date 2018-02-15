/*! 
 * \file  QWebEngineViewWrapper.hxx
 * \brief
 * \author Helfer Thomas
 * \date   07/12/2012
 */

#ifndef LIB_QWEBENGINEVIEWWRAPPER_HXX
#define LIB_QWEBENGINEVIEWWRAPPER_HXX 

#include<QtWebEngineWidgets/QWebEngineView>
#include"QEmacs/Config.hxx"

namespace qemacs {

  struct QEMACS_VISIBILITY_EXPORT QWebEngineViewWrapper
      : public QWidget {
    QWebEngineViewWrapper(QWebEngineView *, QWidget *);

    void focusInEvent(QFocusEvent *) override;

   protected:
    QWebEngineView *const wrappedObject;

   private:
    Q_OBJECT
  }; // end of QWebEngineViewWrapper

} // end of namespace qemacs

#endif /* LIB_QWEBENGINEVIEWWRAPPER_HXX */

