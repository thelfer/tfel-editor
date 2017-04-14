/*! 
 * \file  QAbstractScrollAreaWrapper.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 déc. 2012
 */

#ifndef _LIB_QABSTRACTSCROLLAREAWRAPPER_H_
#define _LIB_QABSTRACTSCROLLAREAWRAPPER_H_ 

#ifdef QEMACS_QT4
#include<QtGui/QAbstractScrollArea>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QAbstractScrollArea>
#endif /* QEMACS_QT5 */


namespace qemacs
{

  class QAbstractScrollAreaWrapper
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

#endif /* _LIB_QABSTRACTSCROLLAREAWRAPPER_H */

