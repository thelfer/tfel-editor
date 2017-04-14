/*! 
 * \file  CastemMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 août 2012
 */

#include<QtCore/QVector>

#include"QEmacs/QEmacsMajorModeBase.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

#include"QEmacs/ProcessInteractionFrame.hxx"
#include"QEmacs/QEmacsFileDownloader.hxx"
#include"QEmacs/QEmacsPlainTextEdit.hxx"

namespace qemacs
{
  
  /*!
   * A major mode dedicated to the castem finite element solver
   */
  class CastemMajorMode
    : public QEmacsMajorModeBase
  {

    Q_OBJECT

  public:
    
    static const QStringList&
    getKeysList(void);

    static bool
    isCastemKeyWord(const QString&);

    CastemMajorMode(QEmacsWidget&,
		    QEmacsBuffer&,
		    QEmacsTextEditBase&);

    virtual QString
    getName() const override;

    virtual QString
    getDescription() const override;

    virtual void
    setSyntaxHighlighter(QTextDocument *) override;

    virtual bool
    handleShortCut(const int,
		   const Qt::KeyboardModifiers,
		   const int) override;

    virtual QCompleter*
    getCompleter(void) override;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    virtual void
    completeContextMenu(QMenu *const,
			const QTextCursor&) override;

    virtual void
    completeCurrentWord(QEmacsTextEditBase&,
			const QString&) override;

    virtual int
    getMinimalCompletionLength(void) override;

    virtual void
    format(void) override;
    
    virtual void
    indentLine(const QTextCursor&) override;

    virtual QString
    getCommentSyntax(void) override;

    virtual ~CastemMajorMode();

  protected slots:

    virtual void
    actionTriggered(QAction *);

  protected:

    static QStringList
    buildKeysList(void);

    virtual void
    displayHelp(const QString&,
		const QString&);

    virtual void
    openWebHelp(const QString&);

    virtual void
    sendToCastem(const QString&);

    virtual void
    startCastem(void);

    // completer
    QCompleter* c;

    ProcessInteractionFrame *co;

    // help action
    QAction *ha1;
    // help action
    QAction *ha2;

  }; // end of struct CastemMajorMode

} // end of namespace qemacs
