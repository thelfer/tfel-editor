/*! 
 * \file  LaTeXMajorMode.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 18 août 2012
 */

#ifndef _LIB_QEMACS_LATEXMAJORMODE_H_
#define _LIB_QEMACS_LATEXMAJORMODE_H_ 

#include<QtCore/QPointer>

#include<QEmacs/SpellChecker.hxx>
#include"QEmacs/QEmacsMajorModeBase.hxx"

namespace qemacs
{
  
  class QEmacsTextEditBase;

  class ProcessOutputFrame;

  class LaTeXSyntaxHighlighter;

  /*!
   * major mode
   */
  class LaTeXMajorMode
    : public QEmacsMajorModeBase
  {

    Q_OBJECT

  public:

    LaTeXMajorMode(QEmacsWidget&,
		   QEmacsBuffer&,
		   QEmacsTextEditBase&);

    virtual QString
    getName(void) const override;

    virtual QString
    getDescription(void) const override;
    
    virtual void
    setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    virtual void
    completeContextMenu(QMenu *const,
			const QTextCursor&) override;

    virtual QCompleter*
    getCompleter(void) override;

    virtual void
    completeCurrentWord(QEmacsTextEditBase&,
			const QString&) override;

    virtual QMenu*
    getSpecificMenu() override;

    virtual bool
    keyPressEvent(QKeyEvent * const) override;

    virtual bool
    mousePressEvent(QMouseEvent * const) override;

    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by QEmacsTextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    virtual bool
    handleShortCut(const int,
		   const Qt::KeyboardModifiers,
		   const int) override;

    /*!
     * indent the current line
     */    
    virtual void
    indentLine(const QTextCursor&) override;

    /*!
     * indent selected region
     */    
    virtual void
    indentRegion(const QTextCursor&) override;

    virtual QIcon
    getIcon(void) const override;

    /*!
     * format the selected paragraph
     */
    virtual void
    format(void) override;

    virtual QString
    getCommentSyntax() override;

    SpellChecker&
    getSpellChecker(void);

    virtual void
    setSpellCheckLanguage(const QString&) override;

    virtual ~LaTeXMajorMode();

  protected slots:

    virtual void
    runLaTeX();

    virtual void
    startLaTeX();

    virtual void
    replaceMispelledWordBySuggestion(QAction *);

  protected:

    struct LaTeXInsertBlock;

    /*!
     * \param[out] m   : context menu
     * \param[in]  t   : portion of text
     * \param[in]  pos : position of the cursor relative to the
     * beginning of the portion of text in the line
     * \param[in] apos : abosolution position of the beginning of the
     * portion of text in the line
     */
    virtual void
    completeContextMenuForMispelledWord(QMenu *,
					const QString&,
					const int,
					const int);

    /*!
     * compute indentation of the current line
     * \param[out] i  : indentation of the previous line (0 if first line)
     * \param[out] di : indentation increment of the given line 
     * \param[out] b  : true if the increment shall be used for the current line 
     */
    virtual void
    getLineIndentation(const QTextCursor&,
		       int&,
		       int&,
		       bool&) const;
    
    /*!
     * \return the indentation increment of the current line
     * \param[in] line : line 
     */
    virtual int
    getIndentationIncrement(const QString&) const;    

    virtual QString
    getTeXMasterFile();


    QRegExp rb;
    QRegExp re;
    QRegExp rb2;
    QRegExp re2;

    QCompleter *c;

    LaTeXSyntaxHighlighter *highlighter;

    SpellChecker spellChecker;

    QVector<QAction *> suggestions;
    //! absolute postion of the beginning of the mispelled word
    int bmwp; 
    //! absolute postion of the end of the mispelled word
    int emwp;

    QPointer<ProcessOutputFrame> po;

  }; // end of struct LaTeXMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LATEXMAJORMODE_H */

