/*! 
 * \file  QEmacsShortCutStyle.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 oct. 2012
 */

#ifndef LIB_QEMACS_QEMACSSHORTCUTSTYLE_HXX
#define LIB_QEMACS_QEMACSSHORTCUTSTYLE_HXX 

#include<QtCore/QObject>
#include"QEmacs/Config.hxx"

namespace qemacs
{

  struct QEMACS_VISIBILITY_EXPORT QEmacsShortCutStyle
    : public QObject
  {
  signals:

    void shortCutStyleChanged();

  public:

    enum ShortCutStyle{
      EMACS,
      QT
    };

    static QEmacsShortCutStyle&
    getQEmacsShortCutStyle();

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

    Q_OBJECT

  }; // end of struct QEmacsShortCutStyle

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSSHORTCUTSTYLE_HXX */

