/*!
 * \file  CxxMajorMode.hxx
 * \brief major mode dedicated to the C++ language.
 * \author Thomas Helfer
 * \date   03/08/2012
 */

#ifndef LIB_TFEL_GUI_CXXMAJORMODE_HXX
#define LIB_TFEL_GUI_CXXMAJORMODE_HXX

#include <QtCore/QString>
#include <QtCore/QVector>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/CMajorModeBase.hxx"
#include "TFEL/GUI/EditorWidget.hxx"
#include "TFEL/GUI/SettingsKey.hxx"

// forward declarations
class QKeyEvent;
class QMouseEvent;

namespace tfel::gui {

  // forward declaration
  struct TextEditBase;
  // forward declaration
  struct GrammarCheckSession;
  // forward declaration
  struct LspNavigationSession;

  /*!
   * \brief major mode dedicated to the C++ language
   *
   * \note the mode is registered by a static \c StandardMajorModeProxy defined
   * at the bottom of its source file, which matches the C and C++ file
   * extensions. It is the only mode derived from \c CMajorModeBase in the
   * editor, and \c clang-format is always called with the C++ dialect
   *
   * \note everything the mode asks a language server is done by a \c
   * LspNavigationSession of its own: the mode holds that session, hands the
   * keys, the clicks, the completion and the context menu over to it, and keeps
   * for itself the policy of \c clangd — when a session is worth starting
   * without being asked for, which menu and which commands start one, and which
   * faces the identifiers of the file are drawn with. The session draws the
   * diagnostics of the server under the text; the \c Alt-Tab, \c Alt-. \c Alt-,
   * \c Alt-R and \c Alt-? keys ask it for completions, definitions, references,
   * renames and hover information; the parameter list of the call the cursor
   * stands in is echoed by \c Alt-S and by the editor itself when a character is
   * typed where a call is being typed, \c Alt-O asks which symbol of the file to
   * go to, and the context menu offers to jump to the definition of the symbol
   * it was opened on and to apply the fixes the server offers for the
   * diagnostics of that line (issue #125)
   *
   * \note what the identifiers of the file are is asked of the server and drawn
   * with the answer: a type, a function or a method, a macro, and a strike-out
   * over a name the server says is deprecated. The faces are an extra selection
   * of the text edit, so the keywords, strings, numbers and comments of the
   * highlighter are untouched, and the locals, parameters and members the
   * server names as well are left as they are; the \c Color identifiers entry
   * of the menu, and the setting `clangd/semantic-tokens` it writes, turn them
   * off
   *
   * \note the word the cursor sits on is underlined when the server knows
   * where it is defined or declared, and the mouse cursor becomes the icon of
   * that jump — \c go-jump-definition or \c go-jump-declaration of the
   * Freedesktop Icon Naming Specification, the pointing hand when the theme
   * has neither — while it rests on such a word. A \c Ctrl-Left click on that
   * word is the jump itself, which is what the icon promised: the same jump
   * \c Alt-. does, to the declaration when nothing defines the symbol. Neither
   * asks the server on every keystroke or every pixel: one settling timer of
   * the session is shared by the cursor and the mouse, and the answers they get
   * are remembered by word, with the places they name, until the text or the
   * session changes
   *
   * \note the message of a diagnostic is shown when it is pointed at: the
   * minibuffer echoes it once the cursor has settled on the underlined range,
   * and resting the mouse on that range shows the same text as a tooltip
   *
   * \note a buffer of a project which holds a compilation database starts a
   * session with it, since the options its files are compiled with are then
   * known and a session which reads them is what such a buffer was opened
   * for; the setting `clangd/start-by-default` turns that off, and a file of a
   * project which holds no database starts nothing either way: a session which
   * guesses the compilation options is a decision, not a discovery, and is what
   * the entries of the "C/C++" menu and the \c start-clangd commands ask for.
   * A session which is running is replaced by the one which is asked for. The
   * session started without an explicit database uses the file given to
   * \c tfel-editor as \c --compile-commands-file, or the database
   * \c searchCompileCommandsFile finds in the project of the buffer, or none.
   * The documents of the buffers are held by the client in the meantime and
   * sent to the session once it is up
   */
  struct TFEL_GUI_VISIBILITY_EXPORT CxxMajorMode : public CMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     */
    CxxMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "C++", the name the mode is registered under
    QString getName() const override;

    //! \return "major mode dedicated to the C++ language"
    QString getDescription() const override;

    /*!
     * \return the entries of the configuration the mode uses: the five which
     * decide what is asked to \c clangd and when a session of it is started, in
     * a theme of their own named after the program
     *
     * \note this static method is what \c StandardMajorModeProxy reports as the
     * settings of the mode, see \c MajorModeProxy::getSettingsGroups. The
     * addresses are the ones \c LspCxxSupport reads
     */
    static QVector<SettingsGroup> declareSettings();

    //! \return the C++ icon of the editor resources
    QIcon getIcon() const override;

    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     *
     * \c Alt-m runs a compilation command in the minibuffer.
     * \c Alt-Tab asks the language server for completions, \c Alt-. jumps to
     * the definition of the symbol under the cursor, \c Alt-, lists the
     * references to it, \c Alt-R renames it through a question of the
     * minibuffer, \c Alt-S shows the parameter list of the call the cursor
     * stands in, and \c Alt-? shows its hover information. The context menu
     * asks the definition and the fixable diagnostics of the line it was
     * opened on together, and offers what came back as entries.
     *
     * \return true if the mode handled the key, that is on those keys only;
     * any other key is left to \c CMajorModeBase, which handles none
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief jump to the definition, or to the declaration, of the symbol the
     * \c Ctrl-Left click was pressed on
     * \param[in] e : the mouse press the text edit is about to handle itself
     *
     * \note the click is the mouse's \c Alt-.: the icon of the jump which the
     * session put under the pointer promised a place to go, and a pointer which
     * promises and does nothing is worse than one which says nothing. Where the
     * symbol is nothing to jump to, the click is handed back to the text edit,
     * which places the cursor as it always does
     *
     * \note the answers the tracking of the mouse already got for that word are
     * used as they are, so the click which follows the icon of the jump asks
     * nothing and lands at once; a word which was never rested on is asked
     * about, and waited for a moment only, as the context menu is
     *
     * \return true when the click jumped, that is when the text edit shall not
     * move the cursor to the place it was clicked on
     */
    bool mousePressEvent(QMouseEvent *const) override;

    /*!
     * \return the completer of the language server while its popup is shown,
     * the one of the base class otherwise, that is none
     */
    QCompleter *getCompleter() override;

    /*!
     * \return 1 while the language server popup is shown, so that typing a
     * single character does not close it, 3 otherwise, as
     * \c MajorModeBase does
     */
    int getMinimalCompletionLength() override;

    /*!
     * \brief replace the word under the cursor by the completion the user
     * chose in the language server popup, without the blank the base class
     * appends: a C++ completion often ends in `(` or `::`
     */
    void completeCurrentWord(TextEditBase &, const QString &) override;

    /*!
     * \return the "C/C++" menu of the mode, with the two entries which start
     * a \c clangd session and the one which ends it
     *
     * \note as any specific menu of a major mode, the caller owns the returned
     * menu, which is why its actions refer to the mode, not to the widget the
     * menu is displayed in
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief add a "Go to definition" entry to the context menu when the
     * language server has a definition for the symbol the menu was opened on,
     * and the entries which apply the fixes it offers for that line
     * \param[in] m : menu to complete
     * \param[in] tc : text cursor at the position where the menu appears
     *
     * \note the entries are inserted at the top of the menu, before the entries
     * of the text edit and separated from them
     *
     * \note the two questions are asked by the session here, as that is the only
     * way to know whether to offer anything, and their answer is waited for a
     * moment only: a server which is not up, or which is busy parsing, leaves
     * the menu untouched rather than delaying it
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \brief install the highlighter of the C++ language on the given document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is owned by the document from this point on; it is
     * released, and the previous one removed, by the base class
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \return the session which reads the prose of this buffer, that is the
     * comments of a source file, whatever the choice of the proofreader is
     */
    GrammarCheckSession *grammarCheckSession() const override;

    //! destructor
    ~CxxMajorMode() override;

   protected:
    /*!
     * \return the name of the mode, "C++", which is also the language name:
     * the compilation command history is stored under the address
     * "C++/compilation/history"
     */
    QString getLanguageName() const override;

   private:
    /*!
     * \brief the session which reads the words of the buffer, built whatever
     * the \c clangd support says: what the comments of a source file say is a
     * question of grammar and none of a compiler's
     */
    GrammarCheckSession *grammar = nullptr;
    /*!
     * \brief the conversation of this buffer with \c clangd, null when the
     * setting `clangd/enable` turns the whole support off
     *
     * \note the session is a child of the mode and is destroyed with it; it is
     * what holds the client, the timers, the tracked words and the faces, which
     * is why the mode has nothing left to hold but these two pointers
     */
    LspNavigationSession *clangd_session = nullptr;
  };  // end of struct CxxMajorMode

}  // end of namespace tfel::gui

#endif
