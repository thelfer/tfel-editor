/*! 
 * \file  QEmacsRectangleMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 sept. 2012
 */

#ifndef _LIB_QEMACS_QEMACSRECTANGLEMODE_H_
#define _LIB_QEMACS_QEMACSRECTANGLEMODE_H_ 

#include<QEmacs/QEmacsLineEdit.hxx>

namespace qemacs
{
  
  class QEmacsTextEditBase;

  /*!
   * 
   */
  struct QEmacsRectangleMode
    : public QEmacsLineEdit
  {

    QEmacsRectangleMode(QEmacsTextEditBase&,
			QEmacsWidget&);

    virtual bool
    isBlocking(void) const override;

    /*!
     * processing is finished
     */
    virtual void
    treatUserInput(void) override;

  protected:

    static QVector<QString> killedRectangle;

    struct RectangleModeLineEdit;

    void killRectangle(void);

    void pasteRectangle(void);

    QEmacsWidget& qemacs;

    QEmacsTextEditBase& textEdit;

  }; // end of struct QEmacsRectangleMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSRECTANGLEMODE_H */

