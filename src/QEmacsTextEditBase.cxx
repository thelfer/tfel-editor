/*!
 * \file  QEmacsTextEditBase.cxx
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
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsRectangleMode.hxx"
#include "QEmacs/QEmacsTextEditSearch.hxx"
#include "QEmacs/QEmacsTextEditQueryReplace.hxx"
#include "QEmacs/QEmacsShortCutStyle.hxx"
#include "QEmacs/QEmacsShellProcessLineEdit.hxx"
#include "QEmacs/QEmacsHunspellDictionariesManager.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"

namespace qemacs {

  QEmacsTextEditBase::QEmacsTextEditBase(QEmacsWidget& g,
                                         QEmacsBuffer& b)
      : QWidget(&g), qemacs(g), buffer(b) {
    auto& dm = QEmacsHunspellDictionariesManager::
        getQEmacsHunspellDictionariesManager();
    this->spellCheckLanguage = dm.getDefaultSpellCheckLanguage();
  }  // end of QEmacsTextEditBase::QEmacsTextEditBase

  QEmacsWidget& QEmacsTextEditBase::getQEmacsWidget() {
    return this->qemacs;
  }  // end of QEmacsTextEditBase::getQEmacsWidget()

  const QEmacsWidget& QEmacsTextEditBase::getQEmacsWidget() const {
    return this->qemacs;
  }  // end of QEmacsTextEditBase::getQEmacsWidget()

  void QEmacsTextEditBase::mouseMoveEvent(QMouseEvent*) {
  }  // end of QEmacsTextEditBase::mouseMoveEvent

  void QEmacsTextEditBase::setFocus() {
    auto w = this->widget();
    if (w != nullptr) {
      w->setFocus();
    }
    }  // end of QEmacsTextEditBase::setFocus

    void QEmacsTextEditBase::setFont(const QFont& f) {
      auto* e = this->widget();
      e->setFont(f);
    }  // end of QEmacsTextEditBase::setFont

    void QEmacsTextEditBase::initialize(QAbstractScrollArea * const e) {
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
                         &QEmacsTextEditBase::highlightCurrentLine);
        QObject::connect(
            te, &QTextEdit::cursorPositionChanged, this,
            &QEmacsTextEditBase::emitCursorPositionChanged);
      } else if (auto* pte = qobject_cast<QPlainTextEdit*>(e)) {
        QObject::connect(pte, &QPlainTextEdit::cursorPositionChanged,
                         this,
                         &QEmacsTextEditBase::highlightCurrentLine);
        QObject::connect(
            pte, &QPlainTextEdit::cursorPositionChanged, this,
            &QEmacsTextEditBase::emitCursorPositionChanged);
      }
      // event filter
      e->installEventFilter(this);
      e->viewport()->installEventFilter(this);
    }  // end of QEmacsTextEditBase::initialize

    bool QEmacsTextEditBase::eventFilter(QObject * o, QEvent * e) {
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

    QString QEmacsTextEditBase::getModifier(const QKeyEvent& e) {
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
    }  // end of QEmacsTextEditBase::getModifier

    struct QEmacsTextEditBase::KillOtherBufferAndWriteFile
        : public QEmacsYesOrNoUserInput {
      KillOtherBufferAndWriteFile(QEmacsWidget& p,
                                  QEmacsTextEditBase& t,
                                  QEmacsBuffer& b,
                                  const QString& f)
          : QEmacsYesOrNoUserInput(
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
          this->qemacs.closeBuffer(this->kb, false);
          this->textEdit.writeFile(this->file);
        }
      }  // end of treatUserInput

      QEmacsTextEditBase& textEdit;
      QEmacsBuffer& kb;
      QString file;
    };

    struct QEmacsTextEditBase::OverWriteFile
        : public QEmacsYesOrNoUserInput {
      OverWriteFile(QEmacsWidget& p,
                    QEmacsTextEditBase& t,
                    const QString& f)
          : QEmacsYesOrNoUserInput(
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
          auto b = this->qemacs.getBufferVisitingFile(this->file);
          if (b != nullptr) {
            this->qemacs.removeUserInput(this);
            auto* l =
                new QEmacsTextEditBase::KillOtherBufferAndWriteFile(
                    this->qemacs, this->textEdit, *b, this->file);
            this->qemacs.setUserInput(l);
          } else {
            this->textEdit.writeFile(this->file);
          }
        }
      }  // end of treatUserInput

      QEmacsTextEditBase& textEdit;
      QString file;
    };  // end of QEmacsTextEditBase::OverWriteFile

    struct QEmacsTextEditBase::WriteFile
        : public QEmacsFilePathUserInput {
      WriteFile(QEmacsWidget& p,
                QEmacsTextEditBase& t,
                const QString& path)
          : QEmacsFilePathUserInput(QObject::tr("write file :"), p),
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
          this->qemacs.displayInformativeMessage(
              "empty file name specified");
          return;
        }
        if (f == of) {
          this->qemacs.displayInformativeMessage(
              "current file specified");
          return;
        }
        QFileInfo fn(f);
        if (fn.isDir()) {
          if (of.isEmpty()) {
            this->qemacs.displayInformativeMessage(
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
            this->qemacs.displayInformativeMessage(msg.arg(f));
            return;
          }
          this->qemacs.removeUserInput(this);
          auto* l = new QEmacsTextEditBase::OverWriteFile(
              this->qemacs, this->textEdit, f);
          this->qemacs.setUserInput(l);
        } else {
          // check if another buffer does not visit this file
          auto* b = this->qemacs.getBufferVisitingFile(f);
          if (b != nullptr) {
            this->qemacs.removeUserInput(this);
            auto* l =
                new QEmacsTextEditBase::KillOtherBufferAndWriteFile(
                    this->qemacs, this->textEdit, *b, f);
            this->qemacs.setUserInput(l);
          } else {
            this->textEdit.writeFile(f);
          }
        }
      }

      QEmacsTextEditBase& textEdit;

    };  // end of struct QEmacsTextEdit::WriteFile

    QEmacsTextEditBase::SaveInput::SaveInput(QEmacsWidget & w,
                                             QEmacsTextEditBase & p)
        : QEmacsYesOrNoUserInput(QObject::tr("save current buffer"), w),
          textEdit(p) {}

    bool QEmacsTextEditBase::SaveInput::isBlocking() const {
      return true;
    }

    void QEmacsTextEditBase::SaveInput::treatUserInput() {
      if (this->input->text() == "y") {
        this->textEdit.save();
      }
    }  // end of treatUserInput

    QEmacsTextEditBase::GotoLine::GotoLine(QEmacsTextEditBase & t,
                                           QEmacsWidget & p)
        : QEmacsCommandLine(QObject::tr("go to line :"), p),
          textEdit(t) {
      this->input->setValidator(new QIntValidator(this->input));
    }

    QEmacsTextEditBase::GotoLine::~GotoLine() = default;

    void QEmacsTextEditBase::GotoLine::treatUserInput() {
      bool b;
      int line = this->input->text().toInt(&b);
      if (b) {
        this->textEdit.gotoLine(line);
      }
    }

    void QEmacsTextEditBase::setSpellCheckLanguage(const QString& l) {
      this->spellCheckLanguage = l;
      if (this->mode != nullptr) {
        this->mode->setSpellCheckLanguage(l);
      }
    }  // end of QEmacsTextEditBase::setSpellCheckLanguage

    QEmacsMajorMode* QEmacsTextEditBase::setMajorMode() {
      auto& fm = QEmacsMajorModeFactory::getQEmacsMajorModeFactory();
      auto* m = fm.getQEmacsMajorModeForFile(
          QFileInfo(this->getFileName()).fileName(), this->qemacs,
          this->buffer, *this);
      this->setMajorMode(m);
      return m;
    }  // end of QEmacsTextEditBase::setMajorMode

    QEmacsMajorMode* QEmacsTextEditBase::setMajorMode(
        const QString& n) {
      auto& fm = QEmacsMajorModeFactory::getQEmacsMajorModeFactory();
      auto* m = fm.getQEmacsMajorModeByName(n, this->qemacs,
                                            this->buffer, *this);
      this->setMajorMode(m);
      return m;
    }  // end of QEmacsTextEditBase::setMajorMode

    void QEmacsTextEditBase::setMajorMode(QEmacsMajorMode * const m) {
      if (m == nullptr) {
        return;
      }
      if (this->mode != nullptr) {
        QObject::disconnect(this->mode, nullptr, this, nullptr);
        delete this->mode;
        this->mode = nullptr;
      }
      if (this->allowMajorModeChange) {
        this->qemacs.displayInformativeMessage(
            QObject::tr("setting major mode '%1'").arg(m->getName()));
        this->mode = m;
        this->mode->setSpellCheckLanguage(this->spellCheckLanguage);
        this->mode->setSyntaxHighlighter(this->document());
        emit majorModeChanged();
      } else {
        this->qemacs.displayInformativeMessage(
            QObject::tr("setting major mode '%1' is not allowed")
                .arg(m->getName()));
      }
    }  // end of QEmacsTextEditBase::setMajorMode

    void QEmacsTextEditBase::deleteContextMenuActions() {
      delete this->ua;
      this->ua = nullptr;
      delete this->ra;
      this->ra = nullptr;
      delete this->ca;
      this->ca = nullptr;
      delete this->coa;
      this->coa = nullptr;
      delete this->pa;
      this->pa = nullptr;
      delete this->sa;
      this->sa = nullptr;
    }  // end of QEmacsTextEditBase::deleteContextMenuActions

    void QEmacsTextEditBase::createContextMenuActions() {
      this->deleteContextMenuActions();
      // undo action
      this->ua = new QAction(QObject::tr("&Undo"), this);
      this->ua->setIcon(QIcon::fromTheme("edit-undo"));
      this->ua->setIconVisibleInMenu(true);
      QObject::connect(this->ua, &QAction::triggered, this,
                       &QEmacsTextEditBase::undo);
      // redo action
      this->ra = new QAction(QObject::tr("&Redo"), this);
      this->ra->setIcon(QIcon::fromTheme("edit-redo"));
      this->ra->setIconVisibleInMenu(true);
      QObject::connect(this->ra, &QAction::triggered, this,
                       &QEmacsTextEditBase::redo);
      // cut action
      this->ca = new QAction(QObject::tr("Cu&t"), this);
      this->ca->setIcon(QIcon::fromTheme("edit-cut"));
      this->ca->setIconVisibleInMenu(true);
      QObject::connect(this->ca, &QAction::triggered, this,
                       &QEmacsTextEditBase::cut);
      // copy action
      this->coa = new QAction(QObject::tr("&Copy"), this);
      this->coa->setIcon(QIcon::fromTheme("edit-copy"));
      this->coa->setIconVisibleInMenu(true);
      QObject::connect(this->coa, &QAction::triggered, this,
                       &QEmacsTextEditBase::copy);
      // paste action
      this->pa = new QAction(QObject::tr("&Paste"), this);
      this->pa->setIcon(QIcon::fromTheme("edit-paste"));
      this->pa->setIconVisibleInMenu(true);
      QObject::connect(this->pa, &QAction::triggered, this,
                       &QEmacsTextEditBase::paste);
      // select all action
      this->sa = new QAction(QObject::tr("Select All"), this);
      this->sa->setIcon(QIcon::fromTheme("edit-select-all"));
      this->sa->setIconVisibleInMenu(true);
      QObject::connect(this->sa, &QAction::triggered, this,
                       &QEmacsTextEditBase::selectAll);
    }  // end of QEmacsTextEditBase::createContextMenuActions

    void QEmacsTextEditBase::gotoLine(const int l) {
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
    }  // end QEmacsTextEditBase::gotoLine

    void QEmacsTextEditBase::gotoPosition(const int l, const int c) {
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
    }  // end of QEmacsTextEditBase::gotoPosition

    QString QEmacsTextEditBase::getCurrentWord() const {
      auto tc = this->textCursor();
      tc.select(QTextCursor::WordUnderCursor);
      return tc.selectedText();
    }  // end of QEmacsTextEditBase::getCurrentWord

    bool QEmacsTextEditBase::hasMajorMode() const {
      return this->mode != nullptr;
    }  // end of QEmacsTextEditBase::hasMajorMode

    const QEmacsMajorMode& QEmacsTextEditBase::getMajorMode() const {
      if (this->mode == nullptr) {
        throw(
            std::runtime_error("QEmacsTextEditBase::getQEmacsMode: "
                               "invalid call"));
      }
      return *(this->mode);
    }  // end of QEmacsTextEditBase::getQEmacsMode

    bool QEmacsTextEditBase::isModified() const {
      return this->document()->isModified();
    }  // end of QEmacsTextEditBase::isModified

    void QEmacsTextEditBase::addToKillRing(const QString& t) {
      this->qemacs.addToKillRing(t);
    }  // end of QEmacsTextEditBase::addToKillRing

    void QEmacsTextEditBase::highlightCurrentLine() {
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

    std::vector<QMenu*> QEmacsTextEditBase::getSpecificMenus() {
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
    }  // end of QEmacsTextEditBase::getSpecificMenu

    QIcon QEmacsTextEditBase::getIcon() const {
      if (this->mode == nullptr) {
        return QIcon();
      }
      return this->mode->getIcon();
    }  // end of QEmacsTextEditBase::getIcon

    void QEmacsTextEditBase::insertCompletion(
        const QString& completion) {
      if (this->mode == nullptr) {
        return;
      }
      this->mode->completeCurrentWord(*this, completion);
    }  // end of QEmacsTextEditBase::insertCompletion

    void QEmacsTextEditBase::removeKeyPressEventFilter() {
      if (this->filter != nullptr) {
        QObject::disconnect(
            this->filter, &QEmacsTextEditKeyPressEventFilter::destroyed,
            this, &QEmacsTextEditBase::keyPressEventFilterDestroyed);
        this->filter->deleteLater();
        this->filter = nullptr;
      }
    }  // end of QEmacsTextEditBase::removeKeyPressEventFilter

    void QEmacsTextEditBase::keyPressEventFilterDestroyed() {
      this->filter = nullptr;
    }  // end of QEmacsTextEditBase::keyPressEventFilterDestroyed

    bool QEmacsTextEditBase::setKeyPressEventFilter(
        QEmacsTextEditKeyPressEventFilter * const f) {
      if (f == nullptr) {
        return false;
      }
      if (this->filter != nullptr) {
        this->qemacs.displayInformativeMessage(
            QObject::tr("a key press event filter already exists"));
        return false;
      }
      if (!f->isOk()) {
        return false;
      }
      this->filter = f;
      QObject::connect(
          this->filter, &QEmacsTextEditKeyPressEventFilter::destroyed,
          this, &QEmacsTextEditBase::keyPressEventFilterDestroyed);
      return true;
    }  // end of QEmacsTextEditBase::setKeyPressEventFilter

    void QEmacsTextEditBase::setFileName(const QString& f) {
      QFileInfo fi(f);
      this->fileName = fi.absoluteFilePath();
      if (this->mode == nullptr) {
        this->setMajorMode();
      }
      emit fileNameChanged(this->fileName);
    }  // end of QEmacsTextEditBase::setFileName

    QString QEmacsTextEditBase::getFileName() const {
      return this->fileName;
    }  // end of QEmacsTextEditBase::getFileName

    QString QEmacsTextEditBase::getCompleteFileName() const {
      return this->fileName;
    }  // end of QEmacsTextEditBase::getCompleteFileName

    QString QEmacsTextEditBase::getDirectory() const {
      QFileInfo fn(this->fileName);
      if (fn.isDir()) {
        return fn.absolutePath();
      }
      return fn.dir().absolutePath();
    }  // end of QEmacsTextEditBase::getDirectory

    QEmacsTextEditBase::SaveInput* QEmacsTextEditBase::getSaveInput() {
      return new SaveInput(this->qemacs, *this);
    }  // end of QEmacsTextEditBase::getSaveInput

    void QEmacsTextEditBase::save() {
      if (!this->document()->isModified()) {
        this->qemacs.displayInformativeMessage(
            QObject::tr("(no changes need to be saved)"));
        return;
      }
      if (this->fileName.isEmpty()) {
        this->writeFile();
        return;
      }
      const auto& af = QFileInfo(this->fileName).absoluteFilePath();
      this->qemacs.displayInformativeMessage(
          QObject::tr("saving file '%1'").arg(af));
      QTextDocumentWriter writer(this->fileName);
      writer.setFormat("plaintext");
      if (writer.write(this->document())) {
        this->document()->setModified(false);
        this->qemacs.displayInformativeMessage(
            QObject::tr("wrote file '%1'").arg(af));
      } else {
        this->qemacs.displayInformativeMessage(
            QObject::tr("saving file '%1' failed").arg(af));
      }
    }  // end of QEmacsTextEditBase::save

    void QEmacsTextEditBase::writeFile() {
      QFileInfo fn(this->getCompleteFileName());
      QDir d(fn.dir());
      QString path;
      if (d.exists()) {
        path = d.absolutePath();
      } else {
        path = QDir::current().absolutePath();
      }
      auto* l =
          new QEmacsTextEditBase::WriteFile(this->qemacs, *this, path);
      l->setInputHistorySettingAddress("recent files");
      this->qemacs.setUserInput(l);
    }

    void QEmacsTextEditBase::writeFile(const QString& f) {
      // basic check
      QFileInfo fi(f);
      if (fi.exists()) {
        if (!fi.isWritable()) {
          QString msg(QObject::tr(
              "file '%1' already exists and is not readable"));
          this->qemacs.displayInformativeMessage(msg.arg(f));
        }
      }
      this->setFileName(f);
      this->document()->setModified();
      this->save();
    }  // end of QEmacsTextEditBase::writeFile

    void QEmacsTextEditBase::emitCursorPositionChanged() {
      emit cursorPositionChanged();
    }  // end of QEmacsTextEditBase::emitCursorPositionChanged

    bool QEmacsTextEditBase::event(QEvent * ev) {
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

    void QEmacsTextEditBase::keyPressEvent(QKeyEvent * e) {
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
    }  // end of QEmacsTextEditBase::keyPressEvent

    bool QEmacsTextEditBase::handleKeyPressEvent(QKeyEvent * e) {
      auto& s = QEmacsShortCutStyle::getQEmacsShortCutStyle();
      if (s.getStyle() == QEmacsShortCutStyle::QT) {
        return this->handleKeyPressEventWithQtShortCuts(e);
      }
      return this->handleKeyPressEventWithEmacsShortCuts(e);
    }

    bool QEmacsTextEditBase::handleKeyPressEventWithQtShortCuts(
        QKeyEvent * e) {
      const int k = e->key();
      const Qt::KeyboardModifiers m = e->modifiers();
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
    }  // end of QEmacsTextEditBase::handleKeyPressEventWithQtShortCuts

    bool QEmacsTextEditBase::handleKeyPressEventWithEmacsShortCuts(
        QKeyEvent * e) {
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
              this->buffer.showSecondaryTasks(Qt::Vertical);
            }
            return true;
          } else if (k == Qt::Key_3) {
            if (this->isMainFrame()) {
              this->buffer.showSecondaryTasks(Qt::Horizontal);
            }
            return true;
          } else if (k == Qt::Key_O) {
            if (this->isMainFrame()) {
              if (this->buffer.hasSecondaryTasks()) {
                if (this->buffer.areSecondaryTasksVisible()) {
                  this->buffer.focusCurrentSecondaryTask();
                } else {
                  if (this->qemacs.hasUserInput()) {
                    this->qemacs.focusUserInput();
                  }
                }
              } else {
                if (this->qemacs.hasUserInput()) {
                  this->qemacs.focusUserInput();
                }
              }
            } else {
              if (this->qemacs.hasUserInput()) {
                this->qemacs.focusUserInput();
              } else {
                auto& b = this->qemacs.getCurrentBuffer();
                b.focusMainFrame();
              }
            }
            return true;
          } else if (k == Qt::Key_P) {
            this->print();
            return true;
          } else if (k == Qt::Key_B) {
            this->qemacs.changeBuffer();
            return true;
          } else if (k == Qt::Key_R) {
            this->qemacs.setUserInput(
                new QEmacsRectangleMode(*this, this->qemacs));
            this->moveMode = QTextCursor::MoveAnchor;
            return true;
          } else if (k == Qt::Key_K) {
            if (this->isMainFrame()) {
              this->qemacs.closeCurrentBuffer();
            } else {
              auto& b = this->qemacs.getCurrentBuffer();
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
            this->qemacs.openFile();
            return true;
          } else if (k == Qt::Key_C) {
            this->qemacs.close();
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
        const auto modifier = QEmacsTextEditBase::getModifier(*e);
        if (modifier.isEmpty()) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-X - %1")
                  .arg(e->text()));
        } else {
          this->qemacs.displayInformativeMessage(
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
        const auto modifier = QEmacsTextEditBase::getModifier(*e);
        if (modifier.isEmpty()) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-C - %1")
                  .arg(e->text()));
        } else {
          this->qemacs.displayInformativeMessage(
              QObject::tr("unknown shortcut : Ctr-C - %1-%2")
                  .arg(modifier, e->text()));
        }
        return true;
      }
      if ((m == Qt::ControlModifier) && (k == Qt::Key_Left)) {
        this->qemacs.previousBuffer();
        return true;
      }
      if ((m == Qt::ControlModifier) && (k == Qt::Key_Right)) {
        this->qemacs.nextBuffer();
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
          this->qemacs.removeUserInput();
          this->qemacs.displayInformativeMessage(QObject::tr("quit"));
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_S)) {
        if (this->filter != nullptr) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("a filter is already set"));
          return true;
        }
        // set a search user input
        // the QEmacsTextEditSearch automatically set itself as a filter
        // for this
        this->qemacs.setUserInput(
            new QEmacsTextEditSearch(*this, this->qemacs));
        // the current buffer shall have the focus : but default, the
        // QEmacsWidget gives focus to the user input
        this->setFocus();
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_R)) {
        if (this->filter != nullptr) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("a filter is already set"));
          return true;
        }
        // set a search user input
        // the QEmacsTextEditSearch automatically set itself as a filter
        // for this
        this->qemacs.setUserInput(new QEmacsTextEditSearch(
            *this, this->qemacs, QTextDocument::FindBackward));
        // the current buffer shall have the focus : but default, the
        // QEmacsWidget gives focus to the user input
        this->setFocus();
        return true;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_Space)) {
        c.clearSelection();
        this->setTextCursor(c);
        this->qemacs.displayInformativeMessage(QObject::tr("mark set"));
        this->moveMode = QTextCursor::KeepAnchor;
      } else if (((m == Qt::ControlModifier) &&
                  (k == Qt::Key_Underscore)) ||
                 ((m == Qt::ControlModifier) && (k == Qt::Key_U))) {
        this->undo();
        this->moveMode = QTextCursor::MoveAnchor;
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_M)) {
        if (this->isReadOnly()) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          this->insertPlainText("\n");
        }
      } else if ((m == Qt::ControlModifier) && (k == Qt::Key_W)) {
        if (this->isReadOnly()) {
          this->qemacs.displayInformativeMessage(
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
          this->qemacs.displayInformativeMessage(
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
          this->qemacs.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          const QStringList& ring = this->qemacs.getKillRing();
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
          this->qemacs.displayInformativeMessage(
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
        this->qemacs.setUserInput(
            new QEmacsTextEditQueryReplace(*this, this->qemacs));

      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Q)) {
        if (this->isReadOnly()) {
          this->qemacs.displayInformativeMessage(
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
          this->qemacs.displayInformativeMessage(
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
          this->qemacs.displayInformativeMessage(
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
          this->qemacs.displayInformativeMessage(
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
        this->qemacs.launchCommand();
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Exclam)) {
        auto* l = new QEmacsShellProcessLineEdit("shell command:", "",
                                                 "", this->qemacs);
        l->setInputHistorySettingAddress("command/shell/history");
        this->qemacs.setUserInput(l);
      } else if (((m == Qt::AltModifier) && (k == Qt::Key_Percent)) ||
                 ((m == (Qt::AltModifier | Qt::ShiftModifier)) &&
                  (k == Qt::Key_Percent))) {
        this->qemacs.setUserInput(
            new QEmacsTextEditQueryReplace(*this, this->qemacs));
      } else if (((m == Qt::AltModifier) && (k == Qt::Key_Percent)) ||
                 ((m == (Qt::AltModifier | Qt::ShiftModifier)) &&
                  (k == Qt::Key_Percent))) {
        this->qemacs.setUserInput(
            new QEmacsTextEditQueryReplace(*this, this->qemacs));
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_G)) {
        this->qemacs.setUserInput(new GotoLine(*this, this->qemacs));
      } else if ((m == Qt::AltModifier) && (k == Qt::Key_Y)) {
        if (this->isReadOnly()) {
          this->qemacs.displayInformativeMessage(
              QObject::tr("read only buffer"));
        } else {
          const auto& ring = this->qemacs.getKillRing();
          if ((ring.isEmpty()) || (!this->yank)) {
            if (ring.isEmpty()) {
              this->qemacs.displayInformativeMessage(
                  QObject::tr("kill ring is empty"));
            }
            if (!this->yank) {
              this->qemacs.displayInformativeMessage(
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
          this->qemacs.displayInformativeMessage(
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
            this->qemacs.displayInformativeMessage(
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
    }  // end of QEmacsTextEdit::handleKeyPressEvent

    bool QEmacsTextEditBase::handleMousePressEvent(QMouseEvent * e) {
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

  void QEmacsTextEditBase::setMainFrame(const bool b) {
    //    this->setMinimumHeight(400);
    this->mainFrame = b;
  }  // end of QEmacsTextEditBase::setMainFrame

  bool QEmacsTextEditBase::isMainFrame() const {
    return this->mainFrame;
  }  // end of QEmacsTextEditBase::isMainFrame

  bool QEmacsTextEditBase::isSecondaryTask() const {
    return !(this->isMainFrame());
  }  // end of QEmacsTextEditBase::isSecondaryTask

  void QEmacsTextEditBase::setMoveMode(QTextCursor::MoveMode m) {
    this->moveMode = m;
  }  // end of QEmacsTextEditBase::setMoveMode

  QEmacsTextEditBase::~QEmacsTextEditBase() {
    if (this->filter != nullptr) {
      QObject::disconnect(
          this->filter, &QEmacsTextEditKeyPressEventFilter::destroyed,
          this, &QEmacsTextEditBase::keyPressEventFilterDestroyed);
    }
    this->deleteContextMenuActions();
    delete this->filter;
    delete this->mode;
  }  // end of QEmacsTextEditBase::~QEmacsTextEditBase

}  // end of namespace qemacs
