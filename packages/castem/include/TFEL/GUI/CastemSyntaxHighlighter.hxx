/*!
 * \file  CastemSyntaxHighlighter.hxx
 * \brief a syntax highlighter for the input files of the Cast3M solver.
 * \author Thomas Helfer
 * \date   05/08/2012
 */

#ifndef LIB_CASTEMSYNTAXHIGHLIGHTER_HXX
#define LIB_CASTEMSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>

namespace tfel::gui {

  /*!
   * \brief a syntax highlighter for the `Cast3M` input files
   */
  struct CastemSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param[in] d : document to highlight
     *
     * The colours of the formats and the rules which match the quoted strings
     * and each keyword of \c CastemMajorMode::getKeysList are built here.
     *
     * \note the highlighter registers itself on the document, which owns it;
     * \c CastemMajorMode::setSyntaxHighlighter keeps a pointer to it
     */
    CastemSyntaxHighlighter(QTextDocument *);

    /*!
     * \brief highlight the given line
     * \param[in] text : line to highlight
     *
     * A line which starts with a '*' is a comment and nothing else is
     * searched for in it; the other lines are matched against the rules of
     * \c highlightingRules, from the first one to the last one, and what those
     * rules do not cover keeps the default format.
     *
     * \note an empty line is left untouched
     *
     * \note the rules are applied in the order they were built, the quoted
     * strings first and the keywords after them, so a keyword which stands
     * inside a quoted string keeps the colour of the keywords
     *
     * \note the characters past the 80th column of a line are highlighted as
     * being out of bounds, whatever the rules decided for them
     *
     * \note this is the entry point \c QSyntaxHighlighter calls for each line
     * it (re)highlights
     */
    virtual void highlightBlock(const QString &) override;

   protected:
    /*!
     * \brief a simple wrapper around the highligthing rule
     */
    struct HighlightingRule {
      //! the pattern which looks for the quotations and the keywords
      QRegularExpression pattern;
      //! the format applied to what the pattern matches
      QTextCharFormat format;
    };  // end of struct HighlightingRule

    //! list of highlighting rules
    QVector<HighlightingRule> highlightingRules;

    /*!
     * \brief the format of the keywords, blue; it is the format the rules of
     * \c highlightingRules apply to what they match
     */
    QTextCharFormat keyFormat;

    //! the format of the comment lines, red
    QTextCharFormat commentFormat;

    //! the format of the quoted strings, dark green
    QTextCharFormat quotationFormat;

    /*!
     * \brief the format of the characters past the 80th column, dark red;
     * `Cast3M` ignores those characters
     */
    QTextCharFormat outOfBoundsFormat;

  };  // end of struct CastemSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_CASTEMSYNTAXHIGHLIGHTER_H */
