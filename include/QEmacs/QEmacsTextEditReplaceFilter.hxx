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
    /*!
     * \brief find the next occurrence of s1
     * \return a boolean stating if a occurrence has been found
     */
    bool findNext();

    QEmacsWidget& qemacs;

    QEmacsTextEditBase& textEdit;

    QEmacsTextEditQueryReplace& qr;
    //! beginning of the working region
    QTextCursor bc;
    //! end of the working region
    QTextCursor ec;
    //! string to be replaced
    const QString s1;
    //! replacement string
    const QString s2;
    //! number of replacements done
    int nb = 0;
    //! options used for searching the replaced string
    QTextDocument::FindFlag opts = QTextDocument::FindCaseSensitively;
    //! states if the filter is called on a portion of the file
    bool hasSelection = false;
    //! current status
    bool bOK = true;

  };  // end of QEmacsTextEditReplaceFilter

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSTEXTEDITREPLACEFILTER_HXX */
