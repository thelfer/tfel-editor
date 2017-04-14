/*! 
 * \file  QEmacsTextEditKeyPressEventFilter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSTEXTEDITKEYPRESSEVENTFILTER_H_
#define _LIB_QEMACS_QEMACSTEXTEDITKEYPRESSEVENTFILTER_H_ 

#include<QtCore/QObject>
#include<QtGui/QKeyEvent>

namespace qemacs
{

  class QEmacsTextEditBase;

  /*!
   * An abstract class using by QEmacsTextEditBase to filter key press
   * event.
   *
   * A filter is called on standard key (the only modifier accepted is
   * the Shift key)
   */
  class QEmacsTextEditKeyPressEventFilter
    : public QObject
  {

    Q_OBJECT

  public:

    QEmacsTextEditKeyPressEventFilter(QEmacsTextEditBase&);

    virtual bool
    filterKeyPressEvent(QKeyEvent * const) = 0;

    /*!
     * return true if this object can be set : if something goes wrong
     * in the constructor, this must be reported through this method
     */
    virtual bool
    isOk() const = 0;

    virtual ~QEmacsTextEditKeyPressEventFilter();

  }; // end of QEmacsTextEditKeyPressEventFilter

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSTEXTEDITKEYPRESSEVENTFILTER_H */

