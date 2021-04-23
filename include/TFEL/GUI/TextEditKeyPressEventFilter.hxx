/*!
 * \file  TextEditKeyPressEventFilter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#ifndef LIB_TFEL_GUI_TEXTEDITKEYPRESSEVENTFILTER_HXX
#define LIB_TFEL_GUI_TEXTEDITKEYPRESSEVENTFILTER_HXX

#include <QtCore/QObject>
#include <QtGui/QKeyEvent>
#include "TFEL/GUI/Config.hxx"

namespace tfel {

  namespace gui {

    // forward declaration
    struct TextEditBase;

    /*!
     * An abstract class using by TextEditBase to filter key press
     * event.
     *
     * A filter is called on standard key (the only modifier accepted is
     * the Shift key)
     */
    struct TFEL_GUI_VISIBILITY_EXPORT TextEditKeyPressEventFilter
        : public QObject {
      TextEditKeyPressEventFilter(TextEditBase&);

      virtual bool filterKeyPressEvent(QKeyEvent* const) = 0;

      /*!
       * return true if this object can be set : if something goes wrong
       * in the constructor, this must be reported through this method
       */
      virtual bool isOk() const = 0;
      //! destructor
      ~TextEditKeyPressEventFilter() override;

     private:
      Q_OBJECT
    };  // end of TextEditKeyPressEventFilter

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_TEXTEDITKEYPRESSEVENTFILTER_HXX */
