/*! 
 * \file  MakefileMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QSettings>
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/QEmacsShellProcessLineEdit.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/MakefileSyntaxHighlighter.hxx"
#include "QEmacs/MakefileMajorMode.hxx"

namespace qemacs {

  MakefileMajorMode::MakefileMajorMode(QEmacsWidget& w,
                                       QEmacsBuffer& b,
                                       QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w, b, t, &t) {
  } // end of MakefileMajorMode::MakefileMajorMode

  QString MakefileMajorMode::getName() const {
    return "Makefile";
  } // end of MakefileMajorMode::MakefileMajorMode

  QString MakefileMajorMode::getDescription() const {
    return "major mode dedicated to Makefile";
  } // end of MakefileMajorMode::MakefileMajorMode

  bool MakefileMajorMode::keyPressEvent(QKeyEvent* const e) {
    const auto k = e->key();
    const auto m = e->modifiers();
    if((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      this->runCompilation();
      return true;
    }
    return false;
  } // end of MakefileMajorMode::keyPressEvent

  QString MakefileMajorMode::getCommentSyntax() {
    return "#";
  } // end of MakefileMajorMode::getCommentSyntax

  void MakefileMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    new MakefileSyntaxHighlighter(d);
  } // end of MakefileMajorMode::setSyntaxHighlighter

  void MakefileMajorMode::format() {}

  void MakefileMajorMode::runCompilation() {
    QSettings s;
    const auto ch = s.value("make/compilation/history").toStringList();
    const auto d = [&ch]() -> QString {
      if (ch.isEmpty()) {
        return "make -k";
      }
      return ch.back();
    }();
    auto* l = new QEmacsShellProcessLineEdit(
        "compilation command :", d, "compilation-output", this->qemacs);
    l->setInputHistorySettingAddress("make/compilation/history");
    this->qemacs.setUserInput(l);
  } // end of MakefileMajorMode::runCompilation
  
  MakefileMajorMode::~MakefileMajorMode() = default;

  static StandardQEmacsMajorModeProxy<MakefileMajorMode> proxy(
      "Makefile", QVector<QRegExp>() << QRegExp("^Makefile$"));

} // end of namespace qemacs
