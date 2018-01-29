/*! 
 * \file  QEmacsTextEditBase.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 août 2012
 */

#ifndef LIB_QEMACS_QEMACSTEXTEDITBASE_HXX
#define LIB_QEMACS_QEMACSTEXTEDITBASE_HXX 

#include<QtCore/QMap>
#include<QtCore/QString>
#include<QtCore/QStringList>
#include<QtGui/QTextCursor>
#include<QtWidgets/QWidget>
#include<QtWidgets/QTextEdit>
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

  class QEMACS_VISIBILITY_EXPORT QEmacsTextEditBase
    : public QWidget
  {

    Q_OBJECT

  signals:

    void fileNameChanged(const QString&);

    void majorModeChanged();

    void cursorPositionChanged();

  public slots:

    virtual void
    clear() = 0;

    virtual void
    undo() = 0;

    virtual void
    redo() = 0;

    virtual void
    cut() = 0;

    virtual void
    copy() = 0;

    virtual void
    paste() = 0;

    virtual void
    selectAll() = 0;

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
    virtual void setMajorMode(const QString&);

    virtual void setMajorMode();

    virtual void setMajorMode(QEmacsMajorMode *const);
    
    virtual void
    setFileName(const QString&);

    virtual void
    writeFile();

    virtual void
    writeFile(const QString&);

    virtual void
    save();

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
      isBlocking() const override;

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
    
    ~QEmacsTextEditBase() override;

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
    isMainFrame() const;

    virtual void
    setSpellCheckLanguage(const QString&);

    /*!
     * \return true if the buffer is not a main frame
     */
    virtual bool
    isSlave() const;

    virtual void
    setFocus();

    virtual QString
    getFileName() const;

    virtual QString
    getCompleteFileName() const;

    virtual SaveInput *
    getSaveInput();

    /*!
     * \return the word under cursor
     */
    virtual QString
    getCurrentWord() const;

    virtual bool
    isModified() const;

    virtual bool
    hasMajorMode() const;

    virtual const QEmacsMajorMode&
    getMajorMode() const;

    virtual QVector<QMenu*>
    getSpecificMenus();

    virtual QIcon
    getIcon() const;

    virtual bool
    setKeyPressEventFilter(QEmacsTextEditKeyPressEventFilter * const);

    virtual void
    removeKeyPressEventFilter();

    virtual QTextDocument*
    document() const = 0;
    
    virtual void
    moveCursor(QTextCursor::MoveOperation,
	       QTextCursor::MoveMode = QTextCursor::MoveAnchor) = 0;

    virtual bool
    isReadOnly() = 0;
    
    virtual bool
    find(const QString&,
	 QTextDocument::FindFlags = nullptr) = 0;

    virtual QTextCursor
    textCursor() const = 0;
    
    virtual QTextCursor
    cursorForPosition(const QPoint &pos) const = 0;

    virtual QRect
    cursorRect(const QTextCursor &cursor) const = 0;

    virtual QRect
    cursorRect() const = 0;

    virtual void
    setExtraSelections(const QList<QTextEdit::ExtraSelection> &) = 0;

    virtual QList<QTextEdit::ExtraSelection>
    extraSelections() const = 0;

    virtual bool
    isUndoRedoEnabled() const = 0;

  protected slots:
    
    virtual void
    highlightCurrentLine();

    virtual void
    keyPressEventFilterDestroyed();

    virtual void
    emitCursorPositionChanged();

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
      
      ~GotoLine() override;
      
    protected:
      
      virtual void
      treatUserInput() override;

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

#endif /* LIB_QEMACS_QEMACSTEXTEDITBASE_HXX */

