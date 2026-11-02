/*!
 * \file  LaTeXSyntaxHighlighter.hxx
 * \brief LaTeXSyntaxHighlighter: a syntax highlighter for latex files.
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
   * \brief a syntax highlighter for LaTeX
   */
  struct LaTeXSyntaxHighlighter : public QSyntaxHighlighter {
    /*!
     * \brief a simple wrapper around the highligthing rule
     */
    struct HighlightingRule {
      //! the pattern which matches the commands the rule highlights
      QRegularExpression pattern;
      /*!
       * \brief the formats applied to the commands the pattern matches; one
       * per capture group, as the rules matching `\begin{equation}` and
       * `\end{equation}` do, or a single one for the whole match
       */
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

    /*!
     * \brief remove the comment of the given line
     * \param[in] l : line
     * \return the line without its comment, that is the part which stands
     * before the '%' which opens it
     *
     * \note the line is returned as it is when it holds no comment and an
     * empty string when it is a comment, that is when the '%' which opens it
     * is its first character
     */
    static QString stripComment(const QString&);

    /*!
     * \return the position of the '%' which opens the comment of the given
     * line, -1 when the line has none
     * \param[in] l : line
     *
     * \note a '%' preceded by an odd number of backslashes is escaped and
     * opens no comment; everything past the returned position is a comment,
     * whatever the highlighting rules may find in it
     */
    static int startOfComment(const QString&);

    /*!
     * \return the list of highlighting rules
     *
     * \note the rules are built once, on the first call, by
     * \c buildHighlightingRules, and shared by every highlighter of
     * every buffer
     */
    const static QVector<HighlightingRule>& getHighlightingRules();

    /*!
     * \return the format the comments are highlighted with, a red foreground
     *
     * \note a single format shared by every highlighter
     */
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
     * \brief constructor
     * \param[in] m : latex mode
     * \param[in] t : text document to be highlighted
     *
     * \note the highlighter registers itself on the document, which owns it;
     * the mode remembers it through \c MajorMode::setHighlighter and detaches
     * it from the document when the mode is destroyed
     */
    LaTeXSyntaxHighlighter(LaTeXMajorMode&, QTextDocument* const);

    /*!
     * \brief highlight the given line
     * \param[in] text : line to highlight
     *
     * The line is split as \c parseLine does: its comment is drawn with the
     * format of \c getCommentFormat, its commands with that of the rule which
     * matched them and the misspelled words of its plain text are underlined.
     *
     * \note this is the entry point \c QSyntaxHighlighter calls for each line
     * it (re)highlights
     */
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
     *
     * The words are looked up in the spell checker of the mode this
     * highlighter belongs to, the very same checker that context menu
     * suggests from.
     *
     * \note a misspelled word is only underlined, it is never modified; the
     * positions the checker reports are those in \c l and are shifted by \c p
     * to be applied to the whole line
     */
    void highLightMispellWords(const QString&, const int);

    /*!
     * \return the highlighting rules of the language: any command, and the
     * `\begin{...}` and `\end{...}` commands, whose keyword and environment
     * name are highlighted with distinct formats
     *
     * \note invalid patterns are dropped, so the returned list may be shorter
     * than the number of rules which are built here
     */
    static QVector<HighlightingRule> buildHighlightingRules();

    /*!
     * \brief the mode of the buffer the highlighted document belongs to, which
     * the highlighter asks for its spell checker
     *
     * \note the highlighter does not own the mode and refers to it, so the mode
     * must outlive the highlighter
     */
    LaTeXMajorMode& mode;

   private:
    Q_OBJECT

  };  // end of struct LaTeXSyntaxHighlighter

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LATEXSYNTAXHIGHLIGHTER_H */
