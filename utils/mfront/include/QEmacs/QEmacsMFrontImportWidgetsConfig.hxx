/*!
 * \file   QEmacsMFrontImportWidgetsConfig.hxx
 * \brief    
 * \author th202608
 * \date   12/03/2018
 */

#ifndef LIB_QEMACS_QEMACSMFRONTIMPORTWIDGETSCONFIG_HXX
#define LIB_QEMACS_QEMACSMFRONTIMPORTWIDGETSCONFIG_HXX

#include "QEmacs/Config.hxx"

#if defined _WIN32 || defined _WIN64 ||defined __CYGWIN__
#  if defined QEmacsMFrontImportWidgets_EXPORTS
#    define QEMACS_MIW_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#  else
#  if !defined TFEL_STATIC_BUILD
#    define QEMACS_MIW_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#  else 
#    define QEMACS_MIW_VISIBILITY_EXPORT 
#  endif
#  endif
#else
#  define QEMACS_MIW_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

#endif /* LIB_QEMACS_QEMACSMFRONTIMPORTWIDGETSCONFIG_HXX */
