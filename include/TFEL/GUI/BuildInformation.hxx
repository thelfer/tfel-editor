/*!
 * \file  BuildInformation.hxx
 * \brief
 * \author Helfer Thomas
 * \date   28/09/2012
 */

#ifndef LIB_TFEL_GUI_BUILDINFORMATION_HXX
#define LIB_TFEL_GUI_BUILDINFORMATION_HXX

#include <QtCore/QString>
#include "TFEL/GUI/Config.hxx"

namespace tfel{

  namespace gui{

  struct TFEL_GUI_VISIBILITY_EXPORT BuildInformation {
    static const char* getInstallationDirectory();

    static const char* getDataDirectory();

    static const char* getVersionName();

    static const char* getVersionNumber();

    static const char* getVersionIdentifier();

    static const char* getBugReportAdress();

    static const char* getCompilerName();

    static const char* getBuildOs();

    static const char* getBuildCpu();

    static const char* getHostOs();

    static const char* getHostCpu();

    static const char* getTargetOs();

    static const char* getTargetCpu();

    static const char* getSVNRevision();

    static QString getHeader(const QString& = "");

  };  // end of struct BuildInformation

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_BUILDINFORMATION_HXX */
