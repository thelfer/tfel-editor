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

  static void releaseHighlighter(QSyntaxHighlighter *const h) {
    if (h == nullptr) {
      return;
    }
    // detaching from the document first, and only then asking for the
    // deletion: a QSyntaxHighlighter highlights the document it is attached
    // to until it is detached, and that may happen before the deferred
    // deletion is honoured
    h->setDocument(nullptr);
    // deleteLater(), not delete: setSyntaxHighlighter() is reached from a
    // QTimer on the periodic-refresh paths, so the highlighter being released
    // may be the one currently running. This is the discipline the MFront mode
    // already used.
    h->deleteLater();
  }  // end of releaseHighlighter

  void MajorMode::setHighlighter(QSyntaxHighlighter *const h) {
    releaseHighlighter(this->highlighter.data());
    this->highlighter = h;
  }  // end of MajorMode::setHighlighter

  MajorMode::~MajorMode() {
    // releasing the highlighter here is what keeps a stale one from
    // highlighting the document after this mode is gone. TextEditBase
    // deleteLater()s the mode it replaces, and a QSyntaxHighlighter is a child
    // of the document, which outlives the mode: a highlighter holding a
    // reference to its mode, such as those of the LaTeX and Markdown modes,
    // would then be called on a destroyed mode by the next rehighlight.
    releaseHighlighter(this->highlighter.data());
    this->highlighter = nullptr;
  }  // end of MajorMode::~MajorMode

}  // end of namespace tfel::gui
