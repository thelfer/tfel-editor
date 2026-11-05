/*!
 * \file  ConfigurationDialog.hxx
 * \brief dialog which lists the entries of the configuration, by theme, and
 *        writes what the user changes.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#ifndef LIB_TFEL_GUI_CONFIGURATIONDIALOG_HXX
#define LIB_TFEL_GUI_CONFIGURATIONDIALOG_HXX

#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtWidgets/QDialog>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SettingsKey.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief the dialog of the configuration: a list of themes on the left, the
   * entries of the chosen theme on the right, each one edited with the widget
   * which suits the kind of value it holds
   *
   * The dialog shows what \c SettingsRegistry declares, which is what the
   * libraries of the editor and their major modes said they read: an address
   * which was written by hand in the configuration file, and an address nothing
   * reads, are both left out on purpose.
   *
   * \note an address the code writes as it works, such as the history of a
   * prompt or the list of recent files, is left out as well: what is shown here
   * is what a person chooses, not what the editor remembers of its own account,
   * since such a field would be stale and rewritten by the next prompt.
   *
   * \note the dialog writes when it is accepted, not while the user edits,
   * since most of the entries are read once, when the widgets which use them
   * are built, and would answer nothing to someone who changes them one by one.
   * The few which can be applied live are applied by \c accept.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT ConfigurationDialog : public QDialog {
    /*!
     * \brief constructor: builds one page per theme of the configuration and
     * fills it with the values which are stored, or with the default value of
     * the entries which were never written \param[in] w : editor the dialog is
     * opened on, which is what an entry applied live is applied to \param[in] p
     * : parent widget
     */
    ConfigurationDialog(EditorWidget &w, QWidget *const = nullptr);
    /*!
     * \return the addresses of the entries the dialog shows, in the order of
     * the themes which hold them
     */
    QStringList keys() const;
    /*!
     * \return true if the dialog shows the entry of the given address
     * \param[in] key : address of the entry in `QSettings`
     */
    bool hasSetting(const QString &key) const;
    /*!
     * \return the value the widget of an entry holds, an invalid value when the
     * dialog shows no such entry
     * \param[in] key : address of the entry in `QSettings`
     */
    QVariant value(const QString &key) const;
    /*!
     * \brief set the value the widget of an entry holds, without writing
     * anything \param[in] key : address of the entry in `QSettings` \param[in]
     * v : value
     *
     * \note an unknown address, and a value of another kind than the entry
     * holds, are left alone: the dialog does not invent an entry nor convert a
     * value the user could not have typed into that widget
     */
    void setValue(const QString &key, const QVariant &v);
    /*!
     * \brief store in `QSettings` every value which differs from what is
     * stored, and forget the addresses whose value is the default one \return
     * the addresses which were written
     *
     * \note writing an entry whose value is the default one would fill a
     * configuration file with what has nothing to say in it, so those addresses
     * are removed instead
     */
    QStringList apply();
    /*!
     * \brief set every widget back to the default value of its entry, which is
     * what accepting the dialog writes: the addresses which are stored
     *
     * \note nothing is written, so the dialog can be left after that without
     * any effect on the configuration
     */
    void restoreDefaults();

   protected:
    /*!
     * \brief write what the user changed, apply live the entries which can be,
     * report the outcome in the minibuffer of the editor and close the dialog
     */
    void accept() override;

    /*!
     * \brief one entry of the dialog: what it is, and the widget which edits it
     */
    struct Field {
      //! declaration of the entry
      SettingsKey key;
      //! widget which edits its value
      QWidget *widget;
    };

    /*!
     * \return the field of the given address, a null pointer when the dialog
     * shows no such entry
     * \param[in] key : address of the entry in `QSettings`
     */
    Field *findField(const QString &key);

    /*!
     * \return the value the widget of the given field holds
     * \param[in] f : field
     */
    QVariant valueOf(const Field &f) const;

    /*!
     * \brief hand the given value to the widget of the given field
     * \param[in] f : field
     * \param[in] v : value
     * \return false if the value is of another kind than the widget edits
     */
    bool setValueOf(Field &f, const QVariant &v);

    /*!
     * \brief apply the entries whose effect the user has to be able to see
     * under their hands, and tell them which of the changes waits for a new
     * buffer or for a restart \param[in] written : addresses which were just
     * written
     */
    void applyLiveEntries(const QStringList &written);

    //! editor the dialog is opened on
    EditorWidget &editor;

    //! entries the dialog shows, in the order of the themes which hold them
    QVector<Field> fields;

   private:
    Q_OBJECT
  };  // end of struct ConfigurationDialog

  /*!
   * \brief open the dialog of the configuration on the given editor, which is
   * what `Options > Configuration...` and the `configure-tfel-editor` command
   * do \param[in] w : editor the dialog is opened on
   *
   * \note the dialog is modal: the entries which are applied live are applied
   * when it is accepted, while the editor behind it waits
   */
  TFEL_GUI_VISIBILITY_EXPORT void configureTfelEditor(EditorWidget &w);

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CONFIGURATIONDIALOG_HXX */
