/*!
 * \file  CxxMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#include <QtCore/QDebug>

#include "TFEL/GUI/Utilities.hxx"
#include "TFEL/GUI/TextEditBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"
#include "TFEL/GUI/CxxSyntaxHighlighter.hxx"
#include "TFEL/GUI/CxxMajorMode.hxx"

namespace tfel {

  namespace gui {

    CxxMajorMode::CxxMajorMode(EditorWidget &w, Buffer &b, TextEditBase &t)
        : CMajorModeBase(w, b, t) {}  // end of CxxMajorMode::CxxMajorMode

    QString CxxMajorMode::getName() const {
      return "C++";
    }  // end of CxxMajorMode::CxxMajorMode

    QString CxxMajorMode::getLanguageName() const {
      return this->getName();
    }  // end of CxxMajorMode::getLanguageName

    QString CxxMajorMode::getDescription() const {
      return "major mode dedicated to the C++ language";
    }  // end of CxxMajorMode::CxxMajorMode

    QIcon CxxMajorMode::getIcon() const {
      return QIcon(":/tfel/editor/languages/cxx.png");
    }  // end of CxxMajorMode::getIcon

    bool CxxMajorMode::keyPressEvent(QKeyEvent *const e) {
      const int k = e->key();
      const Qt::KeyboardModifiers m = e->modifiers();
      if ((m == Qt::AltModifier) && (k == Qt::Key_M)) {
        this->runCompilation();
        return true;
      }
      return CMajorModeBase::keyPressEvent(e);
    }  // end of LicosMajorMode::keyPressEvent

    void CxxMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
      new CxxSyntaxHighlighter(d);
    }  // end of CxxMajorMode::setSyntaxHighlighter

    CxxMajorMode::~CxxMajorMode() = default;

    static StandardMajorModeProxy<CxxMajorMode> proxy(
        "C++",
        QVector<QRegExp>() << QRegExp(fileNameRegExp() + "\\.cxx$")
                           << QRegExp(fileNameRegExp() + "\\.hxx$")
                           << QRegExp(fileNameRegExp() + "\\.ixx$")
                           << QRegExp(fileNameRegExp() + "\\.cpp$")
                           << QRegExp(fileNameRegExp() + "\\.hpp$")
                           << QRegExp(fileNameRegExp() + "\\.ipp$")
                           << QRegExp(fileNameRegExp() + "\\.CC$")
                           << QRegExp(fileNameRegExp() + "\\.hh$")
                           << QRegExp(fileNameRegExp() + "\\.c$")
                           << QRegExp(fileNameRegExp() + "\\.h$"),
        ":/tfel/editor/languages/cxx.png");

  }  // end of namespace gui
}  // end of namespace tfel
