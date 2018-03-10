/*! 
 * \file  CxxMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   03/07/2012
 */

#include<QtCore/QDebug>

#include"QEmacs/Utilities.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"
#include"QEmacs/CxxSyntaxHighlighter.hxx"
#include"QEmacs/CxxMajorMode.hxx"

namespace qemacs {

  CxxMajorMode::CxxMajorMode(QEmacsWidget &w,
                             QEmacsBuffer &b,
                             QEmacsTextEditBase &t)
      : CMajorModeBase(w, b, t) {}  // end of CxxMajorMode::CxxMajorMode

  QString CxxMajorMode::getName() const {
    return "C++";
  } // end of CxxMajorMode::CxxMajorMode

  QString CxxMajorMode::getLanguageName() const {
    return this->getName();
  } // end of CxxMajorMode::getLanguageName

  QString CxxMajorMode::getDescription() const {
    return "major mode dedicated to the C++ language";
  } // end of CxxMajorMode::CxxMajorMode

  QIcon CxxMajorMode::getIcon() const{
    return QIcon(":/qemacs/languages/cxx.png");
  }  // end of CxxMajorMode::getIcon

  bool CxxMajorMode::keyPressEvent(QKeyEvent *const e) {
    const int k = e->key();
    const Qt::KeyboardModifiers m = e->modifiers();
    if((m==Qt::AltModifier)&&(k==Qt::Key_M)){
      this->runCompilation();
      return true;
    }
    return CMajorModeBase::keyPressEvent(e);
  } // end of LicosMajorMode::keyPressEvent

  void CxxMajorMode::setSyntaxHighlighter(QTextDocument *const d) {
    new CxxSyntaxHighlighter(d);
  } // end of CxxMajorMode::setSyntaxHighlighter

  CxxMajorMode::~CxxMajorMode() = default;

  static StandardQEmacsMajorModeProxy<CxxMajorMode> proxy(
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
      ":/qemacs/languages/cxx.png");

} // end of namespace qemacs
