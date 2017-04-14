/*! 
 * \file  LicosStudyOptions.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 04 juin 2012
 */

#ifndef _LIB_QEMACS_LICOSSTUDYOPTIONS_H_
#define _LIB_QEMACS_LICOSSTUDYOPTIONS_H_ 

#ifdef QEMACS_QT4
#include<QtGui/QDialog>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QDialog>
#endif /* QEMACS_QT5 */

namespace qemacs
{

  /*!
   * a set passed to the xlicos-client
   */
  struct LicosStudyOptions
  {
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
  class LicosStudyOptionsDialog
    : public QDialog
  {
    Q_OBJECT
  public:
    /*!
     * \param[in] options : options to be modified
     * \param[in] p       : parent
     */
    LicosStudyOptionsDialog(LicosStudyOptions&,
			    QWidget * const);
  protected slots:
    void verboseLevelChanged(const QString&);
    void warningLevelChanged(const QString&);
    void allowMissingDependenciesChecked(bool);
    void fpeChecked(bool);
    void printBackTraceChecked(bool);
    void debugModeChecked(bool);
  protected:
    LicosStudyOptions& o;
  };

} // end of namespace qemacs

#endif /* _LIB_QEMACS_LICOSSTUDYOPTIONS_H */

