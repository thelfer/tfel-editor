/*!
 * \file   VerboseLevel.cxx
 * \brief
 * \author Thomas Helfer
 * \date   22/02/2018
 */

#include "TFEL/GUI/VerboseLevel.hxx"

namespace tfel::gui {

  VerboseLevel& getVerboseMode() {
    static VerboseLevel verboseMode = VERBOSE_LEVEL1;
    return verboseMode;
  }  // end of getVerboseMode()

  void setVerboseMode(const VerboseLevel l) {
    getVerboseMode() = l;
  }  // end of setVerboseMode

}  // end of namespace tfel::gui
