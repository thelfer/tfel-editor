/*!
 * \file  PythonMajorMode.hxx
 * \brief major mode dedicated to the Python language.
 * \author Thomas Helfer
 * \date   14/11/2026
 */

#ifndef LIB_TFEL_GUI_PYTHONMAJORMODE_HXX
#define LIB_TFEL_GUI_PYTHONMAJORMODE_HXX

#include <QtCore/QPointer>
#include <QtCore/QTemporaryFile>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "TFEL/GUI/CompiledLanguageMajorModeBase.hxx"
#include "TFEL/GUI/SettingsKey.hxx"

namespace tfel::gui {

  // forward declaration
  struct LspNavigationSession;

  // forward declaration
  struct ProcessInteractionFrame;

  /*!
   * \brief major mode dedicated to the Python language
   *
   * \note the mode is registered by a static \c StandardMajorModeProxy defined
   * at the bottom of its source file, which matches the extensions of the files
   * of the language
   *
   * \note everything the mode knows about the symbols of a buffer it owes to a
   * language server, \c pylsp, through a \c LspNavigationSession of its own —
   * the same session the C++ mode uses with \c clangd, and for the same reason:
   * what a name is, where it is defined, what a call takes, and what the file
   * says of itself is the business of a program which parses the language, not
   * of a mode which matches its words. The keys are the keys of the C++ mode —
   * \c Alt-Tab completes, \c Alt-. jumps to a definition, \c Alt-, lists the
   * references, \c Alt-R renames, \c Alt-S shows the parameter list of the call
   * the cursor stands in, \c Alt-? hovers, \c Alt-O opens the overview of the
   * file — and \c Alt-m runs a command on the buffer, which for a script
   * language is how one runs the file one is editing
   *
   * \note the session starts itself for a buffer of the language, since
   * \c pylsp needs nothing to know about a file beyond its text — no database,
   * no project — so a session which is started without being asked for costs
   * nothing but the time it takes to boot, and buys the underlines of the file
   * and the jumps of the keys. The setting `pylsp/start-by-default` turns that
   * off, and the entries of the "Python" menu are where a session is asked for
   * by hand
   *
   * \note what the server answers about a file is limited by what it answers
   * to \c initialize: \c pylsp has no declaration question, so a jump is a jump
   * to a definition only, and it has no semantic tokens, so the faces of the
   * identifiers are those of the highlighter of this mode and nothing else
   *
   * \note \c format asks the server to reformat the buffer, which is what the
   * `textDocument/formatting` question is for; the answer reaches the buffer
   * where it changes something and nowhere else, so a buffer which is already
   * formatted does not lose its cursor
   */
  struct TFEL_GUI_VISIBILITY_EXPORT PythonMajorMode
      : public CompiledLanguageMajorModeBase {
    /*!
     * \brief constructor
     * \param[in] w: editor widget the mode belongs to
     * \param[in] b: buffer the mode is attached to
     * \param[in] t: text editor widget the mode acts on, which is also the
     * parent widget of the mode
     */
    PythonMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "Python", the name the mode is registered under
    QString getName() const override;

    /*!
     * \brief indent the line the cursor stands in, which is what the `Tab` key
     * calls
     *
     * The first press on a line brings it to the indentation its previous line
     * allows: the body of the block the previous line opens, the level a
     * `return` in it closes, the alignment a bracket or a backslash asks for, or
     * the column of the `if` an `else` returns from.
     *
     * Each further press on the same line takes it one level down, and once the
     * line stands at the left margin, the next press brings it back to the first
     * position: the presses walk the line through the places it may stand in,
     * the way \c python.el does. The cycle is what the previous line decides and
     * nothing else, so a line whose previous line allows no indentation at all
     * (a statement after a `return` which closed its block, an `else` with no
     * `if` above it) stays at the margin however often `Tab` is pressed.
     *
     * \c Shift-Tab takes the line one level down as well, without the first press to
     * the maximum the block allows, and stops at the lowest level of the cycle
     * instead of wrapping back to the highest one. With a selection, `Tab' shifts the
     * region one level to the right and `Shift-Tab' one level to the left.
     */
    void indentLine(const QTextCursor &c) override;

    /*!
     * \brief shift the selected region one level to the right, which is what
     * the `Tab` key with a selection calls
     *
     * Selecting a body and pressing `Tab` is what one does when a block was
     * written at the wrong level, and a shift of one level is the move whose
     * effect is visible before the key is released: every line of the region
     * keeps the structure the lines beside it add, and the blank lines of the
     * region are left where they are. `Shift-Tab` with a selection shifts the
     * region one level to the left instead.
     */
    void indentRegion(const QTextCursor &c) override;

    /*!
     * \brief treat `Ctrl-C Ctrl-L`, `Ctrl-C Ctrl-R` and `Ctrl-C Ctrl-B`: send
     * the line, the region and the buffer to the interpreter
     */
    bool handleShortCut(const int, const Qt::KeyboardModifiers,
                        const int) override;

    /*!
     * \brief send the line the cursor stands in to the interpreter
     * \return true when the line was handed to the interpreter
     */
    bool sendLineToPython();

    /*!
     * \brief send the selected region, line by line, to the interpreter
     * \return true when every line of the region was handed over
     */
    bool sendRegionToPython();

    /*!
     * \brief send the whole buffer, line by line, to the interpreter
     * \return true when every line of the buffer was handed over
     */
    bool sendBufferToPython();

    /*!
     * \return `Category::PROGRAMMING`: a Python file is the source of a
     * program, and the output of running it is the compilation output of the
     * buffer like the output of any other compiler
     */
    Category getCategory() const override;

    //! \return "major mode dedicated to the Python language"
    QString getDescription() const override;

    /*!
     * \return the entries of the configuration the mode uses: which \c pylsp to
     * start, what to start it with, and whether a buffer is to start one of its
     * own, in a theme of their own named after the program
     *
     * \note this static method is what \c StandardMajorModeProxy reports as the
     * settings of the mode, see \c MajorModeProxy::getSettingsGroups
     */
    static QVector<SettingsGroup> declareSettings();

    /*!
     * \return the icon of a Python file of the desktop theme, which the editor
     * has no picture of its own for
     */
    QIcon getIcon() const override;

    /*!
     * This method is called before any treatment by TextEditBase
     * and allows the mode to override default shortcuts
     *
     * \c Alt-m runs a command on the buffer, the session's keys ask the
     * questions of the language server, and any other key is left to
     * \c MajorModeBase, which handles none
     *
     * \return true if the mode handled the key
     */
    bool keyPressEvent(QKeyEvent *const) override;

    /*!
     * \brief jump to the definition of the symbol the \c Ctrl-Left click was
     * pressed on, the mouse's \c Alt-.
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
     * single character does not close it, 3 otherwise, as \c MajorModeBase does
     */
    int getMinimalCompletionLength() override;

    /*!
     * \brief replace the word under the cursor by the completion the user chose
     * in the language server popup, without the blank the base class appends: a
     * Python completion often ends in `(`
     */
    void completeCurrentWord(TextEditBase &, const QString &) override;

    /*!
     * \return the "Python" menu of the mode: the entry which starts a \c pylsp
     * session, the overview of the file, and the entry which ends the session
     */
    QMenu *getSpecificMenu() override;

    /*!
     * \brief add a "Go to definition" entry to the context menu when the server
     * has a definition for the symbol the menu was opened on
     * \param[in] m : menu to complete
     * \param[in] tc : text cursor at the position where the menu appears
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \brief install the highlighter of the Python language on the given
     * document
     * \param[in] d : document to highlight
     *
     * \note the highlighter is owned by the document from this point on; it is
     * released, and the previous one removed, by the base class
     */
    void setSyntaxHighlighter(QTextDocument *const) override;

    /*!
     * \brief ask the server to reformat the buffer and apply its answer
     *
     * \note the buffer is left alone, and so is the cursor, when the answer has
     * nothing to change, which is what a buffer that is already formatted
     * answers
     */
    void format() override;

    /*!
     * \brief start the interpreter session if there is none, and hand the given
     * text to it
     * \param[in] text : the text to send
     * \param[in] whole_block : treat the text as a whole program, which is what a
     * region of several lines and a buffer are: they reach the interpreter through
     * a file it reads, see the note below
     * \return true when the text was taken for the interpreter, queued behind
     * the sends before it until the session stands at a prompt, see
     * `flushPythonSends`
     *
     * \note the session is the one of the buffer, started on the first text sent to
     * it and shown as a secondary task where everything the interpreter answers is
     * printed, the tracebacks included
     *
     * \note a text of several lines can not be written to the interpreter as it
     * stands: the interactive parser ends an indented block at a blank line, so a
     * function whose body holds an empty line would be sent as a block whose body
     * stops there. Such a text is written to a temporary file and the interpreter is
     * asked to read and execute that file, which is what \c python.el does
     */
    bool sendToPython(const QString &text, const bool whole_block = false);

    /*!
     * \brief hand a single statement to the interpreter, with the empty line which
     * ends a block when the statement opens one
     * \return true when the statement reached the interpreter
     */
    bool sendStatement(const QString &line);

    /*!
     * \brief hand the sends which wait to the interpreter and show them in the
     * frame of the session, while the frame stands at a prompt
     *
     * \note a terminal echoes what is written to it and a session reads as a
     * conversation because of that; the session of the editor is a `QProcess`
     * behind a pipe, which echoes nothing, so a line a buffer sent is nowhere
     * to be seen and the answer which follows stands behind the `>>> ` of the
     * line it answers. A send therefore goes out in two parts written at the
     * same moment: its echo in the frame, then its bytes to the process.
     *
     * \note a send waits while the frame does not end at a prompt: the first
     * send of a session is what starts it, and the banner and the answer of
     * that first line can arrive in one piece of the stream, in which case a
     * line written at once was answered before any echo could be placed
     * (measured: `>>> toto' on the screen against `>>> print(...)' over
     * `toto'). One send per prompt, in the order of the presses: the
     * interpreter can not answer a line before the prompt that line was
     * written to, and a send which waits behind the send before it is no more
     * out of place than a press which waits behind the press before it.
     *
     * \note the prompts recognised are CPython's `>>> ' and `... ', which is
     * what the program `python/executable` names; a program which prompts
     * otherwise never sees a queued send, the way `comint` in Emacs hands
     * nothing to a program whose prompt it cannot read either.
     */
    void flushPythonSends();

    /*!
     * \brief start the interpreter of the buffer, or tell about the one which
     * is already running
     * \return the frame of the session, null when the program did not start
     *
     * \note the program is the one `python/executable` names and the session runs
     * in the directory of the buffer, so that a script imports the modules beside it
     * the way running the file would
     *
     * \note a frame whose process is gone — because `exit()' was typed in what was
     * sent to it, or because the interpreter died on a line — is taken away and
     * replaced: the key which sends a line is what brings the session back
     */
    ProcessInteractionFrame *startPythonInterpreter();

    /*!
     * \brief sort the imports of the buffer, with the program the
     * `pylsp/import-sorter` setting names
     *
     * \note the two shapes of answer the two sorters give are asked in two
     * different ways: `ruff` offers a change of the `source.organizeImports`
     * kind, which is a question about the changes on offer, while `isort` is a
     * wrapper of the format question and sorts whatever the formatter answered
     * (measured), so asking it is asking for a reformat
     *
     * \note what nothing sorted is reported: unlike \c format, this entry is
     * asked for by name, and a person who asked wants to know that no server
     * was up, or that the file was already in order
     */
    void sortImports();

    /*!
     * \return "#", the string which starts a comment line in this language
     */
    QString getCommentSyntax() override;

    //! destructor
    ~PythonMajorMode() override;

   protected:
    /*!
     * \return "Python", the address under which the history of the commands run
     * on the buffer is stored in the settings
     */
    QString getLanguageName() const override;

    /*!
     * \return `python3 <name of the file>`, which is how one runs the script one
     * is editing; the shell the command runs in is the directory of the buffer
     */
    QString getDefaultCompilationCommand() const override;

   private:
    /*!
     * \brief the conversation of this buffer with \c pylsp: the client, the
     * settling timers, the tracked words, the diagnostics and the completion
     * all live there
     */
    LspNavigationSession *pylsp = nullptr;

    /*!
     * \brief move every non blank line of the selected region by the given number
     * of columns, the lines which go left of the margin being stopped by it
     * \param[in] tc : cursor holding the selection
     * \param[in] offset : columns to add to the indentation of each line, negative
     * to take them away
     */
    void shiftRegion(const QTextCursor &tc, const int offset);

    /*!
     * \brief take the line the given cursor stands in one level down, which is what
     * the `Shift-Tab' key calls
     *
     * The highest level of the line's cycle which stands below the indentation the
     * line holds, and the lowest level of that cycle when nothing stands below: the
     * key means out of the blocks the line was written in, so it stops at the margin
     * instead of wrapping to the top of the cycle the way a run of `Tab' presses
     * does. The press is remembered as the `Tab' press which indented the line is,
     * so a `Tab' following a `Shift-Tab' goes on down the cycle where that press left
     * the line
     */
    void indentLineBackward(const QTextCursor &c);

    /*!
     * \return the program the interpreter session runs: what `python/executable`
     * names, or `python3` when the setting is empty
     *
     * \note the address is read on every launch, so a person who changes the field
     * and asks for an interpreter gets the program they wrote, with no restart of
     * the editor in between
     */
    static QString pythonInterpreter();

    /*!
     * \brief take the frame of the interpreter session away from the buffer, which
     * is the one way the mode ever loses sight of it, and the only one which leaves
     * the registry of the secondary tasks consistent
     */
    void detachPythonFrame();

    //! the interpreter session of the buffer, null when none was started
    ProcessInteractionFrame *python = nullptr;

    /*!
     * \brief a send which waits for its turn at a prompt of the interpreter
     *
     * \note a send is two things written at one prompt: the line the frame
     * shows and the bytes the process is handed. A text of several lines is
     * kept as text and turned into bytes at the flush, because its temporary
     * file is reused by every send: written when it was queued, the file
     * would hold the text of a later send by the time the interpreter read it
     */
    struct PythonSend {
      //! the text the buffer handed over
      QString text;
      //! whether that text reaches the interpreter through a file
      bool whole_block = false;
    };

    /*!
     * \brief the sends the buffer has asked for and the interpreter has not
     * read yet
     *
     * \note the queue is what keeps the transcript in the order of the sends:
     * one send is written per prompt, and a send which finds no prompt on the
     * screen — because the session it starts has written no banner yet, or
     * because the line before it is not answered yet — waits behind the send
     * before it rather than be written in the middle of an answer
     */
    QVector<PythonSend> python_sends;

    /*!
     * \brief the file a region or a buffer is written to when it is handed to the
     * interpreter
     *
     * \note the file is created on the first send of more than one line and reused
     * after that: its name is what the interpreter is asked to read, so it cannot be
     * a file which is gone by the time the text it holds is read
     */
    QTemporaryFile *python_script = nullptr;

    /*!
     * \brief the document the last press of `Tab` indented
     *
     * \c python.el decides whether a press continues the cycle of the previous
     * one from \c last-command; this editor has no such thing, so the mode
     * remembers the line the last press reached and the indentation it gave it:
     * a press which finds that line back where it left it continues the cycle,
     * and one which finds it elsewhere, or on another line, or in another
     * buffer, starts a new one.
     */
    QPointer<QTextDocument> cycle_document;

    //! the block the last press of `Tab` indented
    int cycle_block = -1;

    //! the indentation the last press of `Tab` gave that block
    int cycle_target = -1;
  };  // end of struct PythonMajorMode

}  // end of namespace tfel::gui

#endif
