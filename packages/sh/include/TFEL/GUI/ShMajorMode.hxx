/*!
 * \file  ShMajorMode.hxx
 * \brief ShMajorMode: major mode for sh files, with # comment support.
 * \author Thomas Helfer
 * \date   26/08/2012
 */

#ifndef LIB_TFEL_GUI_SHMAJORMODE_HXX
#define LIB_TFEL_GUI_SHMAJORMODE_HXX

#include "TFEL/Config/TFELConfig.hxx"
#include "TFEL/GUI/MajorModeBase.hxx"

#if defined _WIN32 || defined _WIN64 || defined __CYGWIN__
#if defined TFELEditorShMode_EXPORTS
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#else
#if !defined TFEL_STATIC_BUILD
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_IMPORT
#else
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT
#endif
#endif
#else
#define TFELEDITOR_SHMODE_VISIBILITY_EXPORT TFEL_VISIBILITY_EXPORT
#endif

namespace tfel::gui {

  /*!
   * \brief a major mode for editing `sh` files
   *
   * The mode brings syntax highlighting, a comment syntax and completion
   * over the POSIX shell control words and builtins; it defines no
   * specific menu, so the defaults of \c MajorModeBase apply.
   *
   * \note the mode is registered by a \c StandardMajorModeProxy static
   * initialiser at the end of `ShMajorMode.cxx`, which associates it with
   * the files whose name ends with `.sh`.
   */
  struct TFELEDITOR_SHMODE_VISIBILITY_EXPORT ShMajorMode
      : public MajorModeBase {
    /*!
     * \return the words the completer of the mode is built from: the POSIX
     * shell control words and builtins
     *
     * \note the list is static on purpose: querying the running shell
     * (`compgen`) from the constructor would block the GUI thread; the
     * derived bash mode adds its own words through \c setCompleterList
     */
    static const QStringList& getKeyWordsList();

    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor widget the mode acts on
     */
    ShMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return the name of the mode, "sh"
    QString getName() const override;

    /*!
     * \return the completer installed by the constructor, parented to the
     * mode and displayed over the text edit
     */
    QCompleter* getCompleter() override;

   protected:
    /*!
     * \brief replace the words offered by the completer of the mode
     * \param[in] l : the new words
     *
     * \note used by the derived bash mode to offer the bash builtins and
     * keywords on top of the POSIX ones; the model of the completer is
     * reused, not replaced
     */
    void setCompleterList(const QStringList& l);

    //! the completer of the mode
    QCompleter* c;

   public:

    //! \return a fixed description of the mode
    QString getDescription() const override;

    /*!
     * \brief format a paragraph
     *
     * \note this mode knows how to reformat nothing: this method does
     * nothing
     */
    void format() override;

    /*!
     * \brief install the syntax highlighter of the mode on a document
     * \param[in] d : document to be highlighted
     *
     * \note a \c ShSyntaxHighlighter is installed through
     * \c setHighlighter, which replaces the previous highlighter; the
     * document owns the highlighter it is built with
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \return "#", the string which starts a comment line in `sh`
     *
     * \note the inherited \c comment method uses this string to comment or
     * uncomment the selected lines
     */
    QString getCommentSyntax() override;

    //! destructor
    ~ShMajorMode() override;

   private:
    Q_OBJECT
  };  // end of struct ShMajorMode

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_SHMAJORMODE_HXX */
