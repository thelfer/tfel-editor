/*!
 * \file  Utilities.hxx
 * \brief miscellaneous functions used all over the editor.
 * \author Thomas Helfer
 * \date   01/08/2012
 */

#ifndef LIB_TFEL_GUI_UTILITIES_HXX
#define LIB_TFEL_GUI_UTILITIES_HXX

#include <QtCore/QRegularExpression>
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
//! forward declaration
class QTextCursor;

#ifdef TFEL_GUI_HAVE_WEBENGINE
//! forward declaration
class QWebEngineView;
#endif /* TFEL_GUI_HAVE_WEBENGINE */

namespace tfel::gui {

  /*!
   * \brief find a file in the given directory or one of its parents.
   * \param[in] d: directory
   * \param[in] f: file name
   * \return if found, the absolute path to the file, an empty string
   * otherwise
   *
   * \note the returned path is the directory which contains the file, the
   * file name is not part of it
   *
   * \note when the file is not found in `d`, each parent directory is
   * tried and the existence of the file is checked there; an empty string
   * is returned when no ancestor directory contains it
   *
   * \note an empty string is also returned when `d` does not exist
   */
  TFEL_GUI_VISIBILITY_EXPORT QString
  findFileInDirectoryOrParentDirectory(const QString&, const QString&);

  /*!
   * \return the regular expression matching a file name, that is, a sequence
   * of letters, digits, dots, underscores, pluses and minuses
   *
   * \note the language modes use it to build the regular expressions which
   * associate a file name with a major mode
   */
  TFEL_GUI_VISIBILITY_EXPORT QString fileNameRegExp();

  /*!
   * \return the regular expression matching a file whose name is made of
   * allowed file-name characters followed by the dot and the given extension
   *
   * \note this is the single convention used to associate file names with
   * major modes: the match is anchored at both ends and the dot is escaped,
   * so backup files (`notes.tex~`, `a.mfront.bak`) and near misses
   * (`a.gpl`, `a.md5`) no longer fall into the wrong mode
   */
  TFEL_GUI_VISIBILITY_EXPORT QRegularExpression
  suffixRegExp(const QString&);

  /*!
   * \brief return the word the given cursor is on, completed by the `@` sign
   * which introduces it, if any
   * \param[in] tc: cursor at the position of interest
   * \return the word, or an empty string when the cursor is not on one
   *
   * \note since `@` is not a word character, `QTextCursor::WordUnderCursor`
   * never returns it: `@Strain` yields `Strain` with the sign just before the
   * selection, whereas a cursor on the sign alone selects `@`. Both cases
   * return `@Strain` so that the language modes can look the result up in
   * their keyword list as is
   *
   * \note a word which is not introduced by a `@`, as a CMake command, is
   * returned as it is
   *
   * \note a selection without any letter or digit is rejected: `QTextCursor`
   * treats punctuation as a word, and `;`, which ends an MTest instruction,
   * is a keyword of the MTest parser although `mtest --help-keyword=';'` has
   * nothing to say about it
   */
  TFEL_GUI_VISIBILITY_EXPORT QString getKeywordUnderCursor(const QTextCursor&);

  /*!
   * \brief apply the font of the text editing areas to a widget and remove
   * its contents margins
   * \param[in] w: widget
   *
   * \note the font is the one stored in the "font/textedit" entry of the
   * settings, antialiased; the widget keeps its own font if that entry does
   * not exist
   *
   * \note the contents margins are set to zero so that the widget fits the
   * neighbouring ones without any gap
   */
  void setFontSizeAndContentsMargins(QWidget* const);

  /*!
   * \brief apply the font of the text editing areas to a label, remove its
   * contents margins and set its own margin to zero
   * \param[in] l: label
   *
   * \note this is setFontSizeAndContentsMargins plus `QLabel::setMargin(0)`,
   * which is what removes the space the label reserves around its text
   */
  void setQLabelFontSizeAndMargins(QLabel* const);

  /*!
   * \brief insert a scroll area in a layout with its scroll bar drawn
   * beside it, as the main editing areas do
   * \param[in] hl: layout the scroll area is added to, which takes it over
   * \param[in] a: scroll area
   *
   * \note the scroll bar of the scroll area itself is turned off and the
   * `QScrollBar` widget it uses is added to the layout, so the area is not
   * narrowed by its own scroll bar
   *
   * \note the contents margins and the spacing of the layout are set to
   * zero
   */
  void setQAbstractScrollAreaInLayout(QHBoxLayout* const,
                                      QAbstractScrollArea* const);

#ifdef TFEL_GUI_HAVE_WEBENGINE
  /*!
   * \brief insert a web view in a layout
   * \param[in] hl: layout the view is added to, which takes it over
   * \param[in] a: web view
   *
   * \note the contents margins and the spacing of the layout are set to
   * zero; the scroll bar handling of setQAbstractScrollAreaInLayout is not
   * applied, the view keeps its own scroll bar
   *
   * \note this function only exists when the Qt WebEngine modules have been
   * found when configuring
   */
  void setQWebEngineViewInLayout(QHBoxLayout* const, QWebEngineView* const);
#endif /* TFEL_GUI_HAVE_WEBENGINE */

  /*!
   * \brief compute the part both strings start with
   * \param[in] s1: first string
   * \param[in] s2: second string
   * \return the common part, empty if the strings do not share their first
   * character or if one of them is empty
   */
  QString commonPart(const QString&, const QString&);

  /*!
   * \brief compute the part all the strings start with, which is the text
   * the line edit is completed with
   * \param[in] l: list of strings, typically the possible completions
   * \return the common part, the only string of the list if it holds a
   * single string, an empty string if the list is empty or if the strings
   * share nothing
   */
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
   * \param[in] m: menu whose actions are freed
   *
   * \note the actions and the submenus are scheduled for deletion with
   * `deleteLater()`: they still belong to the menu until the event loop
   * runs, which is when they are actually freed. Separators are left
   * untouched as they are owned by the menu
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
   *
   * \note as in clearMenu, the actions and the submenus are scheduled for
   * deletion with `deleteLater()` and the top level actions of the menu bar
   * are handled as those of a menu: separators are left untouched
   */
  TFEL_GUI_VISIBILITY_EXPORT void clearMenuBar(QMenuBar* const);

#ifdef Q_OS_UNIX

  //! \return the user' login
  //! \note this is the name of the password entry of the effective user
  //! identifier of the process, an empty string if there is no such entry
  TFEL_GUI_VISIBILITY_EXPORT QString getLoginName();
  //! \return the user' name
  //! \note this is the full name stored in that same password entry, an
  //! empty string if there is no such entry
  TFEL_GUI_VISIBILITY_EXPORT QString getUserName();

#endif /* Q_OS_UNIX */

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_UTILITIES_HXX */
