/*! 
 * \file  ProcessInteractionFrame.cxx
 * \brief
 * \author Helfer Thomas
 * \date   24/10/2012
 */

#include"QEmacs/ProcessInteractionFrame.hxx"

namespace qemacs {

  ProcessInteractionFrame::ProcessInteractionFrame(QEmacsWidget& w,
                                                   QEmacsBuffer& b)
      : ProcessOutputFrame(w, b) {}
  // end of ProcessInteractionFrame::ProcessInteractionFrame

  ProcessInteractionFrame::~ProcessInteractionFrame() {
  }  // end of ProcessInteractionFrame::~ProcessInteractionFrame

} // end of namespace qemacs
