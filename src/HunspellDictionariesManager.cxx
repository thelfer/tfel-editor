/*!
 * \file  HunspellDictionariesManager.cxx
 * \brief
 * \author Thomas Helfer
 * \date   03/10/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include "TFEL/GUI/HunspellDictionariesManager.hxx"

namespace tfel::gui {

  HunspellDictionariesManager&
  HunspellDictionariesManager::getHunspellDictionariesManager() {
    static HunspellDictionariesManager d;
    return d;
  }  // end of
     // HunspellDictionariesManager::getHunspellDictionariesManager

  QString HunspellDictionariesManager::getDefaultSpellCheckLanguage() const {
    QSettings s;
    if (s.contains("hunspell default dictionary")) {
      return s.value("hunspell default dictionary").toString();
    }
    QString dict = QLocale::system().name();
    if (this->dictionaries.count(dict) == 1) {
      return dict;
    }
    return "";
  }  // end of
     // HunspellDictionariesManager::getDefaultSpellCheckLanguage

  QString HunspellDictionariesManager::getDictionaryPath(
      const QString& l) const {
    if (this->dictionaries.count(l) != 0) {
      return this->dictionaries.at(l);
    }
    return "";
  }  // end of HunspellDictionariesManager::getDictionaryPath

  QStringList HunspellDictionariesManager::getAvailableDictionnaries() const {
    QStringList keys;
    for (const auto& d : this->dictionaries) {
      keys.append(d.first);
    }
    return keys;
  }  // end of
     // HunspellDictionariesManager::getAvailableDictionnaries

  HunspellDictionariesManager::HunspellDictionariesManager() {
    this->searchDictionariesInDefaultLocations();
  }  // end of
     // HunspellDictionariesManager::HunspellDictionariesManager

  void HunspellDictionariesManager::searchDictionariesInDefaultLocations() {
#ifdef Q_OS_UNIX
    //    this->searchDictionaries("/usr/share/myspell/dicts/");
    this->searchDictionaries("/usr/share/hunspell/");
#endif
  }  // end of
     // HunspellDictionariesManager::searchDictionariesInDefaultLocations

  void HunspellDictionariesManager::searchDictionaries(const QString& n) {
    QDir d(n);
    if (!d.exists()) {
      return;
    }
    const auto files = d.entryList(QStringList("*.dic"), QDir::Files);
    for (const auto& f : files) {
      QFileInfo fi(n + QDir::separator() + f);
      QString lang(fi.baseName());
      if (!fi.isReadable()) {
        continue;
      }
      QFileInfo af(n + QDir::separator() + fi.baseName() + ".aff");
      if ((!af.exists()) || (!af.isReadable())) {
        continue;
      }
      this->dictionaries[lang] = n;
    }
  }  // end of HunspellDictionariesManager::searchDictionaries

}  // end of namespace tfel::gui
