/*!
 * \file   Debug.hxx
 * \brief    
 * \author Thomas Helfer
 * \date   23/02/2018
 */

#ifndef LIB_QEMACS_DEBUG_HXX
#define LIB_QEMACS_DEBUG_HXX

namespace qemacs {

  /*!
   * \brief send arguments to the debug stream if the verbose level is
   * greater than `VERBOSE_DEBUG`.
   */
  template <typename... T>
  void debug(T&&...);

} // end of namespace qemacs

#include"QEmacs/Debug.ixx"

#endif /* LIB_QEMACS_DEBUG_HXX */
