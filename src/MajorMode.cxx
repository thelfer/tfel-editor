/*!
 * \file  MajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   30/06/2012
 */

#include <QtWidgets/QWidget>
#include "TFEL/GUI/MajorMode.hxx"

namespace tfel::gui {

  MajorMode::MajorMode(QWidget *const p) : QObject(p) {}  // end of MajorMode

  MajorMode::~MajorMode() = default;

}  // end of namespace tfel::gui
