/*!
 * \file  BashMajorMode.cxx
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
#include "TFEL/GUI/BashSyntaxHighlighter.hxx"
#include "TFEL/GUI/BashMajorMode.hxx"

namespace tfel{

  namespace gui{

  BashMajorMode::BashMajorMode(EditorWidget& w,
                               Buffer& b,
                               TextEditBase& t)
      : ShMajorMode(w, b, t) {}  // end of BashMajorMode::BashMajorMode

  QString BashMajorMode::getName() const {
    return "bash";
  }  // end of BashMajorMode::BashMajorMode

  QString BashMajorMode::getDescription() const {
    return "major mode dedicated to bash files";
  }  // end of BashMajorMode::BashMajorMode

  void BashMajorMode::setSyntaxHighlighter(QTextDocument* const d) {
    new BashSyntaxHighlighter(d);
  }  // end of BashMajorMode::setSyntaxHighlighter

  BashMajorMode::~BashMajorMode() = default;

  static StandardMajorModeProxy<BashMajorMode> proxy(
      "bash",
      QVector<QRegExp>() << QRegExp("^" + fileNameRegExp() + "\\.bash$"));

}  // end of namespace gui
}// end of namespace tfel
