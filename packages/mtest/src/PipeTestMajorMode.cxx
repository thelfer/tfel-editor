/*! 
 * \file  PipeTestMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 août 2012
 */

#include<QtCore/QDebug>
#include<QtGui/QTextCursor>
                
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/ProcessOutputFrame.hxx"
#include"QEmacs/PipeTestSyntaxHighlighter.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

#include"QEmacs/PipeTestMajorMode.hxx"

namespace qemacs
{
  
  PipeTestMajorMode::PipeTestMajorMode(QEmacsWidget& w,
				       QEmacsBuffer& b,
				       QEmacsTextEditBase& t)
    : MTestMajorMode(w,b,t)
  {
    this->c = new QCompleter(this->getKeyWordsList(),&t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,static_cast<void (QCompleter:: *)(const QString&)>(&QCompleter::activated),
		     &t,&QEmacsTextEditBase::insertCompletion);
  } // end of PipeTestMajorMode::PipeTestMajorMode
  
  QString PipeTestMajorMode::getName() const{
    return "PipeTest";
  } // end of PipeTestMajorMode::getName
    
  QString PipeTestMajorMode::getDescription() const{
    return "major mode dedicated to the mtest extension to pipe";
  } // end of CppMajorMode

  QString PipeTestMajorMode::getScheme() const{
    return "ptest";
  }
  
  void PipeTestMajorMode::setSyntaxHighlighter(QTextDocument *const d)
  {
    new PipeTestSyntaxHighlighter(d);
  } // end of PipeTestMajorMode::setSyntaxHighlighter

  QStringList PipeTestMajorMode::getKeyWordsList() const{
    return PipeTestSyntaxHighlighter::getPipeTestKeys();
  }

  QCompleter* PipeTestMajorMode::getCompleter()
  {
    return this->c;
  } // end of getCompleter
  
  PipeTestMajorMode::~PipeTestMajorMode() = default;
  
  static StandardQEmacsMajorModeProxy<PipeTestMajorMode> proxy("PipeTest",
							       QVector<QRegExp>() << QRegExp("^\\w+\\.ptest"));

} // end of namespace qemacs

