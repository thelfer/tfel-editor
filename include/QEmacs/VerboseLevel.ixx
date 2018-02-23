/*!
 * \file   VerboseLevel.ixx
 * \brief    
 * \author Thomas Helfer
 * \date   22/02/2018
 */

#ifndef LIB_QEMACS_VERBOSELEVEL_IXX
#define LIB_QEMACS_VERBOSELEVEL_IXX

namespace qemacs {

  namespace internals {

    template <typename DebugStream, typename... T>
    void raw_debug(DebugStream&, T&&...);

    template <typename DebugStream>
    void raw_debug(DebugStream&) {}  // end of debug

    template <typename DebugStream, typename T1, typename... T>
    void raw_debug(DebugStream& s, T1&& a, T&&... args) {
      s << std::forward<T1>(a);
      raw_debug(s, std::forward<T>(args)...);
    }  // end of debug

  }  // end of internals

  template <typename... T>
  void debug(T&&... args) {
    if (getVerboseMode() >= VERBOSE_DEBUG) {
      auto s = qDebug();
      internals::raw_debug(s, args...);
    }
  }  // end of debug

}  // end of namespace qemacs

#endif /* LIB_QEMACS_VERBOSELEVEL_IXX */
