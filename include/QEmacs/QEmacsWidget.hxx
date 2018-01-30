/*! 
 * \file  QEmacsWidget.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 27 juin 2012
 */

#ifndef LIB_QEMACS_QEMACSWIDGET_HXX
#define LIB_QEMACS_QEMACSWIDGET_HXX 

#include<QtCore/QString>
#include<QtWidgets/QLabel>
#include<QtWidgets/QAction>
#include<QtWidgets/QWidget>
#include<QtWidgets/QLineEdit>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QStackedWidget>
#include"QEmacs/Config.hxx"

namespace qemacs
{

  //! forward declaration
  class QEmacsBuffer;

  //! forward declaration
  class QEmacsLineEdit;
    
  /*!
   * A wigdet providing an emacs clone editor
   */
  class QEMACS_VISIBILITY_EXPORT QEmacsWidget
    : public QWidget
  {

    Q_OBJECT

  public:
      
    static QStringList&
    getRecentFiles();

    void
    addToRecentFiles(const QString&);
    
    /*!
     * \param[in] p : parent widget
     */
    QEmacsWidget(QWidget *const = nullptr);

    virtual bool
    isOkToClose();

    virtual QStringList
    getBuffersNames() const;

    virtual QVector<QIcon>
    getBuffersIcons() const;

    virtual QVector<int>
    getBuffersIds() const;

    virtual void
    changeBuffer(const QString&);

    virtual void
    changeBuffer(const int);

    virtual QString
    chooseBufferNameSuffix(QEmacsBuffer *,
			   const QString&);

    virtual QVector<QMenu*>
    getCurrentBufferSpecificMenus();

    virtual QEmacsBuffer&
    getCurrentBuffer();

    virtual void
    changeBuffer();

    virtual void
    launchCommand();

    virtual void
    launchCommand(const QString&);

    ~QEmacsWidget() override;

  public slots:

    /*!
     * change the font used by the main frames
     * \param[in] f : new font
     */
    void
    changeMainFramesFont(const QFont&);

    /*!
     * open a file in a new buffer
     */  
    virtual void
    openFile();
      
    /*!
     * open a file in a new buffer
     * \param[in] f : file name
     */  
    virtual void
    openFile(const QString&);

    /*!
     * create a new buffer
     */  
    virtual void
    createEmptyBuffer();

    /*!
     * save all modified buffers
     */  
    virtual void
    saveAllBuffers();

    virtual void
    displayInformativeMessage(const QString& m);

    virtual const QStringList&
    getKillRing() const;

    virtual void
    addToKillRing(const QString&);

    /*!
     * set an user input
     * \param[in] l : the line edit
     */
    virtual void
    setUserInput(QEmacsLineEdit *const);

    virtual QEmacsBuffer *
    getBufferVisitingFile(const QString&);

    virtual bool
    hasUserInput() const;

    virtual void
    focusUserInput();

    virtual void
    removeUserInputs();

    virtual void
    removeUserInput();

    virtual void
    removeUserInput(QEmacsLineEdit *);

    /*!
     * \param[in] b : if true, check if the buffer is ok to close
     */
    virtual void
    closeCurrentBuffer(const bool = true);

    /*!
     * \param[in] b : if true, check if the buffer is ok to close
     */
    virtual void
    closeBuffer(QEmacsBuffer&,
		const bool = true);

    virtual void
    saveCurrentBuffer();

  signals:

    void closed();

    void bufferAdded();

    void bufferRemoved();

    void bufferNameChanged();

    void currentBufferChanged();

    void newTreatedFile(const QString&);

  protected slots:

    virtual void
    resetUserInput();

    virtual void
    updateBufferName(QEmacsBuffer *,
		     const QString&,
		     const QString&);

    virtual void
    emitNewTreatedFile(const QString&);

  protected:

    /*!
     * "open file" user input
     */
    struct OpenFile;

    /*!
     * change buffer user input
     */
    struct ChangeBuffer;

    /*!
     * launch a new command
     */
    struct Command;
    
    struct SaveCurrentBufferAndKill;

    virtual void
    closeEvent(QCloseEvent *) override;

    /*!
     * create a new buffer
     */  
    virtual QEmacsBuffer*
    createNewBuffer(const QString& = "");

    /*!
     * set the current buffer, add it at the top of history, emit
     * the currentBufferChanged signal, and give it the focus
     */
    virtual void
    setCurrentBuffer(QEmacsBuffer* const);

    virtual void
    removeBuffer(QEmacsBuffer* const);

    //! buffers
    QStackedWidget *buffers;
    //! buffer history
    QStringList bHistory;
    //! the minibuffer
    QStackedWidget *minibuffer;
    //! user message
    QLineEdit  *um;
    //! user input
    QVector<QEmacsLineEdit *> ui;
    //! empty line edit
    QLineEdit *eui;
    //! the list of tokens pasted/killed
    QStringList killRing;

    //! id of the next buffer
    int nid;

  }; // end of struct QEmacs

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSWIDGET_HXX */
