/*!
 * \file   SecondaryTask.cxx
 * \brief
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#include "TFEL/GUI/SecondaryTask.hxx"

namespace tfel::gui {

  SecondaryTask::SecondaryTask() = default;
  SecondaryTask::SecondaryTask(const SecondaryTask&) = default;
  SecondaryTask::SecondaryTask(SecondaryTask&&) = default;
  SecondaryTask& SecondaryTask::operator=(const SecondaryTask&) = default;
  SecondaryTask& SecondaryTask::operator=(SecondaryTask&&) = default;
  SecondaryTask::~SecondaryTask() = default;

  bool operator<(const SecondaryTask& a, const SecondaryTask& b) {
    // the addresses, not the pointers: `w` is a QPointer, which has no order of
    // its own, and the widget which died names nothing, whatever the address it
    // wore when it was alive
    return a.w.data() < b.w.data();
  }  // end of operator<

}  // end of namespace tfel::gui
