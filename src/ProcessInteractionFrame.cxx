/*! 
 * \file  ProcessInteractionFrame.cxx
 * \brief
 * \author Helfer Thomas
 * \date   24/10/2012
 */

#include"TFEL/GUI/ProcessInteractionFrame.hxx"

namespace tfel{

  namespace gui{

  ProcessInteractionFrame::ProcessInteractionFrame(EditorWidget& w,
                                                   Buffer& b)
      : ProcessOutputFrame(w, b) {}
  // end of ProcessInteractionFrame::ProcessInteractionFrame

  ProcessInteractionFrame::~ProcessInteractionFrame() {
  }  // end of ProcessInteractionFrame::~ProcessInteractionFrame

} // end of namespace gui
}// end of namespace tfel
