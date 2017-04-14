/*! 
 * \file  QEmacsMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#ifdef QEMACS_QT4
#include<QtGui/QWidget>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QWidget>
#endif /* QEMACS_QT5 */

#include"QEmacs/QEmacsMajorMode.hxx"

namespace qemacs{

  QEmacsMajorMode::QEmacsMajorMode(QWidget *const p)
    : QObject(p)
  {} // end of QEmacsMajorMode::QEmacsMajorMode
  
  QEmacsMajorMode::~QEmacsMajorMode()
  {} // end of QEmacsMajorMode::~QEmacsMajorMode()

} // end of namespace qemacs
