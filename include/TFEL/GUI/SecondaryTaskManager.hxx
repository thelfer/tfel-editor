/*!
 * \file   SecondaryTaskManager.hxx
 * \brief
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

namespace tfel {

  namespace gui {

    // forward declaration
    struct Buffer;

    /*!
     * \brief structure in charge of managing secondary tasks.
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
      //! destructor
      virtual ~SecondaryTaskManager();
      /*!
       * \brief remove the given buffer
       * \param[in] b: buffer
       */
      virtual void removeBuffer(Buffer *const);
      /*!
       * \brief set the given task to be the current one. The
       * secondary task is identifed by the underlying widget.
       * \param[in] b: buffer
       * \param[in] w: widget
       */
      virtual void setCurrentSecondaryTask(Buffer *const, QWidget *const);
      /*!
       * \brief set the title of a given task.
       * \param[in] w: widget
       * \param[in] t: title
       */
      virtual void setSecondaryTaskTitle(QWidget *const, const QString &);
      /*!
       * \brief set the icon of a given task.
       * \param[in] w: widget
       * \param[in] i: icon
       */
      virtual void setSecondaryTaskIcon(QWidget *const, const QIcon &);
      /*!
       * \brief attach a secondary task to a buffer.
       * \param[in] b: buffer
       * \param[in] t: task
       */
      virtual void attachSecondaryTask(Buffer *const, const SecondaryTask &);
      /*!
       * \brief attach an existing secondary task to a buffer. The
       * secondary task is identifed by the underlying widget.
       * \param[in] b: buffer
       * \param[in] w: widget
       */
      virtual const SecondaryTask &attachSecondaryTask(Buffer *const,
                                                       QWidget *const);
      /*!
       * \brief change the state of an existing secondary task for a given
       * buffer.
       * \param[in] b: buffer
       * \param[in] w: widget
       */
      virtual void showSecondaryTask(Buffer *const, QWidget *const);
      /*!
       * \brief change the state of an existing secondary task for a given
       * buffer.
       * \param[in] b: buffer
       * \param[in] w: widget
       */
      virtual void hideSecondaryTask(Buffer *const, QWidget *const);
      /*!
       * \brief detach a secondary task from a buffer. The
       * secondary task is identifed by the underlying widget.
       * \param[in] b: buffer
       * \param[in] w: widget
       */
      virtual void detachSecondaryTask(Buffer *const, QWidget *const);
      /*!
       * \return the list of secondary tasks associated to a buffer
       * \param[in] b: buffer
       */
      virtual const std::vector<SecondaryTask> &getSecondaryTasks(
          Buffer *const);

     protected:
      //! a simple alias
      using map = std::map<Buffer *, std::vector<SecondaryTask>>;
      //! a simple alias
      using iterator = map::iterator;
      //! map associating a buffer to the list of its secondary tasks
      map bufferTasks;

    };  // end of struct SecondaryTaskManager

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_SECONDARYTASKMANAGER_HXX */
