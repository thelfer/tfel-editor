/*!
 * \file  CastemMajorMode.cxx
 * \brief CastemMajorMode: Cast3M mode: keywords, running the solver, help.
 * \author Thomas Helfer
 * \date   05/08/2012
 */

#include <QtCore/QVector>

#include "TFEL/GUI/MajorModeBase.hxx"
#include "TFEL/GUI/MajorModeFactory.hxx"

#include "TFEL/GUI/ProcessInteractionFrame.hxx"
#include "TFEL/GUI/FileDownloader.hxx"
#include "TFEL/GUI/PlainTextEdit.hxx"

namespace tfel::gui {

  /*!
   * \brief a major mode dedicated to the `Cast3M` finite element solver
   */
  struct CastemMajorMode : public MajorModeBase {
    /*!
     * \return the list of `Cast3M` keyword
     *
     * \note the list is built once, on the first call, by
     * \c buildKeysList; it feeds the completer, the syntax highlighter and
     * the keyword recognition of this mode
     */
    static const QStringList &getKeysList();

    /*!
     * \brief check if the given string opens a `Cast3M` keyword
     * \param[in] w : word to test
     * \return true if a keyword of \c getKeysList starts with \c w
     *
     * \note only the four first characters of the keywords are compared, so
     * a longer word never matches. The comparison is case sensitive: the
     * callers pass the word under the cursor truncated to four characters
     */
    static bool isCastemKeyWord(const QString &);

    /*!
     * \brief constructor
     * \param[in] w : editor widget the mode belongs to
     * \param[in] b : buffer the mode is attached to
     * \param[in] t : text editor the mode acts on, which is also its parent
     * widget
     *
     * \note a case insensitive completer filled with the keywords of
     * \c getKeysList is built for \c t and connected to
     * \c TextEditBase::insertCompletion
     */
    CastemMajorMode(EditorWidget &, Buffer &, TextEditBase &);

    //! \return "Cast3M"
    QString getName() const override;

    //! \return "major mode dedicated to the castem finite element solver"
    QString getDescription() const override;

    /*!
     * \brief install a `CastemSyntaxHighlighter` on the given document
     * \param[in] d : document to highlight
     *
     * \note the new highlighter is owned by the document and the previous
     * one is released by \c setHighlighter
     */
    void setSyntaxHighlighter(QTextDocument *) override;

    /*!
     * \brief treat the shortcuts which are specific to `Cast3M`
     * \param[in] k1 : first key, \c Qt::Key_X or \c Qt::Key_C
     * \param[in] m : modifier of the second key
     * \param[in] k2 : second key
     * \return true when the shortcut is handled here
     *
     * "Ctrl-X H" displays the notice of the keyword under the cursor.
     * "Ctrl-C Ctrl-L", "Ctrl-C Ctrl-R" and "Ctrl-C Ctrl-B" send the line,
     * the region and the whole buffer to `Cast3M`; the value returned for
     * those three is the one of the corresponding slot.
     */
    bool handleShortCut(const int,
                        const Qt::KeyboardModifiers,
                        const int) override;

    //! \return the completer of the keywords built by the constructor
    QCompleter *getCompleter() override;
    /*!
     * \return a menu specific to this mode
     *
     * The menu holds the actions which send the line, the region and the
     * buffer to `Cast3M` and those which open the two wizards importing a
     * behaviour.
     *
     * \note a null pointer is returned when the parent of the mode is not a
     * widget; the caller owns the menu
     */
    QMenu *getSpecificMenu() override;
    /*!
     * \brief complete the context menu actions
     * \param[in] m : complete the context menu
     * \param[in] c : text cursor at the position where the menu will
     * appear
     *
     * When the word under the cursor opens a keyword, two actions are added,
     * which display the notice of that keyword and open its web page.
     *
     * \note they are inserted in front of the actions \c MajorModeBase adds
     */
    void completeContextMenu(QMenu *const, const QTextCursor &) override;

    /*!
     * \brief insert the completion the user chose in the word under the
     * cursor of the given text edit
     * \param[in] t : text edit the word is completed in
     * \param[in] w : completion chosen by the user
     *
     * The word is replaced by \c w, put in upper case and surrounded by
     * single quotes and followed by a blank; a quote which already opened
     * the word is removed first.
     *
     * \note the editor's cursor is moved after the inserted text
     *
     * \note unlike the default implementation, this override does not use
     * the completer of the mode
     */
    void completeCurrentWord(TextEditBase &, const QString &) override;

    //! \return 2: a completion is proposed from two characters on
    int getMinimalCompletionLength() override;

    /*!
     * \brief no-op: this mode does not reformat `Cast3M` files
     */
    void format() override;

    /*!
     * \brief indent the line of the given cursor
     * \param[in] c : text cursor of the line to indent
     *
     * Two blanks are inserted at the beginning of the line, whatever its
     * current indentation is; no blank is ever removed.
     */
    void indentLine(const QTextCursor &) override;

    //! \return "*", which opens a comment line in `Cast3M`
    QString getCommentSyntax() override;

    //! \return the `Cast3M` icon held by the resources of the mode
    QIcon getIcon() const override;

    /*!
     * \brief destructor
     *
     * \note the mode owns neither the highlighter, which belongs to the
     * document, nor the completer, which is a child of the text edit, nor
     * the interaction frame of \c co, which is destroyed when closed
     */
    ~CastemMajorMode() override;

   protected slots:

    /*!
     * \brief send the line the editor's cursor is in to `Cast3M`
     * \return true if the line was written to the solver
     *
     * \note \c startCastem is called first, so the solver is started when no
     * process is running yet
     */
    virtual bool sendLineToCastem();

    /*!
     * \brief send the lines covered by the selection to `Cast3M`, one line
     * at a time
     * \return false when the cursor holds no selection or when a line could
     * not be sent; in the latter case the lines which precede it were sent
     * all the same
     */
    virtual bool sendRegionToCastem();

    /*!
     * \brief send the whole buffer to `Cast3M`, one line at a time
     * \return true if every line was written to the solver
     *
     * \note the last line is skipped when it is empty, that is when the
     * buffer ends with a new line character
     */
    virtual bool sendBufferToCastem();

    /*!
     * \brief open the wizard which imports a behaviour and insert the
     * `Cast3M` declarations it generates at the editor's cursor
     *
     * The wizard, \c ImportBehaviourWizard, is a widget of the
     * `MFrontImportWidgets` plugin; nothing is inserted when it is rejected.
     */
    virtual void showImportBehaviourWizard();

    /*!
     * \brief open the wizard which imports a behaviour of the MFM library
     * and insert the `Cast3M` declarations it generates at the editor's
     * cursor
     *
     * \note the wizard is run with its interface set to "Castem", so that
     * only the behaviours available for that interface are proposed
     */
    virtual void showImportMFMBehaviourWizard();

   protected:
    /*!
     * \return the list of the keywords this mode completes, highlights and
     * recognises
     *
     * \note prefer \c getKeysList: this method rebuilds the whole list on
     * each call
     */
    static QStringList buildKeysList();

    /*!
     * \brief display the notice of a `Cast3M` keyword
     * \param[in] w : word under the cursor, which titles the output frame
     * \param[in] w2 : keyword the notice is asked for, the four first
     * characters of \c w
     *
     * The solver is started in a temporary directory and asked for the notice
     * with "INFO" and "FIN"; the output of that run is displayed in a
     * secondary task of the buffer titled after the word, the files the run
     * leaves in the directory are removed and the directory itself is removed
     * once it holds none.
     *
     * \note nothing happens when the name of a temporary file cannot be
     * obtained to build that directory
     */
    virtual void displayHelp(const QString &, const QString &);

    /*!
     * \brief open the web page which documents the given keyword
     * \param[in] w2 : keyword, the four first characters of the word under
     * the cursor; it is turned to lower case to build the address
     *
     * \note the page is opened by the default browser, through
     * \c QDesktopServices::openUrl; no check is made on the existence of the
     * notice
     */
    virtual void openWebHelp(const QString &);

    /*!
     * \brief write a line to the standard input of the `Cast3M` process
     * \param[in] l : line to send; a new line character is appended to it
     * \return true if the line was written
     *
     * \note \c startCastem is called first, so the solver is started when it
     * is not running yet
     */
    virtual bool sendToCastem(const QString &);

    /*!
     * \brief start the `Cast3M` solver unless a process is already running
     * \return true if a running process is available once the call returns
     *
     * The frame which displays the interactions with the solver is created
     * on the first call, with the "castem-output" major mode, and attached to
     * the buffer as a secondary task titled "* castem *"; the process is
     * started in the directory of the buffer, if it exists.
     *
     * \note a process which died between two calls is restarted
     */
    virtual bool startCastem();

    //! completer of the keywords; a child of the mode (issue #67)
    QCompleter *c;

    /*!
     * \brief frame the interactions with the `Cast3M` process are displayed
     * in, a null pointer when no process is associated with the mode
     *
     * \note the frame is created by \c startCastem and destroys itself when
     * it is closed, which resets this pointer to a null pointer
     */
    ProcessInteractionFrame *co;

   private:
    Q_OBJECT
  };  // end of struct CastemMajorMode

}  // end of namespace tfel::gui
