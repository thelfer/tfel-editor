/*!
 * \file   MFrontInitializer.hxx
 * \brief MFrontInitializer: singleton initialising the MFront factories.
 * \author Thomas Helfer
 * \date   17/02/2018
 */

#ifndef LIB_TFEL_GUI_MFRONTINITIALIZER_HXX
#define LIB_TFEL_GUI_MFRONTINITIALIZER_HXX

namespace tfel::gui {

  /*!
   * \brief class in charge of initializing the MFront' DSL and
   * interfaces factories.
   */
  struct MFrontInitializer {
    /*!
     * \return the one and only instance of the class, which is created on the
     * first call
     *
     * \note this is the only way to use the class: the constructor is private
     * and the object holds no state, its sole purpose is to run the
     * initialisation once, as a static local variable is initialised on the
     * first passing
     */
    static MFrontInitializer& init();

   private:
    /*!
     * \brief constructor: register the DSLs and the interfaces of MFront in
     * their factories and turn the warnings of MFront off, so that running a
     * DSL does not print on the standard output of the editor
     */
    MFrontInitializer();
    //! \brief not implemented: the class is a singleton
    MFrontInitializer(MFrontInitializer&&) = delete;
    //! \brief not implemented: the class is a singleton
    MFrontInitializer(const MFrontInitializer&) = delete;
    //! \brief not implemented: the class is a singleton
    MFrontInitializer& operator=(MFrontInitializer&&) = delete;
    //! \brief not implemented: the class is a singleton
    MFrontInitializer& operator=(const MFrontInitializer&) = delete;
  };  // end of struct MFrontInitializer

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTINITIALIZER_HXX */
