/*! 
 * \file  QEmacsTextEditQueryReplace.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 06 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_H_
#define _LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_H_ 

#include"QEmacs/QEmacsLineEdit.hxx"

namespace qemacs{

  class QEmacsWidget;

  class QEmacsTextEdit;

  struct QEmacsTextEditQueryReplace
    : public QEmacsLineEdit
  {

    QEmacsTextEditQueryReplace(QEmacsTextEditBase&,
			       QEmacsWidget&);

    ~QEmacsTextEditQueryReplace();

  protected:

    static QStringList
    getHistory(void);

    static void
    addToHistory(const QString&);

    QEmacsTextEditQueryReplace(QEmacsTextEditBase&,
			       QEmacsWidget&,
			       const QString&);

    QEmacsTextEditQueryReplace(QEmacsTextEditBase&,
			       QEmacsWidget&,
			       const QString&,
			       const QString&);

    virtual void
    treatUserInput(void) override;

    QEmacsWidget&       qemacs;
    QEmacsTextEditBase& textEdit;
    QString s1;
    int stage;

  }; // end of struct QEmacsTextEditQueryReplace

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_H */

