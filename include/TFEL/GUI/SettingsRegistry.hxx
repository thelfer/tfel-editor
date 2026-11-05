/*!
 * \file  SettingsRegistry.hxx
 * \brief gathering of the entries of the configuration, by theme.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#ifndef LIB_TFEL_GUI_SETTINGSREGISTRY_HXX
#define LIB_TFEL_GUI_SETTINGSREGISTRY_HXX

#include <optional>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SettingsKey.hxx"

namespace tfel::gui {

  /*!
   * \brief declares a theme of the configuration and registers it, with the
   * entries it holds, in the settings registry
   *
   * \note a theme is usually declared by a static instance of this class,
   * defined at the bottom of the source file which holds the code that reads
   * the entries, so that loading the library which holds it is enough to make
   * those entries known to the configuration dialog. This is how the commands
   * of \c CommandFactory and the modes of \c MajorModeFactory are registered
   *
   * \note a major mode declares its entries through its own proxy, which
   * registers them under the name of the mode, or under a name of its own if
   * the entries are about a program the mode drives: see \c
   * MajorModeProxy::getSettingsGroups
   *
   * \note only what a person chooses is declared: an address the code writes as
   * it works, such as the history of a prompt, has no declaration, hence no
   * field of the dialog, see the note at the top of \c CoreSettings.cxx
   */
  struct TFEL_GUI_VISIBILITY_EXPORT StandardSettingsGroupProxy {
    /*!
     * \param[in] group : name of the theme, such as "General" or "clangd"
     * \param[in] what : sentence which says what the theme holds
     * \param[in] entries : entries of the theme. An entry which names no theme
     * belongs to \c group
     * \param[in] b : if true, register this declaration in the settings
     * registry. The constructor of a declaration which is itself a registration
     * copy must pass false not to register itself again
     */
    StandardSettingsGroupProxy(const QString &group,
                               const QString &what,
                               const QVector<SettingsKey> &entries,
                               const bool = true);
    //! \return the name of the theme
    QString getName() const;
    //! \return the sentence which says what the theme holds
    QString getDescription() const;
    //! \return the entries of the theme
    QVector<SettingsKey> getSettingsKeys() const;
    //! destructor
    ~StandardSettingsGroupProxy();

   private:
    //! theme name
    const QString name;
    //! what the theme holds
    const QString description;
    //! entries of the theme
    const QVector<SettingsKey> keys;
  };  // end of struct StandardSettingsGroupProxy

  /*!
   * \brief the themes of the configuration and the entries they hold, gathered
   * from what the libraries of the editor declared and from what its major
   * modes export
   *
   * \note the registry holds declarations, never values: the values are read
   * from and written to `QSettings` by the entries themselves, so that the
   * registry never has to be notified of a change made outside of the
   * configuration dialog
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SettingsRegistry {
    /*!
     * \return the unique instance of the registry
     *
     * \note the instance is created on the first call and destroyed when the
     * program ends
     */
    static SettingsRegistry &getSettingsRegistry();

    /*!
     * \brief register a theme and its entries
     * \param[in] group : theme to register
     *
     * The entries of a theme already registered under the same name are
     * appended to it, and an entry whose address is already declared is left
     * out: the first declaration of an address is the one which is shown, and
     * the clash is reported with \c qDebug only.
     *
     * \note a theme whose name is empty is registered under the name of its
     * first entry's address, which cannot happen for a declaration made through
     * \c StandardSettingsGroupProxy or by a major mode
     */
    void addSettingsGroup(const SettingsGroup &);

    /*!
     * \return the themes of the configuration: those the libraries of the
     * editor declared, in the order they were declared, followed by those the
     * major modes export, sorted by name
     *
     * \note the themes of the modes are gathered from \c MajorModeFactory on
     * every call, so the plugins loaded after the registry was built are part
     * of the answer, and a theme an entry of a mode named explicitly is created
     * even when no mode of that name is registered
     */
    QVector<SettingsGroup> getGroups() const;

    /*!
     * \return the names of the themes of the configuration, in the order
     * \c getGroups answers them
     */
    QStringList getGroupsNames() const;

    /*!
     * \return the entries of the given theme, empty when no such theme is
     * registered
     * \param[in] group : name of the theme
     */
    QVector<SettingsKey> getSettingsKeysOfGroup(const QString &group) const;

    /*!
     * \return every entry of the configuration, in the order of the themes
     * which hold them
     */
    QVector<SettingsKey> getSettingsKeys() const;

    /*!
     * \return true if an entry of the given address is declared
     * \param[in] key : address of the entry in `QSettings`
     */
    bool hasSettingsKey(const QString &key) const;

    /*!
     * \return the declaration of the entry of the given address, nothing when
     * no such entry is declared \param[in] key : address of the entry in
     * `QSettings`
     */
    std::optional<SettingsKey> getSettingsKey(const QString &key) const;

   private:
    /*!
     * \brief builds an empty registry
     *
     * \note only `getSettingsRegistry` may build the single registry of the
     * application: the themes are registered by the static initialisers of the
     * `StandardSettingsGroupProxy` objects the libraries declare.
     */
    SettingsRegistry();
    //! destructor
    ~SettingsRegistry();

    /*!
     * \brief append a theme to the given list, or the entries of the theme to
     * the theme of the list which already bears that name
     * \param[in] group : theme to append
     * \param[in] g : list of themes to append it to
     *
     * \note the names are compared case insensitively: a mode named `bash` and
     * a theme named `Bash` are the same corner of the editor
     */
    static void appendGroup(const SettingsGroup &, QVector<SettingsGroup> &);

    /*!
     * \return the declaration of the given address among the given themes, a
     * null pointer when none of them holds it
     * \param[in] key : address of the entry in `QSettings`
     * \param[in] g : themes to look into
     */
    static const SettingsKey *findKey(const QString &key,
                                      const QVector<SettingsGroup> &g);

    //! themes the libraries of the editor declared, in the order of declaration
    QVector<SettingsGroup> groups;
  };  // end of struct SettingsRegistry

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SETTINGSREGISTRY_HXX */
