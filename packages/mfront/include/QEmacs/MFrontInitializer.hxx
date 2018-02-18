/*!
 * \file   MFrontInitializer.hxx
 * \brief
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_QEMACS_MFRONTINITIALIZER_HXX
#define LIB_QEMACS_MFRONTINITIALIZER_HXX

namespace qemacs {

  /*!
   * \brief class in charge of Initializerializing the MFront' DSL and
   * interfaces factories.
   */
  struct MFrontInitializer {
    static MFrontInitializer& init();
   private:
    MFrontInitializer();
    MFrontInitializer(MFrontInitializer&&) = delete;
    MFrontInitializer(const MFrontInitializer&) = delete;
    MFrontInitializer& operator=(MFrontInitializer&&) = delete;
    MFrontInitializer& operator=(const MFrontInitializer&) = delete;
  }; // end of struct MFrontInitializer

}  // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTINITIALIZER_HXX */
