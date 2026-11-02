/*!
 * \file   SecondaryTask.hxx
 * \brief description of a secondary task of a buffer.
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#ifndef LIB_TFEL_GUI_SECONDARYTASK_HXX
#define LIB_TFEL_GUI_SECONDARYTASK_HXX

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtGui/QIcon>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief structure in charge of describing a secondary task.
   *
   * \note a secondary task is a widget a buffer displays in the tab widget
   * shown besides its main editing area, such as the output of a compilation
   * or the content of a directory
   *
   * \note the widget of a task belongs to the Qt widget hierarchy: the tab
   * widget of the buffer displays it and the SecondaryTaskManager schedules it
   * for deletion with `deleteLater` once no buffer holds a task using it
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SecondaryTask {
    //! default constructor
    SecondaryTask();
    //! copy constructor
    SecondaryTask(const SecondaryTask &);
    //! move constructor
    SecondaryTask(SecondaryTask &&);
    //! standard assignement
    SecondaryTask &operator=(const SecondaryTask &);
    //! move assignement
    SecondaryTask &operator=(SecondaryTask &&);
    /*!
     * \brief destructor
     *
     * \note the associated widget is not deleted: it belongs to the Qt widget
     * hierarchy
     */
    ~SecondaryTask();
    /*!
     * \brief associated widget
     *
     * \note the widget identifies the task: the SecondaryTaskManager looks the
     * tasks up with it and accepts either the registered widget or, when that
     * widget is a `QAbstractScrollAreaWrapper`, the scroll area it wraps
     *
     * \note the widget is null for a default constructed task only, as a task
     * without widget cannot be attached to a buffer
     */
    QWidget *w = nullptr;
    //! name of the secondary task
    QString title;
    //! icon associated with the task
    QIcon icon;
    /*!
     * \brief is this task the current one for the buffer
     *
     * \note at most one task of a buffer is current: setting a task as the
     * current one removes that status from all the other tasks of that buffer
     */
    bool current = false;
    /*!
     * \brief if this task is not hidden, which is what decides that the tab
     * widget of the buffer is shown when it is refreshed
     *
     * \note `Buffer::hideSecondaryTask` hides the widget of the task as well;
     * a task is visible when it is created
     */
    bool visible = true;
  };  // end of struct SecondaryTask

  /*!
   * \brief compare two secondary tasks
   * \param[in] a: lhs
   * \param[in] b: rhs
   *
   * \note the tasks are ordered by address of their associated widget, so two
   * tasks holding the same widget are equivalent
   */
  TFEL_GUI_VISIBILITY_EXPORT bool operator<(const SecondaryTask &,
                                            const SecondaryTask &);

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SECONDARYTASK_HXX */
