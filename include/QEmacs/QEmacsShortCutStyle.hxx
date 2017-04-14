/*! 
 * \file  QEmacsShortCutStyle.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 oct. 2012
 */

#ifndef _LIB_QEMACS_QEMACSSHORTCUTSTYLE_H_
#define _LIB_QEMACS_QEMACSSHORTCUTSTYLE_H_ 

#include<QtCore/QObject>
#include"QEmacs/Config.hxx"

namespace qemacs
{

  class QEMACS_VISIBILITY_EXPORT QEmacsShortCutStyle
    : public QObject
  {
    
    Q_OBJECT

  signals:

    void shortCutStyleChanged();

  public:

    enum ShortCutStyle{
      EMACS,
      QT
    };

    static QEmacsShortCutStyle&
    getQEmacsShortCutStyle(void);

    ShortCutStyle
    getStyle();

    void
    setStyle(const ShortCutStyle&);

  private:

    QEmacsShortCutStyle();

    QEmacsShortCutStyle(const QEmacsShortCutStyle&);

    QEmacsShortCutStyle&
    operator=(const QEmacsShortCutStyle&);

    ShortCutStyle style;

  }; // end of struct QEmacsShortCutStyle

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSSHORTCUTSTYLE_H */

