/*!
 * \file   SecondaryTask.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   13/02/2018
 */

#include"QEmacs/SecondaryTask.hxx"

namespace qemacs{

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

}  // end of namespace qemacs
