/*! 
 * \file  QEmacsTextEditBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 août 2012
 */

#ifndef _LIB_QEMACS_QEMACSTEXTEDITBASE_H_
#define _LIB_QEMACS_QEMACSTEXTEDITBASE_H_ 

#include<QtCore/QMap>
#include<QtCore/QString>
#include<QtCore/QStringList>
#ifdef QEMACS_QT4
#include<QtGui/QWidget>
#include<QtGui/QTextEdit>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QWidget>
#include<QtWidgets/QTextEdit>
#endif /* QEMACS_QT5 */
#include<QtGui/QTextCursor>

#include"QEmacs/QEmacsLineEdit.hxx"
#include"QEmacs/QEmacsTextEditKeyPressEventFilter.hxx"

namespace qemacs
{
  //! forward declaration
  class QEmacsWidget;

  //! forward declaration
  class QEmacsBuffer;

  //! forward declaration
  class QEmacsMajorMode;

  class QEmacsTextEditBase
    : public QWidget
  {

    Q_OBJECT

  signals:

    void fileNameChanged(const QString&);

    void majorModeChanged(void);

    void cursorPositionChanged(void);

  public slots:

    virtual void
    clear(void) = 0;

    virtual void
    undo(void) = 0;

    virtual void
    redo(void) = 0;

    virtual void
    cut(void) = 0;

    virtual void
    copy(void) = 0;

    virtual void
    paste(void) = 0;

    virtual void
    selectAll(void) = 0;

    virtual void
    setHtml(const QString&) = 0;

    virtual void
    insertHtml(const QString&) = 0;

    virtual void
    appendHtml(const QString&) = 0;

    virtual void
    setPlainText(const QString&) = 0;

    virtual void
    insertPlainText(const QString&) = 0;

    virtual void
    appendPlainText(const QString&) = 0;
    
    virtual void
    setReadOnly(const bool) = 0;

    virtual void
    setTextInteractionFlags(Qt::TextInteractionFlags) = 0;

    virtual void
    setTextCursor(const QTextCursor&) = 0;

    virtual void
    setUndoRedoEnabled(const bool) = 0;

    virtual void
    setFont(const QFont&);

    /*!
     * \param[in] n : mode name
     */
    virtual void
    setMajorMode(const QString&);

    virtual void
    setFileName(const QString&);

    virtual void
    writeFile();

    virtual void
    writeFile(const QString&);

    virtual void
    save(void);

    virtual void
    gotoLine(int);
    
    virtual void
    insertCompletion(QString);

    virtual void
    setMoveMode(QTextCursor::MoveMode);

  public:

    /*!
     * A QEmacsLineEdit asking the user whether to save
     */
    struct SaveInput
      : public QEmacsYesOrNoUserInput
    {

      virtual bool
      isBlocking(void) const override;

      virtual void
      treatUserInput() override;

    protected:

      friend class QEmacsTextEditBase;

      SaveInput(QEmacsWidget&,
		QEmacsTextEditBase&);


      QEmacsTextEditBase& textEdit;
    }; // end of struct SaveInput

    /*!
     * "kill other buffer and write file" user input
     */
    struct KillOtherBufferAndWriteFile;

    /*!
     * "overwrite file" user input
     */
    struct OverWriteFile;

    /*!
     * "write file" user input
     */
    struct WriteFile;

    QEmacsTextEditBase(QEmacsWidget&,
		       QEmacsBuffer&);
    
    virtual ~QEmacsTextEditBase();

    /*!
     * \param[in] e : event
     * \brief convert a mouse move event to the mouseMoved signal.
     * \note this function is only called if mouse tracking is
     * enabled;
     */
    virtual void
    mouseMoveEvent(QMouseEvent *) override;

    virtual QAbstractScrollArea *
    widget() = 0;

    /*!
     * \return true if the buffer is a main frame
     */
    virtual bool
    isMainFrame(void) const;

    virtual void
    setSpellCheckLanguage(const QString&);

    /*!
     * \return true if the buffer is not a main frame
     */
    virtual bool
    isSlave(void) const;

    virtual void
    setFocus();

    virtual QString
    getFileName(void) const;

    virtual QString
    getCompleteFileName(void) const;

    virtual SaveInput *
    getSaveInput(void);

    /*!
     * \return the word under cursor
     */
    virtual QString
    getCurrentWord() const;

    virtual bool
    isModified(void) const;

    virtual bool
    hasMajorMode(void) const;

    virtual const QEmacsMajorMode&
    getMajorMode(void) const;

    virtual QVector<QMenu*>
    getSpecificMenus();

    virtual QIcon
    getIcon(void) const;

    virtual bool
    setKeyPressEventFilter(QEmacsTextEditKeyPressEventFilter * const);

    virtual void
    removeKeyPressEventFilter();

    virtual QTextDocument*
    document(void) const = 0;
    
    virtual void
    moveCursor(QTextCursor::MoveOperation,
	       QTextCursor::MoveMode = QTextCursor::MoveAnchor) = 0;

    virtual bool
    isReadOnly(void) = 0;
    
    virtual bool
    find(const QString&,
	 QTextDocument::FindFlags = nullptr) = 0;

    virtual QTextCursor
    textCursor(void) const = 0;
    
    virtual QTextCursor
    cursorForPosition(const QPoint &pos) const = 0;

    virtual QRect
    cursorRect(const QTextCursor &cursor) const = 0;

    virtual QRect
    cursorRect(void) const = 0;

    virtual void
    setExtraSelections(const QList<QTextEdit::ExtraSelection> &) = 0;

    virtual QList<QTextEdit::ExtraSelection>
    extraSelections() const = 0;

    virtual bool
    isUndoRedoEnabled(void) const = 0;

  protected slots:
    
    virtual void
    highlightCurrentLine(void);

    virtual void
    keyPressEventFilterDestroyed(void);

    virtual void
    emitCursorPositionChanged(void);

  protected:

    /*!
     * a qemacs user input which asks
     * the user where he wants to go
     */
    struct GotoLine
      : public QEmacsLineEdit
    {
      GotoLine(QEmacsTextEditBase&,
	       QEmacsWidget&);
      
      ~GotoLine();
      
    protected:
      
      virtual void
      treatUserInput(void) override;

      QEmacsTextEditBase& textEdit;
	
    }; // end of struct GotoLine

    static QString
    getModifier(const QKeyEvent&);

    virtual bool
    event(QEvent *) override;

    virtual bool
    eventFilter(QObject *,
		QEvent *) override;

    virtual void
    keyPressEvent(QKeyEvent *) override;

    virtual bool
    handleKeyPressEvent(QKeyEvent *);

    virtual bool
    handleKeyPressEventWithEmacsShortCuts(QKeyEvent *);

    virtual bool
    handleKeyPressEventWithQtShortCuts(QKeyEvent *);

    virtual bool
    handleMousePressEvent(QMouseEvent *);

    void
    initialize(QAbstractScrollArea * const);

    virtual void
    addToKillRing(const QString&);

    virtual void
    setMajorMode(void);

    virtual void
    setMajorMode(QEmacsMajorMode *const);

    virtual void
    createContextMenuActions();

    virtual void
    deleteContextMenuActions();

    //! reference to the widget
    QEmacsWidget& qemacs;

    QEmacsBuffer& buffer;

    //! major mode
    QEmacsMajorMode *mode;

    //! key press filter
    QEmacsTextEditKeyPressEventFilter *filter;    

    QTextCursor::MoveMode moveMode;
 
    /*!
     * save the previous positions of the cursor for Ctrl-P/Ctrl-N
     * moves
     */
    QMap<int,int> positions;

    QString fileName;

    QString spellCheckLanguage;

    // undo action
    QAction *ua;
    // redo action
    QAction *ra;
    // cut action
    QAction *ca;
    // copy action
    QAction *coa;
    // paste action
    QAction *pa;
    // select all action
    QAction *sa;

    int  pring;  //<! position if the ring

    bool yank;

    bool ctrlx;

    bool ctrlc;

  private:

    friend class QEmacsBuffer;
    
    virtual void
    setMainFrame(const bool);

    bool mainFrame;

    bool keyProcessing;

  }; // end of struct QEmacsTextEditBase

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSTEXTEDITBASE_H */

