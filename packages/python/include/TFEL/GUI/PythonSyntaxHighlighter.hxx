/*!
 * \file  PythonSyntaxHighlighter.hxx
 * \brief syntax highlighter of the Python language.
 * \author Thomas Helfer
 * \date   14/11/2026
 */

#ifndef LIB_TFEL_GUI_PYTHONSYNTAXHIGHLIGHTER_HXX
#define LIB_TFEL_GUI_PYTHONSYNTAXHIGHLIGHTER_HXX

#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>
#include <QtGui/QSyntaxHighlighter>
#include <QtGui/QTextCharFormat>

#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief a syntax highlighter for the Python language
   *
   * \note the words of the language are taken one by one and dispatched by
   * name, which is what a language with a single comment character and no
   * declaration syntax asks for: the tokenizer of the C family shares nothing
   * with Python beyond the notion of an identifier
   *
   * \note what a name *is* — a function, a class, a local — is not said here:
   * that is what the language server is asked, and a highlighter which guessed
   * from the spelling of a name would disagree with the answer
   *
   * \note the object registers itself on the document it is built with and is
   * therefore owned by that document, not by the mode which installed it
   */
  struct TFEL_GUI_VISIBILITY_EXPORT PythonSyntaxHighlighter
      : public QSyntaxHighlighter {
    /*!
     * \brief constructor
     * \param[in] d : document highlighted by this highlighter
     */
    explicit PythonSyntaxHighlighter(QTextDocument *const d);

    /*!
     * \brief highlight one block of the document
     * \param[in] text : text of the block
     *
     * \note the state of the previous block says whether a triple-quoted string
     * was left open there, in which case this block begins inside that string
     */
    void highlightBlock(const QString &text) override;

    /*!
     * \brief what a block can be left in: inside a triple-quoted string, which
     * the next block has to keep drawing as a string until the delimiter is
     * found again
     *
     * \note the states name the delimiter because only it can end the string —
     * a `'''` inside a `"""` string is text — and the face because a docstring
     * is drawn as prose from its first line to its last
     */
    enum State {
      //! the block ends with no string open
      DefaultState = 0,
      //! the block ends inside a `'''` string
      SingleQuotedStringState = 1,
      //! the block ends inside a `"""` string
      DoubleQuotedStringState = 2,
      //! the block ends inside a `'''` docstring
      SingleQuotedDocstringState = 3,
      //! the block ends inside a `"""` docstring
      DoubleQuotedDocstringState = 4
    };  // end of enum State

   private:
    //! format of the keywords: bold dark magenta, as every other mode of the editor
    QTextCharFormat keywordFormat;
    //! format of `True`, `False` and `None`: dark red, the colour of literals
    QTextCharFormat constantFormat;
    //! format of the builtins: dark cyan, the colour a function gets
    QTextCharFormat builtinFormat;
    //! format of the name a `def` statement introduces: dark cyan, bold
    QTextCharFormat definitionFormat;
    //! format of the name a `class` statement introduces: dark blue, bold
    QTextCharFormat classFormat;
    //! format of a decorator: dark cyan, italic
    QTextCharFormat decoratorFormat;
    //! format of the numbers: dark red
    QTextCharFormat numberFormat;
    //! format of the strings, triple-quoted or not: dark green
    QTextCharFormat stringFormat;
    //! format of a docstring: dark green, italic, which is what it is
    QTextCharFormat docstringFormat;
    //! format of the comments: dark red lightened by 30 percent
    QTextCharFormat commentFormat;
    /*!
     * \brief a number, in any of the forms the language has
     *
     * The scan of the block asks this regex at the place a digit stands, and
     * uses the answer only when the match begins exactly there: what stands
     * before the place is the identifier character of a name the scan has just
     * taken, and `x1` holds no number
     */
    QRegularExpression number;
    /*!
     * \brief a decorator: the `@` of a line which holds nothing before it, the
     * dotted name which follows, and the arguments it is applied with
     */
    QRegularExpression decorator;
    /*!
     * \return the format of the name, and true when the name is a word of the
     * language the highlighter knows
     * \param[in] word: an identifier of the block, never one inside a string
     *                   or a comment
     * \param[out] format: the face to draw that identifier with
     */
    bool faceOf(const QString &word, QTextCharFormat &format) const;

   private:
    //! the words of the language, in the sense of `keyword.kwlist`
    static const QSet<QString> &keywords();
    //! the words which name a value of their own
    static const QSet<QString> &constants();
    //! the names `builtins` holds, which are not words of the language
    static const QSet<QString> &builtins();

   private:
    Q_OBJECT
  };  // end of struct PythonSyntaxHighlighter

}  // end of namespace tfel::gui

#endif
