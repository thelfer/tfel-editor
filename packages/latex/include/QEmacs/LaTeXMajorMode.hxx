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

  //! forward declaration
  struct QEmacsTextEditBase;
  //! forward declaration
  struct ProcessOutputFrame;
  //! forward declaration
  struct LaTeXSyntaxHighlighter;

  //! \brief major mode for editing LaTeX documents
  struct LaTeXMajorMode
    : public QEmacsMajorModeBase
  {
    LaTeXMajorMode(QEmacsWidget &, QEmacsBuffer &, QEmacsTextEditBase &);

    QString getName() const override;

    QString getDescription() const override;
    
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     */
    void completeContextMenu(QMenu *const,
			     const QTextCursor&) override;

    QCompleter* getCompleter() override;

    void completeCurrentWord(QEmacsTextEditBase&,
			     const QString&) override;

    QMenu* getSpecificMenu() override;

    bool keyPressEvent(QKeyEvent * const) override;

    bool mousePressEvent(QMouseEvent * const) override;

    /*!
     * treat "Ctrl-k1 Mod-k2" shortcuts not handled by QEmacsTextEditBase
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     */
    bool handleShortCut(const int,
			const Qt::KeyboardModifiers,
			const int) override;
    /*!
     * indent the current line
     */
    void indentLine(const QTextCursor&) override;
    /*!
     * indent selected region
     */
    void indentRegion(const QTextCursor&) override;

    QIcon getIcon() const override;

    /*!
     * format the selected paragraph
     */
    void format() override;

    QString getCommentSyntax() override;

    SpellChecker& getSpellChecker();

    void setSpellCheckLanguage(const QString&) override;

    ~LaTeXMajorMode() override;

  protected slots:

    virtual void runLaTeX();

    virtual void startLaTeX();

    virtual void replaceMispelledWordBySuggestion(QAction *);

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
    virtual void completeContextMenuForMispelledWord(QMenu *,
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
    getLineIndentation(const QTextCursor &, int &, int &, bool &) const;

    /*!
     * \return the indentation increment of the current line
     * \param[in] line : line 
     */
    virtual int getIndentationIncrement(const QString&) const;    

    virtual QString getTeXMasterFile();

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

   private:
    Q_OBJECT

  }; // end of struct LaTeXMajorMode

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LATEXMAJORMODE_H */

