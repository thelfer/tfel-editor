/*! 
 * \file  ProcessInteractionFrame.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 24 oct. 2012
 */

#include"QEmacs/ProcessInteractionFrame.hxx"

namespace qemacs
{

  ProcessInteractionFrame::ProcessInteractionFrame(QEmacsWidget& w,
                                                   QEmacsBuffer& b)
      : ProcessOutputFrame(w, b) {
  }  // end of ProcessInteractionFrame::ProcessInteractionFrame

} // end of namespace qemacs
