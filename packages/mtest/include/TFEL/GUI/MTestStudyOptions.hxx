/*!
 * \file  MTestStudyOptions.hxx
 * \brief options passed to mtest and the dialog which edits them.
 * \author Thomas Helfer
 * \dat    04/06/2012
 */

#ifndef LIB_TFEL_GUI_MTESTSTUDYOPTIONS_HXX
#define LIB_TFEL_GUI_MTESTSTUDYOPTIONS_HXX

#include <QtWidgets/QDialog>

namespace tfel::gui {

  //! \brief a set of options passed to the mtest
  struct MTestStudyOptions {
    /*!
     * default constructor: the verbose level is set to "level1", the other
     * options keep the values declared in the class
     */
    MTestStudyOptions();
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
     *
     * \note this is the value given to the \c --verbose option of \c mtest
     */
    QString vlvl;
    /*!
     * output result file
     *
     * \note when this is false, \c --result-file-output=no is passed to
     * \c mtest; the option is simply omitted otherwise
     */
    bool res = true;
    /*!
     * output xml file
     *
     * \note \c --xml-output=true is passed to \c mtest when this is true
     */
    bool xml = false;
    /*!
     * floating point exceptions.
     *
     * \note the dialog collects this value but the study does not use it: no
     * option is passed to \c mtest for it
     */
    bool fpe = false;
    /*!
     * print a backtrace in case of trouble (SIGSEG or SIGFPE)
     *
     * \note the dialog collects this value but the study does not use it: no
     * option is passed to \c mtest for it
     */
    bool printBackTrace = false;
  };

  //! \brief a dialog used to choose the options passed to `MTest`
  struct MTestStudyOptionsDialog : public QDialog {
    /*!
     * \brief constructor
     * \param[in] options : options to be modified
     * \param[in] p       : parent
     *
     * \note the widgets are created with the values a default constructed
     * \c MTestStudyOptions holds, not with those of \c options, which the
     * widgets only update once the user acts on them
     *
     * \note the widgets are owned by the layouts of the dialog
     */
    MTestStudyOptionsDialog(MTestStudyOptions &, QWidget *const);
   protected slots:
    //! \brief set the verbose level to the entry the user has selected
    void verboseLevelChanged(const QString &);
    //! \brief set the option which controls the result file
    void resultFile(bool);
    //! \brief set the option which controls the xml output file
    void xmlFile(bool);
    //! \brief set the option which controls the floating point exceptions
    void fpeChecked(bool);
    //! \brief set the option which controls the backtrace printed on a crash
    void printBackTraceChecked(bool);

   protected:
    /*!
     * \brief options the dialog modifies
     *
     * \note the reference is not owned by the dialog: the caller must keep the
     * options alive until the dialog is closed
     */
    MTestStudyOptions &o;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MTESTSTUDYOPTIONS_H */
