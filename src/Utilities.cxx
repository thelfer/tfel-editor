/*! 
 * \file  Utilities.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#include<QtCore/QDebug>

#ifdef QEMACS_QT4
#include<QtGui/QLabel>
#include<QtGui/QWidget>
#include<QtGui/QScrollBar>
#include<QtGui/QHBoxLayout>
#include<QtGui/QAbstractScrollArea>
#include<QtWebKit/QWebView>
#include<QtWebKit/QWebFrame>
#endif /* QEMACS_QT4 */
#ifdef QEMACS_QT5
#include<QtWidgets/QLabel>
#include<QtWidgets/QWidget>
#include<QtWidgets/QScrollBar>
#include<QtWidgets/QHBoxLayout>
#include<QtWidgets/QAbstractScrollArea>
#include<QtWebKitWidgets/QWebView>
#include<QtWebKitWidgets/QWebFrame>
#endif /* QEMACS_QT5 */


#include"QEmacs/Utilities.hxx"

namespace qemacs
{

  void
  setFontSizeAndContentsMargins(QWidget * const w)
  {
    QFont f = w->font();
    f.setPointSize(8);
    w->setFont(f);
    w->setContentsMargins(0,0,0,0);
  } // end of setQLabelFontSize

  void
  setQLabelFontSizeAndMargins(QLabel * const l)
  {
    setFontSizeAndContentsMargins(l);
    l->setMargin(0);
  } // end of setQLabelFontSize
    
  void
  setQAbstractScrollAreaInLayout(QHBoxLayout * const hl,
				 QAbstractScrollArea *const a)
  {
    a->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar* s = a->verticalScrollBar();
    hl->addWidget(s);
    hl->addWidget(a);
    hl->setMargin(0);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(0);
  }

  void setQWebViewInLayout(QHBoxLayout * const hl,
			   QWebView *const a)
  {
    a->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical,
					       Qt::ScrollBarAlwaysOff);
    // QScrollBar* s = a->page()->mainFrame()->verticalScrollBar();
    //    hl->addWidget(s);
    hl->addWidget(a);
    hl->setMargin(0);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(0);
  }

  QString commonPart(const QString& s1,
		     const QString& s2)
  {
    QString r;
    int l = qMin(s1.size(),s2.size());
    if(l==0){
      return QString();
    }
    r.reserve(l);
    for(int i=0;i!=l;++i){
      if(s1[i]!=s2[i]){
	break;
      }
      r[i]=s1[i];
    }
    return r;
  } // end of commonPart

  QString commonPart(const QStringList& l)
  {
    if(l.size()==0){
      return QString();
    }
    if(l.size()==1){
      return l[0];
    }
    QString r = commonPart(l[0],l[1]);
    for(int i=2;i!=l.size();++i){
      const QString& s = l[i];
      if(!s.startsWith(r)){
	r = commonPart(r,s);
      }
      if(r.isEmpty()){
	return "";
      }
    }
    return r;
  }

  QString fileNameRegExp(void)
  {
    return "[\\w-0-9_\\./]+";
  } // end of fileNameRegExp

} // end of namespace qemacs
