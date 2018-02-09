/*!
 * \file  QEmacsTextEditReplaceFilter.hxx
 * \brief
 * \author Helfer Thomas
 * \date   06/08/2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX
#define LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX

#include "QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct QEmacsTextEdit;
  // forward declaration
  struct QEmacsTextEditQueryReplace;

  /*!
   * \brief a key press event filter which forward the user
   */
  struct QEmacsTextEditReplaceFilter
      : public QEmacsTextEditKeyPressEventFilter {
    QEmacsTextEditReplaceFilter(QEmacsWidget&,
                                QEmacsTextEditBase&,
                                QEmacsTextEditQueryReplace&,
                                const QString&,
                                const QString&);

    bool isOk() const override;

    bool filterKeyPressEvent(QKeyEvent* const) override;
    //! destructor
    ~QEmacsTextEditReplaceFilter() override;

   protected:
    bool findNext();

    QEmacsWidget& qemacs;

    QEmacsTextEditBase& textEdit;

    QEmacsTextEditQueryReplace& qr;

    QTextCursor bc;

    QTextCursor ec;

    const QString s1;

    const QString s2;

    int nb;

    bool hasSelection;

    bool bOK;

  };  // end of QEmacsTextEditReplaceFilter

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX */
