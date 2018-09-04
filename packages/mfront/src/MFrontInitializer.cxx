/*!
 * \file   MFrontInit.cxx
 * \brief    
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#include "MFront/InitDSLs.hxx"
#include "MFront/InitInterfaces.hxx"
#include "TFEL/GUI/MFrontInitializer.hxx"

namespace tfel{

  namespace gui{

  MFrontInitializer::MFrontInitializer() {
    mfront::initDSLs();
    mfront::initInterfaces();
  }  // end of MFrontInitializer::MFrontInitializer

  MFrontInitializer& MFrontInitializer::init() {
    static MFrontInitializer i;
    return i;
  }  // end of initMFront

}  // end of namespace gui
}// end of namespace tfel
