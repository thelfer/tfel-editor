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
    /*!
     * default constructor
     */
    MFrontOptions();
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
    //! debug mode
    bool debug = false;
  };

  //! \brief a dialog used to choose the options passed to `MTest`
  struct MFrontOptionsDialog : public QDialog {
    /*!
     * \param[in] options : options to be modified
     * \param[in] p       : parent
     */
    MFrontOptionsDialog(MFrontOptions &, QWidget *const);

   protected:
    MFrontOptions& o;

   private:
    Q_OBJECT
  };

} // end of namespace qemacs

#endif /* LIB_QEMACS_MFRONTOPTIONS_H */

