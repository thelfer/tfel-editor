/*!
 * \file   TFEL/GUI/Config.hxx
 * \brief
 * \author Helfer Thomas
 * \date   21/06/2013
 */

#ifndef LIB_TFEL_GUI_CONFIG_HXX
#define LIB_TFEL_GUI_CONFIG_HXX

#include "TFEL/Config/TFELConfig.hxx"

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#if defined TFELEditor_EXPORTS
#define TFEL_GUI_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#else
#if !defined TFEL_STATIC_BUILD
#define TFEL_GUI_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#else
#define TFEL_GUI_VISIBILITY_EXPORT
#endif
#endif
#else
#define TFEL_GUI_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

#endif /* LIB_TFEL_GUI_CONFIG_HXX */
