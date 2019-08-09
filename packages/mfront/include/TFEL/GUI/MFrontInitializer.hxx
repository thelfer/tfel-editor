/*!
 * \file   MFrontInitializer.hxx
 * \brief
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_TFEL_GUI_MFRONTINITIALIZER_HXX
#define LIB_TFEL_GUI_MFRONTINITIALIZER_HXX

namespace tfel {

  namespace gui {

    /*!
     * \brief class in charge of initializing the MFront' DSL and
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
    };  // end of struct MFrontInitializer

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MFRONTINITIALIZER_HXX */
