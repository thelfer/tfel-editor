/*!
 * \file  QEmacsBuildInformation.hxx
 * \brief
 * \author Helfer Thomas
 * \date   28/09/2012
 */

#ifndef LIB_QEMACS_QEMACSBUILDINFORMATION_HXX
#define LIB_QEMACS_QEMACSBUILDINFORMATION_HXX

#include <QtCore/QString>
#include "QEmacs/Config.hxx"

namespace qemacs {

  struct QEMACS_VISIBILITY_EXPORT QEmacsBuildInformation {
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

}  // end of namespace qemacs

#endif /* LIB_QEMACS_QEMACSBUILDINFORMATION_HXX */
