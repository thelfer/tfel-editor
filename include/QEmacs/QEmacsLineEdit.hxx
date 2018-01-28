/*! 
 * \file  QEmacsLineEdit.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 juil. 2012
 */

#ifndef LIB_QEMACS_QEMACSLINEEDIT_HXX
#define LIB_QEMACS_QEMACSLINEEDIT_HXX 

#include<QtCore/QPointer>
#include<QtWidgets/QLabel>
#include<QtWidgets/QLineEdit>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QVBoxLayout>
#include"QEmacs/Config.hxx"

namespace qemacs
{

  class QEmacsWidget;

  class QEmacsBuffer;

  class QEmacsTextEdit;
  
  class QEMACS_VISIBILITY_EXPORT QEmacsLineEdit
    : public QWidget
  {
    Q_OBJECT

  public:

    /*!
     * \param[in] l : label
     * \param[in] p : parent
     * \param[in] b : if true, set the default custom line edit
     */
    QEmacsLineEdit(const QString&,
		   QEmacsWidget& p,
		   const bool = true);

    virtual void
    setFocus();
    
    virtual void
    setInputHistorySettingAddress(const QString&);

    virtual void
    setInputHistory(const QStringList&);

    void keyPressEvent(QKeyEvent *) override;

    virtual void
    setLabel(const QString&);

    virtual bool
    isBlocking() const;
    
    ~QEmacsLineEdit() override;

  public slots:

    virtual void
    userEditingFinished();
    
    /*!
     * cancel editing
     */
    virtual void
    cancel();

    virtual void
    showCompletions(const QString&,
		    const QStringList&);

    virtual void
    hideCompletions();
    
  signals:

    void finished(QEmacsLineEdit *);

    void destroyed(QEmacsLineEdit *);

    void textChanged(const QString &);

    void textEdited(const QString &);

  protected slots:
    
    /*!
     * processing is finished
     */
    virtual void
    treatUserInput() = 0;

    virtual void
    inputTextChanged(const QString &);

    virtual void
    inputTextEdited(const QString &);

  protected:

    struct CustomLineEdit
      : public QLineEdit
    {

      CustomLineEdit(QEmacsWidget&,
		     QEmacsLineEdit&);

      bool event(QEvent *) override;
      
      void setInputHistory(const QStringList&);
      
      QStringList getInputHistory() const;

      void keyPressEvent(QKeyEvent *) override;

      /*!
       * \param[in] b : if true, call the QLineEdit::setCompleter
       */
      virtual void
      setCompleter(QCompleter * const,
		   const bool);
      
      virtual QCompleter*
      completer() const;

      ~CustomLineEdit() override;

    protected:

      /*!
       * treat "Ctrl-k1 Mod-k2"
       * where k1 is either Qt::Key_X or Qt::Key_C.
       * \param[in] k1 : first  key
       * \param[in] m  : second key modifier
       * \param[in] k2 : second key
       * \return true if the short cut is handled by this mode
       */
      virtual void
      handleShortCut(const int,
		     const Qt::KeyboardModifiers,
		     const int);

      virtual void
      complete();

      /*!
       * \return a completion for the given input. An empty string is
       * returned if no completion is available.
       * \param[out] b : set to true if only one completion is avaiable
       */
      virtual QString
      findCompletion(bool&);

      virtual QString
      extractBaseForCompletion(const QString&);

      QEmacsWidget& qemacs;
      QEmacsLineEdit& lineEdit;
      QCompleter* c_;
      bool completerHandledByQLineEdit;
      QStringList inputHistory;
      int pring;
      bool ctrlx;
      bool ctrlc;
      bool yank;
      int pHistory;
      bool hMove;

    };

    void setLineEdit(CustomLineEdit *const);

    QEmacsWidget& qemacs;
    QEmacsBuffer& buffer;
    QVBoxLayout    *vl;
    QHBoxLayout    *hl;
    QLabel         *label;
    CustomLineEdit *input;
    QPointer<QEmacsTextEdit> completions;
    QWidget *scompletions;
    QString inputHistorySettingAddress;
    bool isUserEditingFinished;

  }; // end of QEmacsLineEdit

  /*!
   * An helper class asking the user to type y (yes) or n (no)
   */
  struct QEmacsYesOrNoUserInput
    : public QEmacsLineEdit
  {
    QEmacsYesOrNoUserInput(const QString&,
			   QEmacsWidget& p);

  protected:

    struct YesOrNoLineEdit;

  }; // end of struct QEmacsYesOrNoUserInput

  /*!
   * An helper class asking the user for a file path
   */
  struct QEmacsFilePathUserInput
    : public QEmacsLineEdit
  {
    QEmacsFilePathUserInput(const QString&,
			    QEmacsWidget&);

  protected:

    struct FilePathLineEdit;

  }; // end of struct QEmacsYesOrNoUserInput

  /*!
   * An helper class asking the user for a command and displaying the
   * results in a ProcessOutputFrame created as a slave of the current
   * buffer
   */
  struct QEmacsProcessLineEdit
    : public QEmacsLineEdit
  {
    /*!
     * \param[in] l : label
     * \param[in] c : command guess
     * \param[in] m : mode used to display the results
     * \param[in] p : parent     
     */
    QEmacsProcessLineEdit(const QString&,
			  const QString&,
			  const QString&,
			  QEmacsWidget&);

    ~QEmacsProcessLineEdit() override;

  protected:

    void treatUserInput() override;

    virtual void
    run(const QString&,
	const QString&,
	const QStringList&);

    QEmacsWidget& qemacs;
    
    const QString mode;

  }; // end of QEmacsProcessLineEdit

  /*!
   * An helper class asking the user for a shell command and
   * displaying the results in a ProcessOutputFrame created as a slave
   * of the current buffer
   */
  struct QEmacsShellProcessLineEdit
    : public QEmacsProcessLineEdit
  {
    /*!
     * \param[in] l : label
     * \param[in] c : command guess
     * \param[in] m : mode used to display the results
     * \param[in] p : parent     
     */
    QEmacsShellProcessLineEdit(const QString&,
			       const QString&,
			       const QString&,
			       QEmacsWidget&);

    ~QEmacsShellProcessLineEdit() override;

  protected:

    void treatUserInput() override;

  }; // end of QEmacsShellProcessLineEdit

} // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSLINEEDIT_HXX */

