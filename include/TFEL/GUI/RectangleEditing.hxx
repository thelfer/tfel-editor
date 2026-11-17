/*!
 * \file  RectangleEditing.hxx
 * \brief the four commands which act on the columns a selection covers.
 * \author Thomas Helfer
 * \date   17/11/2026
 */

#ifndef LIB_TFEL_GUI_RECTANGLEEDITING_HXX
#define LIB_TFEL_GUI_RECTANGLEEDITING_HXX

#include <QtCore/QVector>

#include "TFEL/GUI/Config.hxx"

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;
  // forward declaration
  struct TextEditBase;

  /*!
   * \brief the commands which act on a rectangle: the columns which lie
   * between the two ends of the selection, on every line the selection touches
   *
   * A line an end of the selection stands on is part of the rectangle, which is
   * what makes a selection dragged to the beginning of the next line reach that
   * line too, and the empty last line a file ending with a newline is one of
   * the lines of a selection which reaches the end of the buffer. The columns
   * are those of the characters of the lines, not those of the display: a tab
   * counts as one character here and moves to the next tab stop on the screen.
   *
   * What a killed rectangle is kept in belongs to this class and to no kill
   * ring: `Ctrl-Y` knows nothing of a rectangle, and `paste-rectangle` is its
   * only way back, as it is in GNU Emacs, whose `kill-rectangle`,
   * `copy-rectangle-as-kill` and `yank-rectangle` these three commands are.
   *
   * \note this is what `Ctrl-X R` opens a minibuffer input for, see
   * `RectangleMode`, and what the entries of the `Rectangle Edit` submenu of
   * the `Edit` menu and the `cut-rectangle`, `copy-rectangle` and
   * `paste-rectangle` commands call directly — `kill-rectangle` is registered
   * as an alias of the first, under the name GNU Emacs gives it. The register
   * is one for the editor, as it is one per Emacs session, so a rectangle
   * killed in a buffer is pasted in another one.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT RectangleEditing {
    /*!
     * \brief writes a text on every line of the rectangle the selection covers
     * \param[in,out] textEdit : text edit the lines of which are rewritten
     * \param[in,out] editor : editor the message of a read only buffer is
     * displayed in
     * \param[in] text : text written on every line of the rectangle
     * \return true when the lines of the rectangle were rewritten
     *
     * Every line keeps what it holds before the left edge and after the right
     * edge of the rectangle, and a line which stops short of the left edge is
     * padded with blanks up to it, so the text of an insertion always starts at
     * that column. The whole text goes on every line, so a text of several
     * lines opens as many lines on each line it is written on.
     *
     * The cursor ends behind the last line of the text written on the last line
     * of the rectangle, which is where Emacs leaves the point.
     */
    static bool insertString(TextEditBase& textEdit,
                             EditorWidget& editor,
                             const QString& text);

    /*!
     * \brief removes the rectangle from the lines it covers and keeps it
     * \param[in,out] textEdit : text edit the rectangle is taken from
     * \param[in,out] editor : editor the messages are displayed in
     * \return true when the rectangle was taken
     *
     * What each line held between the two edges is kept padded with blanks up
     * to the width of the rectangle, so a line which stopped short of the right
     * edge gives a piece as wide as the rest and a paste puts the rectangle back
     * where it was taken from. The line itself is not padded: it only loses what
     * the rectangle covered, which is why deleting what a line does not hold
     * leaves that line alone.
     *
     * The cursor keeps the line it stands on and goes back by the width of the
     * text deleted in front of it, stopping at the left edge when it stood
     * inside of the rectangle.
     */
    static bool killRectangle(TextEditBase& textEdit, EditorWidget& editor);

    /*!
     * \brief keeps the rectangle without taking it from the lines it covers
     * \param[in,out] textEdit : text edit the rectangle is read from
     * \param[in,out] editor : editor the messages are displayed in
     * \return true when the rectangle was kept
     *
     * This is `copy-rectangle-as-kill`: the same text the kill above keeps, with
     * the same padding, and no line of the buffer is touched and no cursor is
     * moved, since nothing is taken from anywhere.
     */
    static bool copyRectangleAsKill(TextEditBase& textEdit,
                                    EditorWidget& editor);

    /*!
     * \brief writes back what was kept, with its upper left corner at the
     * cursor
     * \param[in,out] textEdit : text edit the rectangle is written in
     * \param[in,out] editor : editor the messages are displayed in
     * \return true when the rectangle was written
     *
     * The column of the cursor is the column of every piece, the selection is
     * nothing to the command, and what the lines hold from that column on is
     * kept: the rectangle is inserted, not written over the text. A line shorter
     * than that column is padded with blanks up to it, and the lines the pieces
     * need below the end of the buffer are opened, except for the empty line the
     * final newline of a file opens, which is already the start of a line and
     * takes the pieces on its own place.
     *
     * The cursor ends behind the last piece written, at the lower right corner
     * of the pasted rectangle.
     */
    static bool yankRectangle(TextEditBase& textEdit, EditorWidget& editor);

    /*!
     * \brief ends the rectangle the commands keep, which is what a test of the
     * paste of no rectangle leans on
     */
    static void clearRegister();

    //! \return true when a rectangle stands in the register
    static bool hasRectangle();

   private:
    /*!
     * \brief asks whether a command which writes may write
     * \return false, after having displayed what the user has to know, when the
     * text edit can not be written to
     */
    static bool checkWritable(TextEditBase& textEdit, EditorWidget& editor);

    //! the pieces of the rectangle the last kill or copy kept, one per line
    static QVector<QString> killed;
  };  // end of struct RectangleEditing

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_RECTANGLEEDITING_HXX */
