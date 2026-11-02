/*!
 * \file   SecondaryTaskManager.hxx
 * \brief interface which manages the secondary tasks of a buffer.
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#ifndef LIB_TFEL_GUI_SECONDARYTASKMANAGER_HXX
#define LIB_TFEL_GUI_SECONDARYTASKMANAGER_HXX

#include <map>
#include <vector>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SecondaryTask.hxx"

namespace tfel::gui {

  // forward declaration
  struct Buffer;

  /*!
   * \brief structure in charge of managing secondary tasks.
   *
   * \note `EditorWidget` derives from this class, which is how the buffers
   * attach, show and detach their secondary tasks; every method has a default
   * implementation working on bufferTasks and an override shall call it to keep
   * that registry consistent, as `EditorWidget::removeBuffer` does
   *
   * \note a task is registered under the widget its tab displays: when a scroll
   * area is attached, `Buffer::attachSecondaryTask` registers the
   * `QAbstractScrollAreaWrapper` built around it, so all the methods taking a
   * widget accept either that wrapper or the wrapped scroll area
   *
   * \note the methods which are given a null buffer or widget, or a buffer
   * holding no such task, do nothing and report no failure
   *
   * \note all the methods manipulate widgets, so they shall be called from the
   * thread running the event loop
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SecondaryTaskManager {
    //! default constructor
    SecondaryTaskManager();
    //! copy constructor
    SecondaryTaskManager(const SecondaryTaskManager &);
    //! move constructor
    SecondaryTaskManager(SecondaryTaskManager &&);
    //! standard assignement
    SecondaryTaskManager &operator=(const SecondaryTaskManager &);
    //! move assignement
    SecondaryTaskManager &operator=(SecondaryTaskManager &&);
    /*!
     * \brief destructor
     *
     * \note the widgets of the tasks are not deleted, they belong to the Qt
     * widget hierarchy of the buffers
     */
    virtual ~SecondaryTaskManager();
    /*!
     * \brief remove the given buffer
     * \param[in] b: buffer
     *
     * \note the tasks of the buffer are unregistered and the widget of each of
     * those no other buffer holds is scheduled for deletion with `deleteLater`
     *
     * \note the tabs displaying the tasks are not removed: the buffer does it
     * itself, and it calls the implementation of this class, and not that of
     * the most derived class, in its destructor
     *
     * \note nothing is done if the buffer is null or holds no secondary task
     */
    virtual void removeBuffer(Buffer *const);
    /*!
     * \brief set the given task to be the current one. The
     * secondary task is identifed by the underlying widget.
     * \param[in] b: buffer
     * \param[in] w: widget
     *
     * \note all the other tasks of the buffer lose that status, so a successful
     * call leaves exactly one current task
     *
     * \note the tab widget of the buffer is not updated here: the current task
     * becomes the visible tab when that widget is refreshed
     *
     * \note nothing is done if the buffer holds no task or no task using the
     * given widget
     */
    virtual void setCurrentSecondaryTask(Buffer *const, QWidget *const);
    /*!
     * \brief set the title of a given task.
     * \param[in] w: widget
     * \param[in] t: title
     *
     * \note every buffer holding a task with that widget is updated, as two
     * buffers may share a same task
     *
     * \note the tab widget of an updated buffer which is visible is refreshed,
     * which rebuilds its tabs so that the new title is displayed; the widgets
     * of the tasks are not deleted by that refresh
     *
     * \note nothing is done if the widget is null
     */
    virtual void setSecondaryTaskTitle(QWidget *const, const QString &);
    /*!
     * \brief set the icon of a given task.
     * \param[in] w: widget
     * \param[in] i: icon
     *
     * \note the buffers and the tab widgets are handled as in
     * setSecondaryTaskTitle and nothing is done if the widget is null
     */
    virtual void setSecondaryTaskIcon(QWidget *const, const QIcon &);
    /*!
     * \brief attach a secondary task to a buffer.
     * \param[in] b: buffer
     * \param[in] t: task
     *
     * \note the attached task becomes the current one of the buffer
     *
     * \note if a task of that buffer already uses the same widget, that task is
     * left unchanged: its title and icon are not replaced by those of `t`
     *
     * \note the tab displaying the task is not created: this is up to the
     * caller, which is `Buffer::attachSecondaryTask`
     *
     * \note nothing is done if the buffer is null or if the task holds no
     * widget
     */
    virtual void attachSecondaryTask(Buffer *const, const SecondaryTask &);
    /*!
     * \brief attach an existing secondary task to a buffer. The
     * secondary task is identifed by the underlying widget.
     * \param[in] b: buffer
     * \param[in] w: widget
     * \return the task associated with the widget, which the caller uses to
     * create the tab displaying it
     *
     * \note a task held by another buffer is shared with this one, that is, two
     * buffers hold the very same task
     *
     * \note the task becomes the current one of the buffer
     *
     * \note if no buffer holds a task using that widget, which means that the
     * widget was never attached, a default constructed task, whose widget is
     * null, is returned; the reference then denotes a static object shared by
     * all such calls
     *
     * \note nothing is attached if the buffer or the widget is null
     */
    virtual const SecondaryTask &attachSecondaryTask(Buffer *const,
                                                     QWidget *const);
    /*!
     * \brief change the state of an existing secondary task for a given
     * buffer.
     * \param[in] b: buffer
     * \param[in] w: widget
     *
     * \note the task is marked as visible, which is what makes the tab widget
     * of the buffer shown when it is refreshed; neither the widget of the task
     * nor that tab widget is shown here, `Buffer::showSecondaryTask` does it
     *
     * \note nothing is done if the buffer or the widget is null or if that
     * buffer holds no task using the given widget
     */
    virtual void showSecondaryTask(Buffer *const, QWidget *const);
    /*!
     * \brief change the state of an existing secondary task for a given
     * buffer.
     * \param[in] b: buffer
     * \param[in] w: widget
     *
     * \note the task is marked as hidden, so the tab widget of the buffer is
     * hidden when it is refreshed if that task was the only visible one;
     * `Buffer::hideSecondaryTask` hides the widget of the task itself
     *
     * \note nothing is done if the buffer or the widget is null or if that
     * buffer holds no task using the given widget
     */
    virtual void hideSecondaryTask(Buffer *const, QWidget *const);
    /*!
     * \brief detach a secondary task from a buffer. The
     * secondary task is identifed by the underlying widget.
     * \param[in] b: buffer
     * \param[in] w: widget
     *
     * \note the entry of the buffer is removed when it holds no task anymore
     *
     * \note the widget is scheduled for deletion with `deleteLater` when no
     * other buffer holds a task using it: the caller shall have removed the tab
     * displaying it first, as that deferred deletion happens anyway
     *
     * \note nothing is done if the buffer or the widget is null, if the buffer
     * holds no task, or if the tasks of that buffer do not use that widget
     */
    virtual void detachSecondaryTask(Buffer *const, QWidget *const);
    /*!
     * \return the list of secondary tasks associated to a buffer, in the order
     * they were attached, which is a reference to a static empty list when that
     * buffer holds none, including when the buffer is null
     * \param[in] b: buffer
     *
     * \note the reference is invalidated by attaching or detaching a task to
     * that buffer or by removing it
     */
    virtual const std::vector<SecondaryTask> &getSecondaryTasks(Buffer *const);

   protected:
    //! a simple alias to the map from a buffer to its secondary tasks
    using map = std::map<Buffer *, std::vector<SecondaryTask>>;
    //! a simple alias to the iterator of that map, which no method uses
    using iterator = map::iterator;
    /*!
     * \brief map associating a buffer to the list of its secondary tasks
     *
     * \note an entry holding an empty list may exist: it is created as soon as
     * a task is looked up for a buffer and removed when the last task of that
     * buffer is detached or when the buffer is removed
     */
    map bufferTasks;

  };  // end of struct SecondaryTaskManager

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SECONDARYTASKMANAGER_HXX */
