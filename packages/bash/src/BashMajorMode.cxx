/*!
 * \file  BashMajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/Buffer.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/ProcessOutputFrame.hxx"
#include "TFEL/GUI/ShellProcessLineEdit.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/BashSyntaxHighlighter.hxx"
#include "TFEL/GUI/BashMajorMode.hxx"

namespace tfel::gui {

  BashMajorMode::BashMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
      : ShMajorMode(w, b, t) {
    // issue #77: the bash mode used to inherit the (empty) sh constructor
    // and so completed nothing. On top of the POSIX words, add the bash
    // builtins and reserved words which have no POSIX counterpart; the
    // list is static because `compgen` from the constructor would block
    // the GUI thread.
    auto keys = ShMajorMode::getKeyWordsList();
    keys << // bash reserved words
        "coproc"
        << "select" << "time"
        << // bash builtins without a POSIX counterpart
        "bind" << "builtin" << "caller" << "command"
        << "compgen" << "complete" << "compopt" << "declare"
        << "dirs" << "disown" << "enable" << "help" << "history"
        << "let" << "mapfile" << "popd" << "pushd" << "readarray"
        << "shopt" << "suspend" << "type" << "typeset" << "ulimit"
        << "hash";
    keys.removeDuplicates();
    this->setCompleterList(keys);
  }  // end of BashMajorMode::BashMajorMode

  QString BashMajorMode::getName() const {
    return "bash";
  }  // end of BashMajorMode::BashMajorMode

  QString BashMajorMode::getDescription() const {
    return "major mode dedicated to bash files";
  }  // end of BashMajorMode::BashMajorMode

  void BashMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    this->setHighlighter(new BashSyntaxHighlighter(d));
  }  // end of BashMajorMode::setSyntaxHighlighter

  BashMajorMode::~BashMajorMode() = default;

  static StandardMajorModeProxy<BashMajorMode> proxy(
      "bash",
      QVector<QRegularExpression>()
          << QRegularExpression("^" + fileNameRegExp() + "\\.bash$"));

}  // end of namespace tfel::gui
