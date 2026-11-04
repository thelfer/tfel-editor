/*!
 * \file  Buffer.hxx
 * \brief buffer: a main text edit and its secondary tasks.
 * \author Thomas Helfer
 * \date   27/06/2012
 */

#ifndef LIB_TFEL_GUI_BUFFER_HXX
#define LIB_TFEL_GUI_BUFFER_HXX

#include <vector>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAbstractScrollArea>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  //! forward declaration
  struct EditorWidget;

  //! forward declaration
  struct SecondaryTaskManager;

  //! forward declaration
  struct PlainTextEdit;

  /*!
   * \brief class in charge of managing one buffer.
   *
   * A buffer is made of:
   * - a main widget (most of the time an instance of
   *   `PlainTextEdit`)
   * - SecondaryTasks of that main widget
   *
   *  \note: a SecondaryTask can be shared between several buffers.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT Buffer : public QWidget {
    /*!
     * \brief constructor of a buffer which visits no file
     * \param[in] i: id of the buffer, chosen by the editor widget
     * \param[in] w: editor widget, which becomes the Qt parent of this buffer
     *
     * \note the caller (an instance of EditorWidget) is in charge of adding
     * the buffer to the widget holding the buffers
     */
    Buffer(const int, EditorWidget &);

    /*!
     * \brief constructor of a buffer visiting the given file
     * \param[in] f: name of the visited file
     * \param[in] i: id of the buffer, chosen by the editor widget
     * \param[in] w: editor widget, which becomes the Qt parent of this buffer
     *
     * \note the newTreatedFile signal is emitted with the complete name of
     * the visited file; the caller (an instance of EditorWidget) is in charge
     * of adding the buffer to the widget holding the buffers
     */
    Buffer(const QString &, const int, EditorWidget &);

    /*!
     * \return the id of this buffer: an integer chosen by the editor widget
     * when the buffer is created, used to designate it in the buffers menu
     * (see EditorWidget::changeBuffer(const int))
     */
    virtual int getId() const;

    /*!
     * \return the name of this buffer: the raw name followed by the suffix
     * between angle brackets when a suffix is set (see setBufferNameSuffix),
     * the suffix alone between angle brackets when this buffer visits no file
     * and an empty string otherwise
     */
    virtual QString getBufferName() const;

    //! \return the suffix of the name of this buffer, empty if it needs none
    virtual QString getBufferNameSuffix() const;

    /*!
     * \brief set the suffix of the name of this buffer and refresh its
     * information
     * \param[in] s: new suffix, empty if the name of this buffer needs none
     *
     * \note The suffix of a buffer visiting a file is the first parent
     * directory of this file which makes its name differ from the names of the
     * buffers visiting a file with the same name, so it depends on all the
     * buffers: it shall be set by EditorWidget::updateBufferNameSuffixes only.
     */
    virtual void setBufferNameSuffix(const QString &s);

    /*!
     * \return the base name of the file visited by this buffer, the name of
     * the visited directory if this buffer visits a directory, an empty
     * string if it visits no file
     */
    virtual QString getBufferRawName() const;

    //! \return the main widget of this buffer
    virtual PlainTextEdit &getMainFrame();
    //! \return the current SecondaryTask if any, nullptr otherwise
    virtual QWidget *getCurrentSecondaryTask();

    /*!
     * \return the menus specific to the major mode of the main frame, as
     * given by PlainTextEdit::getSpecificMenus, empty if the main frame has
     * no major mode providing such menus. The menus are owned by the major
     * mode: MainWindow only inserts them in the menu bar.
     */
    virtual std::vector<QMenu *> getSpecificMenus();

    /*!
     * \return the icon of the major mode of the main frame, a null icon if
     * the main frame has no major mode or if its major mode has no icon
     */
    virtual QIcon getIcon() const;

    /*!
     * \return the index of the tab showing the given secondary task in the
     * tab widget, -1 if the given widget is null or is not a secondary task
     * of this buffer
     *
     * \note a secondary task stored in a `QAbstractScrollArea` is displayed
     * through a `QAbstractScrollAreaWrapper`: the index of such a task is
     * found both with the wrapped widget and with the wrapper
     */
    int getSecondaryTaskIndex(QWidget *const p) const;

    /*!
     * \brief rebuild the tab widget of the secondary tasks from the registry
     * of the editor widget: the tabs are cleared, the tasks of this buffer
     * are added again, the tab widget is shown or hidden depending on whether
     * one of the tasks is visible and the current task is restored
     */
    virtual void refreshSecondaryTaskTabWidget();

    /*!
     * \brief attach a secondary task to the buffer.
     * \param[in] t: title
     * \param[in] w: associated widget
     *
     * \note the title is truncated to 8 characters in the tab; the task is
     * registered in the editor widget and a tab is added, selected and shown
     * only if this buffer is visible; the widget is parented to this buffer
     * and may be deleted by the editor widget when the task is detached and
     * no other buffer holds it anymore
     *
     * \return the widget inserted in the tab, which is a wrapper around the
     * given widget when the latter is a `QAbstractScrollArea`, nullptr if the
     * given widget is null
     */
    virtual QWidget *attachSecondaryTask(const QString &, QWidget *const);
    /*!
     * \brief attach an existing task to the buffer
     * \param[in] w: widget
     *
     * \note does nothing if the given widget is null or is already a
     * secondary task of this buffer; the task is added as a tab, selected and
     * shown, even if this buffer is not visible
     */
    virtual void attachSecondaryTask(QWidget *const);

    /*!
     * \brief remove a secondary task from the buffer: drop its tab, which may
     * be refused by the widget, and unregister it from the editor widget,
     * which deletes the widget when no other buffer holds it anymore
     * \param[in] s: widget of the task to remove
     *
     * \note does nothing if the given widget is null or is not a page of the
     * tab widget of this buffer; the tab widget is hidden once empty
     */
    virtual void removeSecondaryTask(QWidget *const);

    /*!
     * \return the title of the given secondary task, an empty string if the
     * given widget is not a secondary task of this buffer
     * \param[in] p: widget of the task
     */
    virtual QString getSecondaryTaskTitle(QWidget *const) const;

    /*!
     * \brief set the title of a secondary task, both in the registry of the
     * editor widget and in the tab, where it is truncated to 8 characters
     * \param[in] p: widget of the task
     * \param[in] n: new title
     *
     * \note does nothing if the given widget is not a secondary task of this
     * buffer
     */
    virtual void setSecondaryTaskTitle(QWidget *const, const QString &);

    /*!
     * \brief set the icon of a secondary task, both in the registry of the
     * editor widget, where all the buffers holding the task are refreshed,
     * and in the tab of this buffer
     * \param[in] p: widget of the task
     * \param[in] i: new icon
     *
     * \note does nothing if the given widget is not a secondary task of this
     * buffer
     */
    virtual void setSecondaryTaskIcon(QWidget *const, const QIcon &);

    /*!
     * \brief show the given secondary task: mark it visible in the registry
     * of the editor widget and show its page and the tab widget
     * \param[in] s: widget of the task
     *
     * \note does nothing if the given widget is null
     */
    virtual void showSecondaryTask(QWidget *const);

    /*!
     * \brief hide the given secondary task: mark it hidden in the registry of
     * the editor widget, hide its page and hide the tab widget if no visible
     * task is left
     * \param[in] s: widget of the task
     *
     * \note does nothing if the given widget is null
     */
    virtual void hideSecondaryTask(QWidget *const);

    //! \return true if this buffer has at least one secondary task
    virtual bool hasSecondaryTasks() const;

    //! \return true if the tab widget showing the secondary tasks is visible
    virtual bool areSecondaryTasksVisible() const;

    /*!
     * \return true if the document of the main frame is not modified, which
     * means this buffer can be closed without losing information; a buffer
     * reporting false triggers a prompt before being closed (see
     * EditorWidget::closeCurrentBuffer) or listed among the unsaved buffers
     * (see EditorWidget::isOkToClose)
     */
    virtual bool isOkToClose() const;

    /*!
     * \brief change the font of the main frame, of the labels of the
     * information line and of the tab bar of the secondary tasks
     * \param[in] f: new font
     *
     * \note those widgets are given the font of the text edits when the buffer
     * is built (see `setQLabelFontSizeAndMargins`) and are no children of one
     * another, so they do not follow a font change by themselves. Since the
     * information line is a plain horizontal layout, a label which keeps its
     * old font also keeps the height that font gives it, and the line, which
     * the vertical layout gives the height its labels ask for, grows to fit it:
     * a mode line whose font is not the one of the text edits is a mode line
     * taking the height of two.
     */
    virtual void changeFont(const QFont &);

    //! destructor
    ~Buffer() override;

   public slots:

    /*!
     * \brief show the tab widget of the secondary tasks, or display "no
     * SecondaryTask to be shown" in the minibuffer if this buffer has none
     */
    virtual void showSecondaryTasks();

    /*!
     * \brief set the orientation of the splitter between the main frame and
     * the secondary tasks
     * \param[in] o: orientation of the secondary tasks
     */
    virtual void setSecondaryTasksOrientation(const Qt::Orientation);

    //! \brief hide the tab widget of the secondary tasks
    virtual void hideSecondaryTasks();

    /*!
     * \brief give the focus to the current secondary task, do nothing if this
     * buffer has none
     */
    virtual void focusCurrentSecondaryTask();

    //! \brief give the focus to the main frame
    virtual void focusMainFrame();

    /*!
     * \brief close the current secondary task, do nothing if this buffer has
     * no secondary task
     */
    virtual void closeCurrentSecondaryTask();

    /*!
     * \brief emit the updatedMenu signal so that the menu bar is rebuilt;
     * called by major modes whose menus changed (see MFrontMajorMode)
     */
    virtual void updateMenu();

   signals:

    //! signal launched when the major mode of the main frame changed
    void mainFrameMajorModeChanged();
    //! signal launched when the menu of the buffer shall be updated
    void updatedMenu();
    /*!
     * \param[out] b : reference to this
     * \param[out] o : old name
     * \param[out] n : new name
     */
    void bufferNameChanged(Buffer *, const QString &, const QString &);

    /*!
     * \brief signal launched when this buffer starts visiting a file, used by
     * MainWindow to maintain the list of recent files
     * \param[in] f : complete name of the visited file
     */
    void newTreatedFile(const QString &);

   protected slots:

    /*!
     * \brief transfer the focus to the main frame when this buffer gets it
     * and the main frame does not already hold it
     * \param[in] ev : focus event, ignored
     */
    void focusInEvent(QFocusEvent *) override;

    /*!
     * \brief update the relative position label (percentage of lines of the
     * document before the cursor) and the absolute position label (line and
     * column of the cursor in the main frame)
     */
    virtual void updatePosition();

    /*!
     * \brief update the time label with the current time; called every
     * second by the timer and each time the cursor of the main frame moves
     */
    virtual void updateDate();

    /*!
     * \brief redraw the information line of this buffer: name in bold (html
     * escaped), positions, icon (hidden if the buffer has none), name of the
     * major mode (hidden if the main frame has none) and time
     */
    virtual void updateBufferInformations();

    /*!
     * \brief refresh the name of this buffer: the suffix which distinguishes
     * the buffers visiting files with the same name depends on all the
     * buffers, so the suffixes of all the buffers are re-chosen at once (see
     * EditorWidget::updateBufferNameSuffixes)
     */
    virtual void updateBufferName();

    /*!
     * \brief close the secondary task displayed in the tab of the given
     * index: drop the tab and unregister the task, which schedules its
     * deletion; the tab widget is hidden once empty
     * \param[in] i : index of the tab to close, nothing happens if the
     * corresponding page does not exist
     *
     * \note triggered by the close button of a tab
     */
    virtual void closeSecondaryTask(int);

    /*!
     * \brief forward the given file name to the newTreatedFile signal
     * \param[in] f : complete name of the visited file
     */
    virtual void emitNewTreatedFile(const QString &);

   protected:
    /*!
     * \brief tab widget in charge of displaying the secondary tasks, defined
     * in Buffer.cxx: its removeTab method only drops the tab if the page
     * accepts to close, and it hides itself once the last tab is removed
     */
    struct SecondaryTaskTabWidget;

    /*!
     * \brief build the widgets of this buffer (main frame, tab widget of the
     * secondary tasks, information line), connect the signals of the main
     * frame and of the tab widget and start the timer refreshing the date
     */
    void initialize();

    //! editor widget this buffer belongs to
    EditorWidget &editor;

    //! widget handling the main buffer and its SecondaryTasks
    QSplitter *splitter;
    //! SecondaryTask widgets
    SecondaryTaskTabWidget *stw;
    //! main widget
    PlainTextEdit *e;
    //! timer used to refresh the date of the information line every second
    QTimer *timer;
    //! display information about the current buffer
    QHBoxLayout *info;
    //! buffer name
    QLabel *bni;
    //! relative position
    QLabel *rpi;
    //! absolute position
    QLabel *api;
    //! icon
    QLabel *ii;
    //! mode name
    QLabel *mi;
    //! time
    QLabel *ti;
    //! buffer suffix
    QString bufferNameSuffix;
    //! buffer id: an integer chosen by the editor widget when this buffer is
    //! created, used to designate it in the buffers menu
    const int id;

   private:
    Q_OBJECT

    friend struct SecondaryTaskManager;

  };  // end of Buffer

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_BUFFER_HXX */
