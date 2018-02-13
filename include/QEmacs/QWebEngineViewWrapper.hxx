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

namespace qemacs
{

  class QEMACS_VISIBILITY_EXPORT QWebEngineViewWrapper
    : public QWidget
  {
    
    Q_OBJECT

  public:

    QWebEngineViewWrapper(QWebEngineView *,
			  QWidget *);

    void focusInEvent(QFocusEvent *) override;

  protected:

    QWebEngineView * const wrappedObject;

  };

} // end of namespace qemacs

#endif /* LIB_QWEBENGINEVIEWWRAPPER_HXX */

