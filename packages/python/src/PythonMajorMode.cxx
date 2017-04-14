/*! 
 * \file  PythonMajorMode.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 26 sept. 2012
 */

#include<QtCore/QDir>
#include<QtCore/QDebug>

#include<QtGui/QSyntaxHighlighter>

#include"QEmacs/QEmacsWidget.hxx"
#include"QEmacs/QEmacsBuffer.hxx"
#include"QEmacs/QEmacsTextEditBase.hxx"

#include"QEmacs/QEmacsMajorMode.hxx"
#include"QEmacs/QEmacsMajorModeBase.hxx"
#include"QEmacs/CSyntaxHighlighterBase.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{

  struct PythonSyntaxHighlighter
    : public CSyntaxHighlighterBase
  {
    PythonSyntaxHighlighter(QTextDocument *p)
      : CSyntaxHighlighterBase(p)
    {
      QStringList keys;
      keys << "and"     << "del"    << "from"
	   << "not"     << "while"  << "as"  
	   << "elif"    << "global" << "or" 
	   << "with"    << "assert" << "else"
	   << "if"      << "pass"   << "yield"
	   << "break"   << "except" << "import"
	   << "print"   << "class"  << "exec"
	   << "in"      << "raise"  << "continue"
	   << "finally" << "is"     << "return"
	   << "def"     << "for"    << "lambda"
	   << "try";
      foreach(const QString& key,keys){
	HighlightingRule rule;
	rule.key     = key;
	rule.format  = this->keyFormat;
	highlightingRules.push_front(rule);
      }
    } // end of PythonSyntaxHighlighter

  }; // end of struct PythonSyntaxHighlighter

  /*!
   * A major mode to display the results of the grepOutput unix command
   */
  struct PythonMajorMode
    : public QEmacsMajorModeBase
  {
    
    PythonMajorMode(QEmacsWidget& w,
			QEmacsBuffer& b,
			QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w,b,t,&t)
    {}

    QString
    getName(void) const
    {
      return "python";
    } // end of LicosMajorMode

    QString
    getDescription(void) const
    {
      return "major mode dedicated to the python programming language";
    } // end of getDescription

    void
    setSyntaxHighlighter(QTextDocument* d)
    {
      new PythonSyntaxHighlighter(d);
    } // end of setSyntaxHighlighter

    bool
    mousePressEvent(QMouseEvent *)
    {
      return false;
    }

    bool
    keyPressEvent(QKeyEvent *)
    {
      return false;
    }

    void format(void)
    {}

    void comment()
    {}

    ~PythonMajorMode()
    {}

  }; // end of PythonMajorMode
  
  static StandardQEmacsMajorModeProxy<PythonMajorMode> proxy("python",QVector<QRegExp>() 
							     << QRegExp("^[\\w-]+\\.py$"));

} // end of namespace qemacs

