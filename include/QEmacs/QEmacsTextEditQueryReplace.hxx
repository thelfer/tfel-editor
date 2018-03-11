/*! 
 * \file   include/QEmacs/QEmacsTextEditQueryReplace.hxx
 * \brief
 * \author Helfer Thomas
 * \date   06/08/2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_HXX
#define LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_HXX 

#include"QEmacs/QEmacsCommandLine.hxx"

namespace qemacs{

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct QEmacsTextEdit;

  struct QEmacsTextEditQueryReplace
    : public QEmacsCommandLine
  {
    QEmacsTextEditQueryReplace(QEmacsTextEditBase&, QEmacsWidget&);
    //! desctructor
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

    QEmacsTextEditBase& textEdit;
    QString s1;
    int stage;

  }; // end of struct QEmacsTextEditQueryReplace

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSTEXTEDITQUERYREPLACE_HXX */

