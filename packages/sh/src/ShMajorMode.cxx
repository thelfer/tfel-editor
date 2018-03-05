/*!
 * \file  ShMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   26/08/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include "QEmacs/Utilities.hxx"
#include "QEmacs/QEmacsWidget.hxx"
#include "QEmacs/QEmacsBuffer.hxx"
#include "QEmacs/QEmacsTextEditBase.hxx"
#include "QEmacs/ProcessOutputFrame.hxx"
#include "QEmacs/QEmacsShellProcessLineEdit.hxx"
#include "QEmacs/QEmacsMajorModeFactory.hxx"
#include "QEmacs/ShSyntaxHighlighter.hxx"
#include "QEmacs/ShMajorMode.hxx"

namespace qemacs {

  ShMajorMode::ShMajorMode(QEmacsWidget& w,
                           QEmacsBuffer& b,
                           QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w, b, t, &t) {
  }  // end of ShMajorMode::ShMajorMode

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
    new ShSyntaxHighlighter(d);
  }  // end of ShMajorMode::setSyntaxHighlighter

  ShMajorMode::~ShMajorMode() = default;

  static StandardQEmacsMajorModeProxy<ShMajorMode> proxy(
      "sh",
      QVector<QRegExp>() << QRegExp("^" + fileNameRegExp() + "\\.sh$"));

}  // end of namespace qemacs
