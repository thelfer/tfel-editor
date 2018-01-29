/*! 
 * \file  QEmacsTextEditKeyPressEventFilter.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITKEYPRESSEVENTFILTER_HXX
#define LIB_QEMACS_QEMACSTEXTEDITKEYPRESSEVENTFILTER_HXX 

#include<QtCore/QObject>
#include<QtGui/QKeyEvent>
#include"QEmacs/Config.hxx"

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
  class QEMACS_VISIBILITY_EXPORT QEmacsTextEditKeyPressEventFilter
    : public QObject
  {

    Q_OBJECT

  public:

    QEmacsTextEditKeyPressEventFilter(QEmacsTextEditBase&);

    virtual bool filterKeyPressEvent(QKeyEvent * const) = 0;

    /*!
     * return true if this object can be set : if something goes wrong
     * in the constructor, this must be reported through this method
     */
    virtual bool isOk() const = 0;

    ~QEmacsTextEditKeyPressEventFilter() override;

  }; // end of QEmacsTextEditKeyPressEventFilter

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSTEXTEDITKEYPRESSEVENTFILTER_HXX */

