/*!
 * \file  BashMajorMode.cxx
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
#include "QEmacs/BashSyntaxHighlighter.hxx"
#include "QEmacs/BashMajorMode.hxx"

namespace qemacs {

  BashMajorMode::BashMajorMode(QEmacsWidget& w,
                               QEmacsBuffer& b,
                               QEmacsTextEditBase& t)
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

  static StandardQEmacsMajorModeProxy<BashMajorMode> proxy(
      "bash",
      QVector<QRegExp>() << QRegExp("^" + fileNameRegExp() + "\\.bash$"));

}  // end of namespace qemacs
