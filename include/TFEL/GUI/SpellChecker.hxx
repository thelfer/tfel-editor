/*!
 * \file  SpellChecker.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 02 oct. 2012
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
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIB_TFEL_GUI_SPELLCHECKER_HXX
#define LIB_TFEL_GUI_SPELLCHECKER_HXX

#include <vector>
#include <utility>

#include <QtCore/QString>
#include <QtCore/QVector>

#include "TFEL/GUI/Config.hxx"

class Hunspell;
class QSyntaxHighlighter;

namespace tfel::gui {

  struct TFEL_GUI_VISIBILITY_EXPORT SpellChecker {
    SpellChecker();

    ~SpellChecker();

    bool spell(const QString &);

    std::vector<std::pair<int, int>> spellLine(const QString &);

    QStringList suggest(const QString &);

    void ignoreWord(const QString &);

    void setSpellCheckLanguage(const QString &l);

    void addToUserWordlist(const QString &);

   private:
    void put_word(const QString &);

    Hunspell *_hunspell;

    QString _userDictionary;

    QString _encoding;

    QTextCodec *_codec;
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SPELLCHECKER_HXX */
