/*!
 * \file  QEmacsCommandLine.cxx
 * \brief
 * \author Helfer Thomas
 * \date   05/07/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QDirModel>
#include <QtWidgets/QAbstractItemView>

#include "QEmacs/Utilities.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEdit.hxx"
#include "QEmacs/QEmacsCommandLine.hxx"

namespace qemacs {

  QEmacsCommandLine::CustomLineEdit::CustomLineEdit(QEmacsWidget& q,
                                                 QEmacsCommandLine& p)
      : QEmacsLineEdit(q,&p),
        lineEdit(p) {
  }  // end of QEmacsCommandLine::CustomLineEdit::CustomLineEdit

  QEmacsCommandLine::CustomLineEdit::~CustomLineEdit() {
    if (!this->completerHandledByQLineEdit) {
      delete this->c_;
    }
  }  // end of QEmacsCommandLine::CustomLineEdit::~CustomLineEdit()

  void QEmacsCommandLine::CustomLineEdit::setCompleter(
      QCompleter* const c, const bool b) {
    if (c == nullptr) {
      return;
    }
    if (!this->completerHandledByQLineEdit) {
      delete this->c_;
    }
    this->c_ = c;
    if (b) {
      QLineEdit::setCompleter(c);
      this->completerHandledByQLineEdit = true;
    } else {
      this->completerHandledByQLineEdit = false;
    }
  }  // end of QEmacsCommandLine::CustomLineEdit::setCompleter

  QCompleter* QEmacsCommandLine::CustomLineEdit::completer() const {
    return this->c_;
  }  // end of QEmacsCommandLine::CustomLineEdit::completer

  void QEmacsCommandLine::setInputHistory(const QStringList& h) {
    if (this->input != nullptr) {
      this->input->setInputHistory(h);
    }
  }

  void QEmacsCommandLine::setInputHistorySettingAddress(const QString& a) {
    this->inputHistorySettingAddress = a;
    QSettings s;
    this->setInputHistory(s.value(a).toStringList());
  }

  bool QEmacsCommandLine::CustomLineEdit::event(QEvent* ev) {
    if (ev->type() == QEvent::KeyPress) {
      auto* kev = static_cast<QKeyEvent*>(ev);
      if (kev->key() == Qt::Key_Tab) {
        auto* c = this->completer();
        if (c != nullptr) {
          this->complete();
        }
        ev->accept();
        return true;
      }
    }
    return QEmacsLineEdit::event(ev);
  }  // end of QEmacsCommandLine::CustomLineEdit::event

  void QEmacsCommandLine::CustomLineEdit::keyPressEvent(QKeyEvent* ev) {
    const auto k = ev->key();
    const auto m = ev->modifiers();
    const auto ctrl = m == Qt::ControlModifier;
    if ((ctrl) && (k == Qt::Key_G)) {
      auto qle = qobject_cast<QEmacsCommandLine*>(this->parent());
      if (qle != nullptr) {
        qle->cancel();
      }
      return;
    }
    if ((ctrl) && (k == Qt::Key_M)) {
      auto qle = qobject_cast<QEmacsCommandLine*>(this->parent());
      if (qle != nullptr) {
        qle->userEditingFinished();
      }
      return;
    }
    QEmacsLineEdit::keyPressEvent(ev);
  } // end of QEmacsCommandLine::CustomLineEdit::keyPressEvent

  void QEmacsCommandLine::CustomLineEdit::complete() {
    bool b;
    const auto c = this->findCompletion(b);
    if (!c.isEmpty()) {
      this->setText(c);
    }
  }

  QString QEmacsCommandLine::CustomLineEdit::findCompletion(bool& b) {
    auto* c = this->completer();
    QString r;
    b = true;
    if (c != nullptr) {
      QStringList cc;
      c->setCompletionPrefix(this->text());
      b = c->completionCount() == 1;
      for (int i = 0; c->setCurrentRow(i); i++) {
        cc.append(c->currentCompletion());
      }
      r = commonPart(cc);
      const auto base = this->extractBaseForCompletion(r);
      if ((r == this->text()) && (cc.size() != 1) && (!cc.empty())) {
        for (int i = 0; i != cc.size(); ++i) {
          QFileInfo fn(cc[i]);
          cc[i] = fn.fileName();
        }
        this->lineEdit.showCompletions(base, cc);
      }
    }
    return r;
  }  // end of QEmacsCommandLine::CustomLineEdit::findCompletion

  QString QEmacsCommandLine::CustomLineEdit::extractBaseForCompletion(
      const QString& r) {
    return r;  //    return "";
  }  // end of QEmacsCommandLine::CustomLineEdit::extractBaseForCompletion

  void QEmacsCommandLine::CustomLineEdit::handleShortCut(
      const int k1, const Qt::KeyboardModifiers m, const int k2) {
    if ((k1 == Qt::Key_X) && (m == Qt::NoModifier) &&
        (k2 == Qt::Key_O)) {
      auto qle = qobject_cast<QEmacsCommandLine*>(this->parent());
      if (qle != nullptr) {
        if (!qle->isBlocking()) {
          this->qemacs.getCurrentBuffer().focusMainFrame();
        }
      }
      return;
    }
    QEmacsLineEdit::handleShortCut(k1,m,k2);
  } // end of QEmacsCommandLine::CustomLineEdit::handleShortCut

  QEmacsCommandLine::QEmacsCommandLine(const QString& l,
                                 QEmacsWidget& p,
                                 const bool b)
      : QWidget(&p),
        qemacs(p),
        buffer(p.getCurrentBuffer()),
        // vl(new QVBoxLayout),
        hl(new QHBoxLayout),
        label(new QLabel(this)),
        input(nullptr),
        completions(nullptr),
        scompletions(nullptr),
        isUserEditingFinished(false) {
    this->setBackgroundRole(QPalette::Foreground);
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
    this->hl->setMargin(0);
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
  }  // end of QEmacsCommandLine::QEmacsCommandLine

  bool QEmacsCommandLine::isBlocking() const {
    return false;
  }  // end of QEmacsCommandLine::isBlocking

  void QEmacsCommandLine::hideCompletions() {
    this->buffer.hideSecondaryTask(this->scompletions);
  }  // end of QEmacsCommandLine::hideCompletions

  void QEmacsCommandLine::showCompletions(const QString&,
                                       const QStringList& cl) {
    if (this->completions == nullptr) {
      this->completions =
          new QEmacsTextEdit(this->qemacs, this->buffer);
      if (this->completions == nullptr) {
        return;
      }
      this->completions->setReadOnly(true);
      this->scompletions =
          this->buffer.attachSecondaryTask("*completions*",
          this->completions);
      QObject::connect(this, &QEmacsCommandLine::textChanged, this,
                       &QEmacsCommandLine::hideCompletions);
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
  }  // end of QEmacsCommandLine::showCompletions

  void QEmacsCommandLine::setLabel(const QString& l) {
    this->label->setText("<font color=blue> " + l + " </font>");
  }

  void QEmacsCommandLine::keyPressEvent(QKeyEvent* e) {
    if (this->input == nullptr) {
      return;
    }
    this->input->event(e);
  }

  void QEmacsCommandLine::setLineEdit(
      QEmacsCommandLine::CustomLineEdit* const i) {
    if (i == nullptr) {
      return;
    }
    if (this->input != nullptr) {
      this->hl->removeWidget(this->input);
      QObject::disconnect(
          this->input, &QEmacsCommandLine::CustomLineEdit::returnPressed,
          this, &QEmacsCommandLine::userEditingFinished);
      QObject::disconnect(this->input,
                          &QEmacsCommandLine::CustomLineEdit::textChanged,
                          this, &QEmacsCommandLine::inputTextChanged);
      QObject::disconnect(this->input,
                          &QEmacsCommandLine::CustomLineEdit::textEdited,
                          this, &QEmacsCommandLine::inputTextEdited);
      this->input->deleteLater();
    }
    this->input = i;
    this->input->setContentsMargins(0, 0, 0, 0);
    this->input->setFont(this->label->font());
    this->input->setFrame(false);
    this->input->setStyleSheet("background: rgba(255,255,255,100%)");
    this->hl->addWidget(this->input);
    QObject::connect(this->input,
                     &QEmacsCommandLine::CustomLineEdit::returnPressed,
                     this, &QEmacsCommandLine::userEditingFinished);
    QObject::connect(this->input,
                     &QEmacsCommandLine::CustomLineEdit::textChanged, this,
                     &QEmacsCommandLine::inputTextChanged);
    QObject::connect(this->input,
                     &QEmacsCommandLine::CustomLineEdit::textEdited, this,
                     &QEmacsCommandLine::inputTextEdited);
  }

  void QEmacsCommandLine::userEditingFinished() {
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
  }  // end of QEmacsCommandLine::userEditingFinished()

  void QEmacsCommandLine::cancel() {
    QObject::disconnect(this->input,
                        &QEmacsCommandLine::CustomLineEdit::returnPressed,
                        this, &QEmacsCommandLine::userEditingFinished);
    this->isUserEditingFinished = true;
    emit finished(this);
  }  // end of QEmacsCommandLine::cancel()

  void QEmacsCommandLine::setFocus() {
    this->input->setFocus();
  }  // end of QEmacsCommandLine::setFocus()

  void QEmacsCommandLine::inputTextChanged(const QString& t) {
    emit textChanged(t);
  }

  void QEmacsCommandLine::inputTextEdited(const QString& t) {
    emit textEdited(t);
  }

  QEmacsCommandLine::~QEmacsCommandLine() {
    if ((this->completions != nullptr) &&
        (this->scompletions != nullptr)) {
      this->buffer.removeSecondaryTask(this->scompletions);
    }
    emit destroyed(this);
  }  // end QEmacsCommandLine::~QEmacsCommandLine()

  struct QEmacsYesOrNoUserInput::YesOrNoLineEdit
      : QEmacsCommandLine::CustomLineEdit {
    YesOrNoLineEdit(QEmacsWidget& q, QEmacsCommandLine& p)
        : CustomLineEdit(q, p) {}  // end of YesOrNoLineEdit

    void keyPressEvent(QKeyEvent* ev) override {
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_G)) {
        auto qle = qobject_cast<QEmacsCommandLine*>(this->parent());
        if (qle != nullptr) {
          qle->cancel();
        }
        return;
      }
      if ((ev->modifiers() == Qt::ControlModifier) &&
          (ev->key() == Qt::Key_O)) {
        auto* qle = qobject_cast<QEmacsCommandLine*>(this->parent());
        if (qle != nullptr) {
          if (!qle->isBlocking()) {
            this->qemacs.getCurrentBuffer().setFocus();
            this->qemacs.getCurrentBuffer().focusMainFrame();
          }
        }
      }
      if (((ev->modifiers() == Qt::NoModifier) &&
           (ev->key() == Qt::Key_Y)) ||
          ((ev->modifiers() == Qt::NoModifier) &&
           (ev->key() == Qt::Key_N))) {
        if (ev->key() == Qt::Key_Y) {
          this->setText("y");
        }
        if (ev->key() == Qt::Key_N) {
          this->setText("n");
        }
        auto* qle = qobject_cast<QEmacsCommandLine*>(this->parent());
        qle->userEditingFinished();
      }
    }
  };

  QEmacsYesOrNoUserInput::QEmacsYesOrNoUserInput(const QString& i,
                                                 QEmacsWidget& p)
      : QEmacsCommandLine(QObject::tr("%1 (y or n)").arg(i), p, false) {
    this->setLineEdit(new YesOrNoLineEdit(p, *this));
  }  // end of QEmacsYesOrNoUserInput::QEmacsYesOrNoUserInput

  struct QEmacsFilePathUserInput::FilePathLineEdit
      : QEmacsCommandLine::CustomLineEdit {
    FilePathLineEdit(QEmacsWidget& q, QEmacsCommandLine& p)
        : CustomLineEdit(q, p) {
      auto* c = new QCompleter(&p);
      auto* dm = new QDirModel(c);
      //      dm->setRootPath(QDir::currentPath());
      //      dm->setNameFilterDisables(false);
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

  QEmacsFilePathUserInput::QEmacsFilePathUserInput(const QString& i,
                                                   QEmacsWidget& p)
      : QEmacsCommandLine(i, p) {
    this->setLineEdit(new FilePathLineEdit(p, *this));
  }  // end of QEmacsFilePathUserInput::QEmacsFilePathUserInput

}  // end of namespace qemacs
