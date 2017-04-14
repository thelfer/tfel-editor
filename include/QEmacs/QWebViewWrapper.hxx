/*! 
 * \file  QWebViewWrapper.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#ifndef _LIB_QWEBVIEWWRAPPER_H_
#define _LIB_QWEBVIEWWRAPPER_H_ 

#ifdef QEMACS_QT4
#include<QtWebKit/QWebView>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWebKitWidgets/QWebView>
#endif /* QEMACS_QT5 */

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

#endif /* _LIB_QWEBVIEWWRAPPER_H */

