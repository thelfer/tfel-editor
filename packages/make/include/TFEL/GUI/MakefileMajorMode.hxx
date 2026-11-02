/*!
 * \file  MakefileMajorMode.hxx
 * \brief MakefileMajorMode: major mode for Makefiles, with a compile prompt.
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_MAKEFILEMAJORMODE_HXX
#define LIB_TFEL_GUI_MAKEFILEMAJORMODE_HXX

#include "TFEL/GUI/MajorModeBase.hxx"

namespace tfel::gui {

  /*!
   * \brief a major mode for editing Makefile's
   *
   * The mode brings syntax highlighting, a `#` comment syntax and the
   * \c Alt+M shortcut which prompts for a compilation command; it defines
   * no completion and no specific menu, so the defaults of
   * \c MajorModeBase apply.
   *
   * \note the mode is registered by a \c StandardMajorModeProxy static
   * initialiser at the end of `MakefileMajorMode.cxx`, which associates it
   * with the files named `Makefile` or `Makefile.am`.
   */
  struct MakefileMajorMode : public MajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor widget the mode acts on
     */
    MakefileMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return the name of the mode, "Makefile"
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
     * \note a \c MakefileSyntaxHighlighter is installed through
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
     * \return "#", the string which starts a comment line in Makefile's
     *
     * \note the inherited \c comment method uses this string to comment or
     * uncomment the selected lines
     */
    QString getCommentSyntax() override;

    //! destructor
    ~MakefileMajorMode() override;

   private:
    /*!
     * \brief prompt for a compilation command and run it
     *
     * The default command is the last entry of the
     * \c make/compilation/history setting, or "make -k" when this history
     * is empty; the history is updated with the command once it is run.
     *
     * \note a \c ShellProcessLineEdit is displayed in the minibuffer
     * through \c EditorWidget::setUserInput, which takes its ownership;
     * the output of the command is displayed in the
     * "compilation-output" buffer
     *
     * \note this method is triggered by the \c Alt+M shortcut handled in
     * \c keyPressEvent
     */
    virtual void runCompilation();

    Q_OBJECT
  };  // end of struct MakefileMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_MAKEFILEMAJORMODE_H */
