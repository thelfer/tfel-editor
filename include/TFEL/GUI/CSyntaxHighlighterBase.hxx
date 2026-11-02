/*!
 * \file  CSyntaxHighlighterBase.hxx
 * \brief base class of the syntax highlighters of the C-based languages.
 * \author Thomas Helfer
 * \date   03/07/2012
 */

#ifndef LIB_TFEL_GUI_CSYNTAXHIGHLIGHTERBASE_HXX
#define LIB_TFEL_GUI_CSYNTAXHIGHLIGHTERBASE_HXX

#include <string>
#include <vector>
#include <QtCore/QString>
#include <QtGui/QTextDocument>
#include <QtGui/QSyntaxHighlighter>

#include "TFEL/Utilities/CxxTokenizerOptions.hxx"
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief base class for language based on `C`
   * (such as C/C++/mfront/mtest/licos)
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CSyntaxHighlighterBase
      : public QSyntaxHighlighter {
    //! \brief flag indicating that an error occured while parsing previous
    //! lines
    static constexpr int invalidState = -2;
    //! \brief flag indicating that normal processing state
    static constexpr int defaultState = 0;
    //! \brief flag indicating that a C-style comment is opened
    static constexpr int cStyleCommentOpenedState = 1;
    //! \brief flag indicating that a raw-string comment is opened
    //! \note the state stored in the blocks is this value plus the identifier
    //! of the delimiter of the opened raw string, which getRawStringDelimiter
    //! translates back into that delimiter
    static constexpr int rawStringOpenedState = 2;
    //
    /*!
     * \brief highlight the tokens of a block of text
     * \param[in] text : text of the block to highlight, which is a line of
     * the document for the languages based on `C`
     *
     * \note called by Qt for each block of the document, on loading and
     * whenever the text of a block changes
     *
     * \note the block is tokenized with the `C++` tokenizer: strings,
     * characters, numbers, comments and preprocessor directives get the
     * corresponding format, the other tokens are highlighted only if their
     * value matches the key of a rule of highlightingRules
     *
     * \note the state of the previous block is handed back to the tokenizer,
     * so a comment or a raw string opened in a previous block is closed, and
     * highlighted, in this one
     *
     * \note the block is left unhighlighted and the state is set to
     * invalidState, which prevents the following blocks from being
     * highlighted, if the previous block is invalid or if the tokenizer
     * throws on the text of this block
     *
     * \note an include of a system header is highlighted as a string up to
     * the closing angular bracket
     */
    virtual void highlightBlock(const QString &) override;

   protected:
    //! \brief a simple wrapper around the highligthing rule
    struct HighlightingRule {
      //! value a token shall hold to be highlighted by this rule
      std::string key;
      //! format applied to the tokens whose value matches the key
      QTextCharFormat format;
    };  // end of struct HighlightingRule

    /*!
     * \brief constructor, which sets the formats used to highlight the
     * tokens and asks the tokenizer to keep the boundaries of the comments
     * \param[in] p : document highlighted by this highlighter
     *
     * \note the list of highlighting rules is left empty: it is up to the
     * derived classes to fill it with the keywords of their language
     *
     * \note as any `QSyntaxHighlighter`, the object registers itself on the
     * document and is thus owned by it, not by the mode using it
     */
    CSyntaxHighlighterBase(QTextDocument *const);

    //! list of highlighting rules
    std::vector<HighlightingRule> highlightingRules;

    //! format of the keywords: bold dark magenta
    QTextCharFormat keyFormat;
    //! format of the numbers: dark red
    QTextCharFormat numberFormat;
    //! format of the comments: dark red lightened by 30 percent
    QTextCharFormat commentFormat;
    //! format of the strings and of the characters: dark green
    QTextCharFormat stringFormat;
    //! format of the preprocessor directives: bold dark magenta
    QTextCharFormat preprocessorFormat;

    /*!
     * \brief options given to the `C++` tokenizer of TFEL; the only one this
     * class sets is `bKeepCommentBoundaries`, which it enables, so that the
     * boundaries of the C-style comments are kept in the comment tokens and get
     * the comment format
     */
    tfel::utilities::CxxTokenizerOptions options;
    //
    /*!
     * \return the raw string delimiter registered with the given identifier
     * \param[in] i : identifier, computed by getRawStringDelimiterId and
     * stored in the state of the block
     *
     * \note an empty string is returned if no delimiter was ever registered
     * with that identifier, which means that no raw string is opened
     */
    static const std::string &getRawStringDelimiter(const int);
    //
    /*!
     * \brief register a raw string delimiter if it is unknown
     * \param[in] n : delimiter of the raw string which is opened at the end
     * of the block
     * \return the identifier of the delimiter, which is its rank in the list
     * of the registered delimiters and is added to rawStringOpenedState to
     * build the state of the block
     */
    static int getRawStringDelimiterId(const std::string &);
    //
    /*!
     * \return the modifiable list of the registered raw string delimiters,
     * shared by all the highlighters of this family, initially empty
     */
    static std::vector<std::string> &getRawStringDelimiters();

   private:
    Q_OBJECT
  };  // end of struct CSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CSYNTAXHIGHLIGHTERBASE_HXX */
