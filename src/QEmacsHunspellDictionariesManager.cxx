/*! 
 * \file  QEmacsHunspellDictionariesManager.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/10/2012
 */

#include<QtCore/QDebug>

#include<QtCore/QDir>
#include<QtCore/QLocale>
#include<QtCore/QSettings>
#include<QtCore/QStringList>

#include"QEmacs/QEmacsHunspellDictionariesManager.hxx"

namespace qemacs
{

  QEmacsHunspellDictionariesManager&
  QEmacsHunspellDictionariesManager::getQEmacsHunspellDictionariesManager()
  {
    static QEmacsHunspellDictionariesManager d;
    return d;
  } // end of QEmacsHunspellDictionariesManager::getQEmacsHunspellDictionariesManager

  QString
  QEmacsHunspellDictionariesManager::getDefaultSpellCheckLanguage() const
  {
    QSettings s;
    if(s.contains("hunspell default dictionary")){
      return s.value("hunspell default dictionary").toString();
    }
    QString dict = QLocale::system().name();
    if(this->dictionaries.contains(dict)){
      return dict;
    }
    return "";
  } // end of QEmacsHunspellDictionariesManager::getDefaultSpellCheckLanguage

  QString
  QEmacsHunspellDictionariesManager::getDictionaryPath(const QString& l) const
  {
    if(this->dictionaries.contains(l)){
      return this->dictionaries[l];
    }
    return "";
  } // end of QEmacsHunspellDictionariesManager::getDictionaryPath

  QStringList
  QEmacsHunspellDictionariesManager::getAvailableDictionnaries() const
  {
    return this->dictionaries.keys();
  } // end of QEmacsHunspellDictionariesManager::getAvailableDictionnaries

  QEmacsHunspellDictionariesManager::QEmacsHunspellDictionariesManager()
  {
    this->searchDictionariesInDefaultLocations();
  } // end of QEmacsHunspellDictionariesManager::QEmacsHunspellDictionariesManager

  void
  QEmacsHunspellDictionariesManager::searchDictionariesInDefaultLocations()
  {
#ifdef Q_OS_UNIX
//    this->searchDictionaries("/usr/share/myspell/dicts/");
    this->searchDictionaries("/usr/share/hunspell/");
#endif
  } // end of QEmacsHunspellDictionariesManager::searchDictionariesInDefaultLocations

  void QEmacsHunspellDictionariesManager::searchDictionaries(
      const QString& n) {
    QDir d(n);
    if(!d.exists()){
      return;
    }
    const auto files =
        d.entryList(QStringList("*.dic"), QDir::Files);
    for(const auto& f : files){
      QFileInfo fi(n+QDir::separator()+f);
      QString lang(fi.baseName());
      if(!fi.isReadable()){
        continue;
      }
      QFileInfo af(n+QDir::separator()+fi.baseName()+".aff");
      if ((!af.exists()) || (!af.isReadable())) {
        continue;
      }
      this->dictionaries[lang] = n;
    }
  } // end of QEmacsHunspellDictionariesManager::searchDictionaries
  
} // end of namespace qemacs
