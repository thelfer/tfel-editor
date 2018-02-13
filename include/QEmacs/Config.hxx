/*! 
 * \file   QEmacs/Config.hxx
 * \brief
 * \author Helfer Thomas
 * \date   21/06/2013
 */

#ifndef LIB_QEMACS_CONFIG_HXX
#define LIB_QEMACS_CONFIG_HXX 

#include"TFEL/Config/TFELConfig.hxx"

#if defined _WIN32 || defined _WIN64 ||defined __CYGWIN__
#  if defined TFELQEmacs_EXPORTS
#    define QEMACS_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#  else
#  if !defined TFEL_STATIC_BUILD
#    define QEMACS_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#  else 
#    define QEMACS_VISIBILITY_EXPORT 
#  endif
#  endif
#else
#  define QEMACS_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

#endif /* LIB_QEMACS_CONFIG_HXX */
