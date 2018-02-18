/*! 
 * \file  MFrontOptions.hxx
 * \brief
 * \author Helfer Thomas
 * \dat    04/06/2012
 */

#ifndef LIB_QEMACS_MFRONTOPTIONS_HXX
#define LIB_QEMACS_MFRONTOPTIONS_HXX 

#include<QtWidgets/QDialog>

namespace qemacs {

  //! \brief a set of options passed to the mtest
  struct MFrontOptions {
    //! default constructor
    MFrontOptions();
    /*!
     * analysis type
     * Valid values are :
     * - Build
     * - Analyse
     * - Generate
     * Default value is "build"
     */
    QString analysis_type;
    /*!
     * optimisation level.
     * Valid values are :
     * - level0
     * - level1
     * - level2
     * Default value is "level1"
     */
    QString olvl;
    /*!
     * Verbose level.
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
    QString vlvl;
    //! interface to be used
    QString interface;
    //! build the libraries
    bool build = true;
    //! debug mode
    bool debug = false;
    //! enable additional warning 
    bool warning = false;
    //! pedantic checks
    bool pedantic = false;
  }; // end of MFrontOptions

  //! \brief a dialog used to choose the options passed to `MTest`
  struct MFrontOptionsDialog : public QDialog {
    //! type of material knowlegde treated
    enum MaterialKnowledgeType { MATERIALPROPERTY, BEHAVIOUR, MODEL };
    /*!
     * \param[in] o: options to be modified
     * \param[in] t: material knowledge type
     * \param[in] p: parent
     */
    MFrontOptionsDialog(MFrontOptions &,
                        const MaterialKnowledgeType,
                        QWidget *const);

   protected:
    MFrontOptions& opts;

   private:
    Q_OBJECT
  };

} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTOPTIONS_H */

