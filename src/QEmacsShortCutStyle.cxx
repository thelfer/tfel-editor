/*! 
 * \file  QEmacsShortCutStyle.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 oct. 2012
 */

#include<QtCore/QSettings>

#include"QEmacs/QEmacsShortCutStyle.hxx"

namespace qemacs
{
  
  QEmacsShortCutStyle&
  QEmacsShortCutStyle::getQEmacsShortCutStyle()
  {
    static QEmacsShortCutStyle s;
    return s;
  }

  QEmacsShortCutStyle::QEmacsShortCutStyle()
    : style(QEmacsShortCutStyle::EMACS)
  {
    QSettings s;
    if(s.contains("shortcut style")){
      QString v = s.value("shortcut style").toString();
      if(v=="EMACS"){
	this->setStyle(EMACS);
      }
      if(v=="QT"){
	this->setStyle(QT);
      }
    }
  }

  void
  QEmacsShortCutStyle::setStyle(const QEmacsShortCutStyle::ShortCutStyle& s)
  {
    QSettings settings;
    this->style = s;
    if(s==EMACS){
      settings.setValue("shortcut style","EMACS");
    } else if(s==QT){
      settings.setValue("shortcut style","QT");
    }
    emit shortCutStyleChanged();
  }

  QEmacsShortCutStyle::ShortCutStyle
  QEmacsShortCutStyle::getStyle()
  {
    return this->style;
  } // end of QEmacsShortCutStyle::getStyle

} // end of namespace qemacs
