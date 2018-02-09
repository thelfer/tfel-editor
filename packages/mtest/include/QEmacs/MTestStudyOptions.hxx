/*! 
 * \file  MTestStudyOptions.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 04 juin 2012
 */

#ifndef LIB_QEMACS_MTESTSTUDYOPTIONS_HXX
#define LIB_QEMACS_MTESTSTUDYOPTIONS_HXX 

#include<QtWidgets/QDialog>

namespace qemacs
{

  /*!
   * a set of options passed to the mtest
   */
  struct MTestStudyOptions
  {
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

  /*!
   * a set passed to the xmtest-client
   */
  class MTestStudyOptionsDialog
    : public QDialog
  {
    Q_OBJECT
  public:
    /*!
     * \param[in] options : options to be modified
     * \param[in] p       : parent
     */
    MTestStudyOptionsDialog(MTestStudyOptions &, QWidget *const);
  protected slots:
    void verboseLevelChanged(const QString&);
    void resultFile(bool);
    void xmlFile(bool);
    void fpeChecked(bool);
    void printBackTraceChecked(bool);
  protected:
    MTestStudyOptions& o;
  };

} // end of namespace qemacs

#endif /* LIB_QEMACS_MTESTSTUDYOPTIONS_H */

