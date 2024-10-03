/*!
 * \file  QAbstractScrollAreaWrapper.hxx
 * \brief
 * \author Thomas Helfer
 * \date   07/12/2012
 */

#ifndef LIB_QABSTRACTSCROLLAREAWRAPPER_HXX
#define LIB_QABSTRACTSCROLLAREAWRAPPER_HXX

#include <QtWidgets/QAbstractScrollArea>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  struct TFEL_GUI_VISIBILITY_EXPORT QAbstractScrollAreaWrapper
      : public QWidget {
    QAbstractScrollAreaWrapper(QAbstractScrollArea *, QWidget *);

    QAbstractScrollArea *getWrappedObject();

    virtual void focusInEvent(QFocusEvent *) override;

   protected:
    QAbstractScrollArea *const wrappedObject;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_QABSTRACTSCROLLAREAWRAPPER_HXX */
