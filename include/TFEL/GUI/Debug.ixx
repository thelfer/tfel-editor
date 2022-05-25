/*!
 * \file   Debug.ixx
 * \brief
 * \author Thomas Helfer
 * \date   23/02/2018
 */

#ifndef LIB_TFEL_GUI_DEBUG_IXX
#define LIB_TFEL_GUI_DEBUG_IXX

#include <QtCore/QDebug>
#include "TFEL/GUI/VerboseLevel.hxx"

namespace tfel::gui::internals {

  template <typename DebugStream, typename... T>
  void raw_debug(DebugStream&, T&&...);

  template <typename DebugStream>
  void raw_debug(DebugStream&) {}  // end of debug

  template <typename DebugStream, typename T1, typename... T>
  void raw_debug(DebugStream& s, T1&& a, T&&... args) {
    s << std::forward<T1>(a);
    raw_debug(s, std::forward<T>(args)...);
  }  // end of debug

}  // namespace tfel::gui::internals

namespace tfel::gui {

  template <typename... T>
  void debug(T&&... args) {
    if (getVerboseMode() >= VERBOSE_DEBUG) {
      auto s = qDebug();
      internals::raw_debug(s, args...);
    }
  }  // end of debug

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_DEBUG_IXX */
