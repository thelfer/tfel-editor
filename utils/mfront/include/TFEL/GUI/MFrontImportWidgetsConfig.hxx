/*!
 * \file   MFrontImportWidgetsConfig.hxx
 * \brief
 * \author Thomas Helfer
 * \date   12/03/2018
 */

#ifndef LIB_TFEL_GUI_MFRONTIMPORTWIDGETSCONFIG_HXX
#define LIB_TFEL_GUI_MFRONTIMPORTWIDGETSCONFIG_HXX

#include "TFEL/GUI/Config.hxx"

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#if defined TFELEditorMFrontImportWidgets_EXPORTS
#define TFEL_GUI_MIW_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#else
#if !defined TFEL_STATIC_BUILD
#define TFEL_GUI_MIW_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#else
#define TFEL_GUI_MIW_VISIBILITY_EXPORT
#endif
#endif
#else
#define TFEL_GUI_MIW_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

#endif /* LIB_TFEL_GUI_MFRONTIMPORTWIDGETSCONFIG_HXX */
