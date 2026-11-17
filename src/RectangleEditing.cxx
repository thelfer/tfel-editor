/*!
 * \file  RectangleEditing.cxx
 * \brief
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QObject>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/RectangleEditing.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {

    /*!
     * \brief the rectangular area the selection of a text cursor covers
     *
     * The rectangle is the set of columns which lies between the two ends of
     * the selection, on every line the selection touches. A line an end stands
     * on is part of the rectangle, so a selection which stops at the beginning
     * of a line covers that line too: that is the usual shape of a selection
     * dragged down a few lines, and of any selection which reaches the end of
     * the buffer, whose last line is the empty one the final newline opens.
     */
    struct Rectangle {
      //! number of the first line the rectangle covers
      int firstBlock = 0;
      //! number of the last line the rectangle covers
      int lastBlock = 0;
      //! first column the rectangle covers on each of those lines
      int left = 0;
      //! one past the last column the rectangle covers
      int right = 0;
      //! number of the line the cursor of the text edit stands on
      int cursorBlock = 0;
      //! column that cursor stands on
      int cursorColumn = 0;
    };  // end of struct Rectangle

    static Rectangle getRectangle(const QTextCursor& tc) {
      Rectangle r;
      QTextDocument* const doc = tc.document();
      const int start = tc.selectionStart();
      const int end = tc.selectionEnd();
      const QTextBlock firstBlock = doc->findBlock(start);
      const QTextBlock lastBlock = doc->findBlock(qMax(start, end));
      const int startColumn = start - firstBlock.position();
      const int endColumn = end - lastBlock.position();
      r.firstBlock = firstBlock.blockNumber();
      r.lastBlock = lastBlock.blockNumber();
      r.left = qMin(startColumn, endColumn);
      r.right = qMax(startColumn, endColumn);
      /* the line and the column of the cursor are taken from its position and
         from the document, not from QTextCursor::blockNumber, which asks the
         layout of the document: that one is brought up to date when the widget
         is drawn, so it can name the line the cursor stood on the last time the
         text was displayed rather than the one it stands on now. */
      const int cursorPosition = tc.position();
      const QTextBlock cursorBlock = doc->findBlock(cursorPosition);
      r.cursorBlock = cursorBlock.blockNumber();
      r.cursorColumn = cursorPosition - cursorBlock.position();
      return r;
    }  // end of getRectangle()

    /*!
     * \return the text a line holds once the columns \a left to \a right of it
     * have been taken over by \a piece
     *
     * What the line holds before and after those columns is kept, a line which
     * stops short of the rectangle is padded with blanks up to \a left when
     * \a pad tells so, and a line which stops short of \a right has nothing
     * left to keep.
     */
    static QString lineOf(const QString& line,
                          int left,
                          int right,
                          const QString& piece,
                          bool pad) {
      QString updated = ((pad) and (line.size() < left))
                            ? line.leftJustified(left, ' ', false)
                            : line.left(left);
      updated += piece;
      if (line.size() > right) {
        updated += line.mid(right);
      }
      return updated;
    }  // end of lineOf()

    /*!
     * \brief replaces the whole text of a line of the document
     * \param[in,out] tc : cursor used to edit the document
     * \param[in] block : line of the document the text of which is replaced
     * \param[in] text : text the line is given
     *
     * A line the rectangle changes not at all is left alone, so that a
     * rectangle command which reaches lines it can not write on does not make
     * the buffer modified down to them.
     */
    static void setLineOfDocument(QTextCursor& tc,
                                  const QTextBlock& block,
                                  const QString& text) {
      if (block.text() == text) {
        return;
      }
      tc.setPosition(block.position(), QTextCursor::MoveAnchor);
      tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      tc.insertText(text);
    }  // end of setLineOfDocument()

    /*!
     * \brief places a cursor on a column of a line of the document
     * \param[in,out] tc : cursor placed
     * \param[in] block : line of the document
     * \param[in] column : column of that line, never past its end
     */
    static void gotoColumn(QTextCursor& tc,
                           const QTextBlock& block,
                           int column) {
      tc.setPosition(block.position(), QTextCursor::MoveAnchor);
      tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                      qMin(column, block.text().size()));
    }  // end of gotoColumn()

    /*!
     * \return what the lines of a rectangle hold between its two edges
     * \param[in] rect : the rectangle
     * \param[in] doc : document the lines of the rectangle are read from
     *
     * Each piece is padded with blanks up to the width of the rectangle, so a
     * line which stopped short of the right edge gives a piece as wide as the
     * rest and a paste puts the rectangle back where it was taken from.
     */
    static QVector<QString> piecesOf(const Rectangle& rect,
                                     QTextDocument& doc) {
      const int width = rect.right - rect.left;
      QVector<QString> pieces;
      for (int line = rect.firstBlock; line <= rect.lastBlock; ++line) {
        pieces << doc.findBlockByNumber(line)
                      .text()
                      .mid(rect.left, width)
                      .leftJustified(width, ' ', false);
      }
      return pieces;
    }  // end of piecesOf()

  }  // end of anonymous namespace

  QVector<QString> RectangleEditing::killed;

  bool RectangleEditing::checkWritable(TextEditBase& textEdit,
                                       EditorWidget& editor) {
    if (not textEdit.isReadOnly()) {
      return true;
    }
    // the freeze of a file modified on disk holds the rectangle commands too
    editor.displayInformativeMessage(QObject::tr("read only buffer"));
    return false;
  }  // end of RectangleEditing::checkWritable

  bool RectangleEditing::insertString(TextEditBase& textEdit,
                                      EditorWidget& editor,
                                      const QString& text) {
    if (text.isEmpty()) {
      return false;
    }
    if (not checkWritable(textEdit, editor)) {
      return false;
    }
    QTextCursor tc = textEdit.textCursor();
    const Rectangle rect = getRectangle(tc);
    QTextDocument* const doc = tc.document();
    const auto answer = text.split(QLatin1Char('\n'));
    tc.beginEditBlock();
    /* the lines are rewritten from the bottom of the rectangle up: an answer
       of several lines opens lines, and the numbering of the lines above the
       one being written is nothing a rewrite of a lower line may depend on. */
    for (int line = rect.lastBlock; line >= rect.firstBlock; --line) {
      // findBlockByNumber, not findBlock: the latter reads its argument as a
      // position of the document, so findBlock(1) is the first line, not the
      // second one.
      const QTextBlock block = doc->findBlockByNumber(line);
      setLineOfDocument(tc, block,
                        lineOf(block.text(), rect.left, rect.right, text, true));
    }
    tc.endEditBlock();
    /* the cursor ends behind the last line of the answer written on the last
       line of the rectangle, the direction the selection was made aside.
       Every line of the rectangle took the whole answer, so each of them
       occupies as many lines as the answer has, and the place of the last
       line of the answer counts them all. */
    const int lines = rect.lastBlock - rect.firstBlock + 1;
    const int last = rect.firstBlock + lines * static_cast<int>(answer.size()) - 1;
    const QTextBlock block = doc->findBlockByNumber(last);
    // a one-line answer stands on the line it was written on, behind the
    // text that line holds before the left edge of the rectangle; the last
    // line of an answer of several ones opens a line of its own
    const int column = (answer.size() == 1)
                           ? rect.left + static_cast<int>(text.size())
                           : static_cast<int>(answer.back().size());
    tc.setPosition(block.position() + qMin(column, block.length() - 1),
                   QTextCursor::MoveAnchor);
    textEdit.setTextCursor(tc);
    return true;
  }  // end of RectangleEditing::insertString

  bool RectangleEditing::killRectangle(TextEditBase& textEdit,
                                       EditorWidget& editor) {
    if (not checkWritable(textEdit, editor)) {
      return false;
    }
    QTextCursor tc = textEdit.textCursor();
    const Rectangle rect = getRectangle(tc);
    QTextDocument* const doc = tc.document();
    const int width = rect.right - rect.left;
    const QVector<QString> pieces = piecesOf(rect, *doc);
    tc.beginEditBlock();
    for (int line = rect.firstBlock; line <= rect.lastBlock; ++line) {
      const QTextBlock block = doc->findBlockByNumber(line);
      setLineOfDocument(
          tc, block,
          lineOf(block.text(), rect.left, rect.right, QString(), false));
    }
    tc.endEditBlock();
    RectangleEditing::killed = pieces;
    /* the cursor keeps the line it stands on and goes back by the width of the
       text deleted in front of it, stopping at the left edge of the rectangle
       when it stood inside of it: the place it names is a place of the text, and
       the text in front of it has moved. */
    int column = rect.cursorColumn;
    if ((rect.cursorBlock >= rect.firstBlock) and
        (rect.cursorBlock <= rect.lastBlock)) {
      column -= qBound(0, rect.cursorColumn - rect.left, width);
    }
    gotoColumn(tc, doc->findBlockByNumber(rect.cursorBlock), column);
    textEdit.setTextCursor(tc);
    return true;
  }  // end of RectangleEditing::killRectangle

  bool RectangleEditing::copyRectangleAsKill(TextEditBase& textEdit,
                                             EditorWidget& editor) {
    QTextCursor tc = textEdit.textCursor();
    const Rectangle rect = getRectangle(tc);
    QTextDocument* const doc = tc.document();
    if (doc == nullptr) {
      return false;
    }
    RectangleEditing::killed = piecesOf(rect, *doc);
    // nothing is taken from the lines, so neither the text nor the cursor moves
    (void)editor;
    return true;
  }  // end of RectangleEditing::copyRectangleAsKill

  bool RectangleEditing::yankRectangle(TextEditBase& textEdit,
                                       EditorWidget& editor) {
    if (RectangleEditing::killed.isEmpty()) {
      editor.displayInformativeMessage(QObject::tr("no rectangle killed"));
      return false;
    }
    if (not checkWritable(textEdit, editor)) {
      return false;
    }
    QTextCursor tc = textEdit.textCursor();
    /* the paste is drawn from the column of the cursor and the selection it is
       asked on is dismissed: the register says what goes on the lines below the
       cursor, so a selection which reaches other lines than that one is nothing
       the paste has to do with, and a selection which stands on it is nothing to
       write over — yank-rectangle inserts, it does not replace what was selected.
       The column is taken before the selection is dropped, since the place of the
       cursor is the place of the paste either way. */
    const int column = tc.positionInBlock();
    tc.clearSelection();
    tc.beginEditBlock();
    bool first = true;
    for (const auto& piece : RectangleEditing::killed) {
      if (!first) {
        const QTextBlock next = tc.block().next();
        if (next.isValid()) {
          tc.setPosition(next.position(), QTextCursor::MoveAnchor);
        } else if (tc.positionInBlock() != 0) {
          // the rectangle reaches below the last line, and that line has no
          // newline to end it: the line opened by a piece which stands
          // somewhere on it is what a file without a final newline asks for.
          // The empty line of a file which has one answers 0 here and is used
          // as it stands, so a rectangle of no width opens nothing on it.
          tc.insertBlock();
        }
        /* the cursor is at the start of the line and has to reach the column of
           the paste: it walks over what the line holds and, when the line stops
           short of that column, takes the blanks which make up the distance. The
           piece therefore goes on the column of the paste on a short line too,
           behind the blanks and not before them. */
        const QString line = tc.block().text();
        tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                        qMin(column, line.size()));
        if (line.size() < column) {
          tc.insertText(QString(column - line.size(), ' '));
        }
      }
      first = false;
      tc.insertText(piece);
    }
    tc.endEditBlock();
    textEdit.setTextCursor(tc);
    return true;
  }  // end of RectangleEditing::yankRectangle

  void RectangleEditing::clearRegister() {
    RectangleEditing::killed.clear();
  }  // end of RectangleEditing::clearRegister

  bool RectangleEditing::hasRectangle() {
    return not RectangleEditing::killed.isEmpty();
  }  // end of RectangleEditing::hasRectangle

  namespace {

    /*!
     * \brief runs a rectangle command on the main frame of the current buffer
     * \param[in,out] w : editor the command was asked from
     * \param[in] command : the command, which works on a text edit
     *
     * The command answers false when it did nothing, which is what it reports
     * itself: a rectangle of no width is a command which does something, a read
     * only buffer and a paste with nothing killed are the two ways out and each
     * says so in the minibuffer.
     */
    void runOnMainFrame(EditorWidget& w,
                        bool (*command)(TextEditBase&, EditorWidget&)) {
      auto& mainFrame = w.getCurrentBuffer().getMainFrame();
      command(mainFrame, w);
    }  // end of runOnMainFrame()

    //! \brief the `cut-rectangle` command, the rectangle of `Ctrl-X R K`
    void rectangleCutCommand(EditorWidget& w) {
      runOnMainFrame(w, &RectangleEditing::killRectangle);
    }  // end of rectangleCutCommand()

    //! \brief the `copy-rectangle` command, the rectangle of `Ctrl-X R W`
    void rectangleCopyCommand(EditorWidget& w) {
      runOnMainFrame(w, &RectangleEditing::copyRectangleAsKill);
    }  // end of rectangleCopyCommand()

    //! \brief the `paste-rectangle` command, the rectangle of `Ctrl-X R Y`
    void rectanglePasteCommand(EditorWidget& w) {
      runOnMainFrame(w, &RectangleEditing::yankRectangle);
    }  // end of rectanglePasteCommand()

  }  // end of anonymous namespace

  static StandardFunctionCommandProxy<rectangleCutCommand> cutRectangleProxy(
      "cut-rectangle");

  /* `kill-rectangle` is the name GNU Emacs gives this command and the name the
     rectangle page of the documentation of the editor spells it with. An alias
     is a second registration of the same function under its own name:
     `CommandFactory` knows no other way, and the name a command is registered
     under is what `Alt-X` completion lists, which is what an alias is for. */
  static StandardFunctionCommandProxy<rectangleCutCommand>
      killRectangleAlias("kill-rectangle");

  static StandardFunctionCommandProxy<rectangleCopyCommand>
      copyRectangleProxy("copy-rectangle");

  static StandardFunctionCommandProxy<rectanglePasteCommand>
      pasteRectangleProxy("paste-rectangle");

}  // end of namespace tfel::gui
