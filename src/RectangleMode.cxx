/*!
 * \file  RectangleMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   30/09/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <QtGui/QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextBlock>

#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/RectangleMode.hxx"

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
     *
     * The columns are those of the characters of the lines, not the columns of
     * the display: a tab is one character here and moves to the next tab stop
     * on the screen.
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
    static QString lineOf(const QString& line, int left, int right,
                          const QString& piece, bool pad) {
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
    static void setLineOfDocument(QTextCursor& tc, const QTextBlock& block,
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
    static void gotoColumn(QTextCursor& tc, const QTextBlock& block,
                           int column) {
      tc.setPosition(block.position(), QTextCursor::MoveAnchor);
      tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                      qMin(column, block.text().size()));
    }  // end of gotoColumn()

  }  // end of anonymous namespace

  struct InsertRectangle : public CommandLine {
    InsertRectangle(TextEditBase& t, EditorWidget& p)
        : CommandLine(QObject::tr("String rectangle"), p), textEdit(t) {
      static QString e("/rectangle-mode/insert/histroy");
      QSettings s;
      this->setInputHistorySettingAddress(e);
      const auto ch = s.value(e).toStringList();
      if (!ch.isEmpty()) {
        this->defaultEntry = ch.back();
        this->setLabel(QObject::tr("String rectangle (default %1)")
                           .arg(this->defaultEntry));
      }
    }  // end of InsertRectangle

    /*!
     * \brief writes the answer on every line of the rectangle the selection
     * covers, which is the \c string-rectangle command of Emacs
     *
     * Every line keeps what it holds before the left edge and after the right
     * edge of the rectangle, and a line which stops short of the left edge is
     * padded with blanks up to it, so the inserted text of a line always starts
     * at that column. The lines the two ends of the selection stand on are part
     * of the rectangle, which is what makes a selection dragged to the
     * beginning of the next line reach that line too, and the final newline of
     * a file the empty last line it opens.
     *
     * The whole answer goes on every line of the rectangle, as Emacs writes it:
     * an answer of several lines therefore opens as many lines on each line it
     * is written on, and the text the rectangle line held after the right edge
     * ends up behind the last line of the answer.
     */
    void treatUserInput() override {
      QString t = this->input->text();
      if (t.isEmpty()) {
        t = this->defaultEntry;
      }
      if (t.isEmpty()) {
        return;
      }
      QTextCursor tc = this->textEdit.textCursor();
      const Rectangle rect = getRectangle(tc);
      QTextDocument* const doc = tc.document();
      const auto answer = t.split(QLatin1Char('\n'));
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
                          lineOf(block.text(), rect.left, rect.right, t, true));
      }
      tc.endEditBlock();
      /* the cursor ends behind the last line of the answer written on the last
         line of the rectangle, the direction the selection was made aside.
         Every line of the rectangle took the whole answer, so each of them
         occupies as many lines as the answer has, and the place of the last
         line of the answer counts them all. */
      const int lines = rect.lastBlock - rect.firstBlock + 1;
      const int last = rect.firstBlock + lines * answer.size() - 1;
      const QTextBlock block = doc->findBlockByNumber(last);
      // a one-line answer stands on the line it was written on, behind the
      // text that line holds before the left edge of the rectangle; the last
      // line of an answer of several ones opens a line of its own
      const int column = (answer.size() == 1)
                             ? rect.left + static_cast<int>(t.size())
                             : static_cast<int>(answer.back().size());
      tc.setPosition(block.position() + qMin(column, block.length() - 1),
                     QTextCursor::MoveAnchor);
      this->textEdit.setTextCursor(tc);
    }  // end of treatUserInput

   protected:
    TextEditBase& textEdit;

    QString defaultEntry;

  };  // end of InsertRectangle

  struct RectangleMode::RectangleModeLineEdit : CommandLine::CustomLineEdit {
    RectangleModeLineEdit(TextEditBase& t, EditorWidget& q, CommandLine& p)
        : CustomLineEdit(q, p), textEdit(t) {}  // end of RectangleModeLineEdit

    void keyPressEvent(QKeyEvent* ev) override {
      auto* qle = qobject_cast<CommandLine*>(this->parent());
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_G)) {
        if (qle != nullptr) {
          qle->cancel();
        }
        return;
      }
      const auto k = ev->key();
      if ((ev->modifiers() == Qt::NoModifier) &&
          ((k == Qt::Key_T) || (k == Qt::Key_K) || (k == Qt::Key_Y))) {
        this->setText(QString(
            QChar(k == Qt::Key_T ? 't' : (k == Qt::Key_K ? 'k' : 'y'))));
        if (qle != nullptr) {
          qle->userEditingFinished();
        }
        return;
      }
      // every other key is edited as usual (Return, Backspace, the
      // ↑/↓ history, plain characters ...): the previous code neither
      // chained to CustomLineEdit::keyPressEvent nor guarded the call
      // to userEditingFinished(), so any stray key (space, z, an arrow)
      // ended the C-x r command with empty input
      CommandLine::CustomLineEdit::keyPressEvent(ev);
    }

   protected:
    TextEditBase& textEdit;
  };

  QVector<QString> RectangleMode::killedRectangle;

  RectangleMode::RectangleMode(TextEditBase& t, EditorWidget& p)
      : CommandLine(QObject::tr("Ctrl-X r "), p, false), textEdit(t) {
    this->setLineEdit(new RectangleModeLineEdit(t, p, *this));
  }  // end of RectangleMode::RectangleMode

  bool RectangleMode::isBlocking() const {
    return true;
  }  // end of RectangleMode::isBlocking

  void RectangleMode::treatUserInput() {
    const QString m = this->input->text();
    TextEditBase& t = this->textEdit;
    EditorWidget& q = this->editor;
    if (m == "t") {
      this->editor.removeUserInput(this);
      this->editor.setUserInput(new InsertRectangle(t, q));
      return;
    } else if (m == "k") {
      this->killRectangle();
    } else if (m == "y") {
      this->pasteRectangle();
    }
  }  // end of RectangleMode::treatUserInput

  /*!
   * \brief inserts the stored rectangle with its upper left corner at the
   * cursor
   *
   * The column of the cursor is the column of every piece, the selection is
   * nothing to the command, and what the lines hold from that column on is
   * kept: the rectangle is inserted, not written over the text. A line shorter
   * than that column is padded with blanks up to it, and the lines the pieces
   * need below the end of the buffer are opened, except for the empty line the
   * final newline of a file opens, which is already the start of a line and
   * takes the pieces on its own place.
   *
   * The cursor ends behind the last piece written, at the lower right corner of
   * the pasted rectangle, where Emacs leaves it.
   */
  void RectangleMode::pasteRectangle() {
    if (RectangleMode::killedRectangle.isEmpty()) {
      this->editor.displayInformativeMessage(
          QObject::tr("no rectangle killed"));
      return;
    }
    QTextCursor tc = this->textEdit.textCursor();
    tc.clearSelection();
    tc.beginEditBlock();
    const int column = tc.positionInBlock();
    bool first = true;
    for (const auto& piece : RectangleMode::killedRectangle) {
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
    this->textEdit.setTextCursor(tc);
  }  // end of RectangleMode::pasteRectangle

  /*!
   * \brief deletes the rectangle the selection covers and stores it
   *
   * The lines the two ends of the selection stand on are part of the rectangle.
   * What each of them held between its edges is stored padded with blanks up to
   * the width of the rectangle, so a line which stopped short of the right edge
   * gives a piece as wide as the rest and a yank puts the rectangle back where
   * it was taken from; the line itself is not padded, it only loses what the
   * rectangle covered, which is why deleting what a line does not hold leaves
   * that line alone.
   *
   * The cursor keeps the line it stands on and goes back by the width of the
   * text deleted before it, stopping at the left edge of the rectangle when it
   * stood inside of it: the place it names is a place of the text, and the text
   * in front of it has moved.
   */
  void RectangleMode::killRectangle() {
    QTextCursor tc = this->textEdit.textCursor();
    const Rectangle rect = getRectangle(tc);
    const int width = rect.right - rect.left;
    QTextDocument* const doc = tc.document();
    QVector<QString> killed;
    tc.beginEditBlock();
    for (int line = rect.firstBlock; line <= rect.lastBlock; ++line) {
      const QTextBlock block = doc->findBlockByNumber(line);
      const QString line_text = block.text();
      killed << line_text.mid(rect.left, width)
                    .leftJustified(width, ' ', false);
      setLineOfDocument(tc, block,
                        lineOf(line_text, rect.left, rect.right, QString(),
                               false));
    }
    tc.endEditBlock();
    RectangleMode::killedRectangle = killed;
    int column = rect.cursorColumn;
    if ((rect.cursorBlock >= rect.firstBlock) and
        (rect.cursorBlock <= rect.lastBlock)) {
      column -= qBound(0, rect.cursorColumn - rect.left, width);
    }
    gotoColumn(tc, doc->findBlockByNumber(rect.cursorBlock), column);
    this->textEdit.setTextCursor(tc);
  }  // end of RectangleMode::killRectange

}  // end of namespace tfel::gui
