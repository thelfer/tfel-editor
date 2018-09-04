/*!
 * \file  Utilities.hxx
 * \brief
 * \author Helfer Thomas
 * \date   01/08/2012
 */

#ifndef LIB_TFEL_GUI_UTILITIES_HXX
#define LIB_TFEL_GUI_UTILITIES_HXX

#include "TFEL/GUI/Config.hxx"

//! forward declaration
class QWidget;
//! forward declaration
class QLabel;
//! forward declaration
class QHBoxLayout;
//! forward declaration
class QAbstractScrollArea;
//! forward declaration
class QMenu;
//! forward declaration
class QMenuBar;

#ifdef TFEL_GUI_HAVE_WEBENGINE
//! forward declaration
class QWebEngineView;
#endif /* TFEL_GUI_HAVE_WEBENGINE */

namespace tfel{

  namespace gui{

  /*!
   * \brief find a file in the given directory or one of its parents.
   * \param[in] d: directory
   * \param[in] f: file name
   * \return if found, the absolute path to the file, an empty string
   * otherwise
   */
  TFEL_GUI_VISIBILITY_EXPORT QString
  findFileInDirectoryOrParentDirectory(const QString&, const QString&);

  TFEL_GUI_VISIBILITY_EXPORT QString fileNameRegExp();

  void setFontSizeAndContentsMargins(QWidget* const);

  void setQLabelFontSizeAndMargins(QLabel* const);

  void setQAbstractScrollAreaInLayout(QHBoxLayout* const,
                                      QAbstractScrollArea* const);

#ifdef TFEL_GUI_HAVE_WEBENGINE
  void setQWebEngineViewInLayout(QHBoxLayout* const,
                                 QWebEngineView* const);
#endif /* TFEL_GUI_HAVE_WEBENGINE */

  QString commonPart(const QString&, const QString&);

  QString commonPart(const QStringList&);

  /*!
   * \brief Recursively free the memory associated with the actions
   * holded by the menu and call `QMenu::clear()`.
   *
   * See
   * https://forum.qt.io/topic/90856/does-qmenubar-clear-release-memory-of-the-action-created-with/12
   * for details.
   *
   * The code is based on:
   * https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
   * \param[in] m: menu bar
   */
  TFEL_GUI_VISIBILITY_EXPORT void clearMenu(QMenu* const);

  /*!
   * \brief Recursively free the memory associated with the actions
   * holded by the menu bar and call `QMenuBar::clear()`.
   *
   * See
   * https://forum.qt.io/topic/90856/does-qmenubar-clear-release-memory-of-the-action-created-with/12
   * for details.
   *
   * The code is based on:
   * https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
   * \param[in] m: menu bar
   */
  TFEL_GUI_VISIBILITY_EXPORT void clearMenuBar(QMenuBar* const);

}  // end of namespace gui
}// end of namespace tfel

#endif /* LIB_TFEL_GUI_UTILITIES_HXX */
