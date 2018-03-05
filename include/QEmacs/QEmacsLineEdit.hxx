/*!
 * \file  QEmacsLineEdit.hxx
 * \brief
 * \author Helfer Thomas
 * \date   05/08/2012
 */

#ifndef LIB_QEMACS_QEMACSLINEEDIT_HXX
#define LIB_QEMACS_QEMACSLINEEDIT_HXX

#include <QtCore/QPointer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include "QEmacs/Config.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsWidget;
  // forward declaration
  struct QEmacsBuffer;
  // forward declaration
  struct QEmacsTextEdit;

  struct QEMACS_VISIBILITY_EXPORT QEmacsLineEdit : public QWidget {
    /*!
     * \param[in] l : label
     * \param[in] p : parent
     * \param[in] b : if true, set the default custom line edit
     */
    QEmacsLineEdit(const QString &, QEmacsWidget &p, const bool = true);

    virtual void setFocus();

    virtual void setInputHistorySettingAddress(const QString &);

    virtual void setInputHistory(const QStringList &);

    void keyPressEvent(QKeyEvent *) override;

    virtual void setLabel(const QString &);

    virtual bool isBlocking() const;
    //! destructor
    ~QEmacsLineEdit() override;

   public slots:

    virtual void userEditingFinished();

    //! cancel editing
    virtual void cancel();

    virtual void showCompletions(const QString &, const QStringList &);

    virtual void hideCompletions();

   signals:

    void finished(QEmacsLineEdit *);

    void destroyed(QEmacsLineEdit *);

    void textChanged(const QString &);

    void textEdited(const QString &);

   protected slots:

    /*!
     * processing is finished
     */
    virtual void treatUserInput() = 0;

    virtual void inputTextChanged(const QString &);

    virtual void inputTextEdited(const QString &);

   protected:
    struct CustomLineEdit : public QLineEdit {
      CustomLineEdit(QEmacsWidget &, QEmacsLineEdit &);

      bool event(QEvent *) override;

      virtual void setInputHistory(const QStringList &);

      virtual QStringList getInputHistory() const;

      void keyPressEvent(QKeyEvent *) override;

      /*!
       * \param[in] b : if true, call the QLineEdit::setCompleter
       */
      virtual void setCompleter(QCompleter *const, const bool);

      virtual QCompleter *completer() const;

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
      virtual void handleShortCut(const int,
                                  const Qt::KeyboardModifiers,
                                  const int);

      virtual void complete();

      /*!
       * \return a completion for the given input. An empty string is
       * returned if no completion is available.
       * \param[out] b : set to true if only one completion is avaiable
       */
      virtual QString findCompletion(bool &);

      virtual QString extractBaseForCompletion(const QString &);

      QEmacsWidget &qemacs;
      QEmacsLineEdit &lineEdit;
      QStringList inputHistory;
      QCompleter *c_ = nullptr ;
      bool completerHandledByQLineEdit = false;
      int pring = 0;
      bool ctrlx = false;
      bool ctrlc = false;
      bool yank = false;
      int pHistory = 0;
      bool hMove = false;

    };

    void setLineEdit(CustomLineEdit *const);

    QEmacsWidget &qemacs;
    QEmacsBuffer &buffer;
    QVBoxLayout *vl;
    QHBoxLayout *hl;
    QLabel *label;
    CustomLineEdit *input;
    QPointer<QEmacsTextEdit> completions;
    QWidget *scompletions;
    QString inputHistorySettingAddress;
    bool isUserEditingFinished;

   private:
    Q_OBJECT
  };  // end of QEmacsLineEdit

  /*!
   * An helper class asking the user to type y (yes) or n (no)
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsYesOrNoUserInput
      : public QEmacsLineEdit {
    QEmacsYesOrNoUserInput(const QString &, QEmacsWidget &p);

   protected:
    struct YesOrNoLineEdit;

  };  // end of struct QEmacsYesOrNoUserInput

  //! An helper class asking the user for a file path
  struct QEMACS_VISIBILITY_EXPORT QEmacsFilePathUserInput
      : public QEmacsLineEdit {
    QEmacsFilePathUserInput(const QString &, QEmacsWidget &);

   protected:
    struct FilePathLineEdit;

  };  // end of struct QEmacsYesOrNoUserInput

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSLINEEDIT_HXX */
