/*!
 * \file  CommandLine.cxx
 * \brief
 * \author Helfer Thomas
 * \date   05/07/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QKeyEvent>
#include <QtGui/QFileSystemModel>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAbstractItemView>

#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEdit.hxx"
#include "TFEL/GUI/CommandLine.hxx"

namespace tfel::gui {

  CommandLine::CustomLineEdit::CustomLineEdit(EditorWidget& q, CommandLine& p)
      : LineEdit(q, &p), lineEdit(p) {}  // end of CustomLineEdit

  CommandLine::CustomLineEdit::~CustomLineEdit() {
    if (!this->completerHandledByQLineEdit) {
      delete this->c_;
    }
  }  // end of ~CustomLineEdit()

  void CommandLine::CustomLineEdit::setCompleter(QCompleter* const c,
                                                 const bool b) {
    if (c == nullptr) {
      return;
    }
    if (!this->completerHandledByQLineEdit) {
      delete this->c_;
    }
    this->c_ = c;
    QLineEdit::setCompleter(c);
    if (b) {
      this->completerHandledByQLineEdit = true;
    } else {
      this->completerHandledByQLineEdit = false;
    }
  }  // end of setCompleter

  QCompleter* CommandLine::CustomLineEdit::completer() const {
    return this->c_;
  }  // end of completer

  void CommandLine::setInputHistory(const QStringList& h) {
    if (this->input != nullptr) {
      this->input->setInputHistory(h);
    }
  }

  void CommandLine::setInputHistorySettingAddress(const QString& a) {
    this->inputHistorySettingAddress = a;
    QSettings s;
    this->setInputHistory(s.value(a).toStringList());
  }

  bool CommandLine::CustomLineEdit::event(QEvent* ev) {
    if (ev->type() == QEvent::KeyPress) {
      auto* kev = static_cast<QKeyEvent*>(ev);
      if (kev->key() == Qt::Key_Tab) {
        if (auto* c = this->completer(); c != nullptr) {
          this->complete();
        }
        ev->accept();
        return true;
      }
    }
    return LineEdit::event(ev);
  }  // end of event

  void CommandLine::CustomLineEdit::keyPressEvent(QKeyEvent* ev) {
    const auto k = ev->key();
    const auto m = ev->modifiers();
    const auto ctrl = m == Qt::ControlModifier;
    if ((ctrl) && (k == Qt::Key_G)) {
      if (auto* qle = qobject_cast<CommandLine*>(this->parent());
          qle != nullptr) {
        qle->cancel();
      }
      return;
    }
    if ((ctrl) && (k == Qt::Key_M)) {
      auto qle = qobject_cast<CommandLine*>(this->parent());
      if (qle != nullptr) {
        qle->userEditingFinished();
      }
      return;
    }
    LineEdit::keyPressEvent(ev);
  }  // end of keyPressEvent

  void CommandLine::CustomLineEdit::complete() {
    auto b = bool{};
    const auto c = this->findCompletion(b);
    if (!c.isEmpty()) {
      this->setText(c);
    }
  }

  QString CommandLine::CustomLineEdit::findCompletion(bool& b) {
    QString r;
    b = true;
    if (auto* const c = this->completer(); c != nullptr) {
      QStringList cc;
      c->setCompletionPrefix(this->text());
      auto* m = qobject_cast<QFileSystemModel*>(c->model());
      if (m != nullptr) {
        QFileInfo fi(this->text());
        m->setRootPath(fi.absolutePath());
      }
      b = c->completionCount() == 1;
      for (int i = 0; c->setCurrentRow(i); i++) {
        cc.append(c->currentCompletion());
      }
      r = commonPart(cc);
      const auto base = this->extractBaseForCompletion(r);
      if ((r == this->text()) && (cc.size() != 1) && (!cc.empty())) {
        if (m != nullptr) {
          for (int i = 0; i != cc.size(); ++i) {
            QFileInfo fn(cc[i]);
            cc[i] = fn.fileName();
          }
        }
        this->lineEdit.showCompletions(base, cc);
      }
    }
    return r;
  }  // end of findCompletion

  QString CommandLine::CustomLineEdit::extractBaseForCompletion(
      const QString& r) {
    return r;  //    return "";
  }            // end of extractBaseForCompletion

  void CommandLine::CustomLineEdit::handleShortCut(
      const int k1, const Qt::KeyboardModifiers m, const int k2) {
    if ((k1 == Qt::Key_X) && (m == Qt::NoModifier) && (k2 == Qt::Key_O)) {
      auto qle = qobject_cast<CommandLine*>(this->parent());
      if (qle != nullptr) {
        if (!qle->isBlocking()) {
          this->editor.getCurrentBuffer().focusMainFrame();
        }
      }
      return;
    }
    LineEdit::handleShortCut(k1, m, k2);
  }  // end of handleShortCut

  CommandLine::CommandLine(const QString& l, EditorWidget& p, const bool b)
      : QWidget(&p),
        editor(p),
        buffer(p.getCurrentBuffer()),
        // vl(new QVBoxLayout),
        hl(new QHBoxLayout),
        label(new QLabel(this)),
        input(nullptr),
        completions(nullptr),
        scompletions(nullptr),
        isUserEditingFinished(false) {
    this->setBackgroundRole(QPalette::Base);
    this->setContentsMargins(0, 0, 0, 0);
    QSizePolicy s;
    s.setHorizontalPolicy(QSizePolicy::Fixed);
    s.setVerticalPolicy(QSizePolicy::Fixed);
    this->label->setSizePolicy(s);
    this->label->setAlignment(Qt::AlignHCenter);
    this->label->setFocusPolicy(Qt::NoFocus);
    this->label->setContentsMargins(0, 0, 0, 0);
    this->label->setLineWidth(0);
    this->setLabel(l);
    //    this->hl->setMargin(0);
    this->hl->setSpacing(0);
    this->hl->setContentsMargins(0, 0, 0, 0);
    this->setStyleSheet("background: rgba(255,255,255,100%)");
    this->label->setStyleSheet("background: rgba(255,255,255,100%)");
    this->hl->addWidget(this->label);
    // this->vl->setMargin(0);
    // this->vl->setSpacing(0);
    // this->vl->setContentsMargins(0,0,0,0);
    // this->vl->addLayout(this->hl);
    // this->completions->hide();
    // this->completions->setFocusPolicy(Qt::NoFocus);
    // this->completions->setContentsMargins(0,0,0,0);
    // this->vl->addWidget(this->completions);
    if (b) {
      this->setLineEdit(new CustomLineEdit(p, *this));
    }
    this->setLayout(this->hl);
  }  // end of CommandLine::CommandLine

  bool CommandLine::isBlocking() const {
    return false;
  }  // end of CommandLine::isBlocking

  void CommandLine::hideCompletions() {
    this->buffer.hideSecondaryTask(this->scompletions);
  }  // end of CommandLine::hideCompletions

  void CommandLine::showCompletions(const QString&, const QStringList& cl) {
    if (this->completions == nullptr) {
      this->completions = new TextEdit(this->editor, this->buffer);
      if (this->completions == nullptr) {
        return;
      }
      this->completions->setReadOnly(true);
      this->scompletions =
          this->buffer.attachSecondaryTask("*completions*", this->completions);
      QObject::connect(this, &CommandLine::textChanged, this,
                       &CommandLine::hideCompletions);
    }
    this->completions->clear();
    this->completions->insertHtml(
        "<font color=\"blue\">Possible completions :</font>");
    const auto d = cl.size() / 2;
    const auto m = cl.size() % 2;
    QString table = "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
    for (int i = 0; i != d; ++i) {
      QString l;
      l = "<tr>";
      l += "<td width=\"45%\">" + cl[i * 2] + "</td>";
      l += "<td width=\"45%\">" + cl[i * 2 + 1] + "</td>";
      l += "</tr>";
      table += l;
    }
    QString ll;
    if (m == 1) {
      ll = "<tr>";
      ll += "<td width=\"45%\">" + cl[d * 2] +
            "</td>"
            "<td width=\"45%\"></td>";
      ll += "</tr>";
      table += ll;
    }
    table += "</table>";
    this->completions->insertHtml(table);
    this->scompletions->show();
    this->buffer.showSecondaryTasks();
  }  // end of CommandLine::showCompletions

  void CommandLine::setLabel(const QString& l) {
    this->label->setText("<font color=blue> " + l + " </font>");
  }

  void CommandLine::keyPressEvent(QKeyEvent* e) {
    if (this->input == nullptr) {
      return;
    }
    this->input->event(e);
  }

  void CommandLine::setLineEdit(CommandLine::CustomLineEdit* const i) {
    if (i == nullptr) {
      return;
    }
    if (this->input != nullptr) {
      this->hl->removeWidget(this->input);
      QObject::disconnect(this->input,
                          &CommandLine::CustomLineEdit::returnPressed, this,
                          &CommandLine::userEditingFinished);
      QObject::disconnect(this->input,
                          &CommandLine::CustomLineEdit::textChanged, this,
                          &CommandLine::inputTextChanged);
      QObject::disconnect(this->input, &CommandLine::CustomLineEdit::textEdited,
                          this, &CommandLine::inputTextEdited);
      this->input->deleteLater();
    }
    this->input = i;
    this->input->setContentsMargins(0, 0, 0, 0);
    this->input->setFont(this->label->font());
    this->input->setFrame(false);
    this->input->setStyleSheet("background: rgba(255,255,255,100%)");
    this->hl->addWidget(this->input);
    QObject::connect(this->input, &CommandLine::CustomLineEdit::returnPressed,
                     this, &CommandLine::userEditingFinished);
    QObject::connect(this->input, &CommandLine::CustomLineEdit::textChanged,
                     this, &CommandLine::inputTextChanged);
    QObject::connect(this->input, &CommandLine::CustomLineEdit::textEdited,
                     this, &CommandLine::inputTextEdited);
  }

  void CommandLine::userEditingFinished() {
    if (!this->isUserEditingFinished) {
      this->isUserEditingFinished = true;
      auto h = this->input->getInputHistory();
      h.append(this->input->text());
      if (h.size() > 100) {
        // number of elements to be erased
        const auto ne = h.size() - 100;
        for (auto i = 0; i != ne; ++i) {
          h.removeAt(0);
        }
      }
      this->input->setInputHistory(h);
      if (!this->inputHistorySettingAddress.isEmpty()) {
        QSettings s;
        s.setValue(this->inputHistorySettingAddress, h);
      }
      this->treatUserInput();
      emit finished(this);
    }
  }  // end of CommandLine::userEditingFinished()

  void CommandLine::cancel() {
    QObject::disconnect(this->input,
                        &CommandLine::CustomLineEdit::returnPressed, this,
                        &CommandLine::userEditingFinished);
    this->isUserEditingFinished = true;
    emit finished(this);
  }  // end of CommandLine::cancel()

  void CommandLine::setFocus() {
    this->input->setFocus();
  }  // end of CommandLine::setFocus()

  void CommandLine::inputTextChanged(const QString& t) { emit textChanged(t); }

  void CommandLine::inputTextEdited(const QString& t) { emit textEdited(t); }

  CommandLine::~CommandLine() {
    if ((this->completions != nullptr) && (this->scompletions != nullptr)) {
      this->buffer.removeSecondaryTask(this->scompletions);
    }
    emit destroyed(this);
  }  // end CommandLine::~CommandLine()

  struct YesOrNoUserInput::YesOrNoLineEdit : CommandLine::CustomLineEdit {
    YesOrNoLineEdit(EditorWidget& q, CommandLine& p)
        : CustomLineEdit(q, p) {}  // end of YesOrNoLineEdit

    void keyPressEvent(QKeyEvent* ev) override {
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_G)) {
        auto qle = qobject_cast<CommandLine*>(this->parent());
        if (qle != nullptr) {
          qle->cancel();
        }
        return;
      }
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_O)) {
        auto* qle = qobject_cast<CommandLine*>(this->parent());
        if (qle != nullptr) {
          if (!qle->isBlocking()) {
            this->editor.getCurrentBuffer().setFocus();
            this->editor.getCurrentBuffer().focusMainFrame();
          }
        }
      }
      if (((ev->modifiers() == Qt::NoModifier) && (ev->key() == Qt::Key_Y)) ||
          ((ev->modifiers() == Qt::NoModifier) && (ev->key() == Qt::Key_N))) {
        if (ev->key() == Qt::Key_Y) {
          this->setText("y");
        }
        if (ev->key() == Qt::Key_N) {
          this->setText("n");
        }
        auto* qle = qobject_cast<CommandLine*>(this->parent());
        qle->userEditingFinished();
      }
    }
  };

  YesOrNoUserInput::YesOrNoUserInput(const QString& i, EditorWidget& p)
      : CommandLine(QObject::tr("%1 (y or n)").arg(i), p, false) {
    this->setLineEdit(new YesOrNoLineEdit(p, *this));
  }  // end of YesOrNoUserInput::YesOrNoUserInput

  struct FilePathUserInput::FilePathLineEdit : CommandLine::CustomLineEdit {
    FilePathLineEdit(EditorWidget& q, CommandLine& p) : CustomLineEdit(q, p) {
      auto* c = new QCompleter(&p);
      auto* dm = new QFileSystemModel(c);
      dm->setRootPath(QDir::currentPath());
      dm->setNameFilterDisables(false);
      c->setModel(dm);
      c->setCompletionMode(QCompleter::InlineCompletion);
      this->setCompleter(c, false);
    }  // end of FilePathLineEdit

   protected:
    void complete() override {
      auto p = this->text();
      if (p.startsWith("~/")) {
        auto homePath = QDir::home().absolutePath();
        if (!homePath.endsWith(QDir::separator())) {
          homePath += QDir::separator();
        }
        p.replace(0, 2, homePath);
        QLineEdit::setText(p);
      }
      bool b;
      const auto c = this->findCompletion(b);
      if (c.isEmpty()) {
        return;
      }
      this->setText(c, b);
    }  // end of complete

    QString extractBaseForCompletion(const QString& c) override {
      QFileInfo f(c);
      auto d = f.dir().absolutePath();
      if (!d.endsWith(QDir::separator())) {
        d += QDir::separator();
      }
      return d;
    }

    virtual void setText(const QString& c, const bool b) {
      if (b) {
        QFileInfo f(c);
        if (f.exists()) {
          if (f.isDir()) {
            if (!c.endsWith(QDir::separator())) {
              QLineEdit::setText(c + QDir::separator());
            } else {
              QLineEdit::setText(c);
            }
          } else {
            QLineEdit::setText(c);
          }
        } else {
          QLineEdit::setText(c);
        }
      } else {
        QLineEdit::setText(c);
      }
    }
  };

  FilePathUserInput::FilePathUserInput(const QString& i, EditorWidget& p)
      : CommandLine(i, p) {
    this->setLineEdit(new FilePathLineEdit(p, *this));
  }  // end of FilePathUserInput

}  // end of namespace tfel::gui
