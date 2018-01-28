/*! 
 * \file  QWebViewWrapper.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#ifndef LIB_QWEBVIEWWRAPPER_HXX
#define LIB_QWEBVIEWWRAPPER_HXX 

#include<QtWebKitWidgets/QWebView>

namespace qemacs
{

  class QWebViewWrapper
    : public QWidget
  {
    
    Q_OBJECT

  public:

    QWebViewWrapper(QWebView *,
		    QWidget *);

    virtual void
    focusInEvent(QFocusEvent *) override;

  protected:

    QWebView * const wrappedObject;

  };

} // end of namespace qemacs

#endif /* LIB_QWEBVIEWWRAPPER_HXX */

