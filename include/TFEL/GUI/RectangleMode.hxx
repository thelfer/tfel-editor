/*!
 * \file  RectangleMode.hxx
 * \brief commands and state of the rectangle selection of the editor.
 * \author Thomas Helfer
 * \date   30/09/2012
 */

#ifndef LIB_TFEL_GUI_RECTANGLEMODE_HXX
#define LIB_TFEL_GUI_RECTANGLEMODE_HXX

#include "TFEL/GUI/CommandLine.hxx"

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;

  /*!
   * \brief the minibuffer input "Ctrl-X r" opens: it waits for a single
   * character and runs one of the rectangle commands on the area the selection
   * covers
   *
   * "t" asks a string and writes it on every line of the rectangle, "k" kills
   * the rectangle, "w" keeps it without taking it from the buffer and "y"
   * writes back what was last killed. "Ctrl-g" cancels.
   *
   * The commands themselves, and the register what "k" and "w" keep the
   * rectangle in, belong to `RectangleEditing`: the entries of the `Rectangle
   * Edit` submenu of the `Edit` menu and the `cut-rectangle`,
   * `copy-rectangle` and `paste-rectangle` commands call them without opening
   * this input.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT RectangleMode : public CommandLine {
    /*!
     * \brief constructor
     * \param[in] t : text edit the rectangles are killed and pasted in
     * \param[in] p : editor widget, which owns the input once it is shown
     */
    RectangleMode(TextEditBase&, EditorWidget&);

    /*!
     * \return always true: the commands which follow "Ctrl-X r" must be typed
     * in the minibuffer, where this input is, and not in the text edit they
     * work on
     */
    bool isBlocking() const override;

    /*!
     * \brief runs the command the answer names
     *
     * "t" is the only answer which does not run a command at once: it removes
     * this input and asks the string to write on the lines of the rectangle.
     */
    void treatUserInput() override;

   protected:
    /*!
     * \brief line edit of the rectangle commands
     *
     * It turns the single keys \c t, \c k, \c w and \c y into the text of the
     * input and validates it, so that a command is run without having to press
     * \c Return. \c Ctrl-g cancels the input.
     */
    struct RectangleModeLineEdit;

    //! text edit the rectangles are killed and pasted in
    TextEditBase& textEdit;
  };  // end of struct RectangleMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_RECTANGLEMODE_HXX */
