/*! 
 * \file  TPlotMajorMode.cxx
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
#include"QEmacs/GnuplotSyntaxHighlighter.hxx"
#include"QEmacs/QEmacsMajorModeFactory.hxx"

namespace qemacs
{

  struct TPlotSyntaxHighlighter
    : public GnuplotSyntaxHighlighter
  {
    TPlotSyntaxHighlighter(QTextDocument *p)
      : GnuplotSyntaxHighlighter(p)
    {
      QStringList keys;
      keys << "set"     << "plot"   << "splot";
      foreach(const QString& key,keys){
	HighlightingRule rule;
	rule.key     = key;
	rule.format  = this->keyFormat;
	highlightingRules.push_front(rule);
      }
      this->cCharAsString = true;
    } // end of TPlotSyntaxHighlighter

  }; // end of struct TPlotSyntaxHighlighter

  /*!
   * A major mode to handle the tplot utility
   */
  struct TPlotMajorMode
    : public QEmacsMajorModeBase
  {
    
    TPlotMajorMode(QEmacsWidget& w,
		   QEmacsBuffer& b,
		   QEmacsTextEditBase& t)
      : QEmacsMajorModeBase(w,b,t,&t)
    {}

    QString getName(void) const
    {
      return "tplot";
    } // end of LicosMajorMode

    QString getDescription(void) const
    {
      return "major mode dedicated to tplot";
    } // end of getDescription

    void setSyntaxHighlighter(QTextDocument* d)
    {
      new TPlotSyntaxHighlighter(d);
    } // end of setSyntaxHighlighter

    bool mousePressEvent(QMouseEvent *)
    {
      return false;
    }

    bool keyPressEvent(QKeyEvent *)
    {
      return false;
    }

    void format(void)
    {}

    void comment()
    {}

    ~TPlotMajorMode()
    {}

  }; // end of TPlotMajorMode
  
  static StandardQEmacsMajorModeProxy<TPlotMajorMode> proxy("tplot",QVector<QRegExp>() 
							    << QRegExp("^[\\w-\\.0-9_]+\\.tplot"));

} // end of namespace qemacs

