/*!
 * \file  HunspellDictionariesManager.hxx
 * \brief manager of the hunspell dictionaries available on the machine.
 * \author Thomas Helfer
 * \date   03/10/2012
 */

#ifndef LIB_HUNSPELL_DICTIONARIESMANAGER_HXX
#define LIB_HUNSPELL_DICTIONARIESMANAGER_HXX

#include <map>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * structure in charge of managing hunspell dictionaries
   *
   * \note the dictionaries are looked for once, when the unique instance
   * of this class is created
   */
  struct TFEL_GUI_VISIBILITY_EXPORT HunspellDictionariesManager {
    /*!
     * \return the unique instance of this class, created on the first
     * call
     */
    static HunspellDictionariesManager& getHunspellDictionariesManager();

    /*!
     * \return the language used for spell checking by default: the value
     * stored in the "hunspell default dictionary" entry of the settings if
     * it exists, else the first of the system locale name, the user
     * interface languages (`QLocale::uiLanguages`) and their language-only
     * forms (`fr_FR` -> `fr`) that has a dictionary, else the first
     * installed dictionary, or an empty string if none is installed
     *
     * \note the entry is written by the *Options ▸ Dictionaries* menu, so a
     * choice made in the interface is now permanent (issue #69)
     */
    QString getDefaultSpellCheckLanguage() const;

    /*!
     * \brief give the location of the files of a dictionary
     * \param[in] l : name of the language
     * \return the directory holding the `.dic` and `.aff` files of the
     * given language, an empty string if no dictionary has been found for
     * that language, in which case no spell checking is possible
     */
    QString getDictionaryPath(const QString&) const;

    /*!
     * \return the names of the found dictionaries, that is, the base names of
     * the `.dic` files, sorted by name
     */
    QStringList getAvailableDictionnaries() const;

   protected:
    /*!
     * \brief constructor, which searches the dictionaries in the default
     * locations
     */
    HunspellDictionariesManager();

    /*!
     * \brief copy constructor, declared to prevent copying the unique
     * instance of this class and never defined
     * \param[in] m : manager to copy
     */
    HunspellDictionariesManager(const HunspellDictionariesManager&);

    /*!
     * \brief search the dictionaries of the default locations, that is,
     * `/usr/share/hunspell/` on unix platforms; nothing is done on the
     * other platforms
     */
    void searchDictionariesInDefaultLocations();

    /*!
     * \brief declare every dictionary of a given directory
     * \param[in] n : directory to scan
     *
     * \note a dictionary is retained only if its `.dic` file is readable
     * and if a readable `.aff` file with the same base name exists; the
     * language name is the base name of the `.dic` file
     *
     * \note nothing is done if the directory does not exist
     */
    void searchDictionaries(const QString&);

    /*!
     * \brief assignment operator, declared to prevent assigning the unique
     * instance of this class and never defined
     * \param[in] m : manager to assign
     */
    HunspellDictionariesManager& operator=(const HunspellDictionariesManager&);

    // first  key : lang
    // second key : path to dictionaries
    /*!
     * \note this map actually has a single key, the name of the language,
     * whose value is the path to the dictionaries of that language
     */
    std::map<QString, QString> dictionaries;

  };  // end of struct HunspellDictionariesManager

}  // end of namespace tfel::gui

#endif /* LIB_HUNSPELL_DICTIONARIESMANAGER_HXX */
