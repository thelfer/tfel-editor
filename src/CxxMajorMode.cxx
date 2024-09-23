/*!
 * \file  CxxMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>

#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"
#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel::gui {

  CxxMajorMode::CxxMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
      : CMajorModeBase(w, b, t) {}  // end of CxxMajorMode

  QString CxxMajorMode::getName() const {
    return "C++";
  }  // end of CxxMajorMode

  QString CxxMajorMode::getLanguageName() const {
    return this->getName();
  }  // end of getLanguageName

  QString CxxMajorMode::getDescription() const {
    return "major mode dedicated to the C++ language";
  }  // end of CxxMajorMode

  QIcon CxxMajorMode::getIcon() const {
    return QIcon(":/tfel/editor/languages/cxx.png");
  }  // end of getIcon

  bool CxxMajorMode::keyPressEvent(QKeyEvent *const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
      this->runCompilation();
      return true;
    }
    return CMajorModeBase::keyPressEvent(e);
  }  // end of keyPressEvent

  void CxxMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    new CxxSyntaxHighlighter(d);
  }  // end of setSyntaxHighlighter

  CxxMajorMode::~CxxMajorMode() = default;

  static StandardMajorModeProxy<CxxMajorMode> proxy(
      "C++",
      QVector<QRegularExpression>()
          << QRegularExpression(fileNameRegExp() + "\\.cxx$")
          << QRegularExpression(fileNameRegExp() + "\\.hxx$")
          << QRegularExpression(fileNameRegExp() + "\\.ixx$")
          << QRegularExpression(fileNameRegExp() + "\\.cpp$")
          << QRegularExpression(fileNameRegExp() + "\\.hpp$")
          << QRegularExpression(fileNameRegExp() + "\\.ipp$")
          << QRegularExpression(fileNameRegExp() + "\\.CC$")
          << QRegularExpression(fileNameRegExp() + "\\.hh$")
          << QRegularExpression(fileNameRegExp() + "\\.c$")
          << QRegularExpression(fileNameRegExp() + "\\.h$"),
      ":/tfel/editor/languages/cxx.png");

}  // end of namespace tfel::gui
