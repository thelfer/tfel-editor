/*!
 * \file  LaTeXSyntaxHighlighter.hxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#ifndef LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_HXX

#include <utility>
#include <vector>
#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>

namespace tfel::gui {

  // forward declaration
  struct LaTeXMajorMode;

  /*!
   * a syntaxt highlighter for LaTeX
   */
  struct LaTeXSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * a simple wrapper around the highligthing rule
     */
    struct HighlightingRule {
      QRegularExpression pattern;
      QVector<QTextCharFormat> format;
    };  // end of struct HighlightingRule

    //! a LaTeX command met in a line
    struct Command {
      //! match of the rule in the line
      QRegularExpressionMatch match;
      //! index of the rule which matched in getHighlightingRules()
      int rule = -1;
    };  // end of struct Command

    /*!
     * \brief the parts of a line which are not highlighted alike: a LaTeX line
     * holds a trailing comment, commands and plain text
     */
    struct Line {
      //! position of the '%' which opens the comment, -1 if the line has none
      int comment = -1;
      //! commands, in the order in which they appear
      std::vector<Command> commands;
      /*!
       * portions of the line which are neither a comment nor a command, as
       * pairs (start, length) of positions in the line. Those are the portions
       * holding the words to be spell checked.
       */
      std::vector<std::pair<int, int>> plainText;
    };  // end of struct Line

    static QString stripComment(const QString&);

    static int startOfComment(const QString&);

    //! list of highlighting rules
    const static QVector<HighlightingRule>& getHighlightingRules();

    //! format of the comments
    static const QTextCharFormat& getCommentFormat();

    /*!
     * \return the given line split in its comment, commands and plain text
     * \param[in] text : line
     *
     * \note The commands are matched by the rules returned by
     * getHighlightingRules(), the leftmost match winning and, among the
     * matches starting at the same position, the longest one: `\begin` is a
     * command but `\begin{equation}` is an environment.
     *
     * \note This is the single place where a line is split that way. The
     * context menu of the LaTeX major mode uses it too, so that the
     * highlighting and the spell checking always agree on what a misspelled
     * word may lie in.
     */
    static Line parseLine(const QString&);

    /*!
     * \param[in] m : latex mode
     * \param[in] t : text document to be highlighted
     */
    LaTeXSyntaxHighlighter(LaTeXMajorMode&, QTextDocument* const);

    //! \brief highlight the given line
    virtual void highlightBlock(const QString&) override;

   protected:
    /*!
     * \brief format a LaTeX command with the format of the rule which matched
     * it
     * \param[in] c : command
     * \param[in] r : rule which matched it
     *
     * A rule holding as many formats as it has capture groups, such as the one
     * matching `\begin{equation}`, gives each group its own format; the others
     * give their first format to the whole match.
     */
    void highLightCommand(const Command&, const HighlightingRule&);

    /*!
     * \brief underline the misspelled words of the given portion of text
     * \param[in] l : portion of text
     * \param[in] p : position of the beginning of that portion in the line
     */
    void highLightMispellWords(const QString&, const int);

    //! build the highlighting rules
    static QVector<HighlightingRule> buildHighlightingRules();

    LaTeXMajorMode& mode;

   private:
    Q_OBJECT

  };  // end of struct LaTeXSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_H */
