/*!
 * \file  QEmacsTextEditSearch.hxx
 * \brief
 * \author Helfer Thomas
 * \date   02/08/2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITSEARCH_HXX
#define LIB_QEMACS_QEMACSTEXTEDITSEARCH_HXX

#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "QEmacs/QEmacsCommandLine.hxx"
#include "QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct QEmacsTextEdit;
  // forward declaration
  struct QEmacsTextEditSearch;

  /*!
   * \brief a key press event filter which forward the user
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsTextEditSearchFilter
      : public QEmacsTextEditKeyPressEventFilter {
    /*!
     * \brief constructor
     */
    QEmacsTextEditSearchFilter(QEmacsTextEditBase&,
                               QEmacsTextEditSearch&);

    bool isOk() const override;

    bool filterKeyPressEvent(QKeyEvent* const) override;
    //! destructor
    ~QEmacsTextEditSearchFilter() override;

   public slots:

    void lineEditDestroyed();

   protected:
    QEmacsTextEditBase& textEdit;

    QEmacsTextEditSearch& s;
   private:
    Q_OBJECT

  };  // end of QEmacsTextEditSearchFilter

  struct QEMACS_VISIBILITY_EXPORT QEmacsTextEditSearch
      : public QEmacsCommandLine {

    QEmacsTextEditSearch(QEmacsTextEditBase&,
                         QEmacsWidget&,
                         const QTextDocument::FindFlags = nullptr);

    virtual void findNext();

    virtual void setFlag(const QTextDocument::FindFlags);

    virtual QTextDocument::FindFlags getFlag() const;
    //! destructor
    ~QEmacsTextEditSearch() override;

   protected slots:

    virtual void search(const QString&);

   public slots:

    virtual void searchFilterDestroyed();

   protected:
    void treatUserInput() override;

    QEmacsTextEditBase& textEdit;

    QEmacsTextEditSearchFilter* sf;

    //! position of the cursor at the beginning of the search
    QTextCursor cursor;

    QTextDocument::FindFlags flag;

   private:
    Q_OBJECT

  };  // end of struct QEmacsTextEditSearch

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSTEXTEDITSEARCH_HXX */
