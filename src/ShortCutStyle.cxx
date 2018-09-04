/*!
 * \file  ShortCutStyle.cxx
 * \brief
 * \author Helfer Thomas
 * \date   01/10/2012
 */

#include <QtCore/QSettings>
#include "TFEL/GUI/ShortCutStyle.hxx"

namespace tfel{

  namespace gui{

  ShortCutStyle& ShortCutStyle::getShortCutStyle() {
    static ShortCutStyle s;
    return s;
  }

  ShortCutStyle::ShortCutStyle()
      : style(Style::EMACS) {
    QSettings s;
    if (s.contains("shortcut style")) {
      const auto v = s.value("shortcut style").toString();
      if (v == "EMACS") {
        this->setStyle(EMACS);
      } else if (v == "QT") {
        this->setStyle(QT);
      }
    }
  }

  void ShortCutStyle::setStyle(const Style& s) {
    QSettings settings;
    this->style = s;
    if (s == EMACS) {
      settings.setValue("shortcut style", "EMACS");
    } else if (s == QT) {
      settings.setValue("shortcut style", "QT");
    }
    emit shortCutStyleChanged();
  }

  ShortCutStyle::Style ShortCutStyle::getStyle() {
    return this->style;
  }  // end of ShortCutStyle::getStyle

}  // end of namespace gui
  
}// end of namespace tfel
