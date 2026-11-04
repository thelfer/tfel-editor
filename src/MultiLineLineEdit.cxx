/*!
 * \file   MultiLineLineEdit.cxx
 * \brief
 * \author Thomas Helfer
 * \date   03/11/2026
 */

#include <QtCore/QEvent>
#include <QtCore/QCoreApplication>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QFontMetrics>
#include <QtGui/QKeyEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QTextDocument>
#include <QtGui/QValidator>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include "TFEL/GUI/MultiLineLineEdit.hxx"

namespace tfel::gui {

  namespace {

    /*!
     * \return the default button of the given widget hierarchy, that is the
     * first visible button which is both the default one and an auto default
     * one, null if there is none
     * \param[in] w: widget the search starts from
     *
     * \note this is what `QDialog::keyPressEvent` does when it gets `Return`,
     * which is how `Return` validated the dialogs of the editor while the
     * inputs were `QLineEdit`s, and what is emulated in
     * `MultiLineLineEdit::keyPressEvent`
     */
    QAbstractButton* findDefaultButton(QWidget* const w) {
      if ((!w->isVisible()) || (!w->isEnabled())) {
        return nullptr;
      }
      if (auto* const b = qobject_cast<QPushButton*>(w)) {
        return ((b->isDefault()) && (b->autoDefault()) && (b->isEnabled()))
                   ? static_cast<QAbstractButton*>(b)
                   : nullptr;
      }
      for (auto* const c : w->children()) {
        // not every child of a widget is a widget: line edits hold validators,
        // documents and other plain QObjects
        if (auto* const cw = qobject_cast<QWidget*>(c)) {
          if (auto* const r = findDefaultButton(cw)) {
            return r;
          }
        }
      }
      return nullptr;
    }  // end of findDefaultButton

    /*!
     * \brief true when the height of a widget is what its own content decides
     * \param[in] w: the widget
     * \param[in] l: the layout of that widget
     * \param[in] slack: the height a single line of the input takes, which the
     * widget is allowed to be taller than its content by, the growth of the
     * content being what the propagation is about
     *
     * A widget is only handed the height its layout asks for while it has no
     * reason to be taller than that: a widget which is, the editor of the
     * application among them, has been given room by the layout which encloses
     * it, the room the widgets it shares that space with live on. Giving such a
     * widget the height of its own layout hint shrinks it to a fraction of the
     * window, leaves the room it was given empty and pushes its own content to
     * the bottom of what is left. A widget which asks to expand, or to which
     * the layout above hands a share of the room, is not sized by its contents
     * either.
     */
    bool sizesToItsContents(const QWidget& w,
                            const QLayout& l,
                            const int slack) {
      const auto policy = w.sizePolicy();
      const auto v = policy.verticalPolicy();
      if ((v == QSizePolicy::Expanding) ||
          (v == QSizePolicy::MinimumExpanding) ||
          (policy.verticalStretch() > 0)) {
        return false;
      }
      return w.height() <= l.totalSizeHint().height() + slack;
    }  // end of sizesToItsContents

    /*!
     * \brief give a dialog the height its content asks for, when the content
     * shrank back
     * \param[in] w: the dialog
     * \param[in] one_line: true when the input which asked for this is a single
     * line tall, that is when the dialog needs no more room than it did when it
     * was opened
     * \param[inout] resting: the smallest height seen for the dialog while the
     * input was a single line tall
     * \param[inout] last: the height the dialog was given by the last call,
     * which tells a dialog that only followed the content from one somebody
     * resized
     *
     * A dialog is sized by its content and followed the growth of its input
     * through the height that input imposed on itself, but Qt only honours a
     * change of the minimum size when it grows: the dialog would stay as tall
     * as the tallest text ever typed in it. It is given back the height it had
     * before the content grew, which is the one it was opened with and which
     * its layout hint, computed from the settled height of the inputs, can be
     * smaller than, never less than what its layout asks for, and never when
     * somebody resized the dialog in the meantime, in which case that geometry
     * is the one to respect.
     */

    void fitDialog(QWidget* const w,
                   const bool one_line,
                   int& resting,
                   int& last) {
      if ((qobject_cast<QDialog*>(w) == nullptr) || (w->layout() == nullptr)) {
        return;
      }
      if ((one_line) && ((resting < 0) ||
                         ((last == w->height()) && (w->height() < resting)))) {
        resting = w->height();
      }
      const auto h = w->layout()->totalSizeHint().height();
      const auto target = qMax(h, resting);
      if ((last == w->height()) && (target < w->height())) {
        w->resize(w->width(), target);
      }
      last = w->height();
    }  // end of fitDialog

  }  // end of anonymous namespace

  MultiLineLineEdit::MultiLineLineEdit(QWidget* const p) : QTextEdit(p) {
    // the pasted content is inserted as plain text: this widget holds prompts
    // and commands, never formatted documents
    this->setAcceptRichText(false);
    // the text is wrapped at the width of the widget, so a horizontal scroll
    // bar is never required
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // the default document margin of `QTextEdit` (4 pixels) would make the
    // single line input noticeably taller than a `QLineEdit`; the margin and
    // the frame together give a line its padding, see `alignDocumentMargin`
    this->alignDocumentMargin();
    QObject::connect(this->document(), &QTextDocument::contentsChange, this,
                     [this](int, int, int) { this->onContentsChange(); });
    this->updateHeight();
  }  // end of MultiLineLineEdit::MultiLineLineEdit

  MultiLineLineEdit::~MultiLineLineEdit() {
    // the window is not filtered any more: it may outlive this widget, which is
    // the case of the editor window and of every user input of the minibuffer
    if (!this->window_.isNull()) {
      this->window_->removeEventFilter(this);
    }
  }  // end of ~MultiLineLineEdit()

  QString MultiLineLineEdit::text() const {
    // `toPlainText` separates the blocks with the paragraph separator
    // U+2029: it is translated back so that the value round-trips through
    // `setText` and compares to plain strings as a `QLineEdit`'s does
    auto t = this->toPlainText();
    t.replace(QChar(0x2029), QChar('\n'));
    return t;
  }  // end of MultiLineLineEdit::text

  void MultiLineLineEdit::setText(const QString& t) {
    this->setPlainText(t);
    auto c = this->textCursor();
    c.movePosition(QTextCursor::End);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::setText

  void MultiLineLineEdit::insert(const QString& t) {
    if (!this->acceptsInsert(t)) {
      return;
    }
    auto c = this->textCursor();
    // as in a `QLineEdit`, the selection, if any, is replaced
    c.insertText(t);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::insert

  int MultiLineLineEdit::cursorPosition() const {
    return this->textCursor().position();
  }  // end of MultiLineLineEdit::cursorPosition

  int MultiLineLineEdit::selectionStart() const {
    const auto c = this->textCursor();
    return c.hasSelection() ? c.selectionStart() : -1;
  }  // end of MultiLineLineEdit::selectionStart

  void MultiLineLineEdit::setSelection(const int pos, const int len) {
    auto c = this->textCursor();
    c.setPosition(pos, QTextCursor::MoveAnchor);
    if (len > 0) {
      c.setPosition(pos + len, QTextCursor::KeepAnchor);
    }
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::setSelection

  void MultiLineLineEdit::home(const bool withSelection) {
    auto c = this->textCursor();
    c.movePosition(QTextCursor::StartOfBlock, withSelection
                                                  ? QTextCursor::KeepAnchor
                                                  : QTextCursor::MoveAnchor);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::home

  void MultiLineLineEdit::end(const bool withSelection) {
    auto c = this->textCursor();
    c.movePosition(QTextCursor::EndOfBlock, withSelection
                                                ? QTextCursor::KeepAnchor
                                                : QTextCursor::MoveAnchor);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::end

  void MultiLineLineEdit::cursorBackward(const bool withSelection,
                                         const int n) {
    auto c = this->textCursor();
    c.movePosition(
        QTextCursor::Left,
        withSelection ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, n);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::cursorBackward

  void MultiLineLineEdit::cursorForward(const bool withSelection, const int n) {
    auto c = this->textCursor();
    c.movePosition(
        QTextCursor::Right,
        withSelection ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor, n);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::cursorForward

  void MultiLineLineEdit::cursorWordBackward(const bool withSelection) {
    auto c = this->textCursor();
    // `PreviousWord` is the Emacs `Alt-b` behaviour: the beginning of the
    // current word, or of the previous one when the cursor already sits at
    // the beginning of a word, as `QLineEdit::cursorWordBackward` does
    c.movePosition(QTextCursor::PreviousWord, withSelection
                                                  ? QTextCursor::KeepAnchor
                                                  : QTextCursor::MoveAnchor);
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::cursorWordBackward

  void MultiLineLineEdit::cursorWordForward(const bool withSelection) {
    auto c = this->textCursor();
    const auto before = c.position();
    const auto mode =
        withSelection ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    c.movePosition(QTextCursor::EndOfWord, mode);
    if (c.selectionEnd() == before && (!withSelection || !c.hasSelection())) {
      // the cursor sits at the end of a word already: the next word is the
      // target, as `QLineEdit::cursorWordForward` does
      if (c.movePosition(QTextCursor::Right, mode)) {
        c.movePosition(QTextCursor::EndOfWord, mode);
      }
    }
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::cursorWordForward

  void MultiLineLineEdit::backspace() {
    auto c = this->textCursor();
    if (c.hasSelection()) {
      c.removeSelectedText();
    } else {
      c.deletePreviousChar();
    }
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::backspace

  void MultiLineLineEdit::del() {
    auto c = this->textCursor();
    if (c.hasSelection()) {
      c.removeSelectedText();
    } else {
      c.deleteChar();
    }
    this->setTextCursor(c);
  }  // end of MultiLineLineEdit::del

  void MultiLineLineEdit::setValidator(const QValidator* const v) {
    this->v_ = v;
  }  // end of MultiLineLineEdit::setValidator

  const QValidator* MultiLineLineEdit::validator() const {
    return this->v_;
  }  // end of MultiLineLineEdit::validator

  void MultiLineLineEdit::keyPressEvent(QKeyEvent* const ev) {
    const auto k = ev->key();
    const auto m = ev->modifiers();
    if ((k == Qt::Key_Return || k == Qt::Key_Enter) &&
        !(m & Qt::ShiftModifier)) {
      // as in a `QLineEdit`, `Return` validates the input instead of
      // inserting a new line; `Shift+Return` falls through to `QTextEdit`,
      // which inserts a new line
      ev->accept();
      emit returnPressed();
      // `QLineEdit` leaves the `Return` event unconsumed, so it reaches the
      // `QDialog` it belongs to, which clicks its default button, that is the
      // first button which is both the default one and an auto default one.
      // This widget consumes the event, as it must to avoid inserting a new
      // line, so the same button is clicked here.
      if (auto* const w = this->window()) {
        if (auto* const b = findDefaultButton(w)) {
          b->click();
        }
      }
      return;
    }
    const auto t = ev->text();
    if (!(t.isEmpty()) &&
        !(m & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) &&
        t[0].isPrint() && !this->acceptsInsert(t)) {
      // the validator rejects the resulting text: the key is ignored, as
      // `QLineEdit` does
      ev->ignore();
      return;
    }
    QTextEdit::keyPressEvent(ev);
  }  // end of MultiLineLineEdit::keyPressEvent

  void MultiLineLineEdit::changeEvent(QEvent* const ev) {
    QTextEdit::changeEvent(ev);
    switch (ev->type()) {
      case QEvent::FontChange:
      case QEvent::StyleChange:
      case QEvent::Polish:
      case QEvent::ContentsRectChange: {
        // the height of a line depends on the font and on the frame, and both
        // are set after the construction of the widget: the font comes from the
        // settings or from the dialog hosting the input, the frame is removed
        // by a command line. The height of the widget, which is a fixed one, is
        // computed again, the padding of the line shared again between the
        // frame and the margin of the document
        this->updateHeight();
        break;
      }
      default:
        break;
    }
  }  // end of MultiLineLineEdit::changeEvent

  void MultiLineLineEdit::showEvent(QShowEvent* const ev) {
    QTextEdit::showEvent(ev);
    // a widget which was built before the font of the application was read from
    // the settings, and which was kept hidden since, has a height computed with
    // the metrics of another font: it is only when it is shown that it gets the
    // one it is drawn with, and the height of the line it holds with it
    this->updateHeight();
  }  // end of MultiLineLineEdit::showEvent

  void MultiLineLineEdit::resizeEvent(QResizeEvent* const ev) {
    QTextEdit::resizeEvent(ev);
    // the wrapped text depends on the width: the height is recomputed each
    // time the widget is resized. A layout which has not been activated since
    // the content changed may give this widget a stale height when it is
    // resized, which updateHeight() reports by scheduling a propagation.
    this->updateHeight();
  }  // end of MultiLineLineEdit::resizeEvent

  int MultiLineLineEdit::singleLineHeight() const {
    // the height of an empty document is the height of a single line, the
    // margins and the frame included
    return this->fontMetrics().height() +
           2 * static_cast<int>(this->document()->documentMargin()) +
           2 * this->frameWidth();
  }  // end of singleLineHeight

  int MultiLineLineEdit::firstLineTop() const {
    // the view port is set inside the frame, below the margins of the view
    // port, and the document is drawn inside the view port, at the margin of
    // the document from its edges
    const auto vp = (this->viewport() != nullptr) ? this->viewport()->y() : 0;
    return vp + static_cast<int>(this->document()->documentMargin());
  }  // end of firstLineTop

  QWidget* MultiLineLineEdit::boundedWindow() const {
    auto* const w = this->window();
    if ((w == nullptr) || (w == this) || (!w->isWindow()) ||
        (qobject_cast<QDialog*>(w) != nullptr)) {
      return nullptr;
    }
    return w;
  }  // end of boundedWindow

  void MultiLineLineEdit::watchWindow() {
    auto* const w = this->boundedWindow();
    if (w == this->window_) {
      return;
    }
    if (!this->window_.isNull()) {
      this->window_->removeEventFilter(this);
    }
    this->window_ = w;
    if (w != nullptr) {
      w->installEventFilter(this);
    }
  }  // end of watchWindow

  bool MultiLineLineEdit::eventFilter(QObject* const o, QEvent* const ev) {
    if ((o == this->window_) && (ev->type() == QEvent::Resize)) {
      // the window the height of the content is bounded by changed size: the
      // scroll bar may have to take over, or the content to grow back
      this->updateHeight();
    }
    return QTextEdit::eventFilter(o, ev);
  }  // end of eventFilter

  int MultiLineLineEdit::maximumContentHeight() const {
    // the content is never allowed to take more than max_lines lines, nor more
    // than a quarter of the window the widget belongs to: an input taking half
    // of the editor window would leave too little room to the documents the
    // user is editing. A quarter of a dialog bounds nothing, as a dialog is
    // sized by its content, so the number of lines is the only cap there.
    const auto line = this->fontMetrics().height();
    const auto min_h = this->singleLineHeight();
    const auto lines_h = min_h + (max_lines - 1) * line;
    auto* const w = this->boundedWindow();
    if (w == nullptr) {
      return lines_h;
    }
    // a single line is always allowed, however small the window is: the text
    // being edited must remain visible
    return qMax(min_h, qMin(lines_h, w->height() / 4));
  }  // end of maximumContentHeight

  void MultiLineLineEdit::alignDocumentMargin() {
    // a line is given `line_padding` pixels of padding, whatever the widget
    // shows it: the frame the widget is drawn with takes its part of them and
    // the document margin holds the rest, so that a framed input and the
    // frameless one of a command line are, for a single line, exactly as tall
    const auto m = qMax(0, (line_padding - 2 * this->frameWidth()) / 2);
    if (static_cast<int>(this->document()->documentMargin()) != m) {
      this->document()->setDocumentMargin(m);
    }
  }  // end of alignDocumentMargin

  void MultiLineLineEdit::updateHeight() {
    this->watchWindow();
    // the frame may have been removed since the margin was last computed, by
    // the command line the widget was made the input of
    this->alignDocumentMargin();
    // the document is laid out in the width the widget has for itself: its own
    // width, the frame and the margins of the view port taken off, the room a
    // vertical scroll bar would take left free. Measuring in the view port
    // instead counts the lines the content makes in a column narrower than the
    // one it is drawn in, either because the view port of a widget which was
    // never shown, or which was resized while it was hidden, still has the
    // width it was built with, or because a bar shown for a content which does
    // not fit the height of the moment takes room the text would have needed: a
    // line more is asked for, which asks for a bar more, and so on
    const auto vm = this->viewportMargins();
    const auto inner =
        this->width() - 2 * this->frameWidth() - vm.left() - vm.right();
    if ((inner > 0) && (qAbs(this->document()->textWidth() - inner) > 0.5)) {
      this->document()->setTextWidth(inner);
    }
    const auto min_h = this->singleLineHeight();
    // the document layout is only up to date once its size has been asked for
    const auto h =
        qCeil(this->document()->documentLayout()->documentSize().height()) +
        2 * this->frameWidth();
    const int target = qMax(min_h, qMin(h, this->maximumContentHeight()));
    if (this->contentHeight_ != target) {
      this->contentHeight_ = target;
      this->setFixedHeight(target);
      this->propagationTries_ = 0;
    } else if ((this->height() == target) || (!this->isVisible())) {
      // the widget has the height of its content, or is hidden, in which case
      // there is nothing to lay out: the layouts above know nothing new
      return;
    } else if (this->propagationTries_ == 0) {
      // the widget does not have the height of its content: the layout which
      // drives it has not been activated since the content changed
      --this->propagationTries_;
    } else {
      // the height of the content is known and the widget was given one
      // propagation already without effect: the layout driving this widget is
      // not the one to blame, asking again would achieve nothing
      return;
    }
    this->schedulePropagation();
  }  // end of MultiLineLineEdit::updateHeight

  void MultiLineLineEdit::schedulePropagation() {
    if (this->pendingPropagation_) {
      return;
    }
    this->pendingPropagation_ = true;
    // the enclosing layouts can not be activated while this widget is being
    // resized: the propagation is deferred to the event loop, where it resizes
    // the command line, the minibuffer and whatever encloses them.
    // the event is posted with a high priority: it is dealt with before the
    // layout requests the change of the constraints posted, and it is run again
    // by the loop of the event loop which deals with the events of a given
    // priority until there is none left. The chain of layouts therefore settles
    // during a single pass of the event loop, as it would as soon as the editor
    // spins one.
    QCoreApplication::postEvent(this,
                                new QEvent(QEvent::Type(QEvent::User + 1)),
                                Qt::HighEventPriority);
  }  // end of MultiLineLineEdit::schedulePropagation

  bool MultiLineLineEdit::event(QEvent* const ev) {
    if (ev->type() == QEvent::Type(QEvent::User + 1)) {
      this->propagateHeightToLayout();
      return true;
    }
    return QTextEdit::event(ev);
  }  // end of MultiLineLineEdit::event

  void MultiLineLineEdit::propagateHeightToLayout() {
    // `setFixedHeight` changes the geometry of this widget and the size hint of
    // the layouts it takes part in, but leaves the size of the widgets those
    // layouts drive: the height of the command line, of the minibuffer, and of
    // whatever else encloses the input would stay the one computed for a one
    // line input. Activating the layouts is not enough either: a layout applies
    // the geometry it computed when it was last valid, and activating a layout
    // whose own widget is not up to date yet hands a stale height up, so that
    // each level lags one event behind the one below it. The height is
    // therefore handed up by hand, the innermost widget first: each widget on
    // the way is activated, resized to the height its own layout asks for, and
    // the layout above it is activated only once, at the very end, so that the
    // widgets sharing that space, the buffers of the editor for instance, are
    // given what is left.
    this->pendingPropagation_ = false;
    // a single line tall input needs no more room than the one the enclosing
    // window had when it was opened, which is what a dialog is given back
    const auto one_line = this->height() <= this->singleLineHeight();
    auto currentHeight = this->height();
    // the height is handed from a widget to the one which encloses it as long
    // as each of them is no taller than its own content asks: from the moment
    // one of them has room of its own, that room is nothing the input has to
    // say a word about and the layouts above are only activated, so that the
    // widgets sharing the space are given what is left of it
    auto hand_height_up = true;
    auto* top = static_cast<QWidget*>(nullptr);
    for (auto* w = static_cast<QWidget*>(this->parentWidget()); w != nullptr;
         w = w->parentWidget()) {
      auto* const l = w->layout();
      if (l == nullptr) {
        break;
      }
      // the layout of the widget is activated first: the size hint the widget
      // above reads is only up to date afterwards
      l->activate();
      auto* const pw = w->parentWidget();
      if (pw == nullptr) {
        // a window driven by its own layout, that is a dialog: there is no
        // widget above to hand the height over to
        fitDialog(w, one_line, this->restingDialogHeight_,
                  this->propagatedDialogHeight_);
        return;
      }
      if ((hand_height_up) &&
          ((qobject_cast<QSplitter*>(pw) != nullptr) ||
           (!sizesToItsContents(*w, *l, this->singleLineHeight())))) {
        hand_height_up = false;
      }
      // the widget is never given more room than its parent has: a page given
      // the height of a window taller than it would reach beyond the window,
      // and the line being typed, at the bottom of the page, would be drawn
      // outside of the screen
      const auto h = qMin(l->totalSizeHint().height(), qMax(1, pw->height()));
      if ((hand_height_up) && (h != currentHeight)) {
        auto g = w->geometry();
        // a widget stuck to the bottom of its parent, which is the case of the
        // minibuffer, keeps that edge when it grows, the other ones keep their
        // top edge
        const auto stuckToBottom = (pw->height() - g.bottom()) <= 1;
        g.setHeight(h);
        if (stuckToBottom) {
          g.moveBottom(pw->height());
        }
        w->setGeometry(g);
        currentHeight = h;
      }
      if (pw->isWindow()) {
        top = pw;
        break;
      }
    }
    if ((top == nullptr) || (top->layout() == nullptr)) {
      return;
    }
    top->layout()->activate();
    fitDialog(top, one_line, this->restingDialogHeight_,
              this->propagatedDialogHeight_);
  }  // end of MultiLineLineEdit::propagateHeightToLayout

  QSize MultiLineLineEdit::sizeHint() const {
    // `QAbstractScrollArea::sizeHint` asks for a wide, tall scroll area: this
    // widget is a line edit, so the width hint is the one `QLineEdit` gives,
    // seventeen `x` characters, and the height hint is the height of the
    // current content, which is the height the widget is fixed to
    const auto fm = this->fontMetrics();
    const auto m = 2 * (static_cast<int>(this->document()->documentMargin()) +
                        this->frameWidth());
    const auto w = 17 * fm.horizontalAdvance(QChar('x')) + m;
    const auto h =
        this->contentHeight_ > 0 ? this->contentHeight_ : fm.height() + m;
    return {w, h};
  }  // end of MultiLineLineEdit::sizeHint

  void MultiLineLineEdit::onContentsChange() {
    emit textChanged(this->text());
    this->updateHeight();
  }  // end of MultiLineLineEdit::onContentsChange

  bool MultiLineLineEdit::acceptsInsert(const QString& t) const {
    if (this->v_ == nullptr) {
      return true;
    }
    const auto c = this->textCursor();
    const auto b = c.hasSelection() ? c.selectionStart() : c.position();
    const auto e = c.hasSelection() ? c.selectionEnd() : c.position();
    auto proposed = this->text();
    proposed.replace(b, e - b, t);
    int p = b + static_cast<int>(t.size());
    return this->v_->validate(proposed, p) != QValidator::Invalid;
  }  // end of MultiLineLineEdit::acceptsInsert

}  // end of namespace tfel::gui
