/*! 
 * \file  QEmacsTextEditSearch.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSTEXTEDITSEARCH_H_
#define _LIB_QEMACS_QEMACSTEXTEDITSEARCH_H_ 

#include<QtGui/QTextCursor>
#include<QtGui/QTextDocument>

#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs{

  class QEmacsWidget;

  class QEmacsTextEdit;

  class QEmacsTextEditSearch;

  /*!
   * A key press event filter which forward the user
   */
  class QEmacsTextEditSearchFilter
    : public QEmacsTextEditKeyPressEventFilter
  {
    
    Q_OBJECT
    
  public:
    
    QEmacsTextEditSearchFilter(QEmacsTextEditBase&,
			       QEmacsTextEditSearch&);
    
    virtual bool
    isOk(void) const override;

    virtual bool
    filterKeyPressEvent(QKeyEvent * const) override;
    
    ~QEmacsTextEditSearchFilter();
		   
  public slots:
    
    void lineEditDestroyed();
    
  protected:
    
    QEmacsTextEditBase& textEdit;
    
    QEmacsTextEditSearch& s;
    
  }; // end of QEmacsTextEditSearchFilter

  class QEmacsTextEditSearch
    : public QEmacsLineEdit
  {

    Q_OBJECT

  public:
    
    QEmacsTextEditSearch(QEmacsTextEditBase&,
			 QEmacsWidget&,
			 const QTextDocument::FindFlags = nullptr);

    virtual void
    findNext();

    virtual void
    setFlag(const QTextDocument::FindFlags);

    virtual QTextDocument::FindFlags
    getFlag() const;

    virtual ~QEmacsTextEditSearch();

  protected slots:

    virtual void
    search(const QString&);

  public slots:

    virtual void
    searchFilterDestroyed(void);

  protected:

    virtual void
    treatUserInput(void) override;
    
    QEmacsWidget& qemacs;

    QEmacsTextEditBase& textEdit;

    QEmacsTextEditSearchFilter *sf;

    //! position of the cursor at the beginning of the search
    QTextCursor cursor;

    QTextDocument::FindFlags flag;

  }; // end of struct QEmacsTextEditSearch

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSTEXTEDITSEARCH_H */

