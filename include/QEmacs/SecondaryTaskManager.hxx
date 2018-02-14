/*!
 * \file   SecondaryTaskManager.hxx
 * \brief
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#ifndef LIB_QEMACS_SECONDARYTASKMANAGER_HXX
#define LIB_QEMACS_SECONDARYTASKMANAGER_HXX

#include <map>
#include <vector>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>
#include "QEmacs/SecondaryTask.hxx"

namespace qemacs {

  // forward declaration
  struct QEmacsBuffer;

  /*!
   * \brief structure in charge of managing secondary tasks.
   */
  struct SecondaryTaskManager {
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
    //! destructor
    virtual ~SecondaryTaskManager();
    /*!
     * \brief remove the given buffer
     * \param[in] b: buffer
     */
    virtual void removeBuffer(const QEmacsBuffer *const);
    /*!
     * \brief set the given task to be the current one. The
     * secondary task is identifed by the underlying widget.
     * \param[in] b: buffer
     * \param[in] w: widget
     */
    virtual void setCurrentSecondaryTask(const QEmacsBuffer *const,
                                         QWidget *const);
    /*!
     * \brief set the title of a given task.
     * \param[in] w: widget
     * \param[in] t: title
     */
    virtual void setTaskTitle(QWidget *const,const QString&);
    /*!
     * \brief set the icon of a given task.
     * \param[in] w: widget
     * \param[in] i: icon
     */
    virtual void setTaskIcon(QWidget *const,const QIcon&);
    /*!
     * \brief attach a secondary task to a buffer.
     * \param[in] b: buffer
     * \param[in] t: task
     */
    virtual void attachSecondaryTask(const QEmacsBuffer *const,
                                     const SecondaryTask &);
    /*!
     * \brief attach an existing secondary task to a buffer. The
     * secondary task is identifed by the underlying widget.
     * \param[in] b: buffer
     * \param[in] w: widget
     */
    virtual void attachExistingSecondaryTask(const QEmacsBuffer *const,
                                             QWidget *const);
    /*!
     * \brief detach a secondary task from a buffer. The
     * secondary task is identifed by the underlying widget.
     * \param[in] b: buffer
     * \param[in] w: widget
     */
    virtual void detachSecondaryTask(const QEmacsBuffer *const,
                                     QWidget *const);
    /*!
     * \return the list of secondary tasks associated to a buffer
     * \param[in] b: buffer
     */
    virtual const std::vector<SecondaryTask> &getSecondaryTasks(
        const QEmacsBuffer *const);

   protected:
    //! a simple alias
    using map =
        std::map<const QEmacsBuffer *, std::vector<SecondaryTask>>;
    //! a simple alias
    using iterator = map::iterator;
    //! map associating a buffer to the list of its secondary tasks
    map m;

  };  // end of struct SecondaryTaskManager

}  // end of namespace qemacs

#endif /* LIB_QEMACS_SECONDARYTASKMANAGER_HXX */
