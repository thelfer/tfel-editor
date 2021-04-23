/*!
 * \file   SecondaryTask.hxx
 * \brief
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#ifndef LIB_TFEL_GUI_SECONDARYTASK_HXX
#define LIB_TFEL_GUI_SECONDARYTASK_HXX

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtGui/QIcon>
#include "TFEL/GUI/Config.hxx"

namespace tfel {

  namespace gui {

    /*!
     * \brief structure in charge of describing a secondary task.
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
      //! destructor
      ~SecondaryTask();
      //! associated widget
      QWidget *w = nullptr;
      //! name of the secondary task
      QString title;
      //! icon associated with the task
      QIcon icon;
      //! is this task the current one for the buffer
      bool current = false;
      //! is this task the current one for the buffer
      bool visible = true;
    };  // end of struct SecondaryTask

    /*!
     * \brief compare two secondary tasks
     * \param[in] a: lhs
     * \param[in] b: rhs
     */
    TFEL_GUI_VISIBILITY_EXPORT bool operator<(const SecondaryTask &,
                                              const SecondaryTask &);

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_SECONDARYTASK_HXX */
