/*!
 * \file  TextEditBase.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 19/08/2012
 */

#include <stdexcept>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QPointer>
#include <QtCore/QSettings>
#include <QtWidgets/QMenu>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QFontDialog>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QTextBlock>
#include <QtGui/QIntValidator>
#include <QtGui/QTextDocumentWriter>
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/RectangleMode.hxx"
#include "TFEL/GUI/TextEditSearch.hxx"
#include "TFEL/GUI/TextEditQueryReplace.hxx"
#include "TFEL/GUI/ShortCutStyle.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/HunspellDictionariesManager.hxx"
#include "TFEL/GUI/TextEditBase.hxx"

namespace tfel {

  namespace gui {

    TextEditBase::TextEditBase(EditorWidget& g, Buffer& b)
        : QWidget(&g), editor(g), buffer(b) {
      auto& dm =
          HunspellDictionariesManager::getHunspellDictionariesManager();
      this->spellCheckLanguage = dm.getDefaultSpellCheckLanguage();
    }  // end of TextEditBase::TextEditBase

    EditorWidget& TextEditBase::getEditorWidget() {
      return this->editor;
    }  // end of TextEditBase::getEditorWidget()

    const EditorWidget& TextEditBase::getEditorWidget() const {
      return this->editor;
    }  // end of TextEditBase::getEditorWidget()

    void TextEditBase::mouseMoveEvent(QMouseEvent*) {
    }  // end of TextEditBase::mouseMoveEvent

    void TextEditBase::format() {
      if (this->mode == nullptr) {
        return;
      }
      this->mode->format();
    }  // end of TextEditBase::format

    void TextEditBase::setFocus() {
      auto w = this->widget();
      if (w != nullptr) {
        w->setFocus();
      }
    }  // end of TextEditBase::setFocus

    void TextEditBase::setFont(const QFont& f) {
      auto* e = this->widget();
      e->setFont(f);
    }  // end of TextEditBase::setFont

    void TextEditBase::initialize(QAbstractScrollArea* const e) {
      this->ua = nullptr;
      this->ra = nullptr;
      this->ca = nullptr;
      this->coa = nullptr;
      this->pa = nullptr;
      this->sa = nullptr;
      // by default, change if main frame
      e->setMinimumHeight(200);
      // font
      QSettings s;
      if (s.contains("font/textedit")) {
        auto f = s.value("font/textedit").value<QFont>();
        f.setStyleStrategy(QFont::PreferAntialias);
        this->setFont(f);
      }
      // signals for line highlighting
      if (auto* te = qobject_cast<QTextEdit*>(e)) {
        QObject::connect(te, &QTextEdit::cursorPositionChanged, this,
                         &TextEditBase::highlightCurrentLine);
        QObject::connect(te, &QTextEdit::cursorPositionChanged, this,
                         &TextEditBase::emitCursorPositionChanged);
      } else if (auto* pte = qobject_cast<QPlainTextEdit*>(e)) {
        QObject::connect(pte, &QPlainTextEdit::cursorPositionChanged,
                         this, &TextEditBase::highlightCurrentLine);
        QObject::connect(pte, &QPlainTextEdit::cursorPositionChanged,
                         this,
                         &TextEditBase::emitCursorPositionChanged);
      }
      // event filter
      e->installEventFilter(this);
      e->viewport()->installEventFilter(this);
    }  // end of TextEditBase::initialize

    bool TextEditBase::eventFilter(QObject* o, QEvent* e) {
      if (o == this->widget()) {
        if (e->type() == QEvent::KeyPress) {
          auto* ke = static_cast<QKeyEvent*>(e);
          return this->handleKeyPressEvent(ke);
        }
      } else if (o == this->widget()->viewport()) {
        if (e->type() == QEvent::MouseButtonPress) {
          auto* me = static_cast<QMouseEvent*>(e);
          return this->handleMousePressEvent(me);
        }
      }
      // standard event processing
      return QObject::eventFilter(o, e);
    }

    QString TextEditBase::getModifier(const QKeyEvent& e) {
      QString m;
      if (e.modifiers() & Qt::ControlModifier) {
        m = "Ctrl";
      }
      if (e.modifiers() & Qt::AltModifier) {
        if (m.isEmpty()) {
          m = "Alt";
        } else {
          m += "-Alt";
        }
      }
      if (e.modifiers() & Qt::ShiftModifier) {
        if (m.isEmpty()) {
          m = "Shift";
        } else {
          m += "-Shift";
        }
      }
      if (e.modifiers() & Qt::MetaModifier) {
        if (m.isEmpty()) {
          m = "Meta";
        } else {
          m += "-Meta";
        }
      }
      return m;
    }  // end of TextEditBase::getModifier

    struct TextEditBase::KillOtherBufferAndWriteFile
        : public YesOrNoUserInput {
      KillOtherBufferAndWriteFile(EditorWidget& p,
                                  TextEditBase& t,
                                  Buffer& b,
                                  const QString& f)
          : YesOrNoUserInput(
                QObject::tr(
                    "another buffer is visiting file '%1', proceed ?")
                    .arg(f),
                p),
            textEdit(t),
            kb(b),
            file(f) {}  // end of KillOtherBufferAndWriteFile

      bool isBlocking() const override {
        return true;
      }  // end of isBlocking

     protected:
      void treatUserInput() override {
        if (this->input->text() == "y") {
          this->editor.closeBuffer(this->kb, false);
          this->textEdit.writeFile(this->file);
        }
      }  // end of treatUserInput

      TextEditBase& textEdit;
      Buffer& kb;
      QString file;
    };

    struct TextEditBase::OverWriteFile : public YesOrNoUserInput {
      OverWriteFile(EditorWidget& p, TextEditBase& t, const QString& f)
          : YesOrNoUserInput(
                QObject::tr("overwrite file '%1' :").arg(f), p),
            textEdit(t),
            file(f) {}  // end of OverWriteFile

      bool isBlocking() const override {
        return true;
      }  // end of isBlocking

     protected:
      void treatUserInput() override {
        if (this->input->text() == "y") {
          // check if another buffer does not visit this file
          auto b = this->editor.getBufferVisitingFile(this->file);
          if (b != nullptr) {
            this->editor.removeUserInput(this);
            auto* l = new TextEditBase::KillOtherBufferAndWriteFile(
                this->editor, this->textEdit, *b, this->file);
            this->editor.setUserInput(l);
          } else {
            this->textEdit.writeFile(this->file);
          }
        }
      }  // end of treatUserInput

      TextEditBase& textEdit;
      QString file;
    };  // end of TextEditBase::OverWriteFile

    struct TextEditBase::WriteFile : public FilePathUserInput {
      WriteFile(EditorWidget& p, TextEditBase& t, const QString& path)
          : FilePathUserInput(QObject::tr("write file :"), p),
            textEdit(t) {
        QString npath(path);
        if (!npath.isEmpty()) {
          if (!npath.endsWith(QDir::separator())) {
            npath += QDir::separator();
          }
        }
        this->input->setText(npath);
      }

      bool isBlocking() const override {
        return true;
      }  // end of isBlocking

      ~WriteFile() override = default;

     protected:
      void treatUserInput() override {
        auto f = this->input->text();
        const auto of = this->textEdit.getCompleteFileName();
        if (f.isEmpty()) {
          this->editor.displayInformativeMessage(
              "empty file name specified");
          return;
        }
        if (f == of) {
          this->editor.displayInformativeMessage(
              "current file specified");
          return;
        }
        QFileInfo fn(f);
        if (fn.isDir()) {
          if (of.isEmpty()) {
            this->editor.displayInformativeMessage(
                "directory specified");
            return;
          }
          QFileInfo ofn(of);
          f += QDir::separator() + ofn.fileName();
          fn.setFile(f);
        }
        if (fn.exists()) {
          if (!fn.isWritable()) {
            QString msg;
            msg = QObject::tr(
                "file '%1' already exists and is not writable");
            this->editor.displayInformativeMessage(msg.arg(f));
            return;
          }
          this->editor.removeUserInput(this);
          auto* l = new TextEditBase::OverWriteFile(this->editor,
                                                    this->textEdit, f);
          this->editor.setUserInput(l);
        } else {
          // check if another buffer does not visit this file
          auto* b = this->editor.getBufferVisitingFile(f);
          if (b != nullptr) {
            this->editor.removeUserInput(this);
            auto* l = new TextEditBase::KillOtherBufferAndWriteFile(
                this->editor, this->textEdit, *b, f);
            this->editor.setUserInput(l);
          } else {
            this->textEdit.writeFile(f);
          }
        }
      }

      TextEditBase& textEdit;

    };  // end of struct TextEdit::WriteFile

    TextEditBase::SaveInput::SaveInput(EditorWidget& w, TextEditBase& p)
        : YesOrNoUserInput(QObject::tr("save current buffer"), w),
          textEdit(p) {}

    bool TextEditBase::SaveInput::isBlocking() const { return true; }

    void TextEditBase::SaveInput::treatUserInput() {
      if (this->input->text() == "y") {
        this->textEdit.save();
      }
    }  // end of treatUserInput

    TextEditBase::GotoLine::GotoLine(TextEditBase& t, EditorWidget& p)
        : CommandLine(QObject::tr("go to line :"), p), textEdit(t) {
      this->input->setValidator(new QIntValidator(this->input));
    }

    TextEditBase::GotoLine::~GotoLine() = default;

    void TextEditBase::GotoLine::treatUserInput() {
      bool b;
      int line = this->input->text().toInt(&b);
      if (b) {
        this->textEdit.gotoLine(line);
      }
    }

    void TextEditBase::setSpellCheckLanguage(const QString& l) {
      this->spellCheckLanguage = l;
      if (this->mode != nullptr) {
        this->mode->setSpellCheckLanguage(l);
      }
    }  // end of TextEditBase::setSpellCheckLanguage

    MajorMode* TextEditBase::setMajorMode() {
      auto& fm = MajorModeFactory::getMajorModeFactory();
      auto* m = fm.getMajorModeForFile(
          QFileInfo(this->getFileName()).fileName(), this->editor,
          this->buffer, *this);
      this->setMajorMode(m);
      return m;
    }  // end of TextEditBase::setMajorMode

    MajorMode* TextEditBase::setMajorMode(const QString& n) {
      auto& fm = MajorModeFactory::getMajorModeFactory();
      auto* m =
          fm.getMajorModeByName(n, this->editor, this->buffer, *this);
      this->setMajorMode(m);
      return m;
    }  // end of TextEditBase::setMajorMode

    void TextEditBase::setMajorMode(MajorMode* const m) {
      if (m == nullptr) {
        return;
      }
      if (this->mode != nullptr) {
        QObject::disconnect(this->mode, nullptr, this, nullptr);
        this->mode->deleteLater();
        this->mode = nullptr;
      }
      if (this->allowMajorModeChange) {
        this->editor.displayInformativeMessage(
            QObject::tr("setting major mode '%1'").arg(m->getName()));
        this->mode = m;
        this->mode->setSpellCheckLanguage(this->spellCheckLanguage);
        this->mode->setSyntaxHighlighter(this->document());
        emit majorModeChanged();
      } else {
        this->editor.displayInformativeMessage(
            QObject::tr("setting major mode '%1' is not allowed")
                .arg(m->getName()));
      }
    }  // end of TextEditBase::setMajorMode

    void TextEditBase::deleteContextMenuActions() {
      using pointer = QAction*;
      auto exe = [](pointer& p) {
        if (p != nullptr) {
          delete p;
          p = nullptr;
        }
      };
      exe(this->ua);
      exe(this->ra);
      exe(this->ca);
      exe(this->coa);
      exe(this->pa);
      exe(this->sa);
    }  // end of TextEditBase::deleteContextMenuActions

    void TextEditBase::createContextMenuActions() {
      this->deleteContextMenuActions();
      // undo action
      this->ua = new QAction(QObject::tr("&Undo"), this);
      this->ua->setIcon(QIcon::fromTheme("edit-undo"));
      this->ua->setIconVisibleInMenu(true);
      QObject::connect(this->ua, &QAction::triggered, this,
                       &TextEditBase::undo);
      // redo action
      this->ra = new QAction(QObject::tr("&Redo"), this);
      this->ra->setIcon(QIcon::fromTheme("edit-redo"));
      this->ra->setIconVisibleInMenu(true);
      QObject::connect(this->ra, &QAction::triggered, this,
                       &TextEditBase::redo);
      // cut action
      this->ca = new QAction(QObject::tr("Cu&t"), this);
      this->ca->setIcon(QIcon::fromTheme("edit-cut"));
      this->ca->setIconVisibleInMenu(true);
      QObject::connect(this->ca, &QAction::triggered, this,
                       &TextEditBase::cut);
      // copy action
      this->coa = new QAction(QObject::tr("&Copy"), this);
      this->coa->setIcon(QIcon::fromTheme("edit-copy"));
      this->coa->setIconVisibleInMenu(true);
      QObject::connect(this->coa, &QAction::triggered, this,
                       &TextEditBase::copy);
      // paste action
      this->pa = new QAction(QObject::tr("&Paste"), this);
      this->pa->setIcon(QIcon::fromTheme("edit-paste"));
      this->pa->setIconVisibleInMenu(true);
      QObject::connect(this->pa, &QAction::triggered, this,
                       &TextEditBase::paste);
      // select all action
      this->sa = new QAction(QObject::tr("Select All"), this);
      this->sa->setIcon(QIcon::fromTheme("edit-select-all"));
      this->sa->setIconVisibleInMenu(true);
      QObject::connect(this->sa, &QAction::triggered, this,
                       &TextEditBase::selectAll);
    }  // end of TextEditBase::createContextMenuActions

    void TextEditBase::gotoLine(const int l) {
      auto c = this->textCursor();
      if (l >= 0) {
        c.movePosition(QTextCursor::Start, this->moveMode);
        if (l > 0) {
          c.movePosition(QTextCursor::NextBlock, this->moveMode, l - 1);
        }
      } else {
        c.movePosition(QTextCursor::End, this->moveMode);
        c.movePosition(QTextCursor::PreviousBlock, this->moveMode, -l);
      }
      this->setTextCursor(c);
      this->centerCursor();
    }  // end TextEditBase::gotoLine

    void TextEditBase::gotoPosition(const int l, const int c) {
      this->gotoLine(l);
      auto tc = this->textCursor();
      auto tc2 = tc;
      tc2.select(QTextCursor::LineUnderCursor);
      const auto mpos = tc2.selectedText().size();
      if (c >= 1) {
        tc.movePosition(QTextCursor::NextCharacter, this->moveMode,
                        std::min(c - 1, mpos));
      } else {
        tc.movePosition(QTextCursor::EndOfLine, this->moveMode);
        tc.movePosition(QTextCursor::NextCharacter, this->moveMode,
                        -std::min(-c + 1, mpos));
      }
      this->setTextCursor(tc);
      this->centerCursor();
    }  // end of TextEditBase::gotoPosition

    QString TextEditBase::getCurrentWord() const {
      auto tc = this->textCursor();
      tc.select(QTextCursor::WordUnderCursor);
      return tc.selectedText();
    }  // end of TextEditBase::getCurrentWord

    bool TextEditBase::hasMajorMode() const {
      return this->mode != nullptr;
    }  // end of TextEditBase::hasMajorMode

    const MajorMode& TextEditBase::getMajorMode() const {
      if (this->mode == nullptr) {
        throw(
            std::runtime_error("TextEditBase::getMode: "
                               "invalid call"));
      }
      return *(this->mode);
    }  // end of TextEditBase::getMode

    bool TextEditBase::isModified() const {
      return this->document()->isModified();
    }  // end of TextEditBase::isModified

    void TextEditBase::addToKillRing(const QString& t) {
      this->editor.addToKillRing(t);
    }  // end of TextEditBase::addToKillRing

    void TextEditBase::highlightCurrentLine() {
      QList<QTextEdit::ExtraSelection> e;
      QTextEdit::ExtraSelection selection;
      auto lineColor = QColor(Qt::yellow).lighter(180);
      selection.format.setBackground(lineColor);
      selection.format.setProperty(QTextFormat::FullWidthSelection,
                                   true);
      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      e.append(selection);
      this->setExtraSelections(e);
    }

    std::vector<QMenu*> TextEditBase::getSpecificMenus() {
      std::vector<QMenu*> m;
      if (this->mode != nullptr) {
        auto* mm = this->mode->getSpecificMenu();
        if (mm != nullptr) {
          //           mm->setSizePolicy(QSizePolicy::MinimumExpanding,
          //                             QSizePolicy::Preferred);
          //           mm->setIcon(this->mode->getIcon());
          m.push_back(mm);
        }
      }
      return m;
    }  // end of TextEditBase::getSpecificMenu

    QIcon TextEditBase::getIcon() const {
      if (this->mode == nullptr) {
        return QIcon();
      }
      return this->mode->getIcon();
    }  // end of TextEditBase::getIcon

    void TextEditBase::insertCompletion(const QString& completion) {
      if (this->mode == nullptr) {
        return;
      }
      this->mode->completeCurrentWord(*this, completion);
    }  // end of TextEditBase::insertCompletion

    void TextEditBase::removeKeyPressEventFilter() {
      if (this->filter != nullptr) {
        QObject::disconnect(
            this->filter, &TextEditKeyPressEventFilter::destroyed, this,
            &TextEditBase::keyPressEventFilterDestroyed);
        this->filter->deleteLater();
        this->filter = nullptr;
      }
    }  // end of TextEditBase::removeKeyPressEventFilter

    void TextEditBase::keyPressEventFilterDestroyed() {
      this->filter = nullptr;
    }  // end of TextEditBase::keyPressEventFilterDestroyed

    bool TextEditBase::setKeyPressEventFilter(
        TextEditKeyPressEventFilter* const f) {
      if (f == nullptr) {
        return false;
      }
      if (this->filter != nullptr) {
        this->editor.displayInformativeMessage(
            QObject::tr("a key press event filter already exists"));
        return false;
      }
      if (!f->isOk()) {
        return false;
      }
      this->filter = f;
      QObject::connect(this->filter,
                       &TextEditKeyPressEventFilter::destroyed, this,
                       &TextEditBase::keyPressEventFilterDestroyed);
      return true;
    }  // end of TextEditBase::setKeyPressEventFilter

    void TextEditBase::setFileName(const QString& f) {
      QFileInfo fi(f);
      this->fileName = fi.absoluteFilePath();
      if (this->mode == nullptr) {
        this->setMajorMode();
      }
      emit fileNameChanged(this->fileName);
    }  // end of TextEditBase::setFileName

    QString TextEditBase::getFileName() const {
      return this->fileName;
    }  // end of TextEditBase::getFileName

    QString TextEditBase::getCompleteFileName() const {
      return this->fileName;
    }  // end of TextEditBase::getCompleteFileName

    QString TextEditBase::getDirectory() const {
      QFileInfo fn(this->fileName);
      if (fn.isDir()) {
        return fn.absolutePath();
      }
      return fn.dir().absolutePath();
    }  // end of TextEditBase::getDirectory

    TextEditBase::SaveInput* TextEditBase::getSaveInput() {
      return new SaveInput(this->editor, *this);
    }  // end of TextEditBase::getSaveInput

    void TextEditBase::save() {
      if (!this->document()->isModified()) {
        this->editor.displayInformativeMessage(
            QObject::tr("(no changes need to be saved)"));
        return;
      }
      if (this->fileName.isEmpty()) {
        this->writeFile();
        return;
      }
      const auto& af = QFileInfo(this->fileName).absoluteFilePath();
      this->editor.displayInformativeMessage(
          QObject::tr("saving file '%1'").arg(af));
      QTextDocumentWriter writer(this->fileName);
      writer.setFormat("plaintext");
      if (writer.write(this->document())) {
        this->document()->setModified(false);
        this->editor.displayInformativeMessage(
            QObject::tr("wrote file '%1'").arg(af));
      } else {
        this->editor.displayInformativeMessage(
            QObject::tr("saving file '%1' failed").arg(af));
      }
    }  // end of TextEditBase::save

    void TextEditBase::writeFile() {
      QFileInfo fn(this->getCompleteFileName());
      QDir d(fn.dir());
      QString path;
      if (d.exists()) {
        path = d.absolutePath();
      } else {
        path = QDir::current().absolutePath();
      }
      auto* l = new TextEditBase::WriteFile(this->editor, *this, path);
      l->setInputHistorySettingAddress("recent files");
      this->editor.setUserInput(l);
    } // end of TextEditBase::writeFile

    void TextEditBase::writeFile(const QString& f) {
      // basic check
      QFileInfo fi(f);
      if (fi.exists()) {
        if (!fi.isWritable()) {
          QString msg(QObject::tr(
              "file '%1' already exists and is not readable"));
          this->editor.displayInformativeMessage(msg.arg(f));
        }
      }
      this->setFileName(f);
      this->document()->setModified();
      this->save();
    }  // end of TextEditBase::writeFile

    void TextEditBase::emitCursorPositionChanged() {
      emit cursorPositionChanged();
    }  // end of TextEditBase::emitCursorPositionChanged

    bool TextEditBase::event(QEvent* ev) {
      if (ev->type() == QEvent::KeyPress) {
        auto* kev = static_cast<QKeyEvent*>(ev);
        if (kev->key() == Qt::Key_Tab) {
          this->handleKeyPressEvent(kev);
          ev->accept();
          return true;
        }
      }
      return QWidget::event(ev);
    }

    void TextEditBase::keyPressEvent(QKeyEvent* e) {
      // This method is called by completers (we can not give the
      // underlying widget to the setWiget, because the completer
      // redirect keyEvent this it, regardless of the event handler we
      // set), so we must define this method. The trouble is that if the
      // underlying does not handle the keyEvent, it would send it to
      // the parent (this object), which may cause an infinite loop...
      // This is why we introduced the keyProcessing flag.
      if (this->keyProcessing) {
        QWidget::keyPressEvent(e);
        return;
      } else {
        this->keyProcessing = true;
        this->handleKeyPressEvent(e);
        this->keyProcessing = false;
      }
    }  // end of TextEditBase::keyPressEvent

    bool TextEditBase::handleKeyPressEvent(QKeyEvent* e) {
      auto& s = ShortCutStyle::getShortCutStyle();
      if (s.getStyle() == ShortCutStyle::QT) {
        return this->handleKeyPressEventWithQtShortCuts(e);
      }
      return this->handleKeyPressEventWithEmacsShortCuts(e);
    }

    bool TextEditBase::handleKeyPressEventWithQtShortCuts(
        QKeyEvent* e) {
      const auto k = e->key();
      const auto m = e->modifiers();
      // treating standard keys
      QCompleter* completer = nullptr;
      if (this->mode != nullptr) {
        completer = this->mode->getCompleter();
      }
      if ((completer != nullptr) && (completer->popup()->isVisible())) {
        // The following keys are forwarded by the completer to the
        // widget
        if (m == Qt::NoModifier) {
          switch (k) {
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
              this->insertCompletion(
                  completer->popup()->currentIndex().data().toString());
              return true;
            default:
              break;
          }
        }
      }
      auto* w = this->widget();
      w->removeEventFilter(this);
      QCoreApplication::sendEvent(w, e);
      w->installEventFilter(this);
      if ((completer == nullptr) || (this->mode == nullptr)) {
        return true;
      }
      auto tc = this->textCursor();
      int pc = tc.position();
      tc.movePosition(QTextCursor::Left);
      tc.movePosition(QTextCursor::EndOfWord);
      if (tc.position() == pc) {
        static QString eow(
            "~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");  // end of word
        // const bool ctrlOrShift = m & (Qt::ControlModifier |
        // Qt::ShiftModifier);
        const bool hasModifier = m != Qt::NoModifier;
        QString completionPrefix = this->mode->getCompletionPrefix();
        if (hasModifier || (completionPrefix.length() <
                            this->mode->getMinimalCompletionLength()) ||
            e->text().isEmpty() || eow.contains(e->text().right(1))) {
          completer->popup()->hide();
          return true;
        }
        if (completionPrefix != completer->completionPrefix()) {
          completer->setCompletionPrefix(completionPrefix);
          completer->popup()->setCurrentIndex(
              completer->completionModel()->index(0, 0));
        }
        if (completer->completionCount() == 1) {
          // do something
          if (completer->currentCompletion() != completionPrefix) {
            QRect cr = cursorRect();
            cr.setWidth(completer->popup()->sizeHintForColumn(0) +
                        completer->popup()
                            ->verticalScrollBar()
                            ->sizeHint()
                            .width());
            completer->complete(cr);  // popup it up!
          } else {
            completer->popup()->hide();
          }
        } else if (completer->completionCount() > 1) {
          QRect cr = cursorRect();
          cr.setWidth(completer->popup()->sizeHintForColumn(0) +
                      completer->popup()
                          ->verticalScrollBar()
                          ->sizeHint()
                          .width());
          completer->complete(cr);  // popup it up!
        }
      } else {
        completer->popup()->hide();
      }
      return true;
    }  // end of TextEditBase::handleKeyPressEventWithQtShortCuts

    bool TextEditBase::handleKeyPressEventWithEmacsShortCuts(
        QKeyEvent* e) {
      const auto k = e->key();
      const auto m = e->modifiers();
      auto c = this->textCursor();
      if (this->filter != nullptr) {
        if (this->filter->filterKeyPressEvent(e)) {
          return true;
        }
      }
      if (this->mode != nullptr) {
        if (this->mode->keyPressEvent(e)) {
          return true;
        }
      }
      if ((k == Qt::Key_Shift) || (k == Qt::Key_Control) ||
          (k == Qt::Key_Alt)) {
        return true;
      }
      if ((this->yank) &&
          ((m == Qt::ControlModifier) && (k != Qt::Key_Y)) &&
          ((m == Qt::AltModifier) && (k == Qt::Key_Y))) {
        this->yank = false;
      }
      if (((m == Qt::ControlModifier) && (k == Qt::Key_X)) &&
          (!this->ctrlx) && (!this->ctrlc)) {
        this->ctrlx = true;
        return true;
      }
      if (((m == Qt::ControlModifier) && (k == Qt::Key_C)) &&
          (!this->ctrlx) && (!this->ctrlc)) {
        this->ctrlc = true;
        return true;
      }
      if (this->ctrlx) {
        this->ctrlx = false;
        if (m != Qt::ControlModifier) {
          if (k == Qt::Key_1) {
            if (this->isMainFrame()) {
              this->buffer.hideSecondaryTasks();
            }
            return true;
          } else if (k == Qt::Key_2) {
            if (this->isMainFrame()) {
              this->editor.setSecondaryTasksOrientation(Qt::Vertical);
              this->buffer.showSecondaryTasks();
            }
            return true;
          } else if (k == Qt::Key_3) {
            if (this->isMainFrame()) {
              this->editor.setSecondaryTasksOrientation(Qt::Horizontal);
              this->buffer.showSecondaryTasks();
            }
            return true;
          } else if (k == Qt::Key_O) {
            if (this->isMainFrame()) {
              if (this->buffer.hasSecondaryTasks()) {
                if (this->buffer.areSecondaryTasksVisible()) {
                  this->buffer.focusCurrentSecondaryTask();
                } else {
                  if (this->editor.hasUserInput()) {
                    this->editor.focusUserInput();
                  }
                }
              } else {
                if (this->editor.hasUserInput()) {
                  this->editor.focusUserInput();
                }
              }
            } else {
              if (this->editor.hasUserInput()) {
                this->editor.focusUserInput();
              } else {
                auto& b = this->editor.getCurrentBuffer();
                b.focusMainFrame();
              }
            }
            return true;
          } else if (k == Qt::Key_P) {
            this->print();
            return true;
          } else if (k == Qt::Key_B) {
            this->editor.changeBuffer();
            return true;
          } else if (k == Qt::Key_R) {
            this->editor.setUserInput(
                new RectangleMode(*this, this->editor));
            this->moveMode = QTextCursor::MoveAnchor;
            return true;
          } else if (k == Qt::Key_K) {
            if (this->isMainFrame()) {
              this->editor.closeCurrentBuffer();
            } else {
              auto& b = this->editor.getCurrentBuffer();
              b.closeCurrentSecondaryTask();
            }
            return true;
          } else if (k == Qt::Key_H) {
            this->selectAll();
            return true;
          }
        } else if (m == Qt::ControlModifier) {
          if (k == Qt::Key_S) {
            this->save();
            return true;
          } else if (k == Qt::Key_F) {
            this->editor.openFile();
            return true;
          } else if (k == Qt::Key_C) {
            this->editor.close();
            return true;
          } else if (k == Qt::Key_W) {
            this->writeFile();
            return true;
          }
        }
        if (this->mode != nullptr) {
          if (this->mode->handleShortCut(Qt::Key_X, m, k)) {
            return true;
          }
        }
        const auto modifier = TextEditBase::getModifier(*e);
        if (modifier.isEmpty()) {
          this->editor.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-X - %1")
                  .arg(e->text()));
        } else {
          this->editor.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-X - %1-%2")
                  .arg(modifier, e->text()));
        }
        return true;
      }
      if (this->ctrlc) {
        this->ctrlc = false;
        if (this->mode != nullptr) {
          if (this->mode->handleShortCut(Qt::Key_C, m, k)) {
            return true;
          }
        }
        const auto modifier = TextEditBase::getModifier(*e);
        if (modifier.isEmpty()) {
          this->editor.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-C - %1")
                  .arg(e->text()));
        } else {
          this->editor.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-C - %1-%2")
                  .arg(modifier, e->text()));
        }
        return true;
      }
      if ((m == Qt::ControlModifier) && (k == Qt::Key_Left)) {
        this->editor.previousBuffer();
        return true;
      }
      if ((m == Qt::ControlModifier) && (k == Qt::Key_Right)) {
        this->editor.nextBuffer();
        return true;
      }
      if (!(((m == Qt::ControlModifier) && (k == Qt::Key_P)) ||
            ((m == Qt::ControlModifier) && (k == Qt::Key_N)))) {
        this->positions.clear();
      }
      // treating standard keys
      QCompleter* completer = nullptr;
      if (this->mode != nullptr) {
        completer = this->mode->getCompleter();
      }
      if ((completer != nullptr) && (completer->popup()->isVisible())) {
        // The following keys are forwarded by the completer to the
        // widget
        if (m == Qt::NoModifier) {
          switch (k) {
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
              this->insertCompletion(
                  completer->popup()->currentIndex().data().toString());
              return true;
            default:
              break;
          }
        }
      }
      if ((m == Qt::ControlModifier) && (k == Qt::Key_G)) {
        if (completer && completer->popup()->isVisible()) {
          completer->popup()->hide();
        }
        c.clearSelection();
        this->setTextCursor(c);
        if (this->moveMode == QTextCursor::KeepAnchor) {
          this->moveMode = QTextCursor::MoveAnchor;
        } else {
          this->editor.removeUserInput();
          this->editor.displayInformativeMessage(QObject::tr("quit"));
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_S)) {
        if (this->filter != nullptr) {
          this->editor.displayInformativeMessage(
              QObject::tr("a filter is already set"));
          return true;
        }
        // set a search user input
        // the TextEditSearch automatically set itself as a filter
        // for this
        this->editor.setUserInput(
            new TextEditSearch(*this, this->editor));
        // the current buffer shall have the focus : but default, the
        // EditorWidget gives focus to the user input
        this->setFocus();
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_R)) {
        if (this->filter != nullptr) {
          this->editor.displayInformativeMessage(
              QObject::tr("a filter is already set"));
          return true;
        }
        // set a search user input
        // the TextEditSearch automatically set itself as a filter
        // for this
        this->editor.setUserInput(new TextEditSearch(
            *this, this->editor, QTextDocument::FindCaseSensitively |
                                     QTextDocument::FindBackward));
        // the current buffer shall have the focus : but default, the
        // EditorWidget gives focus to the user input
        this->setFocus();
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Space)) {
        c.clearSelection();
        this->setTextCursor(c);
        this->editor.displayInformativeMessage(QObject::tr("mark set"));
        this->moveMode = QTextCursor::KeepAnchor;
      } else if (((m == Qt::ControlModifier) &&
                  (k == Qt::Key_Underscore)) ||
                 ((m == Qt::ControlModifier) && (k == Qt::Key_U))) {
        this->undo();
        this->moveMode = QTextCursor::MoveAnchor;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_M)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          this->insertPlainText("\n");
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_W)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          this->addToKillRing(c.selectedText());
          c.removeSelectedText();
          c.clearSelection();
          this->moveMode = QTextCursor::MoveAnchor;
          this->setTextCursor(c);
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_H)) {
        this->selectAll();
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Z)) {
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Q)) {
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_V)) {
        QKeyEvent nev(QEvent::None, Qt::Key_PageDown, Qt::NoModifier);
        auto* w = this->widget();
        w->removeEventFilter(this);
        QCoreApplication::sendEvent(w, &nev);
        w->installEventFilter(this);
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_K)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          if (c.atBlockEnd()) {
            if (!c.atEnd()) {
              c.movePosition(QTextCursor::NextCharacter,
                             QTextCursor::MoveAnchor);
              c.deletePreviousChar();
              this->setTextCursor(c);
            }
          } else {
            c.movePosition(QTextCursor::EndOfBlock,
                           QTextCursor::KeepAnchor);
            this->addToKillRing(c.selectedText());
            c.removeSelectedText();
          }
          this->moveMode = QTextCursor::MoveAnchor;
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Y)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          const QStringList& ring = this->editor.getKillRing();
          if (!ring.isEmpty()) {
            this->yank = true;
            this->pring = ring.size() - 1;
            this->insertPlainText(ring[this->pring]);
          }
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_E)) {
        this->moveCursor(QTextCursor::EndOfBlock, this->moveMode);
      } else if ((k == Qt::Key_Home) ||
                 ((m == Qt::ControlModifier) && (k == Qt::Key_A))) {
        this->moveCursor(QTextCursor::StartOfBlock, this->moveMode);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_B)) {
        this->moveCursor(QTextCursor::PreviousCharacter,
                         this->moveMode);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_F)) {
        this->moveCursor(QTextCursor::NextCharacter, this->moveMode);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_P)) {
        int p = c.position() - c.block().position();
        this->positions[c.block().blockNumber()] = p;
        c.movePosition(QTextCursor::PreviousBlock, this->moveMode);
        if (this->positions.count(c.block().blockNumber()) == 0) {
          p = this->positions[c.block().blockNumber()];
        }
        int npos = qMax(qMin(p, c.block().length() - 1), 0);
        c.movePosition(QTextCursor::NextCharacter, this->moveMode,
                       npos);
        this->setTextCursor(c);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_N)) {
        int p = c.position() - c.block().position();
        this->positions[c.block().blockNumber()] = p;
        c.movePosition(QTextCursor::NextBlock, this->moveMode);
        if (this->positions.count(c.block().blockNumber()) == 0) {
          p = this->positions[c.block().blockNumber()];
        }
        int npos = qMax(qMin(p, c.block().length() - 1), 0);
        c.movePosition(QTextCursor::NextCharacter, this->moveMode,
                       npos);
        this->setTextCursor(c);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_D)) {
        c.movePosition(QTextCursor::NextCharacter,
                       QTextCursor::MoveAnchor);
        c.deletePreviousChar();
        this->setTextCursor(c);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_T)) {
        if (c.hasSelection()) {
          c.clearSelection();
        }
        if (c.position() <= 1) {
          this->editor.displayInformativeMessage(
              QObject::tr("beginning of buffer"));
          return true;
        }
        c.beginEditBlock();
        c.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
        c.movePosition(QTextCursor::PreviousCharacter,
                       QTextCursor::KeepAnchor, 2);
        QString s = c.selectedText();
        c.removeSelectedText();
        QChar tmp = s[0];
        s[0] = s[1];
        s[1] = tmp;
        c.insertText(s);
        c.endEditBlock();
        this->setTextCursor(c);
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Down)) {
        QKeyEvent nev(QEvent::None, Qt::Key_PageDown, Qt::NoModifier);
        auto* const w = this->widget();
        w->removeEventFilter(this);
        QCoreApplication::sendEvent(w, &nev);
        w->installEventFilter(this);
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Up)) {
        QKeyEvent nev(QEvent::None, Qt::Key_PageUp, Qt::NoModifier);
        auto* const w = this->widget();
        w->removeEventFilter(this);
        QCoreApplication::sendEvent(w, &nev);
        w->installEventFilter(this);
        return true;
      } else if (((m == (Qt::AltModifier | Qt::ControlModifier)) &&
                  (k == Qt::Key_Percent)) ||
                 ((m == (Qt::AltModifier | Qt::ControlModifier |
                         Qt::ShiftModifier)) &&
                  (k == Qt::Key_Percent))) {
        this->editor.setUserInput(
            new TextEditQueryReplace(*this, this->editor));

      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Q)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          if (this->mode != nullptr) {
            this->mode->format();
          }
          c = this->textCursor();
          c.clearSelection();
          this->moveMode = QTextCursor::MoveAnchor;
          this->setTextCursor(c);
        }
        return true;
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Semicolon)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          if (this->mode != nullptr) {
            this->mode->comment();
          }
          c = this->textCursor();
          c.clearSelection();
          this->moveMode = QTextCursor::MoveAnchor;
          this->setTextCursor(c);
        }
        return true;
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_U)) {
        // upper case next word
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        }
        QTextCursor tc(c);
        tc.beginEditBlock();
        tc.clearSelection();
        tc.movePosition(QTextCursor::EndOfWord,
                        QTextCursor::KeepAnchor);
        if (tc == c) {
          tc.movePosition(QTextCursor::NextWord,
                          QTextCursor::MoveAnchor);
          tc.movePosition(QTextCursor::EndOfWord,
                          QTextCursor::KeepAnchor);
        }
        QString s = tc.selectedText();
        tc.removeSelectedText();
        tc.insertText(s.toUpper());
        tc.endEditBlock();
        this->setTextCursor(tc);
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_L)) {
        // lower case
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        }
        QTextCursor tc(c);
        tc.beginEditBlock();
        tc.clearSelection();
        tc.movePosition(QTextCursor::EndOfWord,
                        QTextCursor::KeepAnchor);
        if (tc == c) {
          tc.movePosition(QTextCursor::NextWord,
                          QTextCursor::MoveAnchor);
          tc.movePosition(QTextCursor::EndOfWord,
                          QTextCursor::KeepAnchor);
        }
        QString s = tc.selectedText();
        tc.removeSelectedText();
        tc.insertText(s.toLower());
        tc.endEditBlock();
        this->setTextCursor(tc);
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_V)) {
        QKeyEvent nev(QEvent::None, Qt::Key_PageUp, Qt::NoModifier);
        auto* w = this->widget();
        w->removeEventFilter(this);
        QCoreApplication::sendEvent(w, &nev);
        w->installEventFilter(this);
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_X)) {
        this->editor.launchCommand();
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Exclam)) {
        auto* l = new ShellProcessLineEdit("shell command:", "", "",
                                           this->editor);
        l->setInputHistorySettingAddress("command/shell/history");
        this->editor.setUserInput(l);
      } else if (((m == Qt::AltModifier) && (k == Qt::Key_Percent)) ||
                 ((m == (Qt::AltModifier | Qt::ShiftModifier)) &&
                  (k == Qt::Key_Percent))) {
        this->editor.setUserInput(
            new TextEditQueryReplace(*this, this->editor));
      } else if (((m == Qt::AltModifier) && (k == Qt::Key_Percent)) ||
                 ((m == (Qt::AltModifier | Qt::ShiftModifier)) &&
                  (k == Qt::Key_Percent))) {
        this->editor.setUserInput(
            new TextEditQueryReplace(*this, this->editor));
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_G)) {
        this->editor.setUserInput(new GotoLine(*this, this->editor));
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Y)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          const auto& ring = this->editor.getKillRing();
          if ((ring.isEmpty()) || (!this->yank)) {
            if (ring.isEmpty()) {
              this->editor.displayInformativeMessage(
                  QObject::tr("kill ring is empty"));
            }
            if (!this->yank) {
              this->editor.displayInformativeMessage(
                  QObject::tr("previous command was not a yank"));
            }
            return true;
          }
          int s = ring[this->pring].size();
          --(this->pring);
          if (this->pring == -1) {
            this->pring = ring.size();
            --(this->pring);
          }
          c.movePosition(QTextCursor::PreviousCharacter,
                         QTextCursor::KeepAnchor, s);
          c.removeSelectedText();
          c.insertText(ring[this->pring]);
          this->setTextCursor(c);
          this->moveMode = QTextCursor::MoveAnchor;
        }
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_W)) {
        this->addToKillRing(c.selectedText());
        this->copy();
        c.clearSelection();
        this->setTextCursor(c);
        this->moveMode = QTextCursor::MoveAnchor;
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_B)) {
        this->moveCursor(QTextCursor::PreviousWord, this->moveMode);
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_D)) {
        if (this->isReadOnly()) {
          this->editor.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          c.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
          c.movePosition(QTextCursor::NextWord,
                         QTextCursor::KeepAnchor);
          this->addToKillRing(c.selectedText());
          c.removeSelectedText();
          this->moveMode = QTextCursor::MoveAnchor;
        }
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_F)) {
        this->moveCursor(QTextCursor::NextWord, this->moveMode);
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Less)) {
        this->moveCursor(QTextCursor::Start, this->moveMode);
      } else if ((m & Qt::AltModifier) && (k == Qt::Key_Greater)) {
        this->moveCursor(QTextCursor::End, this->moveMode);
      } else {
        /* Cursor Movements */
        if (((m == Qt::MetaModifier) && (k == Qt::Key_Meta)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Super_L)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Super_R)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Hyper_L)) ||
            ((m == Qt::NoModifier) && (k == Qt::Key_Hyper_R))) {
          return true;
        } else if ((m == Qt::NoModifier) && (k == Qt::Key_Left)) {
          this->moveCursor(QTextCursor::PreviousCharacter,
                           this->moveMode);
        } else if ((m == Qt::NoModifier) && (k == Qt::Key_Right)) {
          this->moveCursor(QTextCursor::NextCharacter, this->moveMode);
        } else if ((m == Qt::NoModifier) && (k == Qt::Key_Up)) {
          int p = c.position() - c.block().position();
          this->positions[c.block().blockNumber()] = p;
          c.movePosition(QTextCursor::PreviousBlock, this->moveMode);
          int npos = qMax(qMin(p, c.block().length() - 1), 0);
          c.movePosition(QTextCursor::NextCharacter, this->moveMode,
                         npos);
          this->setTextCursor(c);
        } else if ((m == Qt::NoModifier) && (k == Qt::Key_Down)) {
          int p = c.position() - c.block().position();
          this->positions[c.block().blockNumber()] = p;
          c.movePosition(QTextCursor::NextBlock, this->moveMode);
          int npos = qMax(qMin(p, c.block().length() - 1), 0);
          c.movePosition(QTextCursor::NextCharacter, this->moveMode,
                         npos);
          this->setTextCursor(c);
        } else if ((m == Qt::NoModifier) && (k == Qt::Key_Tab)) {
          /* indentation */
          if (this->mode != nullptr) {
            if (c.hasSelection()) {
              this->mode->indentRegion(c);
            } else {
              this->mode->indentLine(c);
            }
            this->moveMode = QTextCursor::MoveAnchor;
          }
          return true;
        } else {
          if (this->isReadOnly()) {
            this->editor.displayInformativeMessage(
                QObject::tr("read only buffer"));
          } else {
            auto w = this->widget();
            w->removeEventFilter(this);
            QCoreApplication::sendEvent(w, e);
            w->installEventFilter(this);
            if (completer != nullptr) {
              auto tc = this->textCursor();
              int pc = tc.position();
              tc.movePosition(QTextCursor::Left);
              tc.movePosition(QTextCursor::EndOfWord);
              if (tc.position() == pc) {
                static QString eow(
                    "~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");  // end of
                                                           // word
                // const bool ctrlOrShift = m & (Qt::ControlModifier |
                // Qt::ShiftModifier);
                const bool hasModifier = m != Qt::NoModifier;
                QString completionPrefix =
                    this->mode->getCompletionPrefix();
                if (hasModifier ||
                    (completionPrefix.length() <
                     this->mode->getMinimalCompletionLength()) ||
                    e->text().isEmpty() ||
                    eow.contains(e->text().right(1))) {
                  completer->popup()->hide();
                  return true;
                }
                if (completionPrefix != completer->completionPrefix()) {
                  completer->setCompletionPrefix(completionPrefix);
                  completer->popup()->setCurrentIndex(
                      completer->completionModel()->index(0, 0));
                }
                if (completer->completionCount() == 1) {
                  // do something
                  if (completer->currentCompletion() !=
                      completionPrefix) {
                    QRect cr = cursorRect();
                    cr.setWidth(
                        completer->popup()->sizeHintForColumn(0) +
                        completer->popup()
                            ->verticalScrollBar()
                            ->sizeHint()
                            .width());
                    completer->complete(cr);  // popup it up!
                  } else {
                    completer->popup()->hide();
                  }
                } else if (completer->completionCount() > 1) {
                  QRect cr = cursorRect();
                  cr.setWidth(completer->popup()->sizeHintForColumn(0) +
                              completer->popup()
                                  ->verticalScrollBar()
                                  ->sizeHint()
                                  .width());
                  completer->complete(cr);  // popup it up!
                }
              } else {
                completer->popup()->hide();
              }
            }
          }
        }
      }
      return true;
    }  // end of TextEdit::handleKeyPressEvent

    bool TextEditBase::handleMousePressEvent(QMouseEvent* e) {
      if (this->filter != nullptr) {
        this->removeKeyPressEventFilter();
      }
      if (this->mode != nullptr) {
        if (this->mode->mousePressEvent(e)) {
          return true;
        }
      }
      if (e->button() == Qt::RightButton) {
#ifndef QT_NO_CONTEXTMENU
        QPointer<QMenu> m(new QMenu());
        this->createContextMenuActions();
        if (this->isUndoRedoEnabled()) {
          m->addAction(this->ua);
          m->addAction(this->ra);
        }
        if (!this->isReadOnly()) {
          m->addAction(this->ca);
        }
        m->addAction(this->coa);
        if (!this->isReadOnly()) {
          m->addAction(this->pa);
        }
        m->addAction(this->sa);
        if (this->mode != nullptr) {
          this->mode->completeContextMenu(
              m, this->cursorForPosition(e->pos()));
        }
        m->exec(e->globalPos());
        e->accept();
        return true;
#endif
      }
      return false;
    }

    void TextEditBase::setMainFrame(const bool b) {
      //    this->setMinimumHeight(400);
      this->mainFrame = b;
    }  // end of TextEditBase::setMainFrame

    bool TextEditBase::isMainFrame() const {
      return this->mainFrame;
    }  // end of TextEditBase::isMainFrame

    bool TextEditBase::isSecondaryTask() const {
      return !(this->isMainFrame());
    }  // end of TextEditBase::isSecondaryTask

    void TextEditBase::setMoveMode(QTextCursor::MoveMode m) {
      this->moveMode = m;
    }  // end of TextEditBase::setMoveMode

    TextEditBase::~TextEditBase() {
      if (this->filter != nullptr) {
        QObject::disconnect(
            this->filter, &TextEditKeyPressEventFilter::destroyed, this,
            &TextEditBase::keyPressEventFilterDestroyed);
        this->filter->deleteLater();
      }
      this->deleteContextMenuActions();
      if (this->mode != nullptr) {
        this->mode->deleteLater();
      }
    }  // end of TextEditBase::~TextEditBase

  }  // end of namespace gui
}  // end of namespace tfel
