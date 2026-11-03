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
    // issue #69: the dictionary keys are the base names of the `*.dic` files,
    // which rarely equal `language_country` (`de` dictionaries are installed
    // for a `de_DE` locale, `en_US` for `en_GB`), so a single exact test
    // silently disables spell checking; fall back through the user interface
    // languages and the language alone before giving up
    const auto candidate = [this](const QString& l) {
      if (l.isEmpty()) {
        return QString();
      }
      if (this->dictionaries.count(l) == 1) {
        return l;
      }
      const auto p = l.indexOf('_');
      if ((p != -1) && (this->dictionaries.count(l.left(p)) == 1)) {
        return l.left(p);
      }
      return QString();
    };
    auto lang = candidate(QLocale::system().name());
    if (!lang.isEmpty()) {
      return lang;
    }
    for (const auto& l : QLocale::system().uiLanguages()) {
      // BCP 47 names (`fr-FR`) use `-`, the dictionary keys use `_`
      lang = candidate(QString(l).replace('-', '_'));
      if (!lang.isEmpty()) {
        return lang;
      }
    }
    // last installed dictionary: spell checking in an unexpected language
    // beats no spell checking at all
    if (!this->dictionaries.empty()) {
      return this->dictionaries.begin()->first;
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
