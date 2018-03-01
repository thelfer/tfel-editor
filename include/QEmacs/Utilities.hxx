/*!
 * \file  Utilities.hxx
 * \brief
 * \author Helfer Thomas
 * \date   01/08/2012
 */

#ifndef LIB_QEMACS_UTILITIES_HXX
#define LIB_QEMACS_UTILITIES_HXX

#include "QEmacs/Config.hxx"

//! forward declaration
class QWidget;
//! forward declaration
class QLabel;
//! forward declaration
class QHBoxLayout;
//! forward declaration
class QAbstractScrollArea;

#ifdef QEMACS_HAVE_WEBENGINE
//! forward declaration
class QWebEngineView;
#endif /* QEMACS_HAVE_WEBENGINE */

namespace qemacs {

  /*!
   * \brief find a file in the given directory or one of its parents.
   * \param[in] d: directory
   * \param[in] f: file name
   * \return if found, the absolute path to the file, an empty string
   * otherwise
   */
  QEMACS_VISIBILITY_EXPORT QString
  findFileInDirectoryOrParentDirectory(const QString&, const QString&);

  QEMACS_VISIBILITY_EXPORT QString fileNameRegExp();

  void setFontSizeAndContentsMargins(QWidget* const);

  void setQLabelFontSizeAndMargins(QLabel* const);

  void setQAbstractScrollAreaInLayout(QHBoxLayout* const,
                                      QAbstractScrollArea* const);

#ifdef QEMACS_HAVE_WEBENGINE
  void setQWebEngineViewInLayout(QHBoxLayout* const,
                                 QWebEngineView* const);
#endif /* QEMACS_HAVE_WEBENGINE */
  
  QString commonPart(const QString&, const QString&);

  QString commonPart(const QStringList&);

}  // end of namespace qemacs

#endif /* LIB_QEMACS_UTILITIES_HXX */
