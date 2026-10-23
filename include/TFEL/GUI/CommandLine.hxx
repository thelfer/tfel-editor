/*!
 * \file  CommandLine.hxx
 * \brief
 * \author Thomas Helfer
 * \date   05/08/2012
 */

#ifndef LIB_TFEL_GUI_COMMANDLINE_HXX
#define LIB_TFEL_GUI_COMMANDLINE_HXX

#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/LineEdit.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct Buffer;
  // forward declaration
  struct TextEdit;

  struct TFEL_GUI_VISIBILITY_EXPORT CommandLine : public QWidget {
    /*!
     * \param[in] l : label
     * \param[in] p : parent
     * \param[in] b : if true, set the default custom line edit
     */
    CommandLine(const QString &, EditorWidget &p, const bool = true);

    virtual void setFocus();

    virtual void setInputHistorySettingAddress(const QString &);

    virtual void setInputHistory(const QStringList &);

    void keyPressEvent(QKeyEvent *) override;

    virtual void setLabel(const QString &);

    virtual bool isBlocking() const;
    //! destructor
    ~CommandLine() override;

   public slots:

    virtual void userEditingFinished();

    //! cancel editing
    virtual void cancel();

    /*!
     * \brief display the different completions in a secondary task
     * \param[in] b : directory in which the completion is performed, or more
     * generally base shared by all the completions, an empty string if there
     * is none
     * \param[in] cl : list of completions
     *
     * \note The completions are displayed once the text of the line edit has
     * been updated as they are hidden each time this text changes (see
     * CommandLine::hideCompletions).
     */
    virtual void showCompletions(const QString &b, const QStringList &cl);

    virtual void hideCompletions();

   signals:

    void finished(CommandLine *);

    /*!
     * \brief emitted while the command line is being destroyed, so that the
     * editor can drop it from the minibuffer.
     *
     * This signal is deliberately not called \c destroyed: a signal of that
     * name hides QObject::destroyed(QObject*). It is emitted from
     * CommandLine::~CommandLine(), where the QWidget part of the object is
     * still alive, and not from QObject::destroyed, which only fires once
     * QWidget::~QWidget has run and the widget can no longer be removed from
     * its layout.
     */
    void destroying(CommandLine *);

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
    struct CustomLineEdit : public LineEdit {
      CustomLineEdit(EditorWidget &, CommandLine &);

      bool event(QEvent *) override;

      void keyPressEvent(QKeyEvent *) override;
      /*!
       * \param[in] c : completer
       * \param[in] b : if true, the completer is handled by the base
       * `QLineEdit`, which inserts the first completion matching the text as
       * soon as the user types. If false, the completer is only used as a
       * source of completions: nothing is inserted while typing and the
       * completion is performed by the complete() method, triggered by the
       * tabulation key, which inserts the common part of all the completions
       * and displays the completions in a secondary task when they are
       * several.
       */
      virtual void setCompleter(QCompleter *const, const bool);

      /*!
       * \return the completer used as a source of completions, which is not
       * necessarily installed on the base `QLineEdit` (see setCompleter)
       */
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
      void handleShortCut(const int,
                          const Qt::KeyboardModifiers,
                          const int) override;

      virtual void complete();

      /*!
       * \return the common part of all the completions matching the text of
       * the line edit. An empty string is returned if nothing matches.
       * \param[out] b : set to true if only one completion is available
       */
      virtual QString findCompletion(bool &);

      virtual QString extractBaseForCompletion(const QString &);

      /*!
       * \brief insert a completion in the line edit
       * \param[in] c : common part of all the completions
       * \param[in] b : true if this completion is the only one
       */
      virtual void insertCompletion(const QString &, const bool);

      /*!
       * \return the common part of the given completions
       * \param[in] cs : completions matching the text of the line edit
       * \param[in] ns : name under which each completion is displayed
       * \param[out] b : set to true if only one completion is available
       *
       * \note The names of the completions are stored, as they shall be
       * displayed once the text of the line edit has been updated: the
       * completions are indeed hidden each time the text changes (see
       * CommandLine::showCompletions).
       */
      QString handleCompletions(const QStringList &,
                                const QStringList &,
                                bool &);

      CommandLine &lineEdit;
      QCompleter *c_ = nullptr;
      bool completerHandledByQLineEdit = false;
      //! names under which the completions found by the last call to
      //! findCompletion shall be displayed, if they are several
      QStringList completionList;
    };

    void setLineEdit(CustomLineEdit *const);

    EditorWidget &editor;
    Buffer &buffer;
    QVBoxLayout *vl;
    QHBoxLayout *hl;
    QLabel *label;
    CustomLineEdit *input;
    QPointer<TextEdit> completions;
    QWidget *scompletions;
    QString inputHistorySettingAddress;
    bool isUserEditingFinished;

   private:
    Q_OBJECT
  };  // end of CommandLine

  /*!
   * An helper class asking the user to type y (yes) or n (no)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT YesOrNoUserInput : public CommandLine {
    YesOrNoUserInput(const QString &, EditorWidget &p);

   protected:
    struct YesOrNoLineEdit;

  };  // end of struct YesOrNoUserInput

  //! An helper class asking the user for a file path
  struct TFEL_GUI_VISIBILITY_EXPORT FilePathUserInput : public CommandLine {
    FilePathUserInput(const QString &, EditorWidget &);

   protected:
    struct FilePathLineEdit;

  };  // end of struct YesOrNoUserInput

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_COMMANDLINE_HXX */
