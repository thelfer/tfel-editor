/*!
 * \file  SpellChecker.cxx
 * \brief
 * \author Helfer Thomas
 * \date   02/10/2012
 */

/*
  Copyright (c) 2011, Volker Götz
  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
  1. Redistributions of source code must retain the above copyright
  notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice,
  this list of conditions and the following disclaimer in the
  documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.  */

#ifdef QEMACS_HUNSPELL_SUPPORT
#include "hunspell/hunspell.hxx"
#endif /* QEMACS_HUNSPELL_SUPPORT */

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>

#include <QtGui/QDesktopServices>

#include "QEmacs/SpellChecker.hxx"
#ifdef QEMACS_HUNSPELL_SUPPORT
#include "QEmacs/QEmacsHunspellDictionariesManager.hxx"
#endif /* QEMACS_HUNSPELL_SUPPORT */

namespace qemacs {

  SpellChecker::SpellChecker() : _hunspell(nullptr) {}

  void SpellChecker::setSpellCheckLanguage(const QString &l) {
#ifdef QEMACS_HUNSPELL_SUPPORT
    auto &dicts = QEmacsHunspellDictionariesManager::
        getQEmacsHunspellDictionariesManager();
    const auto dictionaryPath = dicts.getDictionaryPath(l);
    delete this->_hunspell;
    if (dictionaryPath.isEmpty()) {
      this->_hunspell = 0;
      return;
    }
    const auto datadir = [] {
      auto *const a = QCoreApplication::instance();
      if (a != nullptr) {
        return QStandardPaths::writableLocation(
                   QStandardPaths::GenericDataLocation) +
               "/data/" + a->organizationName() + "/" +
               a->applicationName() + "/qemacs";
      } else {
        return QStandardPaths::writableLocation(
                   QStandardPaths::GenericDataLocation) +
               "/data/CEA/qemacs";
      }
    }();
    bool dataDirExists = true;
    QDir d(datadir);
    if (!d.exists()) {
      if (!d.mkpath(datadir)) {
        dataDirExists = false;
      }
    }
    if (dataDirExists) {
      this->_userDictionary =
          datadir + QDir::separator() + l + ".dict.extensions";
      QFileInfo f(this->_userDictionary);
      if (f.exists()) {
        if (!f.isReadable()) {
          this->_userDictionary.clear();
        }
      } else {
        QFile file(this->_userDictionary);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
          this->_userDictionary.clear();
        }
      }
    } else {
      this->_userDictionary.clear();
    }
    QString dictFile = dictionaryPath + QDir::separator() + l + ".dic";
    QString affixFile = dictionaryPath + QDir::separator() + l + ".aff";
    QByteArray dictFilePathBA = dictFile.toLocal8Bit();
    QByteArray affixFilePathBA = affixFile.toLocal8Bit();
    this->_hunspell = new Hunspell(affixFilePathBA.constData(),
                                   dictFilePathBA.constData());
    // detect encoding analyzing the SET option in the affix
    // file
    this->_encoding = "ISO8859-1";
    QFile _affixFile(affixFile);
    if (_affixFile.open(QIODevice::ReadOnly)) {
      QTextStream stream(&_affixFile);
      QRegExp enc_detector("^\\s*SET\\s+([A-Z0-9\\-]+)\\s*",
                           Qt::CaseInsensitive);
      for (QString line = stream.readLine(); !line.isEmpty();
           line = stream.readLine()) {
        if (enc_detector.indexIn(line) > -1) {
          _encoding = enc_detector.cap(1);
          break;
        }
      }
      _affixFile.close();
    }
    this->_codec = QTextCodec::codecForName(
        this->_encoding.toLatin1().constData());
    if (!this->_userDictionary.isEmpty()) {
      QFile userDictonaryFile(this->_userDictionary);
      if (userDictonaryFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&userDictonaryFile);
        for (QString word = stream.readLine(); !word.isEmpty();
             word = stream.readLine())
          this->put_word(word);
        userDictonaryFile.close();
      } else {
        qWarning() << "User dictionary in " << this->_userDictionary
                   << "could not be opened";
      }
    } else {
      //      qDebug() << "User dictionary not set.";
    }
#else
    static_cast<void>(l);
#endif /* QEMACS_HUNSPELL_SUPPORT */
  }    // end of SpellChecker::setSpellCheckLanguage

  SpellChecker::~SpellChecker() {
#ifdef QEMACS_HUNSPELL_SUPPORT
    delete this->_hunspell;
#endif /* QEMACS_HUNSPELL_SUPPORT */
  }

  bool SpellChecker::spell(const QString &word) {
#ifdef QEMACS_HUNSPELL_SUPPORT
    // Encode from Unicode to the encoding used by current dictionary
    return this->_hunspell->spell(
               this->_codec->fromUnicode(word).constData()) != 0;
#else  /* QEMACS_HUNSPELL_SUPPORT */
    static_cast<void>(word);
    return true;
#endif /* QEMACS_HUNSPELL_SUPPORT */
  }

  QStringList SpellChecker::suggest(const QString &word) {
#ifdef QEMACS_HUNSPELL_SUPPORT
    char **suggestWordList;
    // Encode from Unicode to the encoding used by current dictionary
    int numSuggestions = this->_hunspell->suggest(
        &suggestWordList, this->_codec->fromUnicode(word).constData());
    QStringList suggestions;
    for (int i = 0; i < numSuggestions; ++i) {
      suggestions << this->_codec->toUnicode(suggestWordList[i]);
      free(suggestWordList[i]);
    }
    return suggestions;
#else  /* QEMACS_HUNSPELL_SUPPORT */
    static_cast<void>(word);
    return QStringList();
#endif /* QEMACS_HUNSPELL_SUPPORT */
  }

  QVector<std::pair<int, int>> SpellChecker::spellLine(
      const QString &l) {
    using namespace std;
    QVector<pair<int, int>> r;
    int pos = 0;
    while (pos != l.size()) {
      if (l[pos].isLetter()) {
        int npos = pos;
        ++pos;
        while ((pos != l.size()) && (l[pos].isLetter())) {
          ++pos;
        }
        if (!this->spell(l.mid(npos, pos - npos))) {
          r.push_back(make_pair(npos, pos));
        }
      } else {
        ++pos;
      }
    }
    return r;
  }  // end of SpellChecker::spellLine

  void SpellChecker::ignoreWord(const QString &word) {
    this->put_word(word);
  }  // end of SpellChecker::ignoreWord

  void SpellChecker::put_word(const QString &word) {
#ifdef QEMACS_HUNSPELL_SUPPORT
    this->_hunspell->add(_codec->fromUnicode(word).constData());
#else
    static_cast<void>(word);
#endif /* QEMACS_HUNSPELL_SUPPORT */
  }    // end of SpellChecker::put_word

  void SpellChecker::addToUserWordlist(const QString &word) {
#ifdef QEMACS_HUNSPELL_SUPPORT
    this->put_word(word);
    if (!this->_userDictionary.isEmpty()) {
      QFile userDictonaryFile(this->_userDictionary);
      if (userDictonaryFile.open(QIODevice::Append)) {
        QTextStream stream(&userDictonaryFile);
        stream << word << "\n";
        userDictonaryFile.close();
      } else {
        qWarning() << "User dictionary in " << this->_userDictionary
                   << "could not be opened for appending a new word";
      }
    } else {
      //      qDebug() << "User dictionary not set.";
    }
#else
    static_cast<void>(word);
#endif /* QEMACS_HUNSPELL_SUPPORT */
  }    // end of SpellChecker::addToUserWordlist

}  // end of namespace qemacs
