/*!
 * \file  BuildInformation.hxx
 * \brief access to the information gathered on the build of the editor.
 * \author Thomas Helfer
 * \date   28/09/2012
 */

#ifndef LIB_TFEL_GUI_BUILDINFORMATION_HXX
#define LIB_TFEL_GUI_BUILDINFORMATION_HXX

#include <QtCore/QString>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief access to the information gathered on the build of the
   * editor
   *
   * \note the values are compiled in `BuildInformation.cxx`, which is
   * generated from `src/BuildInformation.cxx.in` when configuring:
   * they are those of the CMake configuration, not those of the machine
   * running the editor
   *
   * \note the returned C strings are string literals: they are
   * statically allocated and shall not be freed
   */
  struct TFEL_GUI_VISIBILITY_EXPORT BuildInformation {
    //! \return the installation prefix (`CMAKE_INSTALL_PREFIX`)
    static const char* getInstallationDirectory();

    /*!
     * \return the directory where the language modes are installed, that is,
     * the `share/tfel-editor/packages` sub-directory of the installation
     * prefix
     *
     * \note this is the directory scanned by the executable to load the mode
     * plugins
     */
    static const char* getDataDirectory();

    //! \return the name of the project, `tfel-editor`
    static const char* getVersionName();

    //! \return the version number of the editor, such as `1.0-dev`
    static const char* getVersionNumber();

    /*!
     * \return the identifier of the version, which is the version number
     * itself
     */
    static const char* getVersionIdentifier();

    //! \return the address where bug reports shall be submitted
    static const char* getBugReportAdress();

    /*!
     * \return the C++ compiler used to build the editor, as given by the
     * `CXX` variable of CMake
     */
    static const char* getCompilerName();

    /*!
     * \return the operating system of the machine where the editor has
     * been built
     */
    static const char* getBuildOs();

    /*!
     * \return the cpu of the machine where the editor has been built
     */
    static const char* getBuildCpu();

    /*!
     * \return the operating system of the machine where the configuration
     * has been run
     */
    static const char* getHostOs();

    //! \return the cpu of the machine where the configuration has been run
    static const char* getHostCpu();

    //! \return the operating system the editor has been built for
    static const char* getTargetOs();

    //! \return the cpu the editor has been built for
    static const char* getTargetCpu();

    /*!
     * \return the subversion revision of the sources, which the template of
     * `BuildInformation.cxx` always fills with an empty string
     */
    static const char* getSVNRevision();

    /*!
     * \brief build a description of the build of the editor, holding one
     * information per line
     * \param[in] s : string repeated at the beginning of each line, which
     * can be used to indent the header
     *
     * \note the header gives the name and version of the editor, the
     * subversion revision, the compiler and the bug report address
     *
     * \note the target, build and host cpus and operating systems are
     * detailed line by line only when the target differs from the build
     * machine or from the host machine
     */
    static QString getHeader(const QString& = "");

  };  // end of struct BuildInformation

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_BUILDINFORMATION_HXX */
