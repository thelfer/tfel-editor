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
     * The rectangle is the set of columns which lies between the two ends of the
     * selection, on every line the selection touches: the lines the two ends
     * stand on are part of it, which is what makes a selection dragged down to
     * the beginning of the next line reach that line too, and the final newline
     * of a file the empty last line it opens.
     *
     * What a line held between the two edges of the rectangle is stored padded
     * with blanks up to the width of the rectangle, so a line which stopped
     * short of the right edge gives a piece as wide as the rest and a paste puts
     * the rectangle back where it was taken from. The line itself is not padded:
     * it only loses what the rectangle covered, which is why deleting what a
     * line does not hold leaves that line alone.
     *
     * \note the cursor keeps the line it stands on and goes back by the width of
     * the text deleted in front of it, stopping at the left edge of the rectangle
     * when it stood inside of it. A cursor on a line the rectangle does not
     * reach does not move.
     *
     * \note a rectangle of no width stores a piece of no text per line and
     * leaves both the lines and the cursor where they are: killing nothing is a
     * command of its own, as the rectangle one kills is what the columns say,
     * not what the lines happen to hold.
     */
    void killRectangle();

    /*!
     * \brief insert the killed rectangle in the buffer, at the column the
     * editor's cursor stands on
     *
     * The column of the cursor is the column of every piece: the selection the
     * cursor holds is nothing to this command, and what the lines hold from that
     * column on is kept, so the rectangle is inserted, not written over the text.
     * A line shorter than that column is padded with blanks up to it.
     *
     * A piece which stands past the last line of the buffer opens a line, except
     * for the empty line the final newline of a file opens: that line already
     * names a line and takes the pieces on its own place, so a rectangle of no
     * width pasted there opens nothing.
     *
     * \note the cursor ends behind the last piece written, at the lower right
     * corner of the pasted rectangle
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
