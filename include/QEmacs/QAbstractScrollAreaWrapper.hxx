/*! 
 * \file  QAbstractScrollAreaWrapper.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#ifndef LIB_QABSTRACTSCROLLAREAWRAPPER_HXX
#define LIB_QABSTRACTSCROLLAREAWRAPPER_HXX 

#include<QtWidgets/QAbstractScrollArea>
#include"QEmacs/Config.hxx"

namespace qemacs
{

  class QEMACS_VISIBILITY_EXPORT QAbstractScrollAreaWrapper
    : public QWidget
  {
    Q_OBJECT

  public:

    QAbstractScrollAreaWrapper(QAbstractScrollArea *,
			       QWidget *);

    QAbstractScrollArea * 
    getWrappedObject();

    virtual void
    focusInEvent(QFocusEvent *) override;

  protected:

    QAbstractScrollArea * const wrappedObject;

  };

} // end of namespace qemacs

#endif /* LIB_QABSTRACTSCROLLAREAWRAPPER_HXX */

