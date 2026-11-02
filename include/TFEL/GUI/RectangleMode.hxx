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

  struct TextEditBase;

  /*!
   * \brief the rectangle commands, which operate on the rectangular area
   * delimited by the two ends of the selection
   *
   * This is the minibuffer input "Ctrl-X r" opens: it waits for a single
   * character, "t" to insert a string in the rectangle, "k" to kill the
   * rectangle and "y" to paste the last killed one. "Ctrl-g" cancels
   */
  struct RectangleMode : public CommandLine {
    /*!
     * \brief constructor
     * \param[in] t : text edit the rectangles are killed and pasted in
     * \param[in] p : editor widget, which owns the input once it is shown
     */
    RectangleMode(TextEditBase&, EditorWidget&);

    /*!
     * \return always true: the commands which follow "Ctrl-X r" must be typed
     * in the minibuffer, so the buffers are disabled while this input is shown
     * and "Ctrl-X o" can not move the focus out of it
     */
    virtual bool isBlocking() const override;

    /*!
     * \brief method called when processing is finished
     *
     * The text the user typed selects the command: "t" replaces this input by
     * the input asking for the string to insert, "k" kills the rectangle and
     * "y" pastes the last killed one. Anything else does nothing.
     *
     * \note once this method returns, \c userEditingFinished emits
     * \c finished and the editor removes this input from the minibuffer. "t"
     * therefore has to remove the input and install the new one by itself, so
     * that the removal the signal triggers finds nothing left to remove
     */
    virtual void treatUserInput() override;

   protected:
    /*!
     * \brief lines of the last rectangle which has been killed, one entry per
     * line of the buffer the rectangle covers
     *
     * \note this is a static member: all the rectangle commands of the editor
     * share the same killed rectangle and \c killRectangle replaces its whole
     * content
     */
    static QVector<QString> killedRectangle;

    /*!
     * \brief line edit of the rectangle commands
     *
     * It turns the single keys \c t, \c k and \c y into the text of the input
     * and validates it, so that a command is run without having to press
     * \c Return. \c Ctrl-g cancels the input.
     */
    struct RectangleModeLineEdit;

    /*!
     * \brief remove the rectangle the selection covers from the buffer and
     * remember it as the killed rectangle
     *
     * The rectangle is the set of columns which lies between the two ends of
     * the selection, on every line the selection touches; shorter lines are
     * padded with blanks.
     *
     * \note the editor's cursor is left on the column the rectangle starts on,
     * at the last line it covers
     *
     * \note nothing is killed when the cursor holds no selection or when both
     * of its ends stand on the same column
     */
    void killRectangle();

    /*!
     * \brief insert the killed rectangle in the buffer, at the column the
     * editor's cursor stands on
     *
     * The lines the rectangle covers are padded with blanks when they are
     * shorter than that column, and new lines are added when the rectangle
     * reaches past the end of the buffer.
     *
     * \note nothing is inserted and a message is reported in the minibuffer
     * when no rectangle has been killed yet
     */
    void pasteRectangle();

    //! text edit the rectangles are killed and pasted in
    TextEditBase& textEdit;

  };  // end of struct RectangleMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_RECTANGLEMODE_HXX */
