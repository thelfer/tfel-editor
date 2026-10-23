/*!
 * \file  CommandLine.cxx
 * \brief
 * \author Thomas Helfer
 * \date   05/07/2012
 */

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtGui/QKeyEvent>
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
    if (this->completerHandledByQLineEdit) {
      // the previous completer is owned by the base class, which must forget
      // it as it shall not insert any completion any more
      QLineEdit::setCompleter(nullptr);
    } else {
      delete this->c_;
    }
    this->c_ = c;
    this->completerHandledByQLineEdit = b;
    // the completer is not installed on the line edit when it is not handled
    // by it: a line edit owning a completer inserts, and selects, the first
    // completion matching its text as soon as the user types, whereas the
    // completion is performed by complete() in the other case
    if (b) {
      QLineEdit::setCompleter(c);
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
        // complete() is called whatever the completer is as a line edit may
        // gather the completions by itself, without any completer (see
        // FilePathUserInput::FilePathLineEdit)
        this->complete();
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
      this->insertCompletion(c, b);
    }
    // the completions are displayed once the text has been updated: they are
    // hidden each time the text of the line edit changes
    if (this->completionList.size() > 1) {
      this->lineEdit.showCompletions(this->extractBaseForCompletion(c),
                                     this->completionList);
    }
  }  // end of CustomLineEdit::complete

  QString CommandLine::CustomLineEdit::findCompletion(bool& b) {
    QStringList cs;
    if (auto* const c = this->completer(); c != nullptr) {
      c->setCompletionPrefix(this->text());
      for (int i = 0; c->setCurrentRow(i); i++) {
        cs.append(c->currentCompletion());
      }
    }
    return this->handleCompletions(cs, cs, b);
  }  // end of findCompletion

  QString CommandLine::CustomLineEdit::handleCompletions(const QStringList& cs,
                                                         const QStringList& ns,
                                                         bool& b) {
    b = cs.size() == 1;
    // a single completion needs no display, and no name is kept for it as
    // it is inserted in the line edit
    this->completionList = (cs.size() > 1) ? ns : QStringList();
    return commonPart(cs);
  }  // end of handleCompletions

  void CommandLine::CustomLineEdit::insertCompletion(const QString& c,
                                                     const bool) {
    // the text of the line edit is completed with the common part of all the
    // completions, which is the unique completion when there is only one
    this->setText(c);
  }  // end of insertCompletion

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
    // this->setStyleSheet("background: rgba(255,255,255,100%)");
    // this->label->setStyleSheet("background: rgba(255,255,255,100%)");
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

  void CommandLine::showCompletions(const QString& b, const QStringList& cl) {
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
    // the directory in which the completion is performed is displayed as the
    // completions may hold file names only and as the text of the line edit is
    // not the same any more once the completion has been inserted
    auto h = QString("Possible completions");
    if (!b.isEmpty()) {
      h += " for '" + b.toHtmlEscaped() + "'";
    }
    // the completions are escaped as the table below is inserted as html and
    // a completion holding characters such as '<' would not be displayed
    // otherwise
    this->completions->insertHtml("<font color=\"blue\">" + h + " :</font>");
    const auto d = cl.size() / 2;
    const auto m = cl.size() % 2;
    QString table = "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
    for (int i = 0; i != d; ++i) {
      QString l;
      l = "<tr>";
      l += "<td width=\"45%\">" + cl[i * 2].toHtmlEscaped() + "</td>";
      l += "<td width=\"45%\">" + cl[i * 2 + 1].toHtmlEscaped() + "</td>";
      l += "</tr>";
      table += l;
    }
    QString ll;
    if (m == 1) {
      ll = "<tr>";
      ll += "<td width=\"45%\">" + cl[d * 2].toHtmlEscaped() +
            "</td>"
            "<td width=\"45%\"></td>";
      ll += "</tr>";
      table += ll;
    }
    table += "</table>";
    this->completions->insertHtml(table);
    // the task is displayed through Buffer so that the secondary task registry
    // records it as visible: that registry is what restores the tab widget once
    // the buffer is current again, see Buffer::refreshSecondaryTaskTabWidget
    this->buffer.showSecondaryTask(this->scompletions);
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
    // this->input->setStyleSheet("background: rgba(255,255,255,100%)");
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
    // emitted here, and not through QObject::destroyed, so that the receiver
    // still finds a live QWidget: at this point the QWidget part of the object
    // is intact, whereas QObject::destroyed only fires after ~QWidget has run.
    emit destroying(this);
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
    FilePathLineEdit(EditorWidget& q, CommandLine& p) : CustomLineEdit(q, p) {}

   protected:
    void complete() override {
      // '~' at the beginning of the text stands for the home directory
      auto t = this->text();
      if (t.startsWith("~/")) {
        auto homePath = QDir::home().absolutePath();
        if (!homePath.endsWith(QDir::separator())) {
          homePath += QDir::separator();
        }
        t.replace(0, 2, homePath);
        QLineEdit::setText(t);
      }
      CommandLine::CustomLineEdit::complete();
    }  // end of complete

    QString findCompletion(bool& b) override {
      auto ns = QStringList();
      return this->handleCompletions(this->completingPaths(ns), ns, b);
    }  // end of findCompletion

    void insertCompletion(const QString& c, const bool b) override {
      // a separator is appended to a completion which is an unique directory,
      // as it can not be completed any further and the separator makes the
      // next completion start within this directory
      if ((b) && (QFileInfo(c).isDir()) && (!c.endsWith(QDir::separator()))) {
        QLineEdit::setText(c + QDir::separator());
        return;
      }
      QLineEdit::setText(c);
    }  // end of insertCompletion

    QString extractBaseForCompletion(const QString& c) override {
      QFileInfo f(c);
      auto d = f.dir().absolutePath();
      if (!d.endsWith(QDir::separator())) {
        d += QDir::separator();
      }
      return d;
    }

   private:
    /*!
     * \return the paths of the files and directories which can complete the
     * text of the line edit
     * \param[out] ns : name under which each path is displayed
     *
     * \note A `QCompleter` backed by a `QFileSystemModel` can not be used
     * here: such a completer only looks at the rows held by the invisible root
     * of its model, that is the root directory itself, and there is no way to
     * give it another root index since Qt 6. It systematically returns either
     * no completion, or the root directory for an empty text.
     */
    QStringList completingPaths(QStringList& ns) const {
      auto cs = QStringList();
      ns.clear();
      const auto t = this->text();
      // the text is split between the directory in which the completion is
      // performed and the beginning of the name to complete
      const auto i = t.lastIndexOf(QDir::separator());
      auto d = QString(".");
      if (i == 0) {
        d = QString("/");
      } else if (i > 0) {
        d = t.left(i);
      }
      const auto n = t.mid(i + 1);
      const auto dir = QDir(d);
      if (!dir.exists()) {
        return cs;
      }
      // the text of the line edit is kept as a prefix, so that a relative
      // path is completed as a relative path and an absolute one as an
      // absolute path
      const auto p = (i == -1) ? QString() : t.left(i + 1);
      const auto f = QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot;
      for (const auto& e : dir.entryInfoList(f, QDir::Name | QDir::DirsFirst)) {
        const auto& n2 = e.fileName();
        if (!n2.startsWith(n)) {
          continue;
        }
        cs.append(p + n2);
        // a separator is appended to the directories as they are displayed in
        // the completions buffer, where they shall be distinguished from the
        // files, and as it is what the line edit holds once such a completion
        // is inserted
        ns.append(e.isDir() ? (n2 + QDir::separator()) : n2);
      }
      // the names of the completions are the ones displayed in the
      // completions buffer: they are truncated as the buffer would be flooded
      // with the whole content of a directory when nothing is typed. The
      // common part is computed from all the completions anyway
      constexpr auto max_nb_of_displayed_completions = 100;
      if (ns.size() > max_nb_of_displayed_completions) {
        ns = ns.mid(0, max_nb_of_displayed_completions);
        ns << "...";
      }
      return cs;
    }  // end of completingPaths
  };

  FilePathUserInput::FilePathUserInput(const QString& i, EditorWidget& p)
      : CommandLine(i, p) {
    this->setLineEdit(new FilePathLineEdit(p, *this));
  }  // end of FilePathUserInput

}  // end of namespace tfel::gui
