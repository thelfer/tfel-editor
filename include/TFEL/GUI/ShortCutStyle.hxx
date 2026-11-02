/*!
 * \file  ShortCutStyle.hxx
 * \brief key bindings style of the editor: emacs or Qt.
 * \author Thomas Helfer
 * \date   01/10/2012
 */

#ifndef LIB_TFEL_GUI_SHORTCUTSTYLE_HXX
#define LIB_TFEL_GUI_SHORTCUTSTYLE_HXX

#include <QtCore/QObject>
#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  /*!
   * \brief key bindings style of the editor, which is either Emacs-like or
   * Qt-like
   *
   * \note the style is saved in the settings and restored when the editor
   * is started again
   */
  struct TFEL_GUI_VISIBILITY_EXPORT ShortCutStyle : public QObject {
   signals:

    /*!
     * \brief emitted every time the style is set, so that the widgets which do
     * not query the style lazily, such as the menus of `MainWindow`, can be
     * rebuilt
     */
    void shortCutStyleChanged();

   public:
    /*!
     * \brief the supported key bindings styles
     *
     * \note `EMACS` is the style used when the editor holds no preference,
     * whereas `QT` lets the line edits and the text editing areas handle the
     * usual Qt short cuts
     */
    enum Style { EMACS, QT };

    /*!
     * \return the unique instance of this class, created on the first call,
     * whose style is the one stored in the settings
     */
    static ShortCutStyle& getShortCutStyle();

    /*!
     * \return the key bindings style currently in use
     *
     * \note the text editing areas and the line edits query this method each
     * time a key is pressed, so they follow the changes without being
     * notified
     */
    Style getStyle();

    /*!
     * \brief select the key bindings style to use
     * \param[in] s : style to use
     *
     * \note the choice is saved in the settings, whatever the style is, and
     * the `shortCutStyleChanged` signal is emitted, even if the style does
     * not actually change
     *
     * \note the constructor calls this method to restore the saved style, so
     * the signal is emitted while the instance cannot have any subscriber yet
     */
    void setStyle(const Style&);

   private:
    /*!
     * \brief constructor, which reads the style stored in the
     * "shortcut style" entry of the settings
     *
     * \note `EMACS` is used if that entry does not exist or holds a value
     * which is neither "EMACS" nor "QT"
     */
    ShortCutStyle();

    /*!
     * \brief copy constructor, deleted as there is a unique instance of this
     * class
     * \param[in] s : style to copy
     */
    ShortCutStyle(const ShortCutStyle&) = delete;

    /*!
     * \brief assignment operator, deleted as there is a unique instance of
     * this class
     * \param[in] s : style to assign
     */
    ShortCutStyle& operator=(const ShortCutStyle&) = delete;

    //! key bindings style in use
    Style style;

    Q_OBJECT

  };  // end of struct ShortCutStyle

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SHORTCUTSTYLE_HXX */
