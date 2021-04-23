/*!
 * \file   VerboseLevel.hxx
 * \brief
 * \author Thomas Helfer
 * \date   22/02/2018
 */

#ifndef LIB_TFEL_GUI_VERBOSELEVEL_HXX
#define LIB_TFEL_GUI_VERBOSELEVEL_HXX

#include <QtCore/QDebug>
#include "TFEL/GUI/Config.hxx"

namespace tfel {

  namespace gui {

    /*!
     * \brief list the possible values for the logging facilities
     *        provided by the getVerboseMode/getLogStream functions
     */
    enum VerboseLevel {
      VERBOSE_QUIET = -1,  //<! no output
      VERBOSE_LEVEL0 = 0,  //<! minimal output
      VERBOSE_LEVEL1 = 1,  //<! a simpler output
      VERBOSE_LEVEL2 = 2,  //<! a much detailled output
      VERBOSE_LEVEL3 = 3,  //<! the finer level for standard user
      VERBOSE_DEBUG = 4,   //<! an output adapted for debugging
      VERBOSE_FULL = 5     //<! a very detailled output
    };                     // end of enum VerboseLevel

    //! \return the current verbose level
    TFEL_GUI_VISIBILITY_EXPORT VerboseLevel& getVerboseMode();

    /*!
     * \brief change the verbose level
     * \param[in] l: the new verbose level
     */
    TFEL_GUI_VISIBILITY_EXPORT void setVerboseMode(
        const VerboseLevel = VERBOSE_LEVEL0);

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_VERBOSELEVEL_HXX */
