/*!
 * \file  EditorWidget.hxx
 * \brief emacs-inspired editor widget, with buffers and a minibuffer.
 * \author Thomas Helfer
 * \date   27/06/2012
 */

#ifndef LIB_TFEL_GUI_EDITORWIDGET_HXX
#define LIB_TFEL_GUI_EDITORWIDGET_HXX

#include <vector>
#include <utility>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtGui/QAction>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SecondaryTaskManager.hxx"

namespace tfel::gui {

  //! forward declaration
  struct Buffer;
  //! forward declaration
  struct LineEdit;
  //! forward declaration
  struct CommandLine;

  //! \brief a wigdet providing an emacs-inspired editor
  struct TFEL_GUI_VISIBILITY_EXPORT EditorWidget : public QWidget,
                                                   public SecondaryTaskManager {
    /*!
     * \return a reference to the process wide list of recently opened files,
     * maintained in memory by addToRecentFiles
     *
     * \note this list is distinct from the "recent files" setting used by the
     * file menu of MainWindow and by the history of the open file prompt
     */
    static QStringList &getRecentFiles();

    /*!
     * \brief append a file to the process wide list of recently opened files
     * \param[in] f: file name, the list is left untouched if it is empty
     *
     * \note the oldest entry is dropped when the list holds more than 100
     * files; this in memory list is distinct from the "recent files" setting
     * maintained by MainWindow::addToRecentFiles
     */
    void addToRecentFiles(const QString &);

    /*!
     * \param[in] p : parent widget
     *
     * \note the buffers are stacked in a `QStackedWidget` above a minibuffer
     * made of a message line and of an empty read only line edit; no buffer is
     * created: this is the responsibility of the caller, see MainWindow and
     * createEmptyBuffer
     */
    EditorWidget(QWidget *const = nullptr);

    /*!
     * \brief check that all the buffers can be closed
     *
     * If at least one buffer holds unsaved modifications, a message box lists
     * some of them and asks the user to save all, discard or cancel: the
     * modified buffers are saved when "save all" is selected.
     *
     * \return false only if the user cancelled, true otherwise
     */
    virtual bool isOkToClose();

    /*!
     * \return the names of the buffers, in the order in which they are held
     * by this widget, "* unnamed buffer *" standing for the buffers with an
     * empty name
     */
    virtual QStringList getBuffersNames() const;

    /*!
     * \return the icons of the buffers, in the same order as
     * getBuffersNames, a null icon for the buffers with no icon
     */
    virtual std::vector<QIcon> getBuffersIcons() const;

    /*!
     * \return the ids of the buffers, in the same order as getBuffersNames
     * (see Buffer::getId)
     */
    virtual std::vector<int> getBuffersIds() const;

    /*!
     * \brief switch to the buffer designated by the given name
     * \param[in] n: complete name of a buffer, or raw name of the visited
     * file when this designation is not ambiguous
     *
     * \note the current buffer is discarded first, so that switching to a
     * name shared by several buffers selects another one; nothing happens if
     * no buffer matches or if the raw name designates several buffers
     */
    virtual void changeBuffer(const QString &);

    /*!
     * \brief switch to the buffer with the given id (see Buffer::getId)
     * \param[in] id: id of the buffer, nothing happens if no buffer has it
     */
    virtual void changeBuffer(const int);

    /*!
     * \brief re-choose the suffix of the name of every buffer and refresh
     * their information
     *
     * The suffix of a buffer visiting a file is the first parent directory of
     * this file which makes its name differ from the names of the buffers
     * visiting a file with the same name: two buffers visiting
     * `.../tfel/src/tfel/src/CMakeLists.txt` and
     * `.../tfel-editor/src/tfel-editor/src/CMakeLists.txt` are named
     * `CMakeLists.txt <tfel>` and `CMakeLists.txt <tfel-editor>`. Buffers
     * which do not visit any file are distinguished by a counter, as their
     * empty raw name holds no directory information.
     *
     * \note The suffix of a buffer depends on the files visited by all the
     * buffers, so the whole set of buffers must be handled at once: this
     * method shall be called every time a buffer is created, closed or visits
     * another file.
     */
    virtual void updateBufferNameSuffixes();

    /*!
     * \return the menus specific to the major mode of the main frame of the
     * current buffer, empty if there is no such buffer or menu; the menus are
     * owned by the major mode, MainWindow only inserts them in the menu bar
     */
    virtual std::vector<QMenu *> getCurrentBufferSpecificMenus();

    /*!
     * \return the current buffer; an empty buffer is created if no buffer is
     * held by this widget yet, so a valid buffer is always returned
     */
    virtual Buffer &getCurrentBuffer();

    /*!
     * \brief switch to another buffer: do nothing and display a message if
     * this widget holds no other buffer, switch directly if it holds exactly
     * one other buffer, otherwise push a ChangeBuffer user input listing the
     * other buffers, the previously used buffer being the default one
     */
    virtual void changeBuffer();

    /*!
     * \brief execute the command with the given name
     * \param[in] c: name of the command, as known by the `CommandFactory`
     *
     * \note "unknown command" is displayed in the minibuffer if no command
     * matches; the command is created, executed and scheduled for deletion
     */
    virtual void launchCommand(const QString &);

    /*!
     * \brief ask the user for a command to execute: push a Command user input
     * completing the names known by the `CommandFactory`, with its history
     * stored at the "command/history" setting address
     *
     * \note a message is displayed if a user input is already active, as the
     * new one may then be refused by setUserInput
     */
    virtual void launchCommand();

    /*!
     * \return a reference to the kill ring, the list of the texts killed or
     * yanked, the most recent one last; it holds at most 100 entries and may
     * be empty
     */
    virtual const QStringList &getKillRing() const;

    /*!
     * \brief append a text to the kill ring
     * \param[in] t: text, ignored if empty
     *
     * \note the oldest entry is dropped when the kill ring holds 100 texts
     */
    virtual void addToKillRing(const QString &);

    //! \return true if at least one user input is currently active
    virtual bool hasUserInput() const;

    //! destructor
    ~EditorWidget() override;

   public slots:

    /*!
     * change the font used by the main frames
     * \param[in] f : new font
     *
     * \note this setting is not persistent: MainWindow::selectFont is in
     * charge of storing the font in the settings
     */
    void changeMainFramesFont(const QFont &);

    /*!
     * open a file in a new buffer
     *
     * \note the file name is asked through an OpenFile user input, which
     * completes in the directory of the file visited by the current buffer
     * and uses the "recent files" setting as history
     */
    virtual void openFile();

    //! \brief print the current widget main frame
    virtual void print();

    /*!
     * open a file in a new buffer
     * \param[in] f : file name
     *
     * \note nothing happens if the file name is empty; if a buffer already
     * visits this file, a message is displayed and that buffer is reused; the
     * bufferAdded signal is emitted and the buffer history is updated only
     * when a new buffer is created
     */
    virtual void openFile(const QString &);

    /*!
     * create a new buffer
     *
     * \note the buffer visits no file, is added to this widget, becomes the
     * current buffer and the bufferAdded signal is emitted
     */
    virtual void createEmptyBuffer();

    /*!
     * save all modified buffers
     *
     * \note the main frame of a buffer visiting no file asks the user for a
     * file name (see TextEditBase::writeFile); unmodified buffers are skipped
     */
    virtual void saveAllBuffers();

    /*!
     * \brief display a message in the message line of the minibuffer
     * \param[in] m : message
     *
     * \note the message is displayed for 2 seconds, or 10 seconds in debug
     * verbose mode, and resetUserInput restores the previous state once the
     * delay has elapsed; in debug verbose mode the message is also written on
     * the standard output
     */
    virtual void displayInformativeMessage(const QString &m);

    /*!
     * set an user input
     * \param[in] l : the line edit
     *
     * \note this widget takes the ownership of the given command line, which
     * is removed from the minibuffer and scheduled for deletion once it
     * emits its finished or destroying signal (see removeUserInput)
     *
     * \note nothing happens if the given command line is null or already
     * active; the request is refused with a message if the most recent active
     * user input is blocking; the buffers are disabled while a blocking user
     * input is active
     */
    virtual void setUserInput(CommandLine *const);

    /*!
     * \return the buffer visiting the given file, nullptr if no buffer visits
     * it; symbolic links are resolved and the complete names are compared
     * \param[in] f : file name
     */
    virtual Buffer *getBufferVisitingFile(const QString &);

    /*!
     * \brief give the focus to the most recent user input, do nothing if no
     * user input is active
     */
    virtual void focusUserInput();

    /*!
     * \brief remove all the active user inputs, from the most recent one to
     * the oldest one (see removeUserInput)
     */
    virtual void removeUserInputs();

    /*!
     * \brief remove the most recent user input, do nothing if no user input
     * is active (see removeUserInput)
     */
    virtual void removeUserInput();

    /*!
     * \brief remove the given user input from the minibuffer: re-enable the
     * buffers if it was blocking, display the next user input or the empty
     * line edit, give the focus back to the current buffer and schedule the
     * given command line for deletion; nothing happens if it is not active
     * \param[in] p : command line to remove
     */
    virtual void removeUserInput(CommandLine *);

    /*!
     * \brief close the current buffer
     * \param[in] b : if true, check if the buffer is ok to close
     *
     * \note when the check is required and the buffer holds unsaved
     * modifications, a blocking SaveCurrentBufferAndKill user input asks the
     * user whether the buffer shall be killed anyway; nothing happens if no
     * buffer is current
     */
    virtual void closeCurrentBuffer(const bool = true);

    /*!
     * \brief close the given buffer
     * \param[in] b : if true, check if the buffer is ok to close
     *
     * \note the check is not implemented yet: the buffer is closed in any
     * case, as done by removeBuffer, which creates an empty buffer if this
     * one was the last one and emits the bufferRemoved signal
     */
    virtual void closeBuffer(Buffer &, const bool = true);

    /*!
     * \brief save the current buffer if its document is modified; do nothing
     * if no buffer is current
     */
    virtual void saveCurrentBuffer();
    /*!
     * \brief go to the next buffer. If the current buffer is the
     * last one, cycle the first one (index 0).
     *
     * \note nothing happens if this widget holds a single buffer; contrary to
     * setCurrentBuffer, the buffer history is not updated and the
     * currentBufferChanged signal is not emitted
     */
    virtual void nextBuffer();
    /*!
     * \brief go to the previous buffer. If the current buffer is the
     * first one (index 0), cycle the last one.
     *
     * \note nothing happens if this widget holds a single buffer; contrary to
     * setCurrentBuffer, the buffer history is not updated and the
     * currentBufferChanged signal is not emitted
     */
    virtual void previousBuffer();
    /*!
     * \brief change the orientation of the secondary tasks in all
     * buffers
     * \param[in] o: orientation
     *
     * \note the orientation is also stored at the "secondary tasks
     * orientation" setting address, so that new buffers reuse it
     */
    virtual void setSecondaryTasksOrientation(const Qt::Orientation);

   signals:

    //! signal launched when this widget has been closed (see closeEvent)
    void closed();
    //! signal launched when a buffer has been added to this widget
    void bufferAdded();
    //! signal launched when a buffer has been removed from this widget
    void bufferRemoved();
    //! signal launched when the name of a buffer has changed
    void bufferNameChanged();
    //! signal launched when the current buffer has changed
    void currentBufferChanged();
    //! signal launched when the major mode of the current buffer changed
    void currentBufferMajorModeChanged();
    //! signal launched when the menu bar shall be updated
    void updatedMenu();
    /*!
     * \brief signal launched when a buffer starts visiting a file, used by
     * MainWindow to maintain the list of recent files
     * \param[in] f : complete name of the visited file
     */
    void newTreatedFile(const QString &);

   protected slots:

    /*!
     * \brief restore the minibuffer: display the most recent user input, or
     * the empty line edit if no user input is active, and clear the message
     * line; triggered once an informative message has been displayed for
     * some time (see displayInformativeMessage)
     */
    virtual void resetUserInput();

    /*!
     * \brief replace the old name of a buffer by its new one in the buffers
     * history and emit the bufferNameChanged signal; nothing happens if the
     * given buffer is not held by this widget
     * \param[in] b : buffer whose name changed
     * \param[in] o : old name
     * \param[in] n : new name
     */
    virtual void updateBufferName(Buffer *, const QString &, const QString &);

    /*!
     * \brief forward the given file name to the newTreatedFile signal
     * \param[in] f : complete name of the visited file
     */
    virtual void emitNewTreatedFile(const QString &);

   protected:
    /*!
     * "open file" user input: a FilePathUserInput which opens the entered
     * file in a new buffer (see EditorWidget::openFile); defined in
     * EditorWidget.cxx
     */
    struct OpenFile;

    /*!
     * change buffer user input: a CommandLine which switches to the entered
     * buffer, or to the default one when the input is empty (see
     * EditorWidget::changeBuffer); defined in EditorWidget.cxx
     */
    struct ChangeBuffer;

    /*!
     * launch a new command: a CommandLine completing and launching the
     * commands known by the CommandFactory (see the overload of
     * EditorWidget::launchCommand taking a name); defined in
     * EditorWidget.cxx
     */
    struct Command;

    /*!
     * blocking yes or no user input asking whether a modified buffer shall be
     * killed, removing that buffer (the one current when the user input was
     * created) when the user answers positively; defined in EditorWidget.cxx
     */
    struct SaveCurrentBufferAndKill;

    /*!
     * \brief handle the closing of this widget: the event is accepted and the
     * closed signal emitted if isOkToClose returns true, ignored otherwise,
     * so that the user can still save its modifications
     * \param[inout] ev : close event
     */
    virtual void closeEvent(QCloseEvent *) override;

    /*!
     * create a new buffer
     * \param[in] f : name of the file visited by the buffer, empty if the
     * buffer shall visit no file
     *
     * \note if a buffer already visits the given file, a message is displayed
     * and that buffer is returned instead of a new one; the id of the buffer
     * (see Buffer::getId) is taken from nid, which is then incremented; the
     * buffer is not added to this widget, this is the caller's responsibility
     *
     * \return the created or reused buffer
     */
    virtual Buffer *createNewBuffer(const QString & = "");

    /*!
     * set the current buffer, add it at the top of history, emit
     * the currentBufferChanged signal, and give it the focus
     *
     * \note nothing happens if the given buffer is null or not held by this
     * widget; the tab widget of the secondary tasks of the buffer is
     * refreshed first
     */
    virtual void setCurrentBuffer(Buffer *const);

    /*!
     * \brief remove the given buffer from this widget: drop it from the
     * buffers history and from the secondary task manager, make another
     * buffer current (creating an empty one if this was the last buffer),
     * re-choose the suffixes of the names of the remaining buffers, schedule
     * the removal of the buffer itself and emit the bufferRemoved signal
     * \param[in] b : buffer to remove
     *
     * \note nothing happens if the given buffer is null or not held by this
     * widget
     */
    virtual void removeBuffer(Buffer *const) override;

    //! buffers
    QStackedWidget *buffers;
    //! buffer history
    QStringList bHistory;
    //! the minibuffer
    QStackedWidget *minibuffer;
    //! user message
    LineEdit *um;
    //! user input: the command line and whether it was blocking when it was
    //! pushed. The blocking state is recorded because it cannot be recovered
    //! from the command line itself: by the time a command line is destroyed,
    //! its most derived destructor has returned, so a virtual call to
    //! isBlocking() would dispatch to CommandLine::isBlocking() and report
    //! false for an input that really was blocking.
    std::vector<std::pair<CommandLine *, bool>> ui;
    //! empty line edit
    LineEdit *eui;
    /*!
     * \brief single-shot timer which clears the informative message label
     *
     * \note a single timer, restarted by every \c displayInformativeMessage
     * call and connected once to \c resetUserInput: unlike stacked
     * \c QTimer::singleShot calls, a later message can no longer let an
     * earlier timer erase it
     */
    QTimer messageTimer;
    //! the list of tokens pasted/killed
    QStringList killRing;
    //! id of the next buffer
    int nid;

   private:
    Q_OBJECT
  };  // end of struct

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_EDITORWIDGET_HXX */
