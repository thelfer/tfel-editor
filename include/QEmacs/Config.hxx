/*! 
 * \file   QEmacs/Config.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 21 juin 2013
 */

#ifndef _LIB_QEMACS_CONFIG_H_
#define _LIB_QEMACS_CONFIG_H_ 

#include"TFEL/Config/TFELConfig.hxx"

#if defined _WIN32 || defined _WIN64 ||defined __CYGWIN__
#  if defined TFELQEmacs_EXPORTS
#    define QEMACS_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#  else
#  if not defined TFEL_STATIC_BUILD
#    define QEMACS_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#  else 
#    define QEMACS_VISIBILITY_EXPORT 
#  endif
#  endif
#else
#  define QEMACS_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

#endif /* _LIB_QEMACS_CONFIG_H */

