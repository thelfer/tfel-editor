/*!
 * \file   LineEdit.hxx
 * \brief line edit of the minibuffer handling emacs style short cuts.
 * \author Thomas Helfer
 * \date   10/03/2018
 */

#ifndef LIB_TFEL_GUI_LINEEDIT_HXX
#define LIB_TFEL_GUI_LINEEDIT_HXX

#include <TFEL/GUI/MultiLineLineEdit.hxx>
#include <TFEL/GUI/Config.hxx>

namespace tfel::gui {

  // forward declaration
  struct EditorWidget;

  /*!
   * \brief a line edit used by the minibuffer of the editor
   *
   * This widget is a `MultiLineLineEdit`: one line tall by default, growing
   * and shrinking with its content, which supports multi-line editing.
   *
   * When the short cut style is not Qt's (see `ShortCutStyle`), this line
   * edit handles emacs style short cuts: history navigation (`Ctrl-p`,
   * `Ctrl-n`, up and down arrows), kill ring yanking (`Ctrl-y`, `Alt-y`),
   * `Ctrl-h` for select all, `Ctrl-Space` to clear the selection,
   * `Ctrl-a/e/b/f` and `Alt-b/f` to move the cursor, and the
   * `Ctrl-k1 Mod-k2` sequences dispatched to handleShortCut(). Events
   * handled this way are not passed to the base `MultiLineLineEdit`.
   */
  struct TFEL_GUI_VISIBILITY_EXPORT LineEdit : MultiLineLineEdit {
    /*!
     * \brief constructor
     * \param[in] q: editor emacs widget
     * \param[in] p: parent widget
     */
    LineEdit(EditorWidget &, QWidget *const = nullptr);

    /*!
     * \brief set the history browsed with `Ctrl-p`, `Ctrl-n` and the arrow
     * keys
     * \param[in] h: history, the last entry being the most recent one
     */
    virtual void setInputHistory(const QStringList &);

    /*!
     * \return a copy of the history browsed with `Ctrl-p`, `Ctrl-n` and the
     * arrow keys
     */
    virtual QStringList getInputHistory() const;

    /*!
     * \brief handle a key press
     * \param[inout] ev: key event, consumed (not passed to the base class)
     * when the corresponding short cut is handled here
     *
     * \note the event is passed to the base `QLineEdit` as is when the short
     * cut style is Qt's, and when no emacs style short cut matches
     */
    void keyPressEvent(QKeyEvent *) override;
    //! destructor
    ~LineEdit() override;

   protected:
    /*!
     * treat "Ctrl-k1 Mod-k2"
     * where k1 is either Qt::Key_X or Qt::Key_C.
     * \param[in] k1 : first  key
     * \param[in] m  : second key modifier
     * \param[in] k2 : second key
     * \return true if the short cut is handled by this mode
     *
     * \note this method returns nothing although a return tag is present
     * above: the base implementation ignores its arguments and displays
     * "unknown shortcut" in the minibuffer
     */
    virtual void handleShortCut(const int,
                                const Qt::KeyboardModifiers,
                                const int);
    //! editor widget this line edit is attached to
    EditorWidget &editor;
    //! history of the entries entered in this line edit
    QStringList inputHistory;
    //! index in the kill ring of the last string inserted by a yank
    int pring = 0;
    //! true if `Ctrl-x` was pressed and the next key is awaited
    bool ctrlx = false;
    //! true if `Ctrl-c` was pressed and the next key is awaited
    bool ctrlc = false;
    //! true if the last handled short cut was a yank (`Ctrl-y`)
    bool yank = false;
    //! index in the input history of the entry currently displayed
    int pHistory = 0;
    //! true while the history is being browsed with successive key presses
    bool hMove = false;

   private:
    Q_OBJECT
  };

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_LINEEDIT_HXX */
