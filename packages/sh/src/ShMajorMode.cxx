/*!
 * \file  ShMajorMode.cxx
 * \brief
 * \author Helfer Thomas
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
#include "TFEL/GUI/ShSyntaxHighlighter.hxx"
#include "TFEL/GUI/ShMajorMode.hxx"

namespace tfel::gui {

  ShMajorMode::ShMajorMode(EditorWidget& w, Buffer& b, TextEditBase& t)
      : MajorModeBase(w, b, t, &t) {}  // end of ShMajorMode::ShMajorMode

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

  static StandardMajorModeProxy<ShMajorMode> proxy(
      "sh",
      QVector<QRegularExpression>()
          << QRegularExpression("^" + fileNameRegExp() + "\\.sh$"));

}  // end of namespace tfel::gui
