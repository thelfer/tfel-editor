/*!
 * \file  MTestStudyOptions.hxx
 * \brief
 * \author Helfer Thomas
 * \dat    04/06/2012
 */

#ifndef LIB_TFEL_GUI_MTESTSTUDYOPTIONS_HXX
#define LIB_TFEL_GUI_MTESTSTUDYOPTIONS_HXX

#include <QtWidgets/QDialog>

namespace tfel {

  namespace gui {

    //! \brief a set of options passed to the mtest
    struct MTestStudyOptions {
      /*!
       * default constructor
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
       */
      QString vlvl;
      //! output result file
      bool res = true;
      //! output xml file
      bool xml = false;
      //! floating point exceptions.
      bool fpe = false;
      //! print a backtrace in case of trouble (SIGSEG or SIGFPE)
      bool printBackTrace = false;
    };

    //! \brief a dialog used to choose the options passed to `MTest`
    struct MTestStudyOptionsDialog : public QDialog {
      /*!
       * \param[in] options : options to be modified
       * \param[in] p       : parent
       */
      MTestStudyOptionsDialog(MTestStudyOptions &, QWidget *const);
     protected slots:
      void verboseLevelChanged(const QString &);
      void resultFile(bool);
      void xmlFile(bool);
      void fpeChecked(bool);
      void printBackTraceChecked(bool);

     protected:
      MTestStudyOptions &o;

     private:
      Q_OBJECT
    };

  }  // end of namespace gui
}  // end of namespace tfel

#endif /* LIB_TFEL_GUI_MTESTSTUDYOPTIONS_H */
