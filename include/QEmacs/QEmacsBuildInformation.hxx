/*! 
 * \file  QEmacsBuildInformation.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 28 sept. 2012
 */

#ifndef _LIB_QEMACS_QEMACSBUILDINFORMATION_H_
#define _LIB_QEMACS_QEMACSBUILDINFORMATION_H_ 

#include<QtCore/QString>
#include"QEmacs/Config.hxx"

namespace qemacs
{


  struct QEMACS_VISIBILITY_EXPORT QEmacsBuildInformation
  {
    
    static const char *
    getInstallationDirectory();

    static const char *
    getDataDirectory();

    static const char *
    getVersionName(void);

    static const char *
    getVersionNumber(void);

    static const char *
    getVersionIdentifier(void);

    static const char *
    getBugReportAdress(void);

    static const char *
    getCompilerName(void);

    static const char *
    getBuildOs(void);

    static const char *
    getBuildCpu(void);

    static const char *
    getHostOs(void);

    static const char *
    getHostCpu(void);

    static const char *
    getTargetOs(void);

    static const char *
    getTargetCpu(void);

    static const char *
    getSVNRevision(void);

    static QString
    getHeader(const QString& = "");

  }; // end of struct BuildInformation

} // end of namespace qemacs

#endif /* _LIB_QEMACS_QEMACSBUILDINFORMATION_H */

