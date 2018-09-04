/*!
 * \file   SecondaryTask.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#include"TFEL/GUI/SecondaryTask.hxx"

namespace tfel{

  namespace gui{

  SecondaryTask::SecondaryTask() = default;
  SecondaryTask::SecondaryTask(const SecondaryTask&) = default;
  SecondaryTask::SecondaryTask(SecondaryTask&&) = default;
  SecondaryTask& SecondaryTask::operator=(const SecondaryTask&) =
      default;
  SecondaryTask& SecondaryTask::operator=(SecondaryTask&&) = default;
  SecondaryTask::~SecondaryTask() = default;

  bool operator<(const SecondaryTask& a, const SecondaryTask& b) {
    return a.w < b.w;
  }  // end of operator<

}  // end of namespace gui
}// end of namespace tfel
