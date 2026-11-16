/*!
 * \file  MarkdownSyntaxHighlighter.hxx
 * \brief MarkdownSyntaxHighlighter: highlights headers and metadata.
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_HXX

#include <vector>

#include <QtGui/QSyntaxHighlighter>
#include "TFEL/Utilities/CxxTokenizerOptions.hxx"

namespace tfel::gui {

  // forward declaration
  struct MarkdownMajorMode;

  /*!
   * \brief a syntaxt highlighter for the Markdown language
   */
  struct MarkdownSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \param[in] m : calling mode
     * \param[in] t : text document to be highlighted
     */
    MarkdownSyntaxHighlighter(MarkdownMajorMode &, QTextDocument *const);
    /*!
     * \brief highlight current line
     * \param[in] l:current line
     */
    void highlightBlock(const QString &) override;
    /*!
     * \return the words of the given line the spell checker is asked about, as
     * pairs of the position of their first character and of the position which
     * follows them, both being positions in \c l
     * \param[in] l : line to scan
     *
     * A word is a token of the language which starts with a letter: the
     * tokenizer of TFEL is what splits the line of a note, the same way the
     * highlighter reads it, and a token of punctuation, of blanks, or which
     * opens with a digit is nothing to a dictionary.
     *
     * \note the returned positions are positions in the `QString`, not the
     * byte offsets the tokenizer answers with: the tokenizer works on the
     * UTF-8 encoding of the line, and on a line which holds one accented
     * character those two answers differ. The highlighter draws with them and
     * the context menu of the mode replaces a word with them, so both are
     * brought back to the units `QTextDocument` counts in here, in the one
     * place a Markdown line is split for the spell checker
     *
     * \note a line the tokenizer refuses is answered as holding no word: what
     * the highlighter does then is to restart its state from that line, which
     * draws nothing either
     */
    static std::vector<std::pair<int, int>> spellCheckedWords(const QString &l);
    //! destructor
    ~MarkdownSyntaxHighlighter() override;

   protected:
    /*!
     * \return the options the tokenizer of the line is built with, which the
     * constructor hands to the member of the same name
     *
     * \note a static, so that \c spellCheckedWords splits a line exactly as
     * \c highlightBlock does, from the same list of options and not from a copy
     * of it
     */
    static tfel::utilities::CxxTokenizerOptions tokenizingOptions();
    //! calling object, to share the same spell checker
    MarkdownMajorMode &mode;
    //! formatting of level 1 headers
    QTextCharFormat h1;
    //! formatting of level 2 headers
    QTextCharFormat h2;
    //! formatting of level 3 headers
    QTextCharFormat h3;
    //! formatting of level 4 (and above) headers
    QTextCharFormat h4;
    //! metadata
    QTextCharFormat md;
    //! parsing options
    tfel::utilities::CxxTokenizerOptions options;

   private:
    Q_OBJECT

  };  // end of struct MarkdownSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MFRONTSYNTAXHIGHLIGHTER_H */
