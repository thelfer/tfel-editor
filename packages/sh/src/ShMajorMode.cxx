/*!
 * \file  ShMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtWidgets/QCompleter>
#include <QtCore/QStringListModel>
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/ShSyntaxHighlighter.hxx"
#include "TFEL/GUI/ShMajorMode.hxx"

namespace tfel::gui {

  const QStringList& ShMajorMode::getKeyWordsList() {
    static const QStringList keys = {
        // control words
        "if", "then", "else", "elif", "fi", "for", "while", "until", "do",
        "done", "case", "esac", "function",
        // POSIX builtins
        "alias", "bg", "break", "cd", "command", "continue", "eval", "exec",
        "exit", "export", "false", "fg", "getopts", "hash", "jobs", "kill",
        "local", "pwd", "read", "readonly", "return", "set", "shift", "test",
        "times", "trap", "true", "umask", "unalias", "unset", "wait", "echo",
        "printf", "source"};
    return keys;
  }  // end of ShMajorMode::getKeyWordsList

  ShMajorMode::ShMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
      : MajorModeBase(w, b, t, &t) {
    // issue #77: the mode had no completer at all, so *.sh buffers offered
    // no completion. The words come from a static list, not from the
    // shell: `compgen` from the constructor would block the GUI thread
    // (cf. the wait the bash highlighter pays for it).
    this->c = new QCompleter(ShMajorMode::getKeyWordsList(), this);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,
                     static_cast<void (QCompleter::*)(const QString&)>(
                         &QCompleter::activated),
                     &t, &TextEditBase::insertCompletion);
  }  // end of ShMajorMode::ShMajorMode

  QCompleter* ShMajorMode::getCompleter() {
    return this->c;
  }  // end of ShMajorMode::getCompleter

  void ShMajorMode::setCompleterList(const QStringList& l) {
    // the default model of QCompleter is a QStringListModel: it is reused,
    // not replaced, so no model is ever orphaned from the completer
    if (auto* m = qobject_cast<QStringListModel*>(this->c->model())) {
      m->setStringList(l);
    }
  }  // end of ShMajorMode::setCompleterList

  QString ShMajorMode::getName() const {
    return "sh";
  }  // end of ShMajorMode::ShMajorMode

  QString ShMajorMode::getDescription() const {
    return "major mode dedicated to sh files";
  }  // end of ShMajorMode::ShMajorMode

  void ShMajorMode::format() {}  // end of ShMajorMode::format

  QString ShMajorMode::getCommentSyntax() {
    return "#";
  }  // end of ShMajorMode::getCommentSyntax

  void ShMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    this->setHighlighter(new ShSyntaxHighlighter(d));
  }  // end of ShMajorMode::setSyntaxHighlighter

  ShMajorMode::~ShMajorMode() = default;

  static StandardMajorModeProxy<ShMajorMode> proxy("sh",
                                                   QVector<QRegularExpression>()
                                                       << suffixRegExp("sh"));

}  // end of namespace tfel::gui
