/*!
 * \file  HunspellDictionariesManager.hxx
 * \brief
 * \author Helfer Thomas
 * \date   03/10/2012
 */

#ifndef LIB_HUNSPELL_DICTIONARIESMANAGER_HXX
#define LIB_HUNSPELL_DICTIONARIESMANAGER_HXX

#include <map>
#include "TFEL/GUI/Config.hxx"

namespace tfel{

  namespace gui{

  /*!
   * structure in charge of managing hunspell dictionaries
   */
  struct TFEL_GUI_VISIBILITY_EXPORT HunspellDictionariesManager {
    static HunspellDictionariesManager&
    getHunspellDictionariesManager();

    QString getDefaultSpellCheckLanguage() const;

    QString getDictionaryPath(const QString&) const;

    QStringList getAvailableDictionnaries() const;

   protected:
    HunspellDictionariesManager();

    HunspellDictionariesManager(
        const HunspellDictionariesManager&);

    void searchDictionariesInDefaultLocations();

    void searchDictionaries(const QString&);

    HunspellDictionariesManager& operator=(
        const HunspellDictionariesManager&);

    // first  key : lang
    // second key : path to dictionaries
    std::map<QString, QString> dictionaries;

  };  // end of struct HunspellDictionariesManager

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_HUNSPELL_DICTIONARIESMANAGER_HXX */
