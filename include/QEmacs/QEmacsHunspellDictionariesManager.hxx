/*! 
 * \file  QEmacsHunspellDictionariesManager.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 oct. 2012
 */

#ifndef LIB_HUNSPELL_QEMACSDICTIONARIESMANAGER_HXX
#define LIB_HUNSPELL_QEMACSDICTIONARIESMANAGER_HXX 

#include<QtCore/QMap>
#include"QEmacs/Config.hxx"

namespace qemacs
{
  
  /*!
   * structure in charge of managing hunspell dictionaries
   */
  struct QEMACS_VISIBILITY_EXPORT QEmacsHunspellDictionariesManager
  {

    static QEmacsHunspellDictionariesManager&
    getQEmacsHunspellDictionariesManager();

    QString
    getDefaultSpellCheckLanguage() const;

    QString
    getDictionaryPath(const QString&) const;

    QStringList
    getAvailableDictionnaries() const;

  protected:

    QEmacsHunspellDictionariesManager();

    QEmacsHunspellDictionariesManager(const QEmacsHunspellDictionariesManager&);

    void
    searchDictionariesInDefaultLocations();

    void
    searchDictionaries(const QString&);

    QEmacsHunspellDictionariesManager&
    operator=(const QEmacsHunspellDictionariesManager&);

    // first  key : lang
    // second key : path to dictionaries
    QMap<QString,QString> dictionaries;

  }; // end of struct QEmacsHunspellDictionariesManager

} // end of namespace qemacs

#endif /* LIB_HUNSPELL_QEMACSDICTIONARIESMANAGER_HXX */

