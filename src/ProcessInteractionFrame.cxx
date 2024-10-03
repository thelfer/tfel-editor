/*!
 * \file  ProcessInteractionFrame.cxx
 * \brief
 * \author Thomas Helfer
 * \date   24/10/2012
 */

#include "TFEL/GUI/ProcessInteractionFrame.hxx"

namespace tfel::gui {

  ProcessInteractionFrame::ProcessInteractionFrame(EditorWidget& w, Buffer& b)
      : ProcessOutputFrame(w, b) {}
  // end ProcessInteractionFrame

  ProcessInteractionFrame::~ProcessInteractionFrame() {
  }  // end ~ProcessInteractionFrame

}  // end of namespace tfel::gui
