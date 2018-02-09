/*!
 * \file   ProcessOutputMajorModeBase.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   7/02/2018
 */

#include"QEmacs/ProcessOutputMajorModeBase.hxx"

namespace qemacs{

  ProcessOutputMajorModeBase::ProcessOutputMajorModeBase(QEmacsWidget& w,
							 QEmacsBuffer& b,
							 QEmacsTextEditBase& t,
							 QWidget* const p)
      : QEmacsMajorModeBase(w, b, t, p) {}
  // end of ProcessOutputMajorModeBase::ProcessOutputMajorModeBase

  ProcessOutputMajorModeBase::~ProcessOutputMajorModeBase() = default;

}  // end of namespace qemacs
