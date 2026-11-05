/*!
 * \file  SettingsKey.hxx
 * \brief description of an entry of the configuration, and of the theme it
 *        belongs to.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#ifndef LIB_TFEL_GUI_SETTINGSKEY_HXX
#define LIB_TFEL_GUI_SETTINGSKEY_HXX

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief what one entry of the configuration is: the address it lives at in
   * `QSettings`, the name it is shown under, the kind of value it holds, what
   * it answers when nothing was stored, and a sentence which says what it is
   * for
   *
   * \note an entry is declared by the code which reads it, not by the dialog
   * which displays it: a declaration sitting next to the read it describes can
   * not silently drift out of date, and it is what lets a package say which
   * entries of the configuration it uses without the editor having to know
   * anything about that package
   *
   * \note the entries are declared, never discovered: `QSettings` answers the
   * addresses which were written to, which says nothing about the addresses the
   * editor reads and would show a typo in the configuration file as an entry of
   * the editor
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SettingsKey {
    //! \brief the kind of value an entry holds, which decides the widget it is
    //! edited with
    enum Type {
      //! a check box
      BOOLEAN,
      //! a spin box
      INTEGER,
      //! a one line edit
      STRING,
      //! a multi line edit, one entry per line
      STRING_LIST,
      //! a combo box
      CHOICE,
      //! a font button
      FONT
    };

    //! \brief one of the values a `CHOICE` entry offers
    struct Choice {
      //! \brief constructor
      //! \param[in] v : value stored for this choice
      //! \param[in] l : name the choice is offered under
      Choice(const QVariant &v, const QString &l);
      /*!
       * \return true if two choices offer the same value under the same name
       */
      bool operator==(const Choice &o) const;
      //! value stored for this choice
      QVariant value;
      //! name the choice is offered under
      QString label;
    };

    /*!
     * \brief builds a description of nothing, so that a description can be held
     * by value in the containers of this library
     */
    SettingsKey();
    /*!
     * \brief constructor
     * \param[in] k : address of the entry in `QSettings`
     * \param[in] l : name the entry is shown under
     * \param[in] d : sentence which says what the entry is for
     * \param[in] t : kind of value the entry holds
     * \param[in] v : value the entry answers when nothing was stored
     */
    SettingsKey(const QString &k,
                const QString &l,
                const QString &d,
                const Type t,
                const QVariant &v = QVariant());
    //! \return an entry edited with a check box
    static SettingsKey boolean(const QString &key,
                               const QString &label,
                               const QString &description,
                               const bool default_value);
    /*!
     * \return an entry edited with a spin box
     * \param[in] lowest,highest : bounds the spin box offers
     */
    static SettingsKey integer(const QString &key,
                               const QString &label,
                               const QString &description,
                               const int default_value,
                               const int lowest = 0,
                               const int highest = 1000000);
    //! \return an entry edited with a one line edit
    static SettingsKey text(const QString &key,
                            const QString &label,
                            const QString &description,
                            const QString &default_value = QString());
    /*!
     * \return an entry edited with a multi line edit, one entry of the list per
     * line
     */
    static SettingsKey stringList(
        const QString &key,
        const QString &label,
        const QString &description,
        const QStringList &default_value = QStringList());
    /*!
     * \return an entry edited with a combo box
     * \param[in] c : the values the combo box offers, the first one of them the
     * one a freshly written configuration file answers
     */
    static SettingsKey choice(const QString &key,
                              const QString &label,
                              const QString &description,
                              const QVector<Choice> &c,
                              const QVariant &default_value);
    /*!
     * \return an entry edited with a font button
     *
     * \note such an entry has no default value of its own: the fonts of the
     * editor follow the application font until a font is stored, which is what
     * the rest of the code does with the "font/textedit" address
     */
    static SettingsKey font(const QString &key,
                            const QString &label,
                            const QString &description);
    /*!
     * \brief say that this entry belongs to another theme than the one it is
     * declared in
     * \param[in] g : name of the theme
     *
     * \note an entry which names no theme belongs to the theme it is declared
     * in, which is the theme of the mode which declares it. Only an entry which
     * belongs to a theme of its own, such as the entries of a mode whose name
     * is the program it drives, has to say so
     */
    SettingsKey &inGroup(const QString &g);
    /*!
     * \return true if a value is stored at the address of this entry
     */
    bool isSet() const;
    /*!
     * \return the value stored at the address of this entry, or its default
     * value when nothing is stored
     */
    QVariant read() const;
    /*!
     * \brief store a value at the address of this entry
     * \param[in] v : value to store
     *
     * \note a value equal to the default value is not stored: the address is
     * forgotten instead, so that a configuration file holds what the user
     * changed and nothing else, and so that an entry which was never shown
     * keeps the reading of its default the code gives it
     */
    void write(const QVariant &v) const;
    /*!
     * \brief forget the value stored at the address of this entry, which
     * restores the default value
     */
    void reset() const;

    //! address of the entry in `QSettings`
    QString key;
    /*!
     * \brief theme this entry belongs to, empty when it belongs to the theme it
     * is declared in
     */
    QString group;
    //! name the entry is shown under
    QString label;
    //! sentence which says what the entry is for
    QString description;
    //! kind of value the entry holds
    Type type;
    //! value the entry answers when nothing was stored
    QVariant default_value;
    //! the values a `CHOICE` entry offers, empty otherwise
    QVector<Choice> choices;
    //! lowest value an `INTEGER` entry accepts
    int minimum;
    //! highest value an `INTEGER` entry accepts
    int maximum;
  };  // end of struct SettingsKey

  /*!
   * \brief a theme of the configuration: a name, a sentence which says what it
   * holds, the entries it is made of, an icon, and the major mode which
   * exported it
   *
   * \note the theme of an entry is what the configuration dialog puts on the
   * left of its pages and the user picks to narrow the search for a setting to
   * one corner of the editor: the editor as a whole, the language of the
   * underlines, or one of its major modes
   *
   * \note \c mode is what tells those two kinds apart: the themes of the editor
   * are always there, the themes of the modes come and go with the plugins
   * which are loaded, and a theme whose name is that of a program -- `clangd`,
   * `castem` -- says nothing of the kind unless its mode says it
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SettingsGroup {
    //! name of the theme, the name of the declaring mode when it is empty
    QString name;
    /*!
     * \brief name of the major mode which exported the theme, empty for a theme
     * of the editor itself
     *
     * \note \c StandardMajorModeProxy fills it, as it fills the name, the
     * description and the icon of a theme which names none of its own
     */
    QString mode;
    //! sentence which says what the theme holds, empty for the default one
    QString description;
    //! entries of the theme
    QVector<SettingsKey> keys;
    //! icon of the theme, the one of the declaring mode when it is null
    QIcon icon;
  };  // end of struct SettingsGroup

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SETTINGSKEY_HXX */
