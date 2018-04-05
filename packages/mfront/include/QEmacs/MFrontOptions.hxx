/*! 
 * \file  MFrontOptions.hxx
 * \brief
 * \author Helfer Thomas
 * \dat    04/06/2012
 */

#ifndef LIB_QEMACS_MFRONTOPTIONS_HXX
#define LIB_QEMACS_MFRONTOPTIONS_HXX

#include <QtCore/QString>

namespace qemacs {

  //! \brief a set of options passed to the mtest
  struct MFrontOptions {
    //! default constructor
    MFrontOptions();
    //! destructor
    ~MFrontOptions();
    //! dsl name
    QString dsl;
    //! path to the MFront file
    QString file;
    /*!
     * \brief analysis type
     * Valid values are :
     * - Build
     * - Analyse
     * - Generate
     * Default value is "build"
     */
    QString analysis_type = "Build";
    /*!
     * \brief optimisation level.
     * Valid values are :
     * - level0
     * - level1
     * - level2
     * Default value is "level1"
     */
    QString olvl = "level1";
    /*!
     * \brief verbose level.
     * Valid values are :
     * - quiet
     * - level0
     * - level1
     * - level2
     * - level3
     * - debug
     * - full
     * Default value is "level1"
     */
    QString vlvl = "level1";
    //! interface to be used
    QString i;
    //! build the libraries
    bool build = true;
    //! debug mode
    bool debug = false;
    //! enable additional warning 
    bool warning = false;
    //! pedantic checks
    bool pedantic = false;
    //! pedantic checks
    bool profiling = false;
    /* options for the Implicit DSL */
    bool compare_to_numerical_jacobian = false;
    QString numerical_jacobian_criterion;
    QString numerical_jacobian_perturbation;

  }; // end of MFrontOptions

} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTOPTIONS_H */

