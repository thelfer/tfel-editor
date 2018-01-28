/*! 
 * \file  QEmacsTextEditQueryReplace.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 06 août 2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_HXX
#define LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_HXX 

#include"QEmacs/QEmacsLineEdit.hxx"

namespace qemacs{

  class QEmacsWidget;

  class QEmacsTextEdit;

  struct QEmacsTextEditQueryReplace
    : public QEmacsLineEdit
  {

    QEmacsTextEditQueryReplace(QEmacsTextEditBase&,
			       QEmacsWidget&);

    ~QEmacsTextEditQueryReplace() override;

  protected:

    static QStringList getHistory();

    static void addToHistory(const QString&);

    QEmacsTextEditQueryReplace(QEmacsTextEditBase&,
			       QEmacsWidget&,
			       const QString&);

    QEmacsTextEditQueryReplace(QEmacsTextEditBase&,
			       QEmacsWidget&,
			       const QString&,
			       const QString&);

    void treatUserInput() override;

    QEmacsWidget&       qemacs;
    QEmacsTextEditBase& textEdit;
    QString s1;
    int stage;

  }; // end of struct QEmacsTextEditQueryReplace

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_HXX */

