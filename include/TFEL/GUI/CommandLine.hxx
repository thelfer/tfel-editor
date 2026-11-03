/*!
 * \file  CommandLine.hxx
 * \brief base class of the inputs the user is asked for in the minibuffer.
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

  /*!
   * \brief base class of all the inputs the user is asked for in the
   * minibuffer: a label followed by a line edit
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CommandLine : public QWidget {
    /*!
     * \param[in] l : label
     * \param[in] p : parent
     * \param[in] b : if true, set the default custom line edit
     *
     * \note the buffer the input works on is the current one at the time the
     * input is built, not the one which is current when the user answers
     *
     * \note pass false for \c b to install a line edit of your own with
     * \c setLineEdit instead
     *
     * \note an input is created with \c new and given to
     * \c EditorWidget::setUserInput, which takes its ownership and destroys it
     * once the editing is over, unless the input is kept: the editor is the
     * parent widget
     */
    CommandLine(const QString &, EditorWidget &p, const bool = true);

    //! \brief give the focus to the line edit of the input
    virtual void setFocus();

    /*!
     * \brief load the input history stored at the given address of the
     * settings and remember that address
     * \param[in] a : address of the history in the settings
     *
     * \note \c userEditingFinished saves the updated history back at that
     * address, which is how a command proposes its last entries the next time
     * it is used
     */
    virtual void setInputHistorySettingAddress(const QString &);

    /*!
     * \brief set the history the up and down arrow keys walk through
     * \param[in] h : previous entries, the oldest first
     *
     * \note nothing happens when the input has no line edit yet
     */
    virtual void setInputHistory(const QStringList &);

    /*!
     * \brief relay to the line edit of the input the key events the command
     * line widget itself receives
     * \param[in] e : event
     *
     * \note nothing is relayed while the input has no line edit
     */
    void keyPressEvent(QKeyEvent *) override;

    /*!
     * \brief set the text displayed before the line edit
     * \param[in] l : label
     *
     * \note the label is displayed in blue
     */
    virtual void setLabel(const QString &);

    /*!
     * \return true if the user can not move the focus out of this input while
     * it is shown
     *
     * A blocking input disables the buffers and "Ctrl-X o" keeps the focus
     * in the minibuffer, whereas a non blocking one lets the user look at the
     * buffer the command works on without answering it.
     *
     * \note the default implementation returns false. The editor reads this
     * value once, when the input is shown: while the topmost input of the
     * minibuffer blocks, \c EditorWidget::setUserInput refuses to show another
     * one and reports that a command attempted to use the minibuffer while in
     * the minibuffer
     */
    virtual bool isBlocking() const;
    //! destructor
    ~CommandLine() override;

   public slots:

    /*!
     * \brief validate what the user typed, which is what \c Return and
     * \c Ctrl-m trigger
     *
     * The text is appended to the history of the input, whose hundred last
     * entries are kept and saved in the settings when an address is set, then
     * \c treatUserInput runs and \c finished is emitted, which makes the editor
     * remove the input from the minibuffer.
     *
     * \note calling this method once the editing is already over does nothing
     */
    virtual void userEditingFinished();

    /*!
     * cancel editing
     *
     * \c treatUserInput is not called and \c finished is emitted right away, so
     * that the editor removes the input without doing anything of what was
     * typed. \c Ctrl-g triggers this slot
     */
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

    //! \brief hide the completion list shown by `showCompletions`
    /**
     * The secondary task displaying the completions is only hidden, never
     * removed: the same task is displayed again by the next call to
     * `showCompletions`.
     */
    virtual void hideCompletions();

   signals:

    /*!
     * \brief emitted when the editing is over, either because the user
     * validated the input or because it was cancelled
     * \param[in] c : the input which is finished
     *
     * \note the editor connects this signal to \c removeUserInput, which takes
     * the ownership of \c c back and destroys it
     */
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

    /*!
     * \brief emitted whenever the text of the line edit changes, whatever its
     * origin
     * \param[in] t : new text
     *
     * \note this is what hides the completions which are no longer the ones of
     * the text being typed (see \c showCompletions)
     */
    void textChanged(const QString &);

   protected slots:

    /*!
     * \brief run what the user asked for, once the user has validated the text
     * of the line edit
     *
     * \note this is where the derived classes read \c input. The input is
     * removed from the minibuffer, and destroyed, right after this method
     * returns, unless it is removed and replaced by another one here
     */
    virtual void treatUserInput() = 0;

    /*!
     * \brief relay a change of the text of the line edit
     * \param[in] t : new text
     *
     * \note the base implementation only emits \c textChanged
     */
    virtual void inputTextChanged(const QString &);

   protected:
    /*!
     * \brief the line edit of a command line: a \c LineEdit, so it knows the
     * Emacs shortcuts of the editor, wired to the command line it belongs to
     */
    struct CustomLineEdit : public LineEdit {
      /*!
       * \brief constructor
       * \param[in] q : editor widget the line edit is shown in
       * \param[in] p : command line the line edit belongs to, which is also its
       * parent widget
       */
      CustomLineEdit(EditorWidget &, CommandLine &);

      /*!
       * \brief intercept the key events before they reach \c QLineEdit
       * \param[in] ev : event
       * \return true if the event was handled, that is when it is the
       * \c Tab key, which triggers the completion and is not passed to the base
       * class. Any other event is left to \c LineEdit
       */
      bool event(QEvent *) override;

      /*!
       * \brief handle the shortcuts of the minibuffer
       * \param[in] ev : event
       *
       * \c Ctrl-g cancels the input and \c Ctrl-m validates it, as \c Return
       * does. Any other key is left to \c LineEdit, which handles the Emacs
       * shortcuts of the history
       */
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

      /*!
       * \brief destructor
       *
       * \note the completer is destroyed here unless \c QLineEdit owns it, that
       * is unless it is handled by the line edit (see \c setCompleter)
       */
      ~CustomLineEdit() override;

     protected:
      /*!
       * \brief handle the "Ctrl-k1 Mod-k2" shortcuts of the minibuffer,
       * where k1 is either Qt::Key_X or Qt::Key_C
       * \param[in] k1 : first  key
       * \param[in] m  : second key modifier
       * \param[in] k2 : second key
       *
       * "Ctrl-X o" gives the focus back to the main frame of the current
       * buffer, unless the command line blocks, in which case the focus stays
       * in the minibuffer. Any other shortcut is left to \c LineEdit
       */
      void handleShortCut(const int,
                          const Qt::KeyboardModifiers,
                          const int) override;

      /*!
       * \brief complete the text of the line edit, which the \c Tab key
       * triggers
       *
       * The common part of all the completions is inserted, and the completions
       * are displayed in a secondary task when they are several.
       *
       * \note the completions are displayed once the text has been updated:
       * they are hidden each time that text changes
       *
       * \note a line edit may gather the completions by itself, without any
       * completer, as \c FilePathUserInput::FilePathLineEdit does
       */
      virtual void complete();

      /*!
       * \return the common part of all the completions matching the text of
       * the line edit. An empty string is returned if nothing matches.
       * \param[out] b : set to true if only one completion is available
       */
      virtual QString findCompletion(bool &);

      /*!
       * \return the directory the given completion has been found in, which is
       * what \c showCompletions displays as the base of the completions
       * \param[in] r : common part of the completions
       *
       * \note the base implementation returns \c r itself
       */
      virtual QString extractBaseForCompletion(const QString &);

      /*!
       * \brief insert a completion in the line edit
       * \param[in] c : common part of all the completions
       * \param[in] b : true if this completion is the only one
       *
       * \note the base implementation replaces the whole text of the line edit
       * by \c c and ignores \c b
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

      /*!
       * \brief command line this line edit is the input of, which is also the
       * parent widget of the line edit
       *
       * \note \c complete uses it to display the completions it can not insert
       * in the text
       */
      CommandLine &lineEdit;
      /*!
       * \brief completer used as a source of completions, null when the line
       * edit has none
       *
       * \note owned by the base \c QLineEdit when it is handled by it, by this
       * line edit otherwise (see \c setCompleter)
       */
      QCompleter *c_ = nullptr;
      /*!
       * \brief true if the completer is installed on the base \c QLineEdit,
       * which then owns it and completes the text as the user types it
       *
       * \note set by \c setCompleter, read by the destructor
       */
      bool completerHandledByQLineEdit = false;
      //! names under which the completions found by the last call to
      //! findCompletion shall be displayed, if they are several
      QStringList completionList;
    };

    /*!
     * \brief use the given line edit as input of this command line
     * \param[in] i : new input
     *
     * \note nothing happens if \c i is null. The command line takes the
     * ownership of the new input, adds it to its layout and connects it so that
     * \c Return validates the input and text changes are relayed.
     *
     * \note the previous input, if any, is disconnected and destroyed, though
     * not before this method returns, as \c deleteLater is used
     */
    void setLineEdit(CustomLineEdit *const);

    /*!
     * \brief editor widget the command line is shown in
     *
     * \note it is also the parent widget of the command line
     */
    EditorWidget &editor;
    /*!
     * \brief buffer which was the current one when the command line was built
     *
     * \note the completions of the input are displayed as a secondary task of
     * that buffer, not of the one which is current when they are shown
     */
    Buffer &buffer;
    /*!
     * \brief vertical layout the label and the input used to be laid out in
     *
     * \note neither used nor initialised by the constructor, which lays
     * everything out with \c hl
     */
    QVBoxLayout *vl;
    //! layout which holds the label and the input, from left to right
    QHBoxLayout *hl;
    //! text displayed before the input, in blue
    QLabel *label;
    /*!
     * \brief line edit the user types in, null as long as no input has been set
     * with \c setLineEdit
     *
     * \note derived classes read what the user typed through this member in
     * \c treatUserInput. Looking for a \c QLineEdit child of the command line
     * would find the default one the constructor replaced instead, as it is
     * destroyed later only
     */
    CustomLineEdit *input;
    /*!
     * \brief the text edit the completions are displayed in, null as long as no
     * completion has been shown
     *
     * \note it is created on the first call to \c showCompletions, as a read
     * only text edit of the buffer the command line was built for
     */
    QPointer<TextEdit> completions;
    /*!
     * \brief the widget the buffer displays the completions in, as
     * \c Buffer::attachSecondaryTask returns it, null as long as no completion
     * has been shown
     *
     * \note it is what \c hideCompletions and the destructor act on
     */
    QWidget *scompletions;
    /*!
     * \brief address of the input history in the settings, an empty string when
     * the history is not saved
     *
     * \note \c userEditingFinished saves the history back at that address
     */
    QString inputHistorySettingAddress;
    /*!
     * \brief true once the user has finished editing, whatever the way the
     * editing ended
     *
     * \note \c userEditingFinished refuses to run twice because of it
     */
    bool isUserEditingFinished;

   private:
    Q_OBJECT
  };  // end of CommandLine

  /*!
   * An helper class asking the user to type y (yes) or n (no)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT YesOrNoUserInput : public CommandLine {
    /*!
     * \brief constructor
     * \param[in] i : question the user is asked, the label shows it followed by
     * "(y or n)"
     * \param[in] p : editor widget the question is asked in
     *
     * \note the answer is validated as soon as \c y or \c n is typed, no
     * \c Return is needed. Any other key does nothing, so the question stays
     * until it is answered or \c Ctrl-g cancels it
     */
    YesOrNoUserInput(const QString &, EditorWidget &p);

   protected:
    /*!
     * \brief line edit which recognises the \c y and \c n answers and validates
     * the input as soon as one of them is typed
     */
    struct YesOrNoLineEdit;

  };  // end of struct YesOrNoUserInput

  //! An helper class asking the user for a file path
  struct TFEL_GUI_VISIBILITY_EXPORT FilePathUserInput : public CommandLine {
    /*!
     * \brief constructor
     * \param[in] i : label, usually the path the user is asked for
     * \param[in] p : editor widget the path is asked in
     *
     * \note the completions the input offers are the files and directories
     * whose name starts with the text typed, and a separator is appended to a
     * path which is an only directory
     */
    FilePathUserInput(const QString &, EditorWidget &);

   protected:
    /*!
     * \brief line edit which completes its text with the content of the
     * directory it points to, without any completer, and expands a leading
     * "~" as the home directory
     */
    struct FilePathLineEdit;

  };  // end of struct YesOrNoUserInput

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_COMMANDLINE_HXX */
