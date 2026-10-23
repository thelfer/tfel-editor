/*!
 * \file  MajorMode.cxx
 * \brief
 * \author Thomas Helfer
 * \date   30/06/2012
 */

#include <QtWidgets/QWidget>
#include "TFEL/GUI/MajorMode.hxx"

namespace tfel::gui {

  MajorMode::MajorMode(QWidget *const p) : QObject(p) {}  // end of MajorMode

  void MajorMode::setHighlighter(QSyntaxHighlighter *const h) {
    if (this->highlighter != nullptr) {
      // deleteLater(), not delete: setSyntaxHighlighter() is reached from a
      // QTimer on the periodic-refresh paths, so the highlighter being
      // replaced may be the one currently running. This is the same
      // discipline MFrontMajorMode already uses.
      this->highlighter->deleteLater();
    }
    this->highlighter = h;
  }  // end of MajorMode::setHighlighter

  MajorMode::~MajorMode() = default;

}  // end of namespace tfel::gui
