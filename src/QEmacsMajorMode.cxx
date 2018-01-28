/*! 
 * \file  QEmacsMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 30 juin 2012
 */

#include<QtWidgets/QWidget>
#include"QEmacs/QEmacsMajorMode.hxx"

namespace qemacs{

  QEmacsMajorMode::QEmacsMajorMode(QWidget *const p)
    : QObject(p)
  {} // end of QEmacsMajorMode::QEmacsMajorMode
  
  QEmacsMajorMode::~QEmacsMajorMode() = default;

} // end of namespace qemacs
