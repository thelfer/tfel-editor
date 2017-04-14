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
    QObject::connect(this->c,SIGNAL(activated(QString)),
		     &t,SLOT(insertCompletion(QString)));
  } // end of PipeTestMajorMode::PipeTestMajorMode
  
  QString
  PipeTestMajorMode::getName(void) const
  {
    return "PipeTest";
  } // end of PipeTestMajorMode::getName
    
  QString
  PipeTestMajorMode::getDescription(void) const
  {
    return "major mode dedicated to the mtest extension to pipe";
  } // end of CppMajorMode

  QString
  PipeTestMajorMode::getScheme(void) const
  {
    return "ptest";
  }
  
  void
  PipeTestMajorMode::setSyntaxHighlighter(QTextDocument *const d)
  {
    new PipeTestSyntaxHighlighter(d);
  } // end of PipeTestMajorMode::setSyntaxHighlighter

  QStringList
  PipeTestMajorMode::getKeyWordsList() const{
    return PipeTestSyntaxHighlighter::getPipeTestKeys();
  }

  QCompleter*
  PipeTestMajorMode::getCompleter(void)
  {
    return this->c;
  } // end of getCompleter
  
  PipeTestMajorMode::~PipeTestMajorMode()
  {} // end of PipeTestMajorMode::~PipeTestMajorMode()
  
  static StandardQEmacsMajorModeProxy<PipeTestMajorMode> proxy("PipeTest",
							       QVector<QRegExp>() << QRegExp("^\\w+\\.ptest"));

} // end of namespace qemacs

