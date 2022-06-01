/*!
 * \file  LicosStudyOptions.hxx
 * \brief
 * \author Helfer Thomas
 * \date   04/06/2012
 */

#ifndef LIB_TFEL_GUI_LICOSSTUDYOPTIONS_HXX
#define LIB_TFEL_GUI_LICOSSTUDYOPTIONS_HXX

#include <QtWidgets/QDialog>

namespace tfel::gui {

  /*!
   * a set passed to the xlicos-client
   */
  struct LicosStudyOptions {
    /*!
     * default constructor
     */
    LicosStudyOptions();
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
    /*!
     * Warning level.
     * Valid values are :
     * - level0
     * - level1
     * - level2
     * - debug
     * - full
     * Default value is "level1"
     */
    QString wlvl;
    /*!
     * allow missing dependencies.
     * Valid values are "yes" and "no".
     * Default value is "no"
     */
    QString allowMissingDependencies;
    /*!
     * floating point exceptions.
     * Valid values are "yes" and "no".
     * Default value is "no"
     */
    QString fpe;
    /*!
     * print backtrace.
     * Valid values are "yes" and "no".
     * Default value is "no"
     */
    QString printBackTrace;
    /*!
     * debug mode.
     * Valid values are "yes" and "no".
     * Default value is "no"
     */
    QString debugMode;
  };

  /*!
   * a set passed to the xlicos-client
   */
  struct LicosStudyOptionsDialog : public QDialog {
    /*!
     * \param[in] options : options to be modified
     * \param[in] p       : parent
     */
    LicosStudyOptionsDialog(LicosStudyOptions&, QWidget* const);
   protected slots:
    void verboseLevelChanged(const QString&);
    void warningLevelChanged(const QString&);
    void allowMissingDependenciesChecked(bool);
    void fpeChecked(bool);
    void printBackTraceChecked(bool);
    void debugModeChecked(bool);

   protected:
    LicosStudyOptions& o;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSSTUDYOPTIONS_H */
