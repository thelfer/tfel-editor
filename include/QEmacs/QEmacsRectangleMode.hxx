/*! 
 * \file  QEmacsRectangleMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 sept. 2012
 */

#ifndef LIB_QEMACS_QEMACSRECTANGLEMODE_HXX
#define LIB_QEMACS_QEMACSRECTANGLEMODE_HXX 

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
    isBlocking() const override;

    /*!
     * processing is finished
     */
    virtual void
    treatUserInput() override;

  protected:

    static QVector<QString> killedRectangle;

    struct RectangleModeLineEdit;

    void killRectangle();

    void pasteRectangle();

    QEmacsTextEditBase& textEdit;

  }; // end of struct QEmacsRectangleMode

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSRECTANGLEMODE_HXX */

