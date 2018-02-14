/*!
 * \file   SecondaryTask.hxx
 * \brief
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#ifndef LIB_QEMACS_SECONDARYTASK_HXX
#define LIB_QEMACS_SECONDARYTASK_HXX

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtGui/QIcon>
#include "QEmacs/Config.hxx"

namespace qemacs {

  /*!
   * \brief structure in charge of describing a secondary task.
   */
  struct QEMACS_VISIBILITY_EXPORT SecondaryTask {
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
  };  // end of struct SecondaryTask

  /*!
   * \brief compare two secondary tasks
   * \param[in] a: lhs
   * \param[in] b: rhs
   */
  QEMACS_VISIBILITY_EXPORT bool operator<(const SecondaryTask &,
                                          const SecondaryTask &);

}  // end of namespace qemacs

#endif /* LIB_QEMACS_SECONDARYTASK_HXX */
