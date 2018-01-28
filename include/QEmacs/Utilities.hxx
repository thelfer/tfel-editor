/*! 
 * \file  Utilities.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
 */

#ifndef LIB_QEMACS_UTILITIES_HXX
#define LIB_QEMACS_UTILITIES_HXX 

#include"QEmacs/Config.hxx"

//! forward declaration
class QWidget;
//! forward declaration
class QLabel;
//! forward declaration
class QHBoxLayout;
//! forward declaration
class QAbstractScrollArea;
//! forward declaration
class QWebView;

namespace qemacs
{

  void
  setFontSizeAndContentsMargins(QWidget *const);

  void
  setQLabelFontSizeAndMargins(QLabel*const);
    
  void
  setQAbstractScrollAreaInLayout(QHBoxLayout * const,
				 QAbstractScrollArea *const);

  void
  setQWebViewInLayout(QHBoxLayout * const,
		      QWebView *const);

  QString
  commonPart(const QString&,
	     const QString&);

  QString
  commonPart(const QStringList&);

  QEMACS_VISIBILITY_EXPORT QString
  fileNameRegExp();

} // end of namespace qemacs

#endif /* LIB_QEMACS_UTILITIES_HXX */

