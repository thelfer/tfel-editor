/*!
 * \file  GrammarCheckSession.hxx
 * \brief the proofreader of a text edit: the backend which reads its text, the
 *        lints it draws under that text, and the choice which decides between
 *        them.
 * \author Thomas Helfer
 * \date   05/11/2026
 */

#ifndef LIB_TFEL_GUI_GRAMMARCHECKSESSION_HXX
#define LIB_TFEL_GUI_GRAMMARCHECKSESSION_HXX

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QPointer>

#include "TFEL/GUI/Config.hxx"
#include "TFEL/GUI/SpellingAndGrammar.hxx"

// forward declarations
class QAbstractScrollArea;
class QEvent;
class QMenu;
class QObject;
class QTextDocument;

namespace tfel::gui {

  // forward declarations
  struct EditorWidget;
  struct LanguageToolClient;
  struct LspClient;
  struct TextEditBase;

  /*!
   * \brief a grammar checker and the buffer it reads
   *
   * \note one session belongs to one text edit, the way a highlighter does: the
   * checker is asked about the text of a frame and answers about that text only,
   * so a session is held by the mode of the frame, is built with it and dies with
   * it, and a buffer which changes the proofreader it is read with changes it
   * through this class and nobody else
   *
   * \note the session holds two backends and uses one of them: \c LspClient talks
   * to one `harper-ls` process over the Language Server Protocol,
   * \c LanguageToolClient to one Java process over HTTP. What is under the
   * underline — where the program is, how a text is handed to it, what an answer
   * says — stops at those two classes; what a person sees, which is a red line
   * under a run of words and a tooltip saying what the checker said about it, is
   * the same for both, which is why the choice between them is one setting and not
   * two features
   *
   * \note the backend of \c harper-ls is a child of this session and serves this
   * buffer only: unlike \c clangd, which holds an index shared by every file of a
   * project and is therefore one per editor widget, a grammar checker holds
   * nothing but the documents it was given. The backend of LanguageTool is the
   * client of the editor widget, shared, because a virtual machine with the rules
   * of thirty languages loaded in it is worth starting once and not once per
   * buffer; the first buffer to ask starts it and the last to leave ends it
   *
   * \note which of them reads the buffer is the answer of \c
   * SpellingAndGrammar::effectiveChecker, asked again on every call of \c
   * followChecker, which is how a choice made in a menu reaches the buffer on the
   * screen. The choice which names no grammar checker leaves the session holding
   * nothing but the file name of its buffer
   *
   * \note the choice of the proofreader also decides what becomes of the spell
   * checker of hunspell, and \c followChecker is what says so to the mode: an
   * empty language is what a mode reads as 'no dictionary', so the buffer
   * LanguageTool reads on its own gets no dictionary and no misspelled words are
   * underlined twice over. \c stateChanged is what the mode follows
   *
   * \note what is underlined is drawn as the `grammar` contribution to the extra
   * selections of the text edit, so the current line, the diagnostics of \c clangd
   * and the colors of the identifiers are untouched by it. The severity of the
   * protocol lives in the color of the diagnostics of \c clangd because that
   * server sorts what it finds by it; both proofreaders answer their lints with a
   * severity of their own choosing — `harper-ls` says hint for every one of them,
   * LanguageTool says nothing but the category of its rules — so their whole
   * output is drawn in one color, and only the words they say something about are
   * inked. Resting the mouse on an underlined run says what the lint is, as a
   * tooltip
   */
  struct TFEL_GUI_VISIBILITY_EXPORT GrammarCheckSession : QObject {
    Q_OBJECT
   public:
    /*!
     * \brief constructor, which follows the choice held in the settings
     * \param[in] w : editor widget the buffer belongs to, which the minibuffer
     * messages of the conversation are displayed on
     * \param[in] t : text edit whose text is read, and which draws the lints
     * under it
     * \param[in] language_id : what the documents are called in the sense the
     * `harper-ls` server parses them with: `cpp` for a C++ buffer, `latex`,
     * `markdown`
     * \param[in] whole_text_is_prose : true when the whole text of such a buffer
     * is text to be read — a document of LaTeX, a note of Markdown — false when
     * its words are the comments of a source file, which is what decides whether
     * LanguageTool reads it at all, see \c
     * SpellingAndGrammar::effectiveChecker
     * \param[in] p : parent object, which is the mode that owns the session
     *
     * \note the session follows the choice which is held when it is built, and
     * announces nothing: a buffer opened while the editor is set to check its
     * prose says so by being checked, the way the buffers opened afterwards are
     */
    GrammarCheckSession(EditorWidget &w,
                        TextEditBase &t,
                        const QString &language_id,
                        const bool whole_text_is_prose,
                        QObject *const p = nullptr);
    /*!
     * \brief destructor: ends what was started, releases the documents of the
     * shared client, and takes the lints off the text edit, which may be dying
     * around us
     */
    ~GrammarCheckSession() override;
    /*!
     * \return true when at least one grammar checker is to be found: the entries
     * which start one are worth showing only then, and a menu which offered a
     * program that is not there would report a failure on every try
     */
    static bool isAvailable();
    /*!
     * \return true when a grammar checker reads the buffer: a backend is running,
     * or on its way, which is what the mode line of a report reads
     */
    bool isActive() const;
    /*!
     * \return what reads the buffer, which is the choice of the editor as it
     * applies to a buffer of this kind, see \c followChecker
     */
    SpellingAndGrammar::Checker checker() const;
    //! \return true when the whole text of the buffer is text to be read
    bool readsWholeTextAsProse() const;
    /*!
     * \brief make the buffer read by the checker the settings name: switch the
     * backend when it is not the one in use, and read the texts again when it is
     * the same one under a new language
     * \param[in] announced : report the outcome in the minibuffer; false for a
     * session which follows the settings of its own accord, which is what the
     * constructor and \c SpellingAndGrammar::applyTo ask
     *
     * \note the spell checker of the mode follows too: an empty language takes the
     * dictionary away from it, which is what the choice of a proofreader reading
     * the whole text means for the underlines of the misspelled words
     *
     * \note a backend whose program is not to be found is reported and left: the
     * entry of a menu says what was asked for and the minibuffer says why nothing
     * answers, which is clearer than a menu quietly doing something else than what
     * it shows
     */
    void followChecker(const bool announced = false);
    /*!
     * \brief add the four choices of the proofreader, and the languages of the
     * one in use, to the given menu
     * \param[in] menu : menu to complete; the entries are children of it and
     * refer to the editor widget, so they follow the choice and not this session
     *
     * \note this is \c SpellingAndGrammar::addToMenu, of which it says which of
     * the two kinds of buffers the menu is built for: the entry of LanguageTool
     * has to say, in a buffer of code, that no grammar will be read there
     */
    void addToMenu(QMenu &menu);
    /*!
     * \brief show the lints standing under the mouse as a tooltip
     *
     * \note the filter is installed on the viewport of the text edit by the
     * constructor
     */
    bool eventFilter(QObject *const watched, QEvent *const event) override;
   signals:
    /*!
     * \brief the buffer is read by a grammar checker, or is not: what the spell
     * checker of the mode has to follow, see the note in the class documentation
     */
    void stateChanged();

   private slots:
    //! the checker published lints: draw them when they are of this buffer
    void updateDiagnostics(const QString &file);
    /*!
     * \brief follow the buffer to the file it visits now, and to its new name;
     * the previous name is released on the checker
     */
    void openOrRenameDocument();

   private:
    //! the states of the conversation with the checker of `harper-ls`
    bool usesHarper() const;
    //! the states of the conversation with the checker of LanguageTool
    bool usesLanguageTool() const;
    //! start the `harper-ls` process of this session, or say why it can not be
    bool startHarper(QString &error);
    //! start reading the buffer with the shared client of the editor widget
    bool startLanguageTool(QString &error);
    //! end the backend in use; the documents are none of its business
    void stopChecker();
    /*!
     * \brief hand the text of the buffer to the backend in use again
     *
     * This is what a change of the language or of the categories of rules needs:
     * the request reads its settings when it is built, but a text which changed by
     * nothing is never sent again by itself, and the underlines of the English
     * which is gone would stay on the screen
     */
    void recheckChecker();
    /*!
     * \brief release the document on both backends, the one in use and the one
     * which is only held because the choice may come back to it
     */
    void releaseDocument();
    //! open the document of the buffer on the backend in use
    void openDocumentOnChecker();
    /*!
     * \return true when the backend in use holds a document for the file this
     * buffer visits, which is what a buffer which gained its name after the mode
     * was built has to find out before asking again
     */
    bool openedOnChecker() const;
    /*!
     * \return the lints the backend in use answered last for this buffer, empty
     * when none does
     */
    QVector<struct LspDiagnostic> diagnostics() const;
    //! \return what the minibuffer says about the checker in use
    QString reportOf() const;
    //! draw the lints of the buffer, or remove them when there are none
    void applyDiagnostics();
    /*!
     * \return the text of the lints whose range visits the given position,
     * empty when none does; several when the checker has several things to say
     * about one run of text, which it has
     */
    QString diagnosticAt(const QTextDocument &document, int position) const;
    //! the editor widget to report to
    EditorWidget *editor;
    //! the text edit whose text is read, and which draws the lints
    TextEditBase *text_edit;
    /*!
     * \brief the Qt widget behind \c text_edit, the viewport of which this
     * session filters for the tooltips
     *
     * A \c QPointer, and what the destructor checks before drawing: the session
     * dies as a child of the mode, which dies as a child of the text edit, and the
     * Qt widget of that text edit is a child created before the mode and therefore
     * destroyed before it — the lifetime trap of issue #20. A mode which is only
     * replaced, as `setMajorMode` does, leaves it alive, and that is when the
     * lints of the session which is going have to be taken off the text, by hand,
     * from there
     */
    QPointer<QAbstractScrollArea> area;
    //! what the documents are called in the sense `harper-ls` parses them with
    QString language_id;
    //! true when the whole text of the buffer is text to be read
    bool whole_text_is_prose;
    /*!
     * \brief the client of `harper-ls`, a child of this session, null until a
     * choice names that checker
     *
     * Built when it is first needed and kept afterwards: a person who tries the
     * four entries of a menu in turn has no reason to pay for a process per try,
     * and a client which holds no document runs no process. It dies with this
     * session, and with it the `harper-ls` it started
     */
    LspClient *harper = nullptr;
    /*!
     * \brief the program the `harper-ls` above was started with, empty while
     * none runs: what a change of `harper/executable` is compared to, which is the
     * one setting of that checker a server which runs does not follow by itself
     */
    QString harper_program;
    /*!
     * \brief the client of the editor widget's LanguageTool server, null until a
     * choice names that checker
     *
     * Not a child: the server is shared by the buffers of one editor widget, and
     * the client outlives the mode of any one of them. What this session holds is
     * a use of it, which \c openDocument and \c closeDocument count
     */
    LanguageToolClient *language_tool = nullptr;
    /*!
     * \brief what reads this buffer, which is the choice of the editor as it
     * applies to a buffer of this kind
     *
     * What a person chose is what \c SpellingAndGrammar::currentChecker answers;
     * this is what that choice means for this buffer, which is not the same thing
     * for a document of LaTeX and for a source file. The two differ for
     * LanguageTool alone, which reads no code
     */
    SpellingAndGrammar::Checker checker_in_use =
        SpellingAndGrammar::Checker::Hunspell;
    //! the file name the checker knows this buffer by, empty if none
    QString document_file;
    /*!
     * \brief the document opened under \c document_file
     *
     * A \c QPointer, never a call to `text_edit->document()`: see
     * \c CxxMajorMode::document, the lifetime trap of issue #20
     */
    QPointer<QTextDocument> document;
  };  // end of struct GrammarCheckSession

}  // end of namespace tfel::gui

#endif /* LIB_TFEL_GUI_GRAMMARCHECKSESSION_HXX */
