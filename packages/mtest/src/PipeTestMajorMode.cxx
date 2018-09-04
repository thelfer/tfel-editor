/*! 
 * \file  PipeTestMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \date   07/08/2012
 */

#include<QtCore/QDebug>
#include<QtGui/QTextCursor>
                
#include"TFEL/GUI/Buffer.hxx"
#include"TFEL/GUI/ProcessOutputFrame.hxx"
#include"TFEL/GUI/PipeTestSyntaxHighlighter.hxx"
#include"TFEL/GUI/MajorModeFactory.hxx"

#include"TFEL/GUI/PipeTestMajorMode.hxx"

namespace tfel{

  namespace gui
{
  
  PipeTestMajorMode::PipeTestMajorMode(EditorWidget& w,
				       Buffer& b,
				       TextEditBase& t)
    : MTestMajorMode(w,b,t)
  {
    QStringList keys;
    for(const auto& k : this->getKeyWordsList()){
      keys << QString::fromStdString(k);
    }
    this->c = new QCompleter(keys,&t);
    this->c->setWidget(&t);
    this->c->setCaseSensitivity(Qt::CaseInsensitive);
    this->c->setCompletionMode(QCompleter::PopupCompletion);
    QObject::connect(this->c,static_cast<void (QCompleter:: *)(const QString&)>(&QCompleter::activated),
		     &t,&TextEditBase::insertCompletion);
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

  const std::vector<std::string>&
  PipeTestMajorMode::getKeyWordsList() const{
    return PipeTestSyntaxHighlighter::getPipeTestKeys();
  }

  QCompleter* PipeTestMajorMode::getCompleter()
  {
    return this->c;
  } // end of getCompleter
  
  PipeTestMajorMode::~PipeTestMajorMode() = default;
  
  static StandardMajorModeProxy<PipeTestMajorMode> proxy("PipeTest",
							       QVector<QRegExp>() << QRegExp("^\\w+\\.ptest"));

} // end of namespace gui
}// end of namespace tfel

