/*!
 * \file  ShortCutStyle.hxx
 * \brief
 * \author Helfer Thomas
 * \date   01/10/2012
 */

#ifndef LIB_TFEL_GUI_SHORTCUTSTYLE_HXX
#define LIB_TFEL_GUI_SHORTCUTSTYLE_HXX

#include <QtCore/QObject>
#include "TFEL/GUI/Config.hxx"

namespace tfel {

  namespace gui {

    struct TFEL_GUI_VISIBILITY_EXPORT ShortCutStyle : public QObject {
     signals:

      void shortCutStyleChanged();

     public:
      enum Style { EMACS, QT };

      static ShortCutStyle& getShortCutStyle();

      Style getStyle();

      void setStyle(const Style&);

     private:
      ShortCutStyle();

      ShortCutStyle(const ShortCutStyle&) = delete;

      ShortCutStyle& operator=(const ShortCutStyle&) = delete;

      Style style;

      Q_OBJECT

    };  // end of struct ShortCutStyle

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_SHORTCUTSTYLE_HXX */
