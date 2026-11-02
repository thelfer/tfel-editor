/*!
 * \file  LicosStudyOptions.hxx
 * \brief options passed to xlicos-client and the dialog which edits them.
 * \author Thomas Helfer
 * \date   04/06/2012
 */

#ifndef LIB_TFEL_GUI_LICOSSTUDYOPTIONS_HXX
#define LIB_TFEL_GUI_LICOSSTUDYOPTIONS_HXX

#include <QtWidgets/QDialog>

namespace tfel::gui {

  /*!
   * a set passed to the xlicos-client
   *
   * The fields hold the strings the client expects:
   * LicosStudy::processInitialised sends them to it, one option at a time.
   * The constructor sets the default value of each field, as listed below
   */
  struct LicosStudyOptions {
    /*!
     * default constructor
     *
     * \note it gives every field the default value its own documentation
     * mentions
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
   * a dialog which edits the set of options passed to the xlicos-client: the
   * two levels are chosen in combo boxes, the remaining options are check
   * boxes, and the dialog ends with the usual accept and reject buttons
   *
   * \note the edited options are modified as soon as a widget is used, not
   * when the dialog is accepted: rejecting the dialog keeps the changes the
   * user has already made
   */
  struct LicosStudyOptionsDialog : public QDialog {
    /*!
     * \param[in] options : options to be modified
     * \param[in] p       : parent
     *
     * \note the options are referred to, not copied: see the note of the
     * class about the moment the modifications are actually applied
     *
     * \note the widgets are not synchronised with the current values of the
     * edited options: the combo boxes always open on "level1", the default of
     * a default constructed `LicosStudyOptions`, and the check boxes always
     * open unchecked
     */
    LicosStudyOptionsDialog(LicosStudyOptions&, QWidget* const);
   protected slots:
    /*!
     * \brief store the chosen verbose level in the edited options; connected
     * to the `currentTextChanged` signal of the verbose level combo box
     * \param[in] lvl : level chosen among the items of the combo box
     */
    void verboseLevelChanged(const QString&);
    /*!
     * \brief store the chosen warning level in the edited options; connected
     * to the `currentTextChanged` signal of the warning level combo box
     * \param[in] lvl : level chosen among the items of the combo box
     */
    void warningLevelChanged(const QString&);
    /*!
     * \brief store "yes" or "no" in the missing dependencies field of the
     * edited options; connected to the `toggled` signal of the check box
     * \param[in] b : state of the check box
     */
    void allowMissingDependenciesChecked(bool);
    /*!
     * \brief store "yes" or "no" in the floating point exceptions field of
     * the edited options; connected to the `toggled` signal of the check box
     * \param[in] b : state of the check box
     */
    void fpeChecked(bool);
    /*!
     * \brief store "yes" or "no" in the print back trace field of the edited
     * options; connected to the `toggled` signal of the check box labelled
     * "Launch gdb on crash"
     * \param[in] b : state of the check box
     */
    void printBackTraceChecked(bool);
    /*!
     * \brief store "yes" or "no" in the debug mode field of the edited
     * options; connected to the `toggled` signal of the check box
     * \param[in] b : state of the check box
     */
    void debugModeChecked(bool);

   protected:
    /*!
     * \brief options the dialog edits
     *
     * \note the dialog does not own them and never reads them: it only writes
     * the fields the user touches
     */
    LicosStudyOptions& o;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LICOSSTUDYOPTIONS_H */
