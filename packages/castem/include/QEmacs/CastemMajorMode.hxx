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
    getKeysList();

    static bool
    isCastemKeyWord(const QString&);

    CastemMajorMode(QEmacsWidget&,
		    QEmacsBuffer&,
		    QEmacsTextEditBase&);

    QString getName() const override;
    
    QString getDescription() const override;

    void setSyntaxHighlighter(QTextDocument *) override;

    bool handleShortCut(const int,
			const Qt::KeyboardModifiers,
			const int) override;
    
    QCompleter* getCompleter() override;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    void completeContextMenu(QMenu *const,
			     const QTextCursor&) override;

    void completeCurrentWord(QEmacsTextEditBase&,
			     const QString&) override;

    int getMinimalCompletionLength() override;

    void format() override;
    
    void indentLine(const QTextCursor&) override;

    QString getCommentSyntax() override;

    ~CastemMajorMode() override;

  protected slots:

    virtual void
    actionTriggered(QAction *);

  protected:

    static QStringList
    buildKeysList();

    virtual void
    displayHelp(const QString&,
		const QString&);

    virtual void
    openWebHelp(const QString&);

    virtual void
    sendToCastem(const QString&);

    virtual void
    startCastem();

    // completer
    QCompleter* c;

    ProcessInteractionFrame *co;

    // help action
    QAction *ha1;
    // help action
    QAction *ha2;

  }; // end of struct CastemMajorMode

} // end of namespace qemacs
