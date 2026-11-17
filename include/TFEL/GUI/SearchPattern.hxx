/*!
 * \file  SearchPattern.hxx
 * \brief what a text is searched for with, and what a match of it is rewritten
 *        to.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#ifndef LIB_TFEL_GUI_SEARCHPATTERN_HXX
#define LIB_TFEL_GUI_SEARCHPATTERN_HXX

#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "TFEL/GUI/Config.hxx"

// forward declaration
class QTextDocument;

namespace tfel::gui {

  /*!
   * \brief the place a match of a search pattern holds in a document
   *
   * A match is a place of the document, never a text: whoever replaces a match
   * takes these two places and asks the pattern what goes there.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SearchMatch {
    //! place of the first character of the match, -1 when there is none
    int start = -1;
    //! one past the last character of the match, -1 when there is none
    int end = -1;
    /*!
     * \brief what the match holds, the whole match first and then the groups
     * of the regular expression, in the order its parentheses open
     *
     * \note a plain text search holds the whole match alone, so its answer
     * always has one entry
     *
     * \note an empty match, which a regular expression like `x*` answers, is
     * never returned, so `groups` always names at least one text
     */
    QStringList groups;
    //! \return true when there is no match at all
    bool isNull() const;
    //! \return the number of characters the match covers
    int size() const;
  };  // end of struct SearchMatch

  /*!
   * \brief a text or a regular expression looked for in a document, with the
   * options which decide what is found and the text a match is rewritten to
   *
   * This is the one place which knows the two questions the two ways of
   * searching for a text answer differently: `FindReplaceDialog` builds one to
   * search and replace from its dialog, `TextEditQueryReplace` and
   * `TextEditReplaceFilter` build one to walk the matches of a buffer from the
   * minibuffer.
   *
   * The places of the document are the places of the whole text of that
   * document, which is what lets a regular expression be matched against the
   * text and its answer be applied to the document without a conversion. That
   * holds because a block separator is one character of each and because no
   * block of the editor holds a character the whole text would write as more
   * than one, see `normalizeLineSeparators`.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT SearchPattern {
    /*!
     * \brief constructor
     * \param[in] p : text looked for, or the regular expression
     * \param[in] isRegexp : the argument is a regular expression
     * \param[in] withCase : an answer differs from the pattern by no letter
     * case
     * \param[in] wholeWordsOnly : a match stands between two characters which
     * are neither a letter nor a digit, nor at an end of the buffer
     * \param[in] with : the text a match is rewritten to
     *
     * \note a regular expression is compiled by the constructor: an invalid
     * one makes `isValid` answer false and `errorString` say what is wrong,
     * which is what the two users of this class report instead of searching.
     * A pattern of no text is valid and finds nothing, as an empty text is
     * what a search which asks for nothing answers
     *
     * \note the options of the expression are the multiline one, so `^` and
     * `$` name the ends of a line as they do in a buffer, and the unicode
     * properties one, so `\w` and a letter case mean what they mean in the
     * writing system of the buffer
     */
    SearchPattern(
        const QString& p,
        const bool isRegexp,
        const bool withCase,
        const bool wholeWordsOnly,
        const QString& with = QString());

    //! \return the text or the expression looked for
    const QString& pattern() const;
    //! \return true when the pattern is a regular expression
    bool isRegexp() const;
    //! \return true when a match must hold the letter case of the pattern
    bool isCaseSensitive() const;
    //! \return true when a match must stand between two word boundaries
    bool isWholeWords() const;
    //! \return true when a search with this pattern can be asked
    bool isValid() const;
    /*!
     * \return what is wrong with the pattern, an empty text when nothing is
     * wrong with it
     */
    QString errorString() const;

    //! \brief set the text a match is rewritten to
    void setReplacement(const QString&);
    //! \return the text a match is rewritten to
    const QString& replacement() const;
    /*!
     * \return the text the given match is rewritten to
     * \param[in] m : the match, whose groups a regular expression uses
     *
     * A plain text search answers the replacement as it was given: what a
     * match of it holds says nothing of what to write in its place.
     *
     * A regular expression reads the replacement for `\0`, the whole match,
     * and `\1` to `\9`, the groups of the expression, and writes the digit it
     * names there. A backslash before anything else stands for that character
     * alone, which is how a replacement writes a backslash it means literally.
     * A group the match does not answer, which an expression with an
     * alternative can leave unfilled, is written as nothing.
     */
    QString replacementOf(const SearchMatch&) const;

    /*!
     * \brief look for a match of the pattern in a document
     * \param[in] document : document searched
     * \param[in] from : place the search starts from: the first match whose
     * first character stands at or after it when searching forward, the last
     * match whose last character stands at or before it when searching
     * backward
     * \param[in] backward : search before that place instead of after it
     * \return the match, or an empty one when the document holds none
     *
     * Both readings of \a from leave the selection a text edit shows out of
     * the answer when that selection is itself a match, which is what lets the
     * two directions walk through the matches of a buffer without finding the
     * same one twice: the caller gives the end of the selection for the
     * forward search and its start for the backward one.
     *
     * \note the search does not wrap around: it is the caller which decides
     * what reaching an end of the document means, since the dialog says that
     * it searched from the other end and the query replace of a selection
     * stops there.
     */
    SearchMatch find(const QTextDocument& document,
                     const int from,
                     const bool backward = false) const;

    /*!
     * \brief look for a match which starts at a given place of a document
     * \param[in] document : document searched
     * \param[in] position : place the match must start at
     * \return the match, or an empty one when the pattern is satisfied
     * nowhere else than there
     *
     * This is the question of a command which replaces what the selection of a
     * buffer holds: the answer says whether that selection is a match of the
     * pattern and gives the groups the replacement is built from.
     */
    SearchMatch startingAt(const QTextDocument& document,
                           const int position) const;

   protected:
    /*!
     * \return true when the text of the document leaves a match of \a start
     * and \a end alone as a word of its own, which is the question the whole
     * words option asks
     *
     * \note a character which is neither a letter nor a number is a boundary,
     * the underscore included: that is the answer
     * `QTextDocument::FindWholeWords` gives, which this class has to give too
     * for a regular expression, whose search that option of Qt plays no part
     * in.
     */
    bool standsAlone(const QString& text, const int start, const int end) const;

    /*!
     * \return the match which starts at the given place of the whole text of a
     * document, or an empty one
     *
     * The pattern must be satisfied starting exactly there, which is the
     * question of `startingAt`.
     */
    SearchMatch matchStartingAt(const QString& text,
                                const int position) const;

    /*!
     * \return the first match at or after the given place of the whole text of
     * a document, or an empty one
     *
     * A match of no character, which a pattern like `x*` answers everywhere
     * without naming any text, and a match the whole words option rejects, are
     * both retried one character further, which is the way a search which is
     * not satisfied where it looked moves on.
     */
    SearchMatch findNextMatch(const QString& text,
                              const int position) const;

   protected:
    //! text or expression looked for
    const QString text;
    //! true when the text above is a regular expression
    const bool regexp;
    //! true when a match must hold the same letter case
    const bool caseSensitive;
    //! true when a match must stand between two word boundaries
    const bool wholeWords;
    //! text a match is rewritten to
    QString replaceWith;
    //! the compiled expression, valid when the pattern is a valid expression
    QRegularExpression expression;
    //! what is wrong with the expression, empty when nothing is
    QString error;
  };  // end of struct SearchPattern

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SEARCHPATTERN_HXX */
