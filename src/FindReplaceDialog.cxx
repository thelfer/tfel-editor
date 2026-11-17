/*!
 * \file  FindReplaceDialog.cxx
 * \brief
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#include <QtCore/QObject>
#include <QtCore/QLatin1Char>
#include <QtCore/QString>

#include <QtGui/QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>

#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/FindReplaceDialog.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"
#include "TFEL/GUI/StandardFunctionCommand.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel::gui {

  namespace {

    /*!
     * \return the text of a selection, which is a text of a document and not a
     * rectangle of the display: `QTextCursor::selectedText` spells the end of a
     * line with the paragraph separator of the document, which an expression of
     * the find field would read as a metacharacter of its own.
     */
    static QString textOfSelection(const QTextCursor& tc) {
      QString s = tc.selectedText();
      s.replace(QChar(0x2029), QLatin1Char('\n'));
      return s;
    }  // end of textOfSelection()

    /*!
     * \brief writes what a pattern replaces a match with
     * \param[in,out] tc : cursor of the document, used for the edit
     * \param[in] pattern : the pattern, whose replacement is written
     * \param[in] match : the place in the document the match holds
     * \return the text which was written
     *
     * The cursor ends behind the text written, which is the place the next
     * search of a walk starts from.
     */
    static QString replaceMatch(QTextCursor& tc,
                                const SearchPattern& pattern,
                                const SearchMatch& match) {
      const QString text = pattern.replacementOf(match);
      tc.setPosition(match.start);
      tc.setPosition(match.end, QTextCursor::KeepAnchor);
      tc.removeSelectedText();
      tc.insertText(text);
      return text;
    }  // end of replaceMatch()

  }  // end of anonymous namespace

  FindReplaceDialog::FindReplaceDialog(EditorWidget& editor_)
      : QDialog(&editor_), editor(editor_) {
    this->setObjectName("find-replace");
    this->setWindowTitle(QObject::tr("Find/Replace"));
    this->createWidgets();
  }  // end of FindReplaceDialog::FindReplaceDialog

  void FindReplaceDialog::createWidgets() {
    auto* const grid = new QGridLayout(this);
    this->findField = new QLineEdit(this);
    this->findField->setObjectName("find-field");
    this->findField->setMinimumWidth(240);
    this->replaceField = new QLineEdit(this);
    this->replaceField->setObjectName("replace-field");
    this->caseOption = new QCheckBox(QObject::tr("Match &case"), this);
    this->caseOption->setObjectName("match-case");
    this->caseOption->setChecked(true);
    this->wordsOption = new QCheckBox(QObject::tr("Whole &words"), this);
    this->wordsOption->setObjectName("whole-words");
    this->regexpOption = new QCheckBox(QObject::tr("&Regular expression"), this);
    this->regexpOption->setObjectName("regular-expression");
    this->findNextButton = new QPushButton(QObject::tr("Find &next"), this);
    this->findNextButton->setObjectName("find-next");
    this->findPreviousButton =
        new QPushButton(QObject::tr("Find &previous"), this);
    this->findPreviousButton->setObjectName("find-previous");
    this->replaceButton = new QPushButton(QObject::tr("&Replace"), this);
    this->replaceButton->setObjectName("replace");
    this->replaceAllButton = new QPushButton(QObject::tr("Replace &all"), this);
    this->replaceAllButton->setObjectName("replace-all");
    this->closeButton = new QPushButton(this);
    this->closeButton->setObjectName("close");
    // the dialog hides rather than closes: what was typed in it is what the
    // next call to showFor brings back
    this->closeButton->setIcon(QIcon::fromTheme("dialog-close",
                                               QIcon::fromTheme("window-close")));
    this->closeButton->setText(QObject::tr("&Close"));
    this->status = new QLabel(this);
    this->status->setObjectName("status");
    this->status->setWordWrap(true);
    grid->addWidget(new QLabel(QObject::tr("Find :"), this), 0, 0);
    grid->addWidget(this->findField, 0, 1);
    grid->addWidget(this->findNextButton, 0, 2);
    grid->addWidget(new QLabel(QObject::tr("Replace with :"), this), 1, 0);
    grid->addWidget(this->replaceField, 1, 1);
    grid->addWidget(this->findPreviousButton, 1, 2);
    auto* const options = new QHBoxLayout;
    options->addWidget(this->caseOption);
    options->addWidget(this->wordsOption);
    options->addWidget(this->regexpOption);
    // the three options stand on one line under the two fields, and the two
    // buttons which write stand under the two which only ask
    grid->addLayout(options, 2, 0, 1, 2);
    grid->addWidget(this->replaceButton, 2, 2);
    grid->addWidget(this->replaceAllButton, 3, 2);
    grid->addWidget(this->closeButton, 4, 2);
    grid->addWidget(this->status, 5, 0, 1, 3);
    grid->setColumnStretch(1, 1);
    // the button of a question which changes the buffer reads as the more
    // dangerous one of the two which stand beside each other
    QObject::connect(this->findNextButton, &QAbstractButton::clicked, this,
                     &FindReplaceDialog::findNext);
    QObject::connect(this->findPreviousButton, &QAbstractButton::clicked, this,
                     &FindReplaceDialog::findPrevious);
    QObject::connect(this->replaceButton, &QAbstractButton::clicked, this,
                     &FindReplaceDialog::replaceOne);
    QObject::connect(this->replaceAllButton, &QAbstractButton::clicked, this,
                     &FindReplaceDialog::replaceAllMatches);
    QObject::connect(this->closeButton, &QAbstractButton::clicked, this,
                     &FindReplaceDialog::closeDialog);
  }  // end of FindReplaceDialog::createWidgets

  SearchPattern FindReplaceDialog::pattern() const {
    return SearchPattern(this->findField->text(),
                         this->regexpOption->isChecked(),
                         this->caseOption->isChecked(),
                         this->wordsOption->isChecked(),
                         this->replaceField->text());
  }  // end of FindReplaceDialog::pattern

  FindReplaceDialog::Outcome FindReplaceDialog::selectNextMatch(
      const SearchPattern& search, const bool backward) {
    auto& edit = this->textEdit();
    auto* const document = edit.document();
    if (document == nullptr) {
      return Outcome::NotFound;
    }
    const auto tc = edit.textCursor();
    /* the search starts at the far end of the selection, which is the place the
       two directions walk from: the selection a match holds is passed over, so
       the next match is the next one and not the one on the screen, and with no
       selection at all that end is the cursor. */
    const int from = (backward) ? tc.selectionStart() : tc.selectionEnd();
    const int size = static_cast<int>(document->toPlainText().size());
    auto match = search.find(*document, from, backward);
    auto outcome = Outcome::Found;
    if (match.isNull()) {
      // the search wraps once: a match behind the cursor would otherwise stay
      // found in silence, two lines from where the reader was
      outcome = Outcome::Wrapped;
      match = search.find(*document, (backward) ? size : 0, backward);
    }
    if (match.isNull()) {
      return Outcome::NotFound;
    }
    auto found = QTextCursor(document);
    found.setPosition(match.start);
    found.setPosition(match.end, QTextCursor::KeepAnchor);
    edit.setTextCursor(found);
    edit.centerCursor();
    return outcome;
  }  // end of FindReplaceDialog::selectNextMatch

  TextEditBase& FindReplaceDialog::textEdit() const {
    return this->editor.getCurrentBuffer().getMainFrame();
  }  // end of FindReplaceDialog::textEdit

  void FindReplaceDialog::report(const QString& message) {
    this->status->setText(message);
  }  // end of FindReplaceDialog::report

  void FindReplaceDialog::reportNotFound(const QString& what) {
    this->report(QObject::tr("'%1' not found").arg(what));
  }  // end of FindReplaceDialog::reportNotFound

  void FindReplaceDialog::showFor(EditorWidget& editor) {
    auto* const dialog = editor.findChild<FindReplaceDialog*>("find-replace");
    auto* const built = (dialog != nullptr) ? dialog
                                           : new FindReplaceDialog(editor);
    /* what is on the screen is the first thing the reader wants looked for, and
       the two calls have to answer the same way: a selection dragged over a
       paragraph is none, and so is a text of several lines, which the newline of
       the whole text would make an expression of the find field read as a
       metacharacter of its own. */
    if (built->findField->text().isEmpty()) {
      const auto selected = textOfSelection(built->textEdit().textCursor());
      if ((not selected.isEmpty()) and (not selected.contains('\n'))) {
        built->findField->setText(selected);
      }
    }
    built->show();
    built->raise();
    built->activateWindow();
    built->findField->setFocus();
  }  // end of FindReplaceDialog::showFor

  void FindReplaceDialog::keyPressEvent(QKeyEvent* const ev) {
    if ((ev->key() == Qt::Key_Return) or (ev->key() == Qt::Key_Enter)) {
      if (ev->modifiers().testFlag(Qt::ShiftModifier)) {
        this->findPrevious();
      } else {
        this->findNext();
      }
      return;
    }
    QDialog::keyPressEvent(ev);
  }  // end of FindReplaceDialog::keyPressEvent

  void FindReplaceDialog::find(const bool backward) {
    const auto search = this->pattern();
    if (not search.isValid()) {
      this->report(search.errorString());
      return;
    }
    if (search.pattern().isEmpty()) {
      this->report(QObject::tr("empty search"));
      return;
    }
    switch (this->selectNextMatch(search, backward)) {
      case Outcome::NotFound:
        this->reportNotFound(search.pattern());
        break;
      case Outcome::Wrapped:
        this->report(QObject::tr("'%1' searched from the %2 of the buffer")
                         .arg(search.pattern())
                         .arg((backward) ? QObject::tr("end")
                                         : QObject::tr("beginning")));
        break;
      case Outcome::Found:
        this->report(QString());
        break;
    }
  }  // end of FindReplaceDialog::find

  bool FindReplaceDialog::replaceSelected() {
    const auto pattern = this->pattern();
    if (not pattern.isValid()) {
      this->report(pattern.errorString());
      return false;
    }
    if (pattern.pattern().isEmpty()) {
      this->report(QObject::tr("empty search"));
      return false;
    }
    auto& edit = this->textEdit();
    if (edit.isReadOnly()) {
      // the freeze of a file modified on disk, and any other read-only buffer,
      // takes no replacement: the two buttons which write are refused and the
      // two which only ask are answered, so a frozen buffer can still be walked
      this->report(QObject::tr("read only buffer"));
      return false;
    }
    auto* const document = edit.document();
    if (document == nullptr) {
      this->reportNotFound(pattern.pattern());
      return false;
    }
    auto tc = edit.textCursor();
    /* what is replaced is the match the start of the selection begins, and only
       when that match ends where the selection ends: an expression which would
       reach further than the selection is not what the reader selected, and an
       expression which stops short of it is not either. */
    auto match = pattern.startingAt(*document, tc.selectionStart());
    if ((match.isNull()) or (match.end != tc.selectionEnd())) {
      this->find();
      return false;
    }
    const auto written = replaceMatch(tc, pattern, match);
    edit.setTextCursor(tc);
    edit.setMoveMode(QTextCursor::MoveAnchor);
    this->report(QObject::tr("replaced '%1' with '%2'")
                     .arg(pattern.pattern())
                     .arg(written));
    /* the walk goes on: the match which follows what was written is put on the
       screen, so one press of the button is one match of the buffer. What that
       question answers is not reported: the button was asked to replace, and the
       next match is what the reader sees, not a question they asked. */
    this->selectNextMatch(pattern, false);
    return true;
  }  // end of FindReplaceDialog::replaceSelected

  void FindReplaceDialog::replaceAll() {
    const auto pattern = this->pattern();
    if (not pattern.isValid()) {
      this->report(pattern.errorString());
      return;
    }
    if (pattern.pattern().isEmpty()) {
      this->report(QObject::tr("empty search"));
      return;
    }
    auto& edit = this->textEdit();
    if (edit.isReadOnly()) {
      this->report(QObject::tr("read only buffer"));
      return;
    }
    auto* const document = edit.document();
    if (document == nullptr) {
      this->reportNotFound(pattern.pattern());
      return;
    }
    auto tc = edit.textCursor();
    const int from = qMax(tc.position(), tc.selectionEnd());
    tc.clearSelection();
    tc.beginEditBlock();
    int count = 0;
    int position = from;
    bool wrapped = false;
    while (true) {
      auto match = pattern.find(*document, position, false);
      if (match.isNull()) {
        if (wrapped) {
          break;
        }
        wrapped = true;
        position = 0;
        continue;
      }
      if (wrapped and (match.start >= from)) {
        // the walk came back to the place it started from
        break;
      }
      const auto written = replaceMatch(tc, pattern, match);
      ++count;
      position = match.start + written.size();
    }
    tc.endEditBlock();
    if (count == 0) {
      this->reportNotFound(pattern.pattern());
      return;
    }
    tc.setPosition(qMin(position, document->toPlainText().size()));
    edit.setTextCursor(tc);
    edit.setMoveMode(QTextCursor::MoveAnchor);
    if (count == 1) {
      this->report(QObject::tr("replaced 1 occurrence of '%1'")
                       .arg(pattern.pattern()));
    } else {
      this->report(QObject::tr("replaced %1 occurrences of '%2'")
                       .arg(count)
                       .arg(pattern.pattern()));
    }
  }  // end of FindReplaceDialog::replaceAll

  void FindReplaceDialog::findNext() {
    this->find(false);
  }  // end of FindReplaceDialog::findNext

  void FindReplaceDialog::findPrevious() {
    this->find(true);
  }  // end of FindReplaceDialog::findPrevious

  void FindReplaceDialog::replaceOne() {
    this->replaceSelected();
  }  // end of FindReplaceDialog::replaceOne

  void FindReplaceDialog::replaceAllMatches() {
    this->replaceAll();
  }  // end of FindReplaceDialog::replaceAllMatches

  void FindReplaceDialog::closeDialog() {
    this->hide();
  }  // end of FindReplaceDialog::closeDialog

  //! \brief the `find-replace` command, which is the `Edit` menu entry
  void showFindReplaceDialog(EditorWidget& w) {
    FindReplaceDialog::showFor(w);
  }  // end of showFindReplaceDialog()

  static StandardFunctionCommandProxy<showFindReplaceDialog>
      findReplaceProxy("find-replace");

}  // end of namespace tfel::gui
