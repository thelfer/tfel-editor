/*!
 * \file  CMakeMajorMode.hxx
 * \brief CMakeMajorMode: major mode for CMake files, with command help.
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_CMAKEMAJORMODE_HXX
#define LIB_TFEL_GUI_CMAKEMAJORMODE_HXX

#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  /*!
   * \brief a major mode for editing `CMakeLists.txt` files
   *
   * The mode brings syntax highlighting, a `#` comment syntax, the
   * \c Alt+M shortcut and a "Help on" entry in the context menu; it
   * defines no completion and no specific menu, so the defaults of
   * \c MajorModeBase apply.
   *
   * \note the mode is registered by a \c StandardMajorModeProxy static
   * initialiser at the end of `CMakeMajorMode.cxx`, which associates it
   * with the files named `CMakeLists.txt` or whose name ends with
   * `.cmake`.
   */
  struct CMakeMajorMode : public MajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor widget the mode acts on
     */
    CMakeMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return the name of the mode, "CMake"
    QString getName() const override;

    //! \return a fixed description of the mode
    QString getDescription() const override;
    /*!
     * This method is called before any treatment by
     * TextEditBase and allows the mode to override default
     * shortcuts
     * \param[in] e : key event
     *
     * \return true when \c Alt+M is pressed, in which case
     * \c runCompilation is triggered and \c TextEditBase applies no
     * shortcut of its own; false otherwise, so the remaining shortcuts
     * are handled as usual
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief install the syntax highlighter of the mode on a document
     * \param[in] d : document to be highlighted
     *
     * \note a \c CMakeSyntaxHighlighter is installed through
     * \c setHighlighter, which replaces the previous highlighter; the
     * document owns the highlighter it is built with
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \brief format a paragraph
     *
     * \note this mode knows how to reformat nothing: this method does
     * nothing
     */
    void format() override;

    /*!
     * \return "#", the string which starts a comment line in `CMake`
     * scripts
     *
     * \note the inherited \c comment method uses this string to comment or
     * uncomment the selected lines
     */
    QString getCommentSyntax() override;

    //! destructor
    ~CMakeMajorMode() override;

   protected:
    /*!
     * \brief run a compilation command
     *
     * \note the body of this method is currently commented out: it does
     * nothing, although \c keyPressEvent still intercepts \c Alt+M to call
     * it
     */
    virtual void runCompilation();

    /*!
     * \brief complete the context menu actions
     * \param[in] m : context menu to complete
     * \param[in] tc : text cursor at the position where the menu will
     * appear
     *
     * When the first word of the line is a standard `CMake` command (as
     * returned by \c CMakeSyntaxHighlighter::getCMakeCommandsList), a
     * "Help on" action is inserted at the top of the menu, after the
     * actions added by \c MajorModeBase::completeContextMenu. Triggering
     * it attaches a secondary task to the buffer, which displays the
     * output of `cmake --help-command` for that command.
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

   private:
    Q_OBJECT
  };  // end of struct CMakeMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_CMAKEMAJORMODE_HXX */
