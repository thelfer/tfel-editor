/*!
 * \file  Utilities.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 01 août 2012
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
//! forward declaration
class QWebEngineView;

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

  void setQWebEngineViewInLayout(QHBoxLayout* const,
                                 QWebEngineView* const);

  QString commonPart(const QString&, const QString&);

  QString commonPart(const QStringList&);

}  // end of namespace qemacs

#endif /* LIB_QEMACS_UTILITIES_HXX */
