/*! 
 * \file  CxxMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 03 juil. 2012
 */

#include<QtCore/QDebug>

#include"QEmacs/CxxMajorMode.hxx"
#include"QEmacs/CxxSyntaxHighlighter.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{
  
  CxxMajorMode::CxxMajorMode(QEmacsWidget& w,
			     QEmacsBuffer& b,
			     QEmacsTextEditBase& t)
    : CMajorModeBase(w,b,t)
  {} // end of CxxMajorMode::CxxMajorMode

  QString
  CxxMajorMode::getName() const
  {
    return "C++";
  } // end of CxxMajorMode::CxxMajorMode

  QString
  CxxMajorMode::getLanguageName() const
  {
    return this->getName();
  } // end of CxxMajorMode::getLanguageName

  QString
  CxxMajorMode::getDescription() const
  {
    return "major mode dedicated to the C++ language";
  } // end of CxxMajorMode::CxxMajorMode

  bool
  CxxMajorMode::keyPressEvent(QKeyEvent * const e)
  {
    const int k                   = e->key();
    const Qt::KeyboardModifiers m = e->modifiers(); 
    if((m==Qt::AltModifier)&&(k==Qt::Key_M)){
      this->runCompilation();
      return true;
    }
    return false;
  } // end of LicosMajorMode::keyPressEvent


  void
  CxxMajorMode::setSyntaxHighlighter(QTextDocument *const d)
  {
    new CxxSyntaxHighlighter(d);
  } // end of CxxMajorMode::setSyntaxHighlighter

  CxxMajorMode::~CxxMajorMode() = default;

  static StandardQEmacsMajorModeProxy<CxxMajorMode> proxy("C++",QVector<QRegExp>() 
							  << QRegExp("^[\\w-]+\\.cxx$") 
							  << QRegExp("^[\\w-]+\\.hxx$")
							  << QRegExp("^[\\w-]+\\.ixx$")
							  << QRegExp("^[\\w-]+\\.cpp$")
							  << QRegExp("^[\\w-]+\\.hpp$")
							  << QRegExp("^[\\w-]+\\.ipp$")
							  << QRegExp("^[\\w-]+\\.CC$")
							  << QRegExp("^[\\w-]+\\.hh$"));

} // end of namespace qemacs
