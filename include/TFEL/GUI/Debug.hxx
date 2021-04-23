/*!
 * \file   Debug.hxx
 * \brief
 * \author Thomas Helfer
 * \date   23/02/2018
 */

#ifndef LIB_TFEL_GUI_DEBUG_HXX
#define LIB_TFEL_GUI_DEBUG_HXX

namespace tfel {

  namespace gui {

    /*!
     * \brief send arguments to the debug stream if the verbose level is
     * greater than `VERBOSE_DEBUG`.
     */
    template <typename... T>
    void debug(T&&...);

  }  // end of namespace gui
}  // end of namespace tfel

#include "TFEL/GUI/Debug.ixx"

#endif /* LIB_TFEL_GUI_DEBUG_HXX */
