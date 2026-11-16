/*!
 * \file  PythonSyntaxHighlighter.cxx
 * \brief syntax highlighter of the Python language.
 * \author Thomas Helfer
 * \date   14/11/2026
 */

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QTextBlock>

#include "TFEL/GUI/PythonSyntaxHighlighter.hxx"

namespace tfel::gui {

  namespace {

    // the two characters a string literal is written with in this language
    bool is_quote(const QChar c) {
      return (c == QLatin1Char('\'')) || (c == QLatin1Char('"'));
    }  // end of is_quote()

    // the letters which, alone or paired, stand before the quote of a string to
    // say what it is: raw, bytes, unicode, formatted, and the two pairs which
    // mean the raw form of bytes or of a formatted string
    bool is_prefix_letter(const QChar c) {
      static const QString letters{QStringLiteral("rRbBuUfF")};
      return letters.contains(c);
    }  // end of is_prefix_letter()

    // an identifier begins with a letter or an underscore and goes on with
    // letters, digits and underscores — which is what `x1` and `1x` are about:
    // the first is a name, and its scan swallows the digit; the second is a
    // number, which the scan of a name never reaches
    bool is_identifier_start(const QChar c) {
      return c.isLetter() || (c == QLatin1Char('_'));
    }  // end of is_identifier_start()

    bool is_identifier_part(const QChar c) {
      return c.isLetterOrNumber() || (c == QLatin1Char('_'));
    }  // end of is_identifier_part()

    // a number begins with a digit, or with the dot of its fraction
    bool is_number_start(const QString &text, const int i) {
      const auto c = text.at(i);
      if (c.isDigit()) {
        return true;
      }
      return (c == QLatin1Char('.')) && ((i + 1) < text.size()) &&
             text.at(i + 1).isDigit();
    }  // end of is_number_start()

    // the place of the quote of the string which begins at `i`, or `-1` when
    // nothing there opens one: at most two prefix letters and a quote right
    // after them, or a quote at `i` itself
    int quote_at(const QString &text, const int i) {
      auto j = i;
      while ((j < text.size()) && ((j - i) < 2) &&
             is_prefix_letter(text.at(j))) {
        ++j;
      }
      // one or two prefix letters are a prefix only when a quote follows, which
      // is what tells `f"..."` from the tail of a name ending in `f` — and the
      // name is not what reaches here: the scan of an identifier is what stops
      // at a character which cannot be part of one, quote included
      return ((j < text.size()) && is_quote(text.at(j))) ? j : -1;
    }  // end of quote_at()

    // the length of the delimiter the quote at `q` opens: three when the same
    // quote stands twice more, which is what makes a docstring and a string
    // spanning lines the same thing to the eye of the scanner
    int quote_length(const QString &text, const int q) {
      const auto c = text.at(q);
      if (((q + 2) < text.size()) && (text.at(q + 1) == c) &&
          (text.at(q + 2) == c)) {
        return 3;
      }
      return 1;
    }  // end of quote_length()

    /*!
     * \return the place one past the last character of the string whose
     * delimiter begins at `q`, or `-1` when the text holds no closing delimiter
     * \param[in] text: the text of the block
     * \param[in] q: the place of the opening delimiter
     * \param[in] qlen: 1 or 3, what `quote_length` answered for that delimiter
     *
     * \note a backslash hides whatever stands after it, the quote included,
     * which is true of the raw forms too: `r"\""` is a string of two characters,
     * not an empty string followed by another one
     *
     * \note inside a triple-quoted string, a run of one or two quotes is prose,
     * so the run is walked over rather than taken for the end of the string
     */
    int string_end(const QString &text, const int q, const int qlen) {
      const auto delimiter = text.at(q);
      auto i = q + qlen;
      while (i < text.size()) {
        if (text.at(i) == QLatin1Char('\\')) {
          i += 2;
          continue;
        }
        if (text.at(i) == delimiter) {
          if (qlen == 1) {
            return i + 1;
          }
          auto run = 1;
          while (((i + run) < text.size()) && (text.at(i + run) == delimiter) &&
                 (run < 3)) {
            ++run;
          }
          if (run == 3) {
            return i + 3;
          }
          i += run;
          continue;
        }
        ++i;
      }
      return -1;
    }  // end of string_end()

    // the two statements which give a name to what they define: the name they
    // name is drawn with the face of what it is, since the outline of a Python
    // file is read down those names
    bool introduces_a_name(const QString &word) {
      return (word == QLatin1String("def")) ||
             (word == QLatin1String("class"));
    }  // end of introduces_a_name()

    // the first line above which says something, the blanks and the comments
    // walked over — which is where the question of a docstring is decided: a
    // string which opens the body of a `def`, or of a `class`, or of a module,
    // is prose, and the very same string written anywhere else is a literal
    bool opens_a_statement(const QTextBlock &previous) {
      auto block = previous;
      while (block.isValid()) {
        const auto text = block.text().trimmed();
        if ((text.isEmpty()) || (text.startsWith(QLatin1Char('#')))) {
          block = block.previous();
          continue;
        }
        // the head of a body is the only line which ends with a colon, and no
        // line which does not end with one introduces a body
        return text.endsWith(QLatin1Char(':'));
      }
      // nothing above: the docstring of the module itself
      return true;
    }  // end of opens_a_statement()

    // the state of a block which ends inside a triple-quoted string: what the
    // next block has to look for, and what it has to draw with
    int opened_state(const int quote, const int docstring) {
      if (quote == '\'') {
        return (docstring != 0)
                   ? PythonSyntaxHighlighter::SingleQuotedDocstringState
                   : PythonSyntaxHighlighter::SingleQuotedStringState;
      }
      return (docstring != 0)
                 ? PythonSyntaxHighlighter::DoubleQuotedDocstringState
                 : PythonSyntaxHighlighter::DoubleQuotedStringState;
    }  // end of opened_state()

  }  // end of namespace

  const QSet<QString> &PythonSyntaxHighlighter::keywords() {
    // `keyword.kwlist` of Python 3, with the three constants of the language
    // taken out — they are drawn as the literals they are, see `constants`
    //
    // `match`, `case` and `type` are deliberately absent: they are words of the
    // language only where its grammar puts them, and a highlighter which colored
    // every `match` would color the variable, the method and the database column
    // a file holds of that name. The soft keywords are the business of a parser,
    // which is what the language server of this mode is
    static const QSet<QString> keys{QStringLiteral("and"),
                                    QStringLiteral("as"),
                                    QStringLiteral("assert"),
                                    QStringLiteral("async"),
                                    QStringLiteral("await"),
                                    QStringLiteral("break"),
                                    QStringLiteral("class"),
                                    QStringLiteral("continue"),
                                    QStringLiteral("def"),
                                    QStringLiteral("del"),
                                    QStringLiteral("elif"),
                                    QStringLiteral("else"),
                                    QStringLiteral("except"),
                                    QStringLiteral("finally"),
                                    QStringLiteral("for"),
                                    QStringLiteral("from"),
                                    QStringLiteral("global"),
                                    QStringLiteral("if"),
                                    QStringLiteral("import"),
                                    QStringLiteral("in"),
                                    QStringLiteral("is"),
                                    QStringLiteral("lambda"),
                                    QStringLiteral("nonlocal"),
                                    QStringLiteral("not"),
                                    QStringLiteral("or"),
                                    QStringLiteral("pass"),
                                    QStringLiteral("raise"),
                                    QStringLiteral("return"),
                                    QStringLiteral("try"),
                                    QStringLiteral("while"),
                                    QStringLiteral("with"),
                                    QStringLiteral("yield")};
    return keys;
  }  // end of keywords

  const QSet<QString> &PythonSyntaxHighlighter::constants() {
    // the three which are keywords of the language, and the three names the
    // interpreter gives every file
    static const QSet<QString> values{QStringLiteral("False"),
                                       QStringLiteral("None"),
                                       QStringLiteral("True"),
                                       QStringLiteral("NotImplemented"),
                                       QStringLiteral("Ellipsis"),
                                       QStringLiteral("__debug__")};
    return values;
  }  // end of constants

  const QSet<QString> &PythonSyntaxHighlighter::builtins() {
    // `dir(builtins)`, without the names the interpreter reserves to itself and
    // without the exceptions — a hundred names of errors, which a reader meets
    // by their own name and not as a class of words
    static const QSet<QString> names{QStringLiteral("abs"),
                                     QStringLiteral("aiter"),
                                     QStringLiteral("all"),
                                     QStringLiteral("anext"),
                                     QStringLiteral("any"),
                                     QStringLiteral("ascii"),
                                     QStringLiteral("bin"),
                                     QStringLiteral("bool"),
                                     QStringLiteral("breakpoint"),
                                     QStringLiteral("bytearray"),
                                     QStringLiteral("bytes"),
                                     QStringLiteral("callable"),
                                     QStringLiteral("chr"),
                                     QStringLiteral("compile"),
                                     QStringLiteral("complex"),
                                     QStringLiteral("delattr"),
                                     QStringLiteral("dict"),
                                     QStringLiteral("dir"),
                                     QStringLiteral("divmod"),
                                     QStringLiteral("enumerate"),
                                     QStringLiteral("eval"),
                                     QStringLiteral("exec"),
                                     QStringLiteral("filter"),
                                     QStringLiteral("float"),
                                     QStringLiteral("format"),
                                     QStringLiteral("frozenset"),
                                     QStringLiteral("getattr"),
                                     QStringLiteral("globals"),
                                     QStringLiteral("hasattr"),
                                     QStringLiteral("hash"),
                                     QStringLiteral("help"),
                                     QStringLiteral("hex"),
                                     QStringLiteral("id"),
                                     QStringLiteral("input"),
                                     QStringLiteral("int"),
                                     QStringLiteral("isinstance"),
                                     QStringLiteral("issubclass"),
                                     QStringLiteral("iter"),
                                     QStringLiteral("len"),
                                     QStringLiteral("list"),
                                     QStringLiteral("locals"),
                                     QStringLiteral("map"),
                                     QStringLiteral("max"),
                                     QStringLiteral("memoryview"),
                                     QStringLiteral("min"),
                                     QStringLiteral("next"),
                                     QStringLiteral("object"),
                                     QStringLiteral("oct"),
                                     QStringLiteral("open"),
                                     QStringLiteral("ord"),
                                     QStringLiteral("pow"),
                                     QStringLiteral("print"),
                                     QStringLiteral("property"),
                                     QStringLiteral("repr"),
                                     QStringLiteral("reversed"),
                                     QStringLiteral("round"),
                                     QStringLiteral("set"),
                                     QStringLiteral("setattr"),
                                     QStringLiteral("slice"),
                                     QStringLiteral("sorted"),
                                     QStringLiteral("staticmethod"),
                                     QStringLiteral("str"),
                                     QStringLiteral("sum"),
                                     QStringLiteral("super"),
                                     QStringLiteral("tuple"),
                                     QStringLiteral("type"),
                                     QStringLiteral("vars"),
                                     QStringLiteral("zip")};
    return names;
  }  // end of builtins

  PythonSyntaxHighlighter::PythonSyntaxHighlighter(QTextDocument *const d)
      : QSyntaxHighlighter(d),
        number(QRegularExpression(
            QStringLiteral("(?:0[xXbBoO][0-9a-fA-F_]+"
                           "|(?:\\d[\\d_]*\\.?[\\d_]*)"
                           "(?:[eE][+-]?\\d[\\d_]*)?[jJ]?"
                           "|\\.[\\d_]+(?:[eE][+-]?\\d[\\d_]*)?[jJ]?)"))),
        decorator(QRegularExpression(
            QStringLiteral("@[A-Za-z_][A-Za-z0-9_.]*(?:\\([^()]*\\))?"))) {
    // the faces are those of the other modes of the editor: a keyword is a
    // keyword in a C++ buffer and in a Python one, and the two sit on the same
    // screen for whoever has both of them open
    this->keywordFormat.setForeground(Qt::darkMagenta);
    this->keywordFormat.setFontWeight(QFont::Bold);
    this->constantFormat.setForeground(Qt::darkRed);
    this->builtinFormat.setForeground(Qt::darkCyan);
    this->definitionFormat.setForeground(Qt::darkCyan);
    this->definitionFormat.setFontWeight(QFont::Bold);
    this->classFormat.setForeground(Qt::darkBlue);
    this->classFormat.setFontWeight(QFont::Bold);
    this->decoratorFormat.setForeground(Qt::darkCyan);
    this->decoratorFormat.setFontItalic(true);
    this->numberFormat.setForeground(Qt::darkRed);
    this->stringFormat.setForeground(Qt::darkGreen);
    this->docstringFormat.setForeground(Qt::darkGreen);
    this->docstringFormat.setFontItalic(true);
    this->commentFormat.setForeground(QColor(Qt::darkRed).lighter(130));
  }  // end of PythonSyntaxHighlighter::PythonSyntaxHighlighter

  bool PythonSyntaxHighlighter::faceOf(const QString &word,
                                       QTextCharFormat &format) const {
    if (keywords().contains(word)) {
      format = this->keywordFormat;
      return true;
    }
    if (constants().contains(word)) {
      format = this->constantFormat;
      return true;
    }
    if (builtins().contains(word)) {
      format = this->builtinFormat;
      return true;
    }
    return false;
  }  // end of faceOf

  void PythonSyntaxHighlighter::highlightBlock(const QString &text) {
    // nothing is kept of a previous pass: the state and the faces are set anew,
    // and a block which leaves no string open says so by the state it stores
    this->setCurrentBlockState(DefaultState);
    auto state = this->previousBlockState();
    if ((state < DefaultState) || (state > DoubleQuotedDocstringState)) {
      state = DefaultState;
    }
    const auto size = text.size();
    auto i = 0;
    if (state != DefaultState) {
      // the block opens inside the string the one above left open, and only a
      // delimiter of that kind can end it: a `'''` standing in a `"""` string is
      // prose, and the state says which of the two the block went on with
      const auto single = ((state == SingleQuotedStringState) ||
                           (state == SingleQuotedDocstringState));
      const auto docstring = (state >= SingleQuotedDocstringState);
      const auto &format =
          (docstring) ? this->docstringFormat : this->stringFormat;
      const auto closed = text.indexOf(single ? QStringLiteral("'''")
                                              : QStringLiteral("\"\"\""));
      if (closed < 0) {
        this->setFormat(0, size, format);
        this->setCurrentBlockState(state);
        return;
      }
      this->setFormat(0, closed + 3, format);
      i = closed + 3;
    }
    while (i < size) {
      const auto c = text.at(i);
      if (c == QLatin1Char('#')) {
        // a comment reaches the end of the line, and nothing is behind it
        this->setFormat(i, size - i, this->commentFormat);
        return;
      }
      const auto q = quote_at(text, i);
      if (q >= 0) {
        const auto qlen = quote_length(text, q);
        const auto end = string_end(text, q, qlen);
        const auto closed = (end >= 0);
        // a docstring is a string which opens the body it belongs to, which is
        // what the line above says, and whose block holds nothing but blanks
        // before the quotes — the prefix of a formatted docstring being a thing
        // nobody writes, `text.left(i)` is taken up to the prefix as it stands
        const auto docstring =
            (qlen == 3) && (text.left(i).trimmed().isEmpty()) &&
            opens_a_statement(this->currentBlock().previous());
        const auto &format =
            (docstring) ? this->docstringFormat : this->stringFormat;
        // what the block does not close is drawn to its end all the same: an
        // unterminated string is a mistake of the file, and a mistake reads best
        // as the string it is
        this->setFormat(i, ((closed) ? end : size) - i, format);
        if (!closed) {
          if (qlen == 3) {
            this->setCurrentBlockState(opened_state(c.unicode(),
                                                     docstring ? 1 : 0));
          }
          return;
        }
        i = end;
        continue;
      }
      if (is_number_start(text, i)) {
        const auto m = this->number.match(
            text, i, QRegularExpression::NormalMatch,
            QRegularExpression::AnchorAtOffsetMatchOption);
        if ((m.hasMatch()) && (m.capturedStart() == i) &&
            (m.capturedLength() > 0)) {
          this->setFormat(i, m.capturedLength(), this->numberFormat);
          i += m.capturedLength();
          continue;
        }
      }
      if ((c == QLatin1Char('@')) && (text.left(i).trimmed().isEmpty())) {
        // a decorator is a statement of its own: the same character standing in
        // the middle of a line belongs to no expression of this language
        const auto m = this->decorator.match(
            text, i, QRegularExpression::NormalMatch,
            QRegularExpression::AnchorAtOffsetMatchOption);
        if ((m.hasMatch()) && (m.capturedLength() > 0)) {
          this->setFormat(i, m.capturedLength(), this->decoratorFormat);
          i += m.capturedLength();
          continue;
        }
      }
      if (is_identifier_start(c)) {
        auto j = i + 1;
        while ((j < size) && is_identifier_part(text.at(j))) {
          ++j;
        }
        const auto word = text.mid(i, j - i);
        // an attribute of an object is none of the language's business, whatever
        // builtin happens to share its name, which is why `self.type` and
        // `row.id` keep the face of an ordinary name
        const auto attribute =
            (i > 0) && (text.at(i - 1) == QLatin1Char('.'));
        auto format = QTextCharFormat();
        if ((!attribute) && this->faceOf(word, format)) {
          this->setFormat(i, j - i, format);
        }
        if (introduces_a_name(word)) {
          const auto &name_format = (word == QLatin1String("class"))
                                        ? this->classFormat
                                        : this->definitionFormat;
          auto k = j;
          while ((k < size) && (text.at(k).isSpace())) {
            ++k;
          }
          if ((k < size) && is_identifier_start(text.at(k))) {
            auto l = k + 1;
            while ((l < size) && is_identifier_part(text.at(l))) {
              ++l;
            }
            this->setFormat(k, l - k, name_format);
            // the name is spent: taken as the word it is, `type` would be drawn
            // again as the builtin of that name, and the bold face of a
            // definition lost under it
            i = l;
            continue;
          }
        }
        i = j;
        continue;
      }
      ++i;
    }
  }  // end of highlightBlock

}  // end of namespace tfel::gui
