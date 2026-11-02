/*!
 * \file  SpellChecker.hxx
 * \brief spell checker used by the major modes, based on hunspell.
 * \author Thomas Helfer
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

  /*!
   * \brief spell checker based on the `Hunspell` library
   *
   * \note the `hunspell` API is only used when the
   * `TFEL_GUI_HUNSPELL_SUPPORT` macro is defined, which CMake does when that
   * library is found; without it, all the queries are answered as if
   * everything was correctly spelled and no suggestion was available
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SpellChecker {
    /*!
     * \brief default constructor: no dictionary is associated with the
     * spell checker until setSpellCheckLanguage is called
     */
    SpellChecker();

    //! destructor, frees the dictionary the spell checker owns
    ~SpellChecker();

    /*!
     * \brief check the spelling of a word
     * \param[in] word : word to check
     * \return true if the word is correctly spelled, false otherwise
     *
     * \note a word is considered as correctly spelled when no dictionary
     * is associated with the spell checker, which is the common case on a
     * machine where no hunspell dictionary has been installed
     *
     * \note the word is handed to `hunspell` through `std::string`, which
     * holds UTF-8: the encoding of the dictionary, _encoding, is only used
     * to decode the suggestions and to encode the words added to the
     * dictionary
     */
    bool spell(const QString &);

    /*!
     * \brief check the spelling of all the words of a line
     * \param[in] l : line to check
     * \return one pair per mispelled word, holding the position of its
     * first letter and the position of the character which follows its
     * last letter, both being positions in the given line
     *
     * \note a word is a sequence of letters: digits, punctuation and
     * white spaces separate the words and are never reported
     *
     * \note the positions are Qt character positions in the string, not
     * byte offsets: the second value is one past the word, so the word is
     * `l.mid(first, second - first)`. Adding the position of the block in
     * the document to the first value gives an offset which can be given
     * to `QSyntaxHighlighter::setFormat`, as it is done to underline the
     * mispelled words
     */
    std::vector<std::pair<int, int>> spellLine(const QString &);

    /*!
     * \brief ask for suggestions replacing a mispelled word
     * \param[in] word : word the suggestions shall replace
     * \return the suggestions, in the order given by `hunspell`, which is
     * an empty list if no suggestion is found or if no dictionary is
     * associated with the spell checker
     */
    QStringList suggest(const QString &);

    /*!
     * \brief accept the given word for the rest of the session
     * \param[in] word : word to accept
     *
     * \note the word is only added to the loaded dictionary: it is not
     * written in the user dictionary file, so it is reported as mispelled
     * again once another language is selected
     */
    void ignoreWord(const QString &);

    /*!
     * \brief select the dictionary used to check the spelling
     * \param[in] l : name of the language, as given by
     * HunspellDictionariesManager::getAvailableDictionnaries
     *
     * \note the previously loaded dictionary is freed, even when no
     * dictionary is found for the given language, in which case every
     * word is considered as correctly spelled
     *
     * \note the encoding used to exchange words with `hunspell` is that
     * declared by the `SET` option of the affix file, `Latin1` otherwise
     *
     * \note the words stored in the user dictionary file of the writable
     * data location of the application are added to the loaded dictionary
     */
    void setSpellCheckLanguage(const QString &l);

    /*!
     * \brief add a word to the user word list
     * \param[in] word : word to accept
     *
     * \note the word is added to the loaded dictionary and appended to the
     * user dictionary file, so it stays accepted in the next sessions
     *
     * \note nothing is written if the user dictionary file could not be
     * created or opened, which happens when the data location of the
     * application is not writable
     */
    void addToUserWordlist(const QString &);

   private:
    /*!
     * \brief add a word to the loaded dictionary, doing nothing when no
     * dictionary is associated with the spell checker
     * \param[in] word : word to accept
     */
    void put_word(const QString &);

    /*!
     * \brief dictionary used to check the spelling, null as long as no
     * dictionary has been loaded for the current language
     *
     * \note the spell checker owns this object and deletes it when the
     * language changes or when the spell checker is destroyed
     */
    Hunspell *_hunspell;

    /*!
     * \brief path of the file where the words added to the user word list
     * are stored, one word per line, empty if such a file could not be
     * created or opened
     */
    QString _userDictionary;

    /*!
     * \brief encoding declared by the `SET` option of the affix file of
     * the loaded dictionary, `Latin1` when the affix file does not declare
     * any encoding
     *
     * \note only suggest and put_word use it, to decode the suggestions and
     * to encode the words they give to `hunspell`
     */
    QStringConverter::Encoding _encoding = QStringConverter::Latin1;
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SPELLCHECKER_HXX */
